#include "../headers/Response.hpp"

Response::Response(): _version("HTTP/1.1"), _status("500"), _content_length(0) {
    
}

Response::~Response() {}

/* -------------------------------- Disabled -------------------------------- */
Response& Response::operator=(const Response &rhs) {
    (void)rhs;
    return *this;
}
Response::Response(const Response &src){ *this = src; }

void Response::set_status_code(std::string code) {
    this->_status = code;
}

// void Response::set_content_data(std::string data) {
//     this->_content_data = data;
// }

void Response::set_content_type(const std::string type) {
    this->_content_type = type;
}
void Response::set_content_length(std::size_t length){
    this->_content_length = length;
}

std::string Response::get_status_code() const{
    return (this->_status);
}

// std::string  Response::get_content_data() const{
//     return (this->_content_data);
// }
std::string Response::get_content_type() const{
    return (this->_content_type);
}
std::size_t Response::get_content_length() const{
    return (this->_content_length);
}