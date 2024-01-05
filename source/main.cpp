#include "../headers/HTTP.hpp"
#include "../headers/ParserConfigFile.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        print_error("missing config file");
        return (1);
    }

    ParserConfFile config_file(argv[1]);
    if (config_file.open_config_file()) {
        return 1;
    }

    config_file.printMembers();

    std::vector<struct ip_port> unique_addrs = config_file.get_unique_addresses();

    // printVector(unique_addrs);

    // return 0;
    try {
        HTTP http;
        http.open_listening_sockets(unique_addrs);
        return 1;
    } catch (const std::exception &e) {
        print_error(e.what());
    }
    return (0);
}