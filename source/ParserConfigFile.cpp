#include "../headers/ParserConfigFile.hpp"


/* -------------------------------------------------------------------------- */
/*                          Constructors & Destructor                          */
/* -------------------------------------------------------------------------- */

ParserConfFile::ParserConfFile(std::string path) : _path(path)
{
    servers_count = 0;
}
ParserConfFile::~ParserConfFile() {}
ParserConfFile::ParserConfFile(){}
ParserConfFile::ParserConfFile(const ParserConfFile& src) {(void)src;}

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

std::vector<SServer>  ParserConfFile::open_config_file()
{
    std::ifstream   in_file_stream(this->_path);
    if(!in_file_stream.is_open())
    {
        std::cerr << "Error opening config file" << std::endl;
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
    if (this->extract()){
        std::cerr << "Error: Config File Invalid Sintax" << std::endl;
    }
    // printVector(tokens);
    // exit(0);
    // std::cout << "host=>" << servers[0].host << std::endl;
    // std::cout << "port=>" << servers[1].port << std::endl;
    return servers;
}

std::vector<std::string>::iterator ParserConfFile::get_serv_data(std::vector<std::string>::iterator it, struct SServer& s)
{
    int brackets_count = 0;
    ++it;
    if (*it == "{")
    {
        brackets_count++;
        ++it;
        while (brackets_count)
        {
            if (*it == "{")
                brackets_count++;
            else if (*it == "}")
                brackets_count--;
            else if (*it == "host"){
                ++it;
                s.host = *it;}
            else if (*it == "listen"){
                ++it;
                s.port = *it;
                if ( *(it + 1) == "default_server") {
                    s.default_server = true;
                } else {
                    s.default_server = false;
                }
            } else if (*it == "root"){
                ++it;
                s.root = *it;
            }
            else if (*it == "client_max_body_size"){
                ++it;
                s.client_max_body_size = atoi(((*it).c_str()));
            }
            ++it;       
        }
    } else {
        std::cerr << "Invalid syntax" << std::endl;
        return it;
    }
    return it;
}   

void    init_server(SServer s)
{
    s.host = " ";
    s.port = " ";
    s.server_name.push_back("");
    s.client_max_body_size = 0;
    s.root = " ";
    s.default_server = false;
}

int ParserConfFile::extract()
{
    std::vector<std::string>::iterator it;
    bool    inside_http = false;
    int     brackets_count = 0;
    for(it = tokens.begin(); it != tokens.end(); )
    {
        if (*it == "http" && *(it + 1) == "{") {
            inside_http = true;
            it++;
            continue;
        }
        if (*it == "{")
            brackets_count++;
        if (*it == "}")
            brackets_count--;
        if (*it == "server" && inside_http) {
            SServer s;
            init_server(s);    
            it = get_serv_data(it, s);
            servers_count++;
            servers.push_back(s);
        }
        else
            it++;
        if (brackets_count == 0)
            inside_http = false;
        if (brackets_count < 0)
            return 1;
    }
    return 0;
}
void ParserConfFile::print_server_data()
{
    
}


void ParserConfFile::printMembers(void) const
{
    std::vector<SServer> tmp = servers;
    std::vector<SServer>::iterator ite;
    
    for (ite = tmp.begin(); ite != tmp.end(); ite++)
    {
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Host: " << ite->host << std::endl;
        std::cout << "Port: " << ite->port << std::endl;
        std::cout << "Server name: " << ite->host << std::endl;
        std::cout << "Max body size: " << ite->client_max_body_size << std::endl;
        std::cout << "Root: " << ite->root << std::endl;
        std::cout << std::boolalpha << "Default: " << ite->default_server << std::endl;
    }
}

std::ostream& operator<<(std::ostream &out, const ParserConfFile &obj)
{
    obj.printMembers();
    return (out);
}



const char *ParserConfFile::FailedToOpenConfFile::what() const throw() {
    return ("Failed to open Configuration file");
}

ParserConfFile& ParserConfFile::operator=(const ParserConfFile& src) {
    *this = src;
    return *this;    
}