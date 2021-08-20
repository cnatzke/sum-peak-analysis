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
void HistogramManager::BuildAngularMatrices(TFile *source_file, TFile *bg_file)
// 1 - Retrieve Histograms
// 2 - Gate on 1760 sum peak
// 3 - Project out TH1D
// 4 - Use TH1D to build gamma1 vs angle
{
    int verbosity = 1;
    Int_t angle_indices = 51;
    Int_t gate_low = 1750;
    Int_t gate_high = 1800;

    LoadHistogramFile(source_file, "source");
    LoadHistogramFile(bg_file, "background");

    out_file = new TFile("test.root", "RECREATE");

    for (auto i = 0; i < angle_indices; i++) {
        TH2D *subtracted_matrix = SubtractRoomBackground(i);
        TH1D *projected_hist = GetProjection(subtracted_matrix, gate_low, gate_high, i);

        // cleaning up
        delete subtracted_matrix;
        delete projected_hist;
    }
    out_file->Close();

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
    std::cout << "Subtracting room background for index: " << index << std::endl;

    TH2D *src_hist = (TH2D*) source_file->Get(Form("prompt-angle/index_%02i_sum", index));
    TH2D *bg_hist = (TH2D*) bg_file->Get(Form("prompt-angle/index_%02i_sum", index));
    TH2D *h = (TH2D*) src_hist->Clone();

    // subtracting room bg from spectrum
    h->Add(bg_hist, -1.0 * bg_scaling_factors[index]);
    angle_matrix_vec.push_back(h);

    // writing resultant histogram to file
    out_file->cd();
    h->Write(Form("index_%02i_bg_subtracted", index));

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

    p->Write();
    return p;
} // end GetProjection


/************************************************************//**
 * Keep this function in. linking libraries breaks when it is removed; don't know why
 ***************************************************************/
void HistogramManager::PreProcessData()
{
    for (auto j = 0; j < fGrif->GetSuppressedMultiplicity(fGriffinBgo); ++j) {
        continue;
    }
} // PreProcessData
