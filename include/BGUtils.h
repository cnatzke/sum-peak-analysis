#ifndef BGUTILS_H
#define  BGUTILS_H

#include <map>
#include "FileHandler.h"
#include "TH2.h"

class BGUtils
{
private:
    FileHandler *file_man;
    int angle_indices = 51; // number of GRIFFIN opening angles
    bool optimize_values;
    std::map<int, float> bg_scaling_factors_map;


public:
    BGUtils(FileHandler *file_man);
    ~BGUtils(void);
    void SubtractAllBackground();
    void SubtractTimeRandomBg(std::string file_type, TFile *out_file);
    void SubtractAngleDependentBg(TFile *out_file);

    void SubtractAngleDependentBg();
    float OptimizeBGScaleFactor(TH2D* src_h, TH2D* bg_h, int peak, float init_guess, int i, float steps = 100);
    void OptimizeBGScaling(bool optimize);
    std::map<int, float> GetBgScalingFactors() {return bg_scaling_factors_map;};

};

#endif
