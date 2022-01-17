#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include <map>
#include "TH1.h"
#include "TH2.h"
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TMath.h"
#include "FileHandler.h"

class HistogramManager
{
public:
    HistogramManager(FileHandler *file_man);
    ~HistogramManager();
    void BuildAngularMatrix(std::string selector);
    void BuildGatedAngularMatrix(std::string selector, int gate_low, int gate_high);
    void BuildSingleGammaMatrices(std::string selector, int gate_low, int gate_high);
    void BuildAllAngularMatrices();

private:
    void PreProcessData();
    TH1D* GetGatedProjection(TH2D *h, Int_t gate_low, Int_t gate_high, Int_t index);

    FileHandler *file_man;
    int angle_indices = 51;

    std::vector<TH2D*> angle_matrix_vec;
    std::vector<TH1D*> gated_projection_vec;
    std::map<int, float> bg_scaling_factors_map;

    double degree_to_rad = TMath::Pi() / 180.;
    double rad_to_degree = 180. / TMath::Pi();
    TGriffin *fGrif = NULL;
    TGriffinBgo *fGriffinBgo = NULL;

    int num_crystals = 64;
    // angles for 145mm
    std::vector<double> angle_combinations_vec = {15.442, 21.9054, 29.1432, 33.1433, 38.382, 44.57, 47.4453, 48.7411, 51.4734, 55.1704, 59.9782, 60.1024, 62.3396, 62.4924, 63.4231, 68.9567, 71.4314, 73.3582, 73.6291, 75.7736, 80.9423, 81.5464, 83.8936, 86.868, 88.9658, 91.0342, 93.132, 96.1064, 98.4536, 99.0577, 104.226, 106.371, 106.642, 108.569, 111.043, 116.577, 117.508, 117.66, 119.898, 120.022, 124.83, 128.527, 131.259, 132.555, 135.43, 141.618, 146.857, 150.857, 158.095, 164.558, 180.0};

};

#endif
