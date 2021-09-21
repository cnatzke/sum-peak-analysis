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
#include <map>
#include "TGRSIUtilities.h"
#include "TParserLibrary.h"
#include "TEnv.h"

#include "SumPeakAnalysis.h"
#include "FileHandler.h"
#include "BGUtils.h"
#include "HistogramManager.h"


int main(int argc, char **argv)
{
    if (argc == 1) { // no inputs given
        PrintUsage(argv);
        return 0;
    }
    else if (argc == 2) {
        FileHandler * inputs = new FileHandler(argv[1]);

        // Create basic angular histograms
        HistogramManager * hist_man = new HistogramManager(inputs);
        hist_man->BuildAngularMatrix("source");
        hist_man->BuildAngularMatrix("background");

        // sum peak gating
        //HistogramManager *hist_man = new HistogramManager(inputs);
        //hist_man->BuildGatedAngularMatrix(1770);

        delete inputs;
        delete hist_man;
    }
    else if (argc == 3) {
        InitGRSISort();
        // makes output look nicer
        std::cout << std::endl;
        // read in data files
        FileHandler * inputs = new FileHandler(argv[1], argv[2]);

        // Background subtraction
        BGUtils *bg_utils = new BGUtils(inputs);
        //bg_utils->OptimizeBGScaling(true);
        bg_utils->SubtractAllBackground();

        // cleaning up
        delete inputs;
        delete bg_utils;
    }

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
              << "\n----- Background Subtractions ------\n"
              << "usage: " << argv[0] << " source_file background_file \n"
              << " source_file: Source histograms\n"
              << " background_file: Background histograms\n"
              << "\n----- Matrix Creation ------\n"
              << "usage: " << argv[0] << " histogram_file\n"
              << " histogram_file: ROOT file containing background subtracted histograms\n"
              << std::endl;
} // end PrintUsage
