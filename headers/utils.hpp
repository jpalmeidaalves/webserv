#ifndef UTILS_HPP
#define UTILS_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <iostream>
#include <string>
#include <vector>

struct Connection {
    int fd;
    Request request;
    Response response;
    // Connection();
    // ~Connection();
};

void print_error(const std::string &error_msg);
bool has_suffix(const std::string &str, const std::string &suffix);
void *ft_memset(void *s, int c, std::size_t n);

template <typename T>
void printVector(std::vector<T> v) {
    for (size_t i = 0; i < v.size(); i++)
        std::cout << v[i] << std::endl;
    std::cout << "size: " << v.size();
}

#endif /* UTILS_HPP */
