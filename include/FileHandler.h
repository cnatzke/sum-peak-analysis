#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "TFile.h"

class FileHandler
{
public:
    FileHandler(std::string source_filepath, std::string bg_filepath);
    FileHandler(std::string histogram_file);
    ~FileHandler(void);
    void CreateOutputFile(std::string filename);
    TFile * OpenOutputFile(std::string filename);

    TFile *src_file;
    TFile *bg_file;
    std::string hist_file_name;
    TFile *output_file;

private:
    void LoadFile(std::string filename, std::string type);
    bool FileIsValid(std::string filename);

};

#endif
