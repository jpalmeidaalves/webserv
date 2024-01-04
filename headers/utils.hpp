#ifndef UTILS_HPP
#define UTILS_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <ctime>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

struct Connection {
    Request request;
    Response response;
    std::string host_port;
};

void print_error(const std::string &error_msg);
bool has_suffix(const std::string &str, const std::string &suffix);
void *ft_memset(void *s, int c, std::size_t n);
int is_file(const char *name);
std::string get_formated_time(long rawtime, const std::string &format);
int get_stat_info(int cfd, Request &request, Response &response);
int file_exists(std::string path);
std::string get_port_host_from_sockfd(int sockfd);

template <typename T>
void printVector(std::vector<T> v) {
    for (size_t i = 0; i < v.size(); i++)
        std::cout << v[i] << std::endl;
    std::cout << "size: " << v.size();
}

#endif /* UTILS_HPP */
