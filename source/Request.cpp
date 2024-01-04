#include "../headers/Request.hpp"

Request::Request() : _req_file_fd(0), _is_complete(false) {}

Request::~Request() {}

/* -------------------------------- Disabled -------------------------------- */
Request &Request::operator=(const Request &rhs) {
    (void)rhs;
    return *this;
}
Request::Request(const Request &src) { *this = src; }

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

void Request::parse_request() {
    // std::cout << "data: " << data << std::endl;
    // parse request
    std::stringstream ss(this->_raw);
    std::string line;

    // process first line
    getline(ss, this->_method, ' ');
    getline(ss, this->_url, ' ');
    getline(ss, line); // ignore the rest of the line

    // get host
    getline(ss, line);
    // std::cout << line << std::endl;
    if (line.find("Host:") == 0) {
        this->_host = line.substr(line.find(" ") + 1);
    }
}

std::string Request::getMethod() const { return (this->_method); }
std::string Request::getUrl() const { return (this->_url); }
std::string Request::getBody() const { return (this->_body); }
std::string Request::getHost() const { return (this->_host); }
std::string Request::getRaw() const { return (this->_raw); }

void Request::append_raw(std::string buf) { this->_raw += buf; }

std::ostream &operator<<(std::ostream &out, const Request &obj) {
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Host => " << obj.getHost() << std::endl;
    out << "url => " << obj.getUrl() << std::endl;
    out << "body => " << obj.getBody() << std::endl;
    return out;
}

void Request::setUrl(std::string url) { this->_url = url; }

int Request::get_requested_fd() { return (this->_req_file_fd); }

void Request::setIsComplete() { this->_is_complete = true; }

bool Request::getIsComplete() const { return this->_is_complete; }

void Request::set_req_file_fd(int ffd) { this->_req_file_fd = ffd; }
