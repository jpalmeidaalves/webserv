#include "../headers/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                         Constructors & Desctructor                         */
/* -------------------------------------------------------------------------- */

/*  Constructor: Initialize the members of the struct sockaddr_in and its adress_len
    a atribute. Also intalls a signal handler  */
Server::Server() : client_max_body_size(4194304), s_addr(0), sin_port(0) {

    // init error pages
    this->_default_error_pages["403"] = "./etc/default_pages/403.html";
    this->_default_error_pages["404"] = "./etc/default_pages/404.html";
    this->_default_error_pages["413"] = "./etc/default_pages/413.html";
    this->_default_error_pages["500"] = "./etc/default_pages/500.html";
}

Server::~Server() {}

/* -------------------------------------------------------------------------- */
/*                              Member Functions                              */
/* -------------------------------------------------------------------------- */

/**
 * Returns the path of the error page from this server when available
 * 
 * @return if found the path string, 
 * @return if not found returns empty string
*/
std::string Server::get_error_page(std::string error_code) {
    std::map<std::string, std::string>::iterator it;
    for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++) {
        if (it->first == error_code) {
            return it->second;
        }
    }

    return "";
}

void Server::update_error_page(std::string error_code, std::string path) {
    this->_error_pages[error_code] = path;
}

std::string Server::get_default_error_page(std::string code) {
    std::cout << "searching default page with code: " << code << std::endl;
    std::map<std::string, std::string>::iterator it;
    for (it = this->_default_error_pages.begin(); it != this->_default_error_pages.end(); it++) {
        if (it->first == code) {
            std::cout << "found defualt error page: " << it->second << std::endl;
            return it->second;
        }
    }

    std::cout << "DID NOT FOUND DEFAULT ERROR PAGE" << std::endl;

    return "";
}