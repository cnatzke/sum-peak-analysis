#ifndef BGUTILS_H
#define  BGUTILS_H

#include "TH2.h"

class BGUtils
{
private:
    TFile* source_file;
    TFile* bg_file;
    std::vector<TH1D*> angle_projections;
    TH2D* prompt_matrix;
    TH2D* time_random_matrix;
    TH2D* bg_prompt_matrix;
    TH2D* bg_time_random_matrix;
    int angle_indices = 51;

    // Scaling factors from fitting 1460 keV
    //float bg_channel_scaling[51] = {1.1427525575567756, 1.1963459385618895, 1.1235910131272997, 1.2170931129587597, 1.0966516233018315, 1.1941367184558629, 1.0748346529755974, 1.0580895110765016, 1.0339742044932556, 0.8747156278958534, 0.8033889245081004, 0.8903299522034871, 1.1403694659508572, 0.7029288227912965, 0.793916075313146, 0.9409631373154851, 0.678758047090022, 2.497935151220951, 0.9947705181210781, 1.708958109848199, 1.0, 1.2474602249799396, 0.5311862244686293, 0.253477632178042, 1.109379327867466, 1.0646027460335425, 0.5024527801938472, 0.0, 0.5338144648523754, 1.0, 0.6525205449313747, 0.7095533856730193, 1.0, 1.3861438813414484, 1.0, 1.0, 1.0, 1.0, 1.0156872014663187, 0.9394285311346517, 0.6920891718141803, 1.1268628299099306, 1.4409588449323623, 0.6953137336877294, 0.9806348544049973, 1.0455896749067821, 1.316108571507815, 0.9198579868798213, 1.3926946547172807, 1.0555752425893627, 0.9841227196603527};
    // manually adjusted
    float bg_channel_scaling[51] = {0.92, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 1.0580895110765016, 1.0339742044932556, 0.8747156278958534, 0.8033889245081004, 1.0, 1.1403694659508572, 0.7029288227912965, 1.0, 0.9409631373154851, 0.678758047090022, 1.0, 0.9947705181210781, 1.0, 1.0, 1.2474602249799396, 0.5311862244686293, 0.253477632178042, 1.109379327867466, 1.0646027460335425, 0.5024527801938472, 0.0, 0.5338144648523754, 1.0, 0.6525205449313747, 0.7095533856730193, 1.0, 1.3861438813414484, 1.0, 1.0, 1.0, 1.0, 1.0156872014663187, 0.9394285311346517, 0.6920891718141803, 0.9, 1.0, 0.6953137336877294, 0.9806348544049973, 0.95, 0.95, 1.0, 1.1, 0.9, 0.9841227196603527};
public:
    BGUtils(TFile *source_file, TFile *bg_file);
    void SubtractAllBackground();
    void FindAvgTimeRandom(TFile *h);
    void SubtractAngleDependentBg();
    void MakeAngleHistograms();
    void CreateAngleMatrix(TFile *histogram_file);
    float OptimizeBGScaleFactor(TH1D* src_h, TH1D* bg_h, int peak, float init_guess, float steps = 100);

};

#endif