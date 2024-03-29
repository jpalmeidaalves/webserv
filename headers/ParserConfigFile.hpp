#ifndef PARSERCONFIGFILE_HPP
#define PARSERCONFIGFILE_HPP

#include "Server.hpp"

#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class ParserConfFile {
  private:
    std::string _path;
    std::vector<Server> _servers;
    int _servers_count;
    ParserConfFile(const ParserConfFile &src);
    ParserConfFile &operator=(const ParserConfFile &src);
    ParserConfFile();
    std::vector<std::string> _tokens;

  public:
    ParserConfFile(std::string path);
    ~ParserConfFile();
    int open_config_file();
    int extract_server();
    std::vector<Server> &get_servers();
    std::vector<struct sockaddr_in> get_unique_addresses();
    int get_serv_data(std::vector<std::string>::iterator &it, Server &s);
    int check_brackets_integrity();

    void printMembers(void) const;
    std::vector<Server> &extract_servers_data();
    int extract_location(std::vector<std::string>::iterator &it, Server &s);
    bool is_directive(const std::string &line);
    int get_server_count(void);
};
std::ostream &operator<<(std::ostream &out, const Server &obj);

#endif /* PARSERCONFIGFILE_HPP */
