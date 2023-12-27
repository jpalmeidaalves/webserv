#include "../headers/Request.hpp"

Request::Request(const std::string &data) {
    
    std::cout << "data: " << data << std::endl;
    // parse request
    std::stringstream ss(data);

    getline(ss, this->_method, ' ');
    std::cout << "method: " << this->_method << std::endl; 
    getline(ss, this->_url, ' ');
    std::cout << "url: " << this->_url << std::endl; 
}

Request::~Request() {}

/* -------------------------------- Disabled -------------------------------- */
Request::Request() {}
Request& Request::operator=(const Request &rhs) { 
    (void)rhs;
    return *this;
}
Request::Request(const Request &src){ *this = src; }

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */
std::string Request::getMethod() const{ return (this->_method);}
std::string Request::getUrl() const{return (this->_url);}
std::string Request::getContent() const{return (this->_content);}

std::ostream& operator<<(std::ostream & out,const Request &obj)
{
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Method => " << obj.getContent() << std::endl;
    out << "Method => " << obj.getUrl() << std::endl;
    return out;
}

