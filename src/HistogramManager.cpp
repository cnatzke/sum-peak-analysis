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
HistogramManager::HistogramManager(FileHandler * file_man) : file_man(file_man)
{
    // std::cout << "Histogram manager initialized" << std::endl;
}

/************************************************************//**
 * Destructor
 ***************************************************************/
HistogramManager::~HistogramManager(void){
    // std::cout << "Histogram manager deleted" << std::endl;
}

/************************************************************//**
 * Builds multiple angular matrices
 ***************************************************************/
void HistogramManager::BuildAllAngularMatrices()
{
    int gate_low = 1759;
    int gate_high = 1769;

    // sum energy angular matrices
    BuildAngularMatrix("source");
    BuildAngularMatrix("background");

    // gamma 1 angular matrices
    BuildGatedAngularMatrix("source", 1759, 1769);
    BuildGatedAngularMatrix("background", 1759, 1769);
    BuildGatedAngularMatrix("subtracted", 1759, 1769);
    
} // end BuildAllAngularMatrices

/************************************************************//**
 * Builds angular matrices
 *
 * @param
 ***************************************************************/
void HistogramManager::BuildAngularMatrix(std::string selector){

    TFile in_file(file_man->hist_file_name.c_str(), "UPDATE");

    if (selector.compare("source") == 0) {
        std::cout << "Building source matrix ..." << std::endl;
    } else if (selector.compare("background") == 0) {
        std::cout << "Building background matrix ..." << std::endl;
    } else {
        std::cerr << "Unknown file designation: " << selector << std::endl;
        std::cerr << "Exiting ..." << std::endl;
        exit(EXIT_FAILURE);
    }

    TH2D * angle_matrix = new TH2D("angle_matrix", "", 55, 0, 55, 3000, 0, 3000);
    angle_matrix->Sumw2();

    for (auto i = 0; i < angle_indices; i++) {
        std::cout << "Processing angular index: " << i + 1 << " of " << angle_indices << "\r";
        std::cout.flush();

        TH2D * sum_energy_matrix = NULL;
        // Make sure we get the correct matrix
        if (selector.compare("source") == 0) {
            sum_energy_matrix = (TH2D*)in_file.Get(Form("room_background_subtracted/source_%02i", i));
            angle_matrix->SetName("angle_matrix_src");
            angle_matrix->SetTitle("Sum Energy (Room Bg Subtracted);Angular Index [arb.];Energy [keV]");
        } else {
            sum_energy_matrix = (TH2D*)in_file.Get(Form("background/index_%02i_sum", i));
            angle_matrix->SetName("angle_matrix_bg");
            angle_matrix->SetTitle("Sum Energy (Room Bg);Angular Index [arb.];Energy [keV]");
        }

        TH1D *projected_hist = sum_energy_matrix->ProjectionX();

        for (auto my_bin = 0; my_bin < projected_hist->GetXaxis()->GetNbins() + 1; my_bin++) {
            double val = projected_hist->GetBinContent(my_bin);
            double val_error = projected_hist->GetBinError(my_bin);
            // Fill TH2D
            angle_matrix->Fill(i, my_bin, val);
            angle_matrix->SetBinError(i, my_bin, val_error);
        } // end bin loop

        // cleaning up
        delete sum_energy_matrix;
        delete projected_hist;
    } // end angle index loop
    std::cout << std::endl;
    angle_matrix->Write("", TObject::kOverwrite);
    in_file.Close();

} // end BuildAngularMatrix

/************************************************************//**
 * Builds sum-peak gated angular matrix
 *
 * @param
 ***************************************************************/

void HistogramManager::BuildGatedAngularMatrix(std::string selector, int gate_low, int gate_high){

    TFile in_file(file_man->hist_file_name.c_str(), "UPDATE");
    TH2D * gated_angle_matrix = new TH2D("gated_angle_matrix", Form("#gamma_{1} Sum Gated [%i-%i];Angular Index; Energy [keV]", gate_low, gate_high), 55, 0.0, 55, gate_high + 10, 0, gate_high + 10);
    // make sure errors are properly calculated
    gated_angle_matrix->Sumw2();

    TH2D * sum_energy_matrix = NULL;
    std::cout << "Building " << selector << " energy gated angular matrix ... \r";
    for (auto i = 0; i < angle_indices; i++) {
        //std::cout << "Processing angular index: " << i + 1 << " of " << angle_indices << "\r";
        std::cout << "Building " << selector << " energy gated angular matrix (" << i + 1 << " of " << angle_indices << ")\r";
        std::cout.flush();

        if (selector.compare("source") == 0) {
            sum_energy_matrix = (TH2D*)in_file.Get(Form("source/index_%02i_sum", i));
            gated_angle_matrix->SetName("gamma1_matrix_src");
            gated_angle_matrix->SetTitle("Sum Energy (Source);Angular Index;Energy [keV]");
        } else if (selector.compare("background") == 0) {
            sum_energy_matrix = (TH2D*)in_file.Get(Form("background/index_%02i_sum", i));
            gated_angle_matrix->SetName("gamma1_matrix_bg");
            gated_angle_matrix->SetTitle("Sum Energy (Background);Angular Index;Energy [keV]");
        } else {
            sum_energy_matrix = (TH2D*)in_file.Get(Form("room_background_subtracted/source_%02i", i));
            gated_angle_matrix->SetName("gamma1_matrix_bg_subtracted");
            gated_angle_matrix->SetTitle("Sum Energy (Source, Background Subtracted);Angular Index;Energy [keV]");
        }


        TH1D *projected_hist = GetGatedProjection(sum_energy_matrix, gate_low, gate_high, i);

        for (auto my_bin = 0; my_bin < projected_hist->GetXaxis()->GetNbins() + 1; my_bin++) {
            double val = projected_hist->GetBinContent(my_bin);
            double val_error = projected_hist->GetBinError(my_bin);
            // Fill TH2D
            gated_angle_matrix->Fill(i, my_bin, val);
            gated_angle_matrix->SetBinError(i, my_bin, val_error);
        } // end bin loop

        // cleaning up
        delete sum_energy_matrix;
        delete projected_hist;
    } // end angle index loop
    std::cout << std::endl;
    gated_angle_matrix->Write("", TObject::kOverwrite);
    in_file.Close();

} // end BuildGatedAngularMatrix()

/************************************************************//**
 * Gates on given sum energy and projects out Y axis
 ***************************************************************/
TH1D* HistogramManager::GetGatedProjection(TH2D *h, Int_t gate_low, Int_t gate_high, Int_t index)
{
    TH1D* p = h->ProjectionY("p", gate_low, gate_high);
    p->SetName(Form("index_%02i_gated_%i_%i", index, gate_low, gate_high));
    p->SetTitle(";#gamma_1 energy [keV]");

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
