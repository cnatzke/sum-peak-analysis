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
#include "TGRSIUtilities.h"
#include "TParserLibrary.h"
#include "TEnv.h"

#include "SumPeakAnalysis.h"
//#include "BGUtils.h"
#include "HistogramManager.h"


int main(int argc, char **argv)
{
    if (argc == 1) { // no inputs given
        PrintUsage(argv);
        return 0;
    }

    if (argc == 3) {
        InitGRSISort();
        source_file =  new TFile(argv[1]);
        if (source_file->IsZombie()) {
            std::cout << "Error opening source file" << std::endl;
            exit(-1);
        } else {
            std::cout << "Found source file: " << source_file->GetName() << std::endl;
        }
        bg_file = new TFile(argv[2]);
        if (bg_file->IsZombie()) {
            std::cout << "Error opening background file" << std::endl;
            exit(-1);
        } else {
            std::cout << "Found background file: " << bg_file->GetName() << std::endl;
        }
    }

    // Background subtraction
    //BGUtils BGUtils(source_file, bg_file);
    //BGUtils.SubtractAllBackground();

    // sum peak gating
    HistogramManager *hist_man = new HistogramManager();
    hist_man->BuildAngularMatrices(source_file, bg_file);


    return 0;
} // main()

void InitGRSISort(){
    // makes time retrival happy and loads GRSIEnv
    grsi_path = getenv("GRSISYS");
    if(grsi_path.length() > 0) {
        grsi_path += "/";
    }
    grsi_path += ".grsirc";
    gEnv->ReadFile(grsi_path.c_str(), kEnvChange);

    TParserLibrary::Get()->Load();
} // end InitGRSISort

/******************************************************************************
 * Prints usage message and version
 *****************************************************************************/
void PrintUsage(char* argv[]){
    std::cerr << argv[0] << " Version: " << SumPeakAnalysis_VERSION_MAJOR << "." << SumPeakAnalysis_VERSION_MINOR << "\n"
              << "usage: " << argv[0] << " source_file background_file \n"
              << " source_file: Source histograms\n"
              << " background_file: Background histograms"
              << std::endl;
} // end PrintUsage
