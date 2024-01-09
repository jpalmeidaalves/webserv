#include "../headers/Response.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

Response::Response()
    : _version("HTTP/1.1"), _status("200"), _content_type("text/html"), _content_length(0),
      _req_file_fd(0), isdir(false), _sent_header(false) {}

Response::~Response() {}

/* -------------------------------- Disabled -------------------------------- */
Response &Response::operator=(const Response &rhs) {
    (void)rhs;
    return *this;
}
Response::Response(const Response &src) { *this = src; }

int Response::get_requested_fd() { return (this->_req_file_fd); }

void Response::set_req_file_fd(int ffd) { this->_req_file_fd = ffd; }

void Response::set_error_page_fd(std::string full_path) {

    std::cout << "setting error page: " << full_path << std::endl;

    if (get_stat_info(full_path, *this)) {
        return;
    }

    if (!(this->permissions & S_IROTH)) {
        return;
    }

    int file_fd = open(full_path.c_str(), O_RDONLY);
    if (!file_fd) {
        print_error("Error opening file");
        return;
    }

    this->set_content_type(MimeTypes::identify(full_path));
    this->set_req_file_fd(file_fd);
}

void Response::set_status_code(std::string code, Server *server) {
    // update status code
    this->_status = code;

    // check if has custom error page
    if (server->get_error_page(code) != "") {
        this->set_error_page_fd(server->root + server->get_error_page(code));
    } else {
        // else use the default error page
        this->set_error_page_fd(server->get_default_error_page(code));
    }
}

// void Response::set_content_data(std::string data) {
//     this->_content_data = data;
// }

void Response::set_content_type(const std::string type) { this->_content_type = type; }
void Response::set_content_length(std::size_t length) { this->_content_length = length; }

std::string Response::get_status_code() const { return (this->_status); }

// std::string  Response::get_content_data() const{
//     return (this->_content_data);
// }
std::string Response::get_content_type() const { return (this->_content_type); }
std::size_t Response::get_content_length() const { return (this->_content_length); }