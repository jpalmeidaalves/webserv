#include "../headers/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                         Constructors & Desctructor                         */
/* -------------------------------------------------------------------------- */

/*  Constructor: Initialize the members of the struct sockaddr_in and its adress_len
    a atribute. Also intalls a signal handler  */
Server::Server() : client_max_body_size(0), s_addr(0), sin_port(0) {}

Server::~Server() {}

/* -------------------------------------------------------------------------- */
/*                              Member Functions                              */
/* -------------------------------------------------------------------------- */
std::string Server::get_error_page(std::string error_code) {
    std::multimap<std::string, std::string>::iterator it;
    for (it = error_pages.begin(); it != error_pages.end(); it++) {
        if (it->first == error_code) {
            return it->second;
        }
    }

    // TODO send the default error page
    return "";
}