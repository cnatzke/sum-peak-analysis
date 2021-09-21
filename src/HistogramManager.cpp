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
 * Builds angular matrices
 *
 * @param
 ***************************************************************/
void HistogramManager::BuildAngularMatrix(std::string selector){

    if (selector.compare("source") == 0) {
        std::cout << "Building source matrix" << std::endl;
    } else if (selector.compare("background") == 0) {
        std::cout << "Building background matrix" << std::endl;
    } else {
        std::cerr << "Unknown file designation: " << selector << std::endl;
        std::cerr << "Exiting ..." << std::endl;
        exit(EXIT_FAILURE);
    }

} // end BuildAngularMatrix

/************************************************************//**
 * Builds sum-peak gated angular matrix
 *
 * @param
 ***************************************************************/
void HistogramManager::BuildGatedAngularMatrix(float gate_center){

    int gate_range_high = 10;
    int gate_range_low = 10;

    TFile * out_file = file_man->output_file;
    //TH2D * angle_matrix = new TH2D("angle_matrix", "Sum Energy;Cos(#theta); Energy [keV]", 100, -1.0, 1.0, 3000, 0, 3000);
    TH2D * angle_matrix = new TH2D("angle_matrix", "Sum Energy;Index;Energy [keV]", 55, 0, 55, 3000, 0, 3000);
    TH2D * gated_angle_matrix = new TH2D("gated_angle_matrix", Form("#gamma_{1} %i-%i;Cos(#theta); Energy [keV]", static_cast<int>(gate_center - gate_range_low), static_cast<int>(gate_center + gate_range_high)),100, -1.0, 1.0, 3000, 0, 3000);
    // make sure errors are properly calculated
    gated_angle_matrix->Sumw2();
    out_file->cd();

    for (auto i = 0; i < angle_indices; i++) {
        std::cout << "Processing angular index: " << i + 1 << " of " << angle_indices << "\r";
        std::cout.flush();
        TH2D *sum_energy_matrix = (TH2D*)out_file->Get(Form("room_background_subtracted/source_%02i", i));

        TH1D *projected_hist = sum_energy_matrix->ProjectionX();
        //TH1D *projected_hist = GetGatedProjection(sum_energy_matrix, gate_center - gate_range_low, gate_center + gate_range_high, i);
        //std::cout << projected_hist->GetName() << std::endl;

        double cos_angle = TMath::Cos(angle_combinations_vec.at(i) * degree_to_rad);

        for (auto my_bin = 0; my_bin < projected_hist->GetXaxis()->GetNbins() + 1; my_bin++) {
            double val = projected_hist->GetBinContent(my_bin);
            double val_error = projected_hist->GetBinError(my_bin);
            // Fill TH2D
            //angle_matrix->Fill(cos_angle, my_bin, val);
            angle_matrix->Fill(i, my_bin, val);
            angle_matrix->SetBinError(i, my_bin, val_error);
        } // end bin loop

        // cleaning up
        delete sum_energy_matrix;
        delete projected_hist;
    } // end angle index loop
    std::cout << std::endl;
    angle_matrix->Write("angle_matrix");
    out_file->Close();
    std::cout << "Done" << std::endl;

    delete angle_matrix;
    delete gated_angle_matrix;

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
