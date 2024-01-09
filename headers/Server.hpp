#ifndef SERVER_HPP
#define SERVER_HPP

#include "../headers/utils.hpp"
#include "colors.hpp"

#include <arpa/inet.h> // htons, htonl
#include <map>
#include <string>
#include <sys/socket.h> // socket, bind, struct sockaddr_in
#include <unistd.h>     // close
#include <vector>

struct Connection;

class Server {

  private:
    struct sockaddr_in _address;
    unsigned long _address_len;
    int _sockfd;

  public:
    Server(); // disable default constructer
    Connection *connection;
    ~Server();
    /* ----- */
    int client_max_body_size;
    std::string host;
    std::string port;
    uint32_t s_addr;
    uint16_t sin_port;
    std::vector<std::string> server_names;
    std::string root;
    std::string get_error_page(std::string error_code);
    std::multimap<std::string, std::string> error_pages;
};

#endif /* SERVER_HPP */
