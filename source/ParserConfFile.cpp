#include "../headers/ParserConfFile.hpp"


/* -------------------------------------------------------------------------- */
/*                          Constructors & Destructor                          */
/* -------------------------------------------------------------------------- */

ParserConfFile::ParserConfFile(std::string path) : _path(path)
{}
ParserConfFile::~ParserConfFile() {}
ParserConfFile::ParserConfFile(){}
ParserConfFile::ParserConfFile(const ParserConfFile& src) {(void)src;}

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

int  ParserConfFile::open_config_file()
{
    std::vector<std::string> tokens;
    std::ifstream   in_file_stream(this->_path);
    if(!in_file_stream.is_open())
    {
        std::cerr << "Error opening config file" << std::endl;
        return 1;
    }
    std::string line;
    while (std::getline(in_file_stream, line)) {
        std::istringstream ss(line);
        std::string member;
        std::string curr;
        while (ss >> curr)
        {
      
            std::size_t found = curr.find_first_of("{};");

            while (found != std::string::npos)
            {
                if (found == 0) {
                    // if brackets are at the start of the string
                    tokens.push_back(curr.substr(0,1));
                    curr.erase(0, 1);
                } else {
                    // brackets are in the middle of the string
                    tokens.push_back(curr.substr(0, found));
                    curr.erase(0, found);
                }
                // find the next occurrence
                found = curr.find_first_of("{}");
            }
                
            if (curr.size())
                tokens.push_back(curr);
        }
    }
    printVector(tokens);

    return 0;
}
// template <typename T>
// void ParserConfFile::printVector(std::vector<T> v)
// {
//     for (size_t i = 0; i < v.size(); i++)
//         std::cout <<  v[i] << std::endl;
//     std::cout <<  "size: " << v.size();
// }


void ParserConfFile::printMembers(std::ostream &out) const
{
    std::vector<SServer> tmp = servers;
    std::vector<SServer>::iterator it;
    
    for (it = tmp.begin(); it != tmp.end(); it++)
    {
        out << "Host: " << it->host << std::endl;
        out << "Port: " << it->port << std::endl;
        out << "Server name: " << it->host << std::endl;
        out << "Max body size: " << it->client_max_body_size << std::endl;
        out << "Root: " << it->root << std::endl;
    }

    
}

std::ostream& operator<<(std::ostream &out, const ParserConfFile &obj)
{
    obj.printMembers(out);
    return (out);
}



const char *ParserConfFile::FailedToOpenConfFile::what() const throw() {
    return ("Failed to open Configuration file");
}

ParserConfFile& ParserConfFile::operator=(const ParserConfFile& src) {
    *this = src;
    return *this;    
}