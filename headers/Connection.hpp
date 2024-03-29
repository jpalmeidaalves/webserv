#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include <sys/epoll.h>

struct Connection {
    Request request;
    Response response;
    Server *server;
    std::string host;
    std::string port;
    uint32_t s_addr;
    uint16_t sin_port;
    struct epoll_event *ev_ptr;
    int fd;
    int cgi_pid; // if has CGI add the process ID of the child process
    int cgi_fd;
    long last_operation;
    bool timedout;
};

#endif /* CONNECTION_HPP */
