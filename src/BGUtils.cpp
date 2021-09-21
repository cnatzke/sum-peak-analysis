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
    TFile* out_file = file_man->output_file;

    SubtractTimeRandomBg("source", out_file);
    SubtractTimeRandomBg("background", out_file);

    SubtractAngleDependentBg(out_file);

    std::cout << "Histograms written to file: " << out_file->GetName() << std::endl;
    out_file->Close();

    delete out_file;

} // end SubtractBackground()

/************************************************************//**
 * Subtracts average time-random background
 ***************************************************************/
void BGUtils::SubtractTimeRandomBg(std::string file_type, TFile *out_file)
{
    TFile* hist_file;

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
    out_file->cd();
    TDirectory *target_dir = out_file->mkdir(file_type.c_str());
    target_dir->cd();
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

    // cleaning up
    delete hist_file;

    return;

} // SubtractTimeRandomBg()

/************************************************************//**
 * Subtracts scaled room background_file
 *
 * @param out_file output root file
 ***************************************************************/
void BGUtils::SubtractAngleDependentBg(TFile *out_file)
{
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

    // peak for background fitting
    int bg_peak = 1460;
    float bg_scaling_factor = 1.0;
    float bg_scaling_factor_init = bg_scaling_factor;

    // create directory for subtracted histograms
    TDirectory* target_dir = (TDirectory*) out_file->mkdir("room_background_subtracted");
    target_dir->cd();

    for (auto i=0; i < angle_indices; i++) {


        TH2D *src_h = (TH2D*)out_file->Get(Form("source/index_%02i_sum", i));
        TH2D* bg_h = (TH2D*)out_file->Get(Form("background/index_%02i_sum", i));

        if (optimize_values) {
            std::cout << "Optimizing background scaling factor for index: " << i + 1 << " of " << angle_indices << "\r";
            std::cout.flush();
            bg_scaling_factor = OptimizeBGScaleFactor(src_h, bg_h, bg_peak, bg_scaling_factor_init, static_cast<int>(i), 100);
            if (bg_scaling_factor == bg_scaling_factor_init) {
                std::cerr << "\n  Could not optimize index: " << i << std::endl;
            }
            // Subtract background angle by angle
            src_h->Add(bg_h, -1.0 * bg_scaling_factor);
            // write factors to file
            bg_scale_file << i << "," << bg_scaling_factor << std::endl;
        } else {
            std::cout << "Subtracting scaled room background for index: " << i + 1 << " of " << angle_indices << "\r";
            std::cout.flush();
            // Subtract background angle by angle
            src_h->Add(bg_h, -1.0 * bg_scaling_factors_map[i]);
        }
        //total->Add(src_h, 1.0);

        src_h->Write(Form("source_%02i", i));

        // cleaning up
        delete src_h;
        delete bg_h;
    }
    std::cout << std::endl;

    if (optimize_values) bg_scale_file.close();
}

float BGUtils::OptimizeBGScaleFactor(TH2D* src_h, TH2D* bg_h, int peak, float init_guess, int i, float steps){
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
    best_chi2 = 10000.;
    TF1 *pol1 = new TF1("pol1", "pol1", peak - 20, peak + 20);
    TH1D * src_projection = (TH1D*)src_h->ProjectionX("src_projection");
    TH1D * bg_projection = (TH1D*)bg_h->ProjectionX("bg_projection");

    while (current_guess < range_high) {

        // this status is only needed when the optimization is taking a long time
        /*
           float percentage = (current_guess - range_low) / (range_high - range_low);
           std::cout << "Optimizing background scaling factor for index: " << i + 1 << " of " << angle_indices;
           std::cout << " [" << std::fixed << std::setprecision(2) << percentage << '%' << "]\r";
           std::cout.flush();
         */

        TH1D * src_clone = (TH1D*)src_projection->Clone();

        // subtract background
        src_clone->Add(bg_projection, -1. * current_guess);

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
        delete src_clone;

    }

    // if we are on the edges of our range, return initial guess
    if ((best_guess == range_low) || (best_guess == range_high)) {
        best_guess = init_guess;
    }

    // cleaing up
    delete src_projection;
    delete bg_projection;
    delete pol1;

    return best_guess;
}    // end OptimizeBGScaleFactors

void BGUtils::OptimizeBGScaling(bool optimize){
    optimize_values = optimize;
}
