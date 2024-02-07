#include "../headers/Server.hpp"
#include "../headers/Connection.hpp"
#include "../headers/Request.hpp"

/* -------------------------------------------------------------------------- */
/*                         Constructors & Desctructor                         */
/* -------------------------------------------------------------------------- */

/*  Constructor: Initialize the members of the struct sockaddr_in and its adress_len
    a atribute. Also intalls a signal handler  */
Server::Server() : client_max_body_size(4194304), s_addr(0), sin_port(0) {

    // init error pages
    this->_default_error_pages["400"] = "./etc/default_pages/400.html";
    this->_default_error_pages["401"] = "./etc/default_pages/401.html";
    this->_default_error_pages["403"] = "./etc/default_pages/403.html";
    this->_default_error_pages["404"] = "./etc/default_pages/404.html";
    this->_default_error_pages["405"] = "./etc/default_pages/405.html";
    this->_default_error_pages["406"] = "./etc/default_pages/406.html";
    this->_default_error_pages["407"] = "./etc/default_pages/407.html";
    this->_default_error_pages["408"] = "./etc/default_pages/408.html";
    this->_default_error_pages["409"] = "./etc/default_pages/409.html";
    this->_default_error_pages["410"] = "./etc/default_pages/410.html";
    this->_default_error_pages["411"] = "./etc/default_pages/411.html";
    this->_default_error_pages["412"] = "./etc/default_pages/412.html";
    this->_default_error_pages["413"] = "./etc/default_pages/413.html";
    this->_default_error_pages["414"] = "./etc/default_pages/414.html";
    this->_default_error_pages["415"] = "./etc/default_pages/415.html";
    this->_default_error_pages["416"] = "./etc/default_pages/416.html";
    this->_default_error_pages["417"] = "./etc/default_pages/417.html";
    this->_default_error_pages["418"] = "./etc/default_pages/418.html";
    this->_default_error_pages["421"] = "./etc/default_pages/421.html";
    this->_default_error_pages["422"] = "./etc/default_pages/422.html";
    this->_default_error_pages["423"] = "./etc/default_pages/423.html";
    this->_default_error_pages["424"] = "./etc/default_pages/424.html";
    this->_default_error_pages["426"] = "./etc/default_pages/426.html";
    this->_default_error_pages["428"] = "./etc/default_pages/428.html";
    this->_default_error_pages["429"] = "./etc/default_pages/429.html";
    this->_default_error_pages["431"] = "./etc/default_pages/431.html";
    this->_default_error_pages["451"] = "./etc/default_pages/451.html";

    this->_default_error_pages["500"] = "./etc/default_pages/500.html";
    this->_default_error_pages["501"] = "./etc/default_pages/501.html";
    this->_default_error_pages["502"] = "./etc/default_pages/502.html";
    this->_default_error_pages["503"] = "./etc/default_pages/503.html";
    this->_default_error_pages["504"] = "./etc/default_pages/504.html";
    this->_default_error_pages["505"] = "./etc/default_pages/505.html";
    this->_default_error_pages["506"] = "./etc/default_pages/506.html";
    this->_default_error_pages["507"] = "./etc/default_pages/507.html";
    this->_default_error_pages["508"] = "./etc/default_pages/508.html";
    this->_default_error_pages["510"] = "./etc/default_pages/510.html";
    this->_default_error_pages["511"] = "./etc/default_pages/511.html";
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

bool Server::server_dir_listing(Connection *conn) {
    std::map<std::string, struct LocationOptions>::iterator it;

    for (it = this->locations.begin(); it != this->locations.end(); it++) {
        if (it->first == conn->request.getUrl()) {
            return it->second.autoindex;
        }
    }

    return false;
}

bool Server::server_index_page_exists(Connection *conn) {
    std::map<std::string, struct LocationOptions>::iterator it;

    std::string full_path = conn->server->root + conn->request.getUrl();
    if (full_path.size() && full_path.at(full_path.size() - 1) != '/')
            full_path += "/";

    for (it = this->locations.begin(); it != this->locations.end(); it++) {
        if (it->first == conn->request.getUrl() || (it->first + "/") == conn->request.getUrl()) {

            std::vector<std::string>::iterator pages_it;
            for (pages_it = it->second.index_pages.begin(); pages_it != it->second.index_pages.end(); pages_it++) {
                
                // Check if the file exists in the FileSystem
                std::string filename = full_path + *pages_it;
                if (file_exists(filename.c_str()) == 0) {
                    conn->request.setUrl(full_path + *pages_it);
                    return true;
                }

            }

        }
    }

    // TODO check server index pages

    return false;
}