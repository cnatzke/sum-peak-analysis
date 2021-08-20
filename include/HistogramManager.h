#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include <map>
#include "TH1.h"
#include "TH2.h"
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TMath.h"

class HistogramManager
{
public:
    HistogramManager();
    ~HistogramManager();
    void LoadHistogramFile(TFile *file, std::string file_type);
    void BuildAngularMatrices(TFile *file, TFile *bg_file);

private:
    void PreProcessData();
    TH2D* SubtractRoomBackground(Int_t index);
    TH1D* GetProjection(TH2D *h, Int_t gate_low, Int_t gate_high, Int_t index);

    TFile *source_file;
    TFile *bg_file;
    TFile *out_file;
    std::vector<TH2D*> angle_matrix_vec;
    std::vector<TH1D*> gated_projection_vec;

    double degree_to_rad = TMath::Pi() / 180.;
    double rad_to_degree = 180. / TMath::Pi();
    TGriffin *fGrif = NULL;
    TGriffinBgo *fGriffinBgo = NULL;

    int num_crystals = 64;
    // angles for 145mm
    std::vector<double> angle_combinations_vec = {15.442, 21.9054, 29.1432, 33.1433, 38.382, 44.57, 47.4453, 48.7411, 51.4734, 55.1704, 59.9782, 60.1024, 62.3396, 62.4924, 63.4231, 68.9567, 71.4314, 73.3582, 73.6291, 75.7736, 80.9423, 81.5464, 83.8936, 86.868, 88.9658, 91.0342, 93.132, 96.1064, 98.4536, 99.0577, 104.226, 106.371, 106.642, 108.569, 111.043, 116.577, 117.508, 117.66, 119.898, 120.022, 124.83, 128.527, 131.259, 132.555, 135.43, 141.618, 146.857, 150.857, 158.095, 164.558, 180.0};
    float bg_scaling_factors[51] = {0.92, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 1.0580895110765016, 1.0339742044932556, 0.8747156278958534, 0.8033889245081004, 1.0, 1.1403694659508572, 0.7029288227912965, 1.0, 0.9409631373154851, 0.678758047090022, 1.0, 0.9947705181210781, 1.0, 1.0, 1.2474602249799396, 0.5311862244686293, 0.253477632178042, 1.109379327867466, 1.0646027460335425, 0.5024527801938472, 0.0, 0.5338144648523754, 1.0, 0.6525205449313747, 0.7095533856730193, 1.0, 1.3861438813414484, 1.0, 1.0, 1.0, 1.0, 1.0156872014663187, 0.9394285311346517, 0.6920891718141803, 0.9, 1.0, 0.6953137336877294, 0.9806348544049973, 0.95, 0.95, 1.0, 1.1, 0.9, 0.9841227196603527};

};

#endif
