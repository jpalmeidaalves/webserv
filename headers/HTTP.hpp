#ifndef HTTP_HPP
#define HTTP_HPP

#include "../headers/utils.hpp"
#include "MimeTypes.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"
#include "colors.hpp"
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // close
#include <vector>

#define MAXEPOLLSIZE SOMAXCONN
#define BUFFERSIZE 8000
#define BACKLOG 200 // how many pending connections queue will hold

// Connection::Connection() {}
// Connection::~Connection() {}

// typedef std::map<int, Connection *> inc_connects_t;

class HTTP {
  private:
    HTTP(const HTTP &src);
    HTTP &operator=(const HTTP &rhs);

    int _epfd; // epoll fd
    std::vector<Server *> _servers;
    // inc_connects_t _inc_connects;

  public:
    HTTP();
    ~HTTP();
    int handle_connections();
    int accept_and_add_to_poll(struct epoll_event &ev, int &epfd, int sockfd);
    int close_connection(int &cfd, int &epfd, epoll_event &ev);
    int add_listening_socket_to_poll(struct epoll_event &ev, Server *server);
    bool is_listening_socket(int sockfd);
    int read_socket(struct epoll_event &ev);
    int send_header(int &cfd, const Response &response);

    class FailedToInit : public std::exception {
        virtual const char *what() const throw();
    };

    class FailedToCreateServer : public std::exception {
        virtual const char *what() const throw();
    };
};

#endif /* HTTP_HPP */
