//////////////////////////////////////////////////////////////////////////////////
// Reads in datafiles and creates output files
//
// Author:        Connor Natzke (cnatzke@triumf.ca)
// Creation Date: 08-09-2021
// Last Update:   08-09-2021
//////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stdlib.h>
#include "FileHandler.h"

/************************************************************//**
 * Constructor
 ***************************************************************/
FileHandler::FileHandler(std::string source_filepath, std::string bg_filepath)
{
    LoadFile(source_filepath, "source");
    LoadFile(bg_filepath, "background");
    //std::cout << "FileHandler initialized" << std::endl;
} // end Constructor

/************************************************************//**
 * Constructor 2
 ***************************************************************/
FileHandler::FileHandler(std::string histogram_file)
{
    LoadFile(histogram_file, "histograms");
    //std::cout << "FileHandler initialized" << std::endl;
} // end Constructor 2

/************************************************************//**
 * Destructor
 ***************************************************************/
FileHandler::~FileHandler(void)
{
    //std::cout << "FileHandler destroyed" << std::endl;
} // end destructor

/************************************************************//**
 * Loads root files into TFiles
 *
 * @param filename name of input file
 * @param type type of file (source/background)
 ***************************************************************/
void FileHandler::LoadFile(std::string filename, std::string type)
{
    if (type.compare("source") == 0) {
        src_file = new TFile(filename.c_str());
        std::cout << "Found source file: " << src_file->GetName() << std::endl;
    } else if (type.compare("background") == 0) {
        bg_file = new TFile(filename.c_str());
        std::cout << "Found background file: " << bg_file->GetName() << std::endl;
    } else if (type.compare("histograms") == 0) {
        if (FileIsValid(filename)) {
            hist_file_name = filename;
            std::cout << "Found histogram file: " << hist_file_name << std::endl;
        }
    } else {
        std::cerr << "Unknown filetype: " << type << std::endl;
        std::cerr << "Please pass either 'source' or 'background'" << std::endl;
        exit(EXIT_FAILURE);
    }
} // end LoadFile

/************************************************************//**
 * Check root file exists
 *
 * @param filename name of ROOT file
 ***************************************************************/
bool FileHandler::FileIsValid(std::string filename)
{
    TFile f(filename.c_str());
    if (f.IsOpen()) {
        f.Close();
        return true;
    } else {
        std::cerr << "ERROR --- Could not open file: " << filename << std::endl;
        return false;
    }
} // end FileIsValid()

/************************************************************//**
 * Created new root file for outputs
 *
 * @param filename name of output file
 ***************************************************************/
void FileHandler::CreateOutputFile(std::string filename)
{
    output_file = new TFile(filename.c_str(), "RECREATE");
} // end CreateOutputFile()

/************************************************************//**
 * Opens output file
 *
 * @param filename name of output file
 ***************************************************************/
TFile * FileHandler::OpenOutputFile(std::string filename)
{
    TFile * file = new TFile(filename.c_str(), "UPDATE");
    return file;
} // end CreateOutputFile()
