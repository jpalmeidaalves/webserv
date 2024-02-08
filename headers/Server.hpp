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
    std::map<std::string, std::string> _error_pages;
    std::map<std::string, std::string> _default_error_pages;

  public:
    Server(); // disable default constructer
    std::map<std::string, struct LocationOptions> locations;
    Connection *connection;
    ~Server();
    /* ----- */
    std::size_t client_max_body_size;
    std::string host;
    std::string port;
    uint32_t s_addr;
    uint16_t sin_port;
    std::vector<std::string> server_names;
    std::string root;
    std::vector<std::string> index_pages;
    std::string get_error_page(std::string error_code);
    void update_error_page(std::string error_code, std::string path);
    std::string get_default_error_page(std::string code);
    // std::multimap<std::string, std::string> error_pages;
    bool server_dir_listing(Connection *conn);
    // bool server_index_page_exists(Connection *conn);
    void update_url_with_index_page(Connection *conn);
};

#endif/* SERVER_HPP */
