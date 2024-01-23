#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <ctime>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h> // TODO remove
#include <sys/types.h>
#include <unistd.h>
#include <vector>
// #include "Request.hpp"
// #include "Response.hpp"

typedef enum file_types_t { TYPE_FILE, TYPE_DIR, TYPE_UNKOWN } file_types;

struct Connection;
class Request;
class Response;

void print_error(const std::string &error_msg);
bool has_suffix(const std::string &str, const std::string &suffix);
void *ft_memset(void *s, int c, std::size_t n);
file_types get_file_type(const char *name);
std::string get_formated_time(long rawtime, const std::string &format);
int get_stat_info(std::string full_path, Response &response);
int file_exists(std::string path);
void get_port_host_from_sockfd(int sockfd, Connection *conn);
int ft_stoi(std::string str);
std::string ft_ultos(unsigned long nb);
std::string ft_itos(int nb);
uint32_t convert_str_to_uint32(const std::string &str);
std::string convert_uint32_to_str(uint32_t nb);
bool is_listening_socket(int sockfd, std::vector<int> &_listening_sockets);
void remove_char_from_string(std::string &str, char to_remove);
void print_ascii(const char *str);
bool has_permissions(std::string full_path, mode_t permissions);
bool ft_strcmp_insensitive(std::string str1, std::string str2);
Connection *is_cgi_socket(int sockfd, std::map<int, Connection *> &_active_connects);
std::size_t remaining_bytes(const std::stringstream &s);

template <typename T>
void printVector(std::vector<T> v) {
    for (size_t i = 0; i < v.size(); i++)
        std::cout << v[i] << std::endl;
    std::cout << "size: " << v.size();
}

void start_timer(struct timeval *begin);
void end_timer(struct timeval *begin, struct timeval *end);

#endif /* UTILS_HPP */
