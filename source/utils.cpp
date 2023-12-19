#include "../headers/utils.hpp"

#include <signal.h>

void print_error(const std::string &error_msg) {
    std::cerr << "Error: " << error_msg << std::endl;
}