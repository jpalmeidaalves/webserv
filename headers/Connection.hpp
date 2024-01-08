#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <sys/epoll.h>

// class Request;
// class Response;

struct Connection {
    Request request;
    Response response;
    std::string host;
    std::string port;
    uint32_t s_addr;
    uint16_t sin_port;
};

#endif/* CONNECTION_HPP */
