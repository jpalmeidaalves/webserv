#ifndef SERVER_HPP
#define SERVER_HPP

#include "../headers/utils.hpp"
#include "colors.hpp"

#include <arpa/inet.h> // htons, htonl
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h> // socket, bind, struct sockaddr_in
#include <sys/types.h>
#include <unistd.h> // close

class Server {

  private:
    int _sockfd;
    struct sockaddr_in _address;
    unsigned long _address_len;

    Server(); // disable default constructer

  public:
    Server(const std::string &configfile, int port);
    ~Server();

    void create_server();
    void start_listen();
    int accept_connection();

    class SocketException : public std::exception {
        virtual const char *what() const throw();
    };
    class BindException : public std::exception {
        virtual const char *what() const throw();
    };
    class ListenException : public std::exception {
        virtual const char *what() const throw();
    };
};

#endif /* SERVER_HPP */
