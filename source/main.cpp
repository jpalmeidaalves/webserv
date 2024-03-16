#include "../headers/HTTP.hpp"
#include "../headers/ParserConfigFile.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc > 2) {
        print_error("Error: usage ./webserv [config file]");
        return (1);
    }

    std::cout << "pid: " << getpid() << std::endl;

    std::string config_file_path;
    if (argc == 1) {
        config_file_path = "./etc/configFile.conf";
    } else {
        config_file_path = argv[1];
    }

    ParserConfFile config_file(config_file_path);
    if (config_file.open_config_file()) {
        std::cerr << "Invalid Config file. Finishing program" << std::endl;
        return 1;
    }

    if (config_file.get_server_count() == 0) {
        std::cout << "No servers in the config file. Must set atleast one server!" << std::endl;
        return 1;
    }

    std::vector<struct sockaddr_in> unique_addrs = config_file.get_unique_addresses();

    HTTP http(config_file.get_servers());
    http.open_listening_sockets(unique_addrs);
   
    http.handle_connections();

    return (0);
}

