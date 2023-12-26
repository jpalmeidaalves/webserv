#include "../headers/Server.hpp"
#include "../headers/HTTP.hpp"
#include "../headers/utils.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        print_error("missing config file");
        return (1);
    }

    (void) argv;

    try {
        HTTP http;
    } catch(const std::exception& e) {
        print_error(e.what());
    }
    
    return (0);
}