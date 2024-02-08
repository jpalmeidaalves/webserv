#include <iostream>
#include "../headers/colors.hpp"

bool has_suffix(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}


int main() {
    std::string root = "/ola/";

    std::string path = "/something";

    if (has_suffix(root, "/")) {
        path.erase(0, 1);
        path = root + path;
    } else
        path = root + path;
    std::cout << YELLOW << "final path: " << path << RESET << std::endl;

    return 0;
}