#ifndef SERVER_HPP
#define SERVER_HPP

#include "../headers/utils.hpp"
#include "colors.hpp"

#include <arpa/inet.h> // htons, htonl
#include <sys/socket.h> // socket, bind, struct sockaddr_in
#include <string>
#include <unistd.h> // close


// TODO remove unused
#include <netdb.h>
#include <stdlib.h>
#include <cerrno>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>


class Server {

  private:
    // struct sockaddr_in _address;
    // unsigned long _address_len;
    int _sockfd;
    struct addrinfo          _hints;
    struct addrinfo          *result;
    int _port;

    Server(); // disable default constructer

  public:
    Server(int port);
    ~Server();
    
    int create_server();
    int get_sockfd() const;
};

#endif/* SERVER_HPP */
