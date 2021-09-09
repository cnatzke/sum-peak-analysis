//////////////////////////////////////////////////////////////////////////////////
// Creates and fills histograms
//
// Author:          Connor Natzke (cnatzke@triumf.ca)
// Creation Date: Wednesday July 29, 2020	T15:22:33-07:00
// Last Update:   Wednesday July 29, 2020	T15:22:45-07:00
// Usage:
//
//////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <map>
#include "HistogramManager.h"
#include "progress_bar.h"
#include "LoadingMessenger.h"
#include "TFile.h"

/************************************************************//**
 * Constructor
 ***************************************************************/
HistogramManager::HistogramManager(){
    // std::cout << "Histogram manager initialized" << std::endl;
}

/************************************************************//**
 * Destructor
 ***************************************************************/
HistogramManager::~HistogramManager(void){
    // std::cout << "Histogram manager deleted" << std::endl;
}

/************************************************************//**
 * Creates and Fills histograms
 *
 * @param verbose Verbosity level
 ***************************************************************/
void HistogramManager::BuildAngularMatrices(TFile *source_file, TFile *bg_file, std::map<int, float> bg_scaling_factors_map)
{
    int verbosity = 1;
    Int_t angle_indices = 51;
    Int_t gate_low = 1750;
    Int_t gate_high = 1800;

    bg_scaling_factors_map = bg_scaling_factors_map;
    LoadHistogramFile(source_file, "source");
    LoadHistogramFile(bg_file, "background");

    // create output file and make required directories
    out_file = new TFile("test.root", "RECREATE");
    bg_dir = out_file->mkdir("bg_subtracted");
    gated_dir = out_file->mkdir("sum_energy_gated");
    TH2D *angle_matrix = new TH2D("angle_matrix", Form("#gamma_1 gated %i-%i;Angular Index;Energy [keV]", gate_low, gate_high), 70, 0, 70, 3000, 0, 3000);
    // making sure errors are correctly calculated
    angle_matrix->Sumw2();

    for (auto i = 0; i < angle_indices; i++) {

        std::cout << "Processing angular index: " << i << std::endl;
        TH2D *subtracted_matrix = SubtractRoomBackground(i);
        TH1D *projected_hist = GetProjection(subtracted_matrix, gate_low, gate_high, i);

        // fill angle vs gamma1 matrix
        for (auto my_bin = 0; my_bin < projected_hist->GetXaxis()->GetNbins() + 1; my_bin++) {
            double val = projected_hist->GetBinContent(my_bin);
            double val_error = projected_hist->GetBinError(my_bin);
            // Fill TH2D
            angle_matrix->SetBinContent(i, my_bin, val);
            angle_matrix->SetBinError(i, my_bin, val_error);
        } // end bin loop

        // cleaning up
        delete subtracted_matrix;
        delete projected_hist;
    }
    angle_matrix->Write("gamma1_energy_matrix_gated");
    out_file->Close();

    std::cout << "Histograms written to file: " << out_file->GetName() << std::endl;
} // GeneraHistogramFile()

/************************************************************//**
 * loads histogram file
 ***************************************************************/
void HistogramManager::LoadHistogramFile(TFile *file, std::string file_type){
    if (file_type.compare("source") == 0) {
        source_file = file;
    }
    else if (file_type.compare("background") == 0) {
        bg_file = file;
    } else {
        std::cerr << "Unknown filetype: " << file_type << std::endl;
    }

} // end LoadHistograms

/************************************************************//**
 * Subtracts room bg
 ***************************************************************/
TH2D* HistogramManager::SubtractRoomBackground(Int_t index)
{

    TH2D *src_hist = (TH2D*) source_file->Get(Form("prompt_angle/index_%02i_sum", index));
    TH2D *bg_hist = (TH2D*) bg_file->Get(Form("prompt_angle/index_%02i_sum", index));
    TH2D *h = (TH2D*) src_hist->Clone();

    // Making sure errors are propagated correctly
    src_hist->Sumw2();
    bg_hist->Sumw2();
    h->Sumw2();

    // subtracting room bg from spectrum
    h->Add(bg_hist, -1.0 * bg_scaling_factors_map[index + 1]);
    angle_matrix_vec.push_back(h);

    // writing resultant histogram to file
    bg_dir->cd();
    h->Write(Form("index_%02i_bg_subtracted", index));
    out_file->cd();

    delete src_hist;
    delete bg_hist;

    return h;
}  // end SubtractRoomBackground

/************************************************************//**
 * Gates on given sum energy and projects out Y axis
 ***************************************************************/
TH1D* HistogramManager::GetProjection(TH2D *h, Int_t gate_low, Int_t gate_high, Int_t index)
{
    TH1D* p = h->ProjectionY("p", gate_low, gate_high);
    p->SetName(Form("index_%02i_gated_%i_%i", index, gate_low, gate_high));
    p->SetTitle(";#gamma_1 energy [keV]");

    gated_dir->cd();
    p->Write();
    out_file->cd();
    return p;
} // end GetProjection


/************************************************************//**
 * Keep this function in. linking libraries breaks when it is removed; don't know why
 * Hours wasted trying to fix: 1.5
 ***************************************************************/
void HistogramManager::PreProcessData()
{
    for (auto j = 0; j < fGrif->GetSuppressedMultiplicity(fGriffinBgo); ++j) {
        continue;
    }
} // PreProcessData
