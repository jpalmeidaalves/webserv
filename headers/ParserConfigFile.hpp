#ifndef PARSERCONFIGFILE_HPP
#define PARSERCONFIGFILE_HPP

#include "Server.hpp"

#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class ParserConfFile {
  private:
    int fd;
    std::string _path;
    std::vector<Server> servers;
    int servers_count;
    ParserConfFile(const ParserConfFile &src);
    ParserConfFile &operator=(const ParserConfFile &src);
    ParserConfFile();
    std::vector<std::string> tokens;

  public:
    ParserConfFile(std::string path);
    ~ParserConfFile();
    int open_config_file();
    int extract_server();
    std::vector<Server> &get_servers();
    std::vector<struct sockaddr_in> get_unique_addresses();
    std::vector<std::string>::iterator get_serv_data(std::vector<std::string>::iterator it,
                                                     Server &s);

    void printMembers(void) const;
    void print_server_data();
    std::vector<Server> &extract_servers_data();
};
std::ostream &operator<<(std::ostream &out, const Server &obj);

#endif/* PARSERCONFIGFILE_HPP */
