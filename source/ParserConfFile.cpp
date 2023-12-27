#include "ParserConfFile.hpp"

ParserConfFile::ParserConfFile(std::string path) : _path(path)
{
   
}
ParserConfFile::~ParserConfFile() {}
        

ParserConfFile::ParserConfFile(){}
ParserConfFile::ParserConfFile(const ParserConfFile& src) {}
ParserConfFile& ParserConfFile::operator=(const ParserConfFile& src) {}

std::ifstream&  ParserConfFile::open_config_file()
{
    std::ifstream   inputFileStream(this->_path);
    if(!inputFileStream.is_open())
    {
        std::cerr << "Error opening config file" << std::endl;
        return;
    }
    return inputFileStream;
}


const char *ParserConfFile::FailedToOpenConfFile::what() const throw() {
    return ("Failed to open Configuration file");
}
