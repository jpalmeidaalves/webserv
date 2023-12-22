#ifndef SERVER_HPP
#define SERVER_HPP

#include "../headers/utils.hpp"
#include "colors.hpp"

#include <arpa/inet.h> // htons, htonl
#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h> // socket, bind, struct sockaddr_in
#include <sys/types.h>
#include <unistd.h> // close
#include <vector>

#define MAXEPOLLSIZE SOMAXCONN
#define BUFSIZ 10000 // TODO change this
#define BACKLOG 200  // how many pending connections queue will hold
typedef std::map<int, std::string> sss;

class Server {

  private:
    int _sockfd;
    struct sockaddr_in _address;
    unsigned long _address_len;

    sss _inc_msgs;

    Server(); // disable default constructer

  public:
    Server(const std::string &configfile, int port);
    ~Server();
    
    int create_server();
    int monitor_multiple_fds();
    int setup_epoll(struct epoll_event &ev, int &ret, int &epfd);
    int send_response(int &cfd);
    int accept_and_add_to_poll(struct epoll_event &ev, int &ret, int &epfd);
    int read_complete(int &cfd, int &epfd, epoll_event &ev, int &ret);
    int still_reading(char *buf, int buflen);
};

#endif /* SERVER_HPP */
