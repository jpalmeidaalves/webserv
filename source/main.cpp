#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        print_error("missing config file");
        return (EXIT_FAILURE);
    }

    try {
        Server server(argv[1], 8080);
        server.start_listen();
    } catch (const std::exception &e) {
        print_error(e.what());
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}