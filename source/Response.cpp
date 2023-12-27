#include "../headers/Response.hpp"

Response::Response(): _version("HTTP/1.1"), _status(500) {
    
}

Response::~Response() {}

/* -------------------------------- Disabled -------------------------------- */
Response& Response::operator=(const Response &rhs) {
    (void)rhs;
    return *this;
}
Response::Response(const Response &src){ *this = src; }
