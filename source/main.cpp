#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        print_error("missing config file");
        return (1);
    }

    Server server(argv[1], 8080);

    if (server.create_server())
        return (1);

    if (server.monitor_multiple_fds())
        return (1);

    return (0);
}