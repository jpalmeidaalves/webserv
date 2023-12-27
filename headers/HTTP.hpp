#ifndef HTTP_HPP
#define HTTP_HPP

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h> // close
#include <vector>

#include "../headers/utils.hpp"
#include "MimeTypes.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "colors.hpp"

#define MAXEPOLLSIZE SOMAXCONN
#define BUFSIZ 1000 // TODO change this
#define BACKLOG 200 // how many pending connections queue will hold

typedef std::map<int, std::string> msgs_map;

class HTTP {
  private:
    HTTP(const HTTP &src);
    HTTP &operator=(const HTTP &rhs);

    int _epfd; // epoll fd
    std::vector<Server *> _servers;
    msgs_map _inc_msgs;

  public:
    HTTP();
    ~HTTP();
    int monitor_multiple_fds();
    int send_response(int &cfd);
    int accept_and_add_to_poll(struct epoll_event &ev, int &epfd, int sockfd);
    int close_connection(int &cfd, int &epfd, epoll_event &ev);
    int add_listening_socket_to_poll(struct epoll_event &ev, int listening_socket);
    bool is_listening_socket(int sockfd);
    int read_socket(struct epoll_event &ev);

    class FailedToInit : public std::exception {
        virtual const char *what() const throw();
    };

    class FailedToCreateServer : public std::exception {
        virtual const char *what() const throw();
    };
};

#endif /* HTTP_HPP */
