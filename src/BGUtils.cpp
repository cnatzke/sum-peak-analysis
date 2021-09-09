//////////////////////////////////////////////////////////////////////////////////
// Creates angle dependent background subtraction
//
// Author:        Connor Natzke (cnatzke@triumf.ca)
// Creation Date: 13-07-2021
// Last Update:   13-07-2021
// Usage:
//
//////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <string.h>
#include <fstream>
#include "csv.h"
#include "TPeakFitter.h"
#include "TRWPeak.h"
#include "TH1.h"
#include "BGUtils.h"


/************************************************************//**
 * Constructor
 ***************************************************************/
BGUtils::BGUtils(FileHandler *file_man) : file_man(file_man)
{
    //std::cout << "BGUtils initialized" << std::endl;
} // end Constructor

/************************************************************//**
 * Destructor
 ***************************************************************/
BGUtils::~BGUtils(void)
{
    //std::cout << "BGUtils destroyed" << std::endl;
} // end Constructor

/************************************************************//**
 * Subtracts background
 ***************************************************************/
void BGUtils::SubtractAllBackground()
{

    // subtract time-random coincidences
    file_man->CreateOutputFile("outputs.root");

    SubtractTimeRandomBg("source");
    SubtractTimeRandomBg("background");


    /*
       FindAvgTimeRandom(source_file);
       FindAvgTimeRandom(bg_file);

       optimize_values = optimize_values;
       SubtractAngleDependentBg();
     */

} // end SubtractBackground()

/************************************************************//**
 * Subtracts average time-random background
 ***************************************************************/
void BGUtils::SubtractTimeRandomBg(std::string file_type)
{
    TFile* hist_file;
    TFile* out_file = file_man->output_file;

    if (file_type.compare("source") == 0) {
        hist_file = file_man->src_file;
    } else if (file_type.compare("background") == 0) {
        hist_file = file_man->bg_file;
    } else {
        std::cerr << "Unknown file designation: " << file_type << std::endl;
        std::cerr << "Exiting ..." << std::endl;
        exit(EXIT_FAILURE);
    }

    // change into output file for writing matrices
    TDirectory* prompt_dir = out_file->mkdir(file_type.c_str());
    prompt_dir->cd();
    // loop through each angular index
    for (auto i = 0; i < angle_indices; i++) {
        std::cout << "Subtracting time-random background of " << file_type << " file: " << i + 1 << " of " << angle_indices << "\r";
        std::cout.flush();
        TH2D * prompt_matrix = (TH2D*) hist_file->Get(Form("prompt_angle/index_%02i_sum", i));
        TH2D * time_random_matrix = (TH2D*) hist_file->Get(Form("time_random/index_%02i_sum_tr_avg", i));

        // no scaling since time-random matrix was created identically to the prompt
        prompt_matrix->Add(time_random_matrix, -1.0);
        // write to output file
        prompt_matrix->Write();

        // cleaning up
        delete time_random_matrix;
        delete prompt_matrix;
    } // end index loop
    std::cout << std::endl;

    hist_file->Close();
    out_file->Close();

    // cleaning up
    //delete prompt_dir; // causes segfault, not sure why
    //delete hist_file;
    //delete out_file;

    return;

} // SubtractTimeRandomBg()

