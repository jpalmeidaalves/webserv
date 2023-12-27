#include "../headers/Request.hpp"

Request::Request(const std::string &data) {

    // std::cout << "data: " << data << std::endl;
    // parse request
    std::stringstream ss(data);
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

Request::~Request() {}

/* -------------------------------- Disabled -------------------------------- */
Request::Request() {}
Request &Request::operator=(const Request &rhs) {
    (void)rhs;
    return *this;
}
Request::Request(const Request &src) { *this = src; }

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */
std::string Request::getMethod() const { return (this->_method); }
std::string Request::getUrl() const { return (this->_url); }
std::string Request::getBody() const { return (this->_body); }
std::string Request::getHost() const { return (this->_host); }

std::ostream &operator<<(std::ostream &out, const Request &obj) {
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Host => " << obj.getHost() << std::endl;
    out << "url => " << obj.getUrl() << std::endl;
    out << "body => " << obj.getBody() << std::endl;
    return out;
}