/*
   void BGUtils::SubtractAngleDependentBg(){
    // create matrices for each angular index
    MakeAngleHistograms();

    TFile* histogram_file = new TFile("diagnostics/channel_histograms.root", "UPDATE");
    TFile* out_file = new TFile("bg_subtracted_histograms.root", "RECREATE");
    if (histogram_file->IsZombie()) {
        std::cerr << "Cannot open histogram file. Exiting." << std::endl;
        exit(-1);
    }

    // read in bg scale factors
    std::string bg_scale_filename = "bg_index_scaling.csv";
    std::fstream bg_scale_file;

    bg_scale_file.open(bg_scale_filename, std::ios_base::in);
    // if bg file doesn't exist, create it
    if (!bg_scale_file || optimize_values) {
        bg_scale_file.close();
        std::cout << "Could not open " << bg_scale_filename << ", creating new file..." << std::endl;
        bg_scale_file.open(bg_scale_filename, std::ios_base::out | std::ios_base::trunc);
        bg_scale_file << "index,scale\n";
    } else { // use existing file
        std::cout << "Found background scaling file: " << bg_scale_filename << std::endl;
        io::CSVReader<2> in(bg_scale_filename);
        in.read_header(io::ignore_extra_column, "index", "scale");
        int index; float scale;
        while(in.read_row(index, scale)) {
            bg_scaling_factors_map.insert(std::pair<int, float>(index, scale));
        }
        bg_scale_file.close();
    }

    // Try to find optimal bg subtraction factors by fitting a line
    if (optimize_values) {
        std::cout << "Optimizing background subtraction factors ..." << std::endl;
    } else {
        std::cout << "Subtracting room background ..." << std::endl;
    }
    int num_channels = 51;
    int bg_peak = 1460;
    float bg_scaling_factor = 1.0;
    float bg_scaling_factor_init = 1.0;
    TH1D* src_h;
    TH1D* bg_h;
    TH1D* total = new TH1D("total", "Sum of All angles", 3000, 0, 3000);
    out_file->cd();
    for (auto i=0; i < num_channels; i++) {
        src_h = (TH1D*)histogram_file->Get(Form("source_%02i", i + 1));
        bg_h = (TH1D*)histogram_file->Get(Form("bg_%02i", i + 1));

        if (optimize_values) {
            bg_scaling_factor = OptimizeBGScaleFactor(src_h, bg_h, bg_peak, bg_scaling_factor_init, 100);
            if (bg_scaling_factor == bg_scaling_factor_init) {
                std::cerr << "  Could not optimize index: " << i + 1 << std::endl;
            }
            // Subtract background angle by angle
            src_h->Add(bg_h, -1.0 * bg_scaling_factor);
            // write factors to file
            bg_scale_file << i + 1 << "," << bg_scaling_factor << std::endl;
        } else {
            // Subtract background angle by angle
            src_h->Add(bg_h, -1.0 * bg_scaling_factors_map[i + 1]);
        }
        total->Add(src_h, 1.0);
        src_h->Write(Form("source_%02i", i + 1));
    }
    total->Write();

    // Create angle-sum_energy matrix
    CreateAngleMatrix(out_file);

    std::cout << "Background subtracted histograms written to: " << out_file->GetName() << std::endl;
    histogram_file->Close();
    out_file->Close();
    if (optimize_values) bg_scale_file.close();
   }

   float BGUtils::OptimizeBGScaleFactor(TH1D* src_h, TH1D* bg_h, int peak, float init_guess, float steps){
    // Attempt to optimize bg scale factors by fitting line to region to minimize Chi2
    float current_guess, best_guess;
    float current_chi2, best_chi2;
    float range_low = 0.5 * init_guess;
    float range_high = 1.5 * init_guess;
    best_guess = init_guess;

    // calculate step size
    float step = std::abs(range_high - range_low) / steps;

    // scan parameter space to find best chi2 value
    current_guess = range_low;
    best_chi2 = 1000.;
    TH1D* src_clone;
    TF1 *pol1 = new TF1("pol1", "pol1", peak - 20, peak + 20);
    while (current_guess < range_high) {
        // make clones to avoid compounding machine numerical errors
        src_clone = (TH1D*)src_h->Clone();

        // subtract background
        src_clone->Add(bg_h, -1. * current_guess);

        // fit line and extract goodness of fit
        src_clone->GetXaxis()->SetRangeUser(peak - 20, peak + 20);
        src_clone->Fit("pol1", "LQM0");

        current_chi2 = pol1->GetChisquare();

        // keep track of guesses for later
        if (current_chi2 < best_chi2) {
            best_guess = current_guess;
            best_chi2 = current_chi2;
        }
        current_guess += step;
    }

    // if we are on the edges of our range, return initial guess
    if ((best_guess == range_low) || (best_guess == range_high)) {
        best_guess = init_guess;
    }

    return best_guess;
   } // end OptimizeBGScaleFactors

   void BGUtils::CreateAngleMatrix(TFile *histogram_file){
    // Takes TH1's and creates a TH2
    TH2D *angle_matrix = new TH2D("angle_matrix", ";Angular Index;Energy [keV]", 70, 0, 70, 3000, 0, 3000);
    TH1D *angle_hist;

    for (auto i = 1; i < angle_indices + 1; i++) {
        angle_hist = (TH1D*)histogram_file->Get(Form("source_%02i", i));
        for (auto my_bin = 0; my_bin < angle_hist->GetXaxis()->GetNbins() + 1; my_bin++) {
            double val = angle_hist->GetBinContent(my_bin);
            double val_error = angle_hist->GetBinError(my_bin);
            // Fill TH2D
            angle_matrix->SetBinContent(i, my_bin, val);
            angle_matrix->SetBinError(i, my_bin, val_error);
        } // end bin loop
    } // end index loop

    histogram_file->cd();
    angle_matrix->Write("index_energy_matrix");

    return;

   } // end CreateAngleMatrix

   void BGUtils::MakeAngleHistograms(){
    // Subtracts angle dependent background
    TH2D* source_matrix = (TH2D*)source_file->Get("sum_energy_angle");
    TH2D* bg_matrix = (TH2D*)bg_file->Get("sum_energy_angle");
    TFile* out_file = new TFile("diagnostics/channel_histograms.root", "RECREATE");
    TH1D *my_source_projection;
    TH1D *my_bg_projection;
    //TPeakFitter* pf;
    //* peak;
    // peak_list[3] = {1460, 1729, 2614};
    for (auto index = 0; index < source_matrix->GetNbinsX(); index++) {
        my_source_projection = source_matrix->ProjectionY(Form("source_%02i", index), index, index);
        my_bg_projection = bg_matrix->ProjectionY(Form("bg_%02i", index), index, index);
        if ((my_source_projection->GetEntries() < 100) || (my_bg_projection->GetEntries() < 100)) {
            continue;
        }

        // Write projections to file
        out_file->cd();
        my_source_projection->Write();
        my_bg_projection->Write();
    }
    out_file->Close();

    return;
   } // end SubtractAngleDependentBg()

   void BGUtils::FindAvgTimeRandom(TFile *histogram_file)
   {
    TH2D* time_random_matrix;
    TH1D* average_histogram;
    TH1D* proj_hist;
    int slice_width = 30;
    int slice_edges[5] = {510, 617, 725, 832, 940};


    // open histogram file to write averaged histograms
    TFile *in_file = new TFile(histogram_file->GetName(), "READ");
    TFile *out_file = new TFile("average_time_random.root", "RECREATE");

    // loop through each angular index
    for (auto i = 0; i < angle_indices; i++) {
        average_histogram = NULL;
        time_random_matrix = (TH2D*) histogram_file->Get(Form("time_random/index_%02i_sum_tr", i));

        for (unsigned int my_slice = 0; my_slice < sizeof(slice_edges)/sizeof(slice_edges[0]); my_slice++) {
            // get 30 ns slice
            time_random_matrix->GetYaxis()->SetRangeUser(slice_edges[my_slice], slice_edges[my_slice] + slice_width);
            proj_hist = time_random_matrix->ProjectionX();
            if (average_histogram == NULL) {
                average_histogram = proj_hist;
                average_histogram->Sumw2();
            } else {
                average_histogram->Add(proj_hist, 1.0);
            }
        } // end slice loop
        // scale by 1/5 for averaging
        //average_histogram->Scale( 1 / 5.0 );

        out_file->cd();
        average_histogram->Write(Form("index_%i_tr_avg", i));
    } // end index loop
    out_file->Close();
    in_file->Close();

    std::cout << "Time-random histograms written to: " << out_file->GetName() << std::endl;

    return;
   } // end FindAvgTimeRandom()

   void BGUtils::OptimizeBGScaling(bool optimize){
    optimize_values = optimize;
   }
 */
