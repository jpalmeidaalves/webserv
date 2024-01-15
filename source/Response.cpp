#include "../headers/Response.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

Response::Response()
    : _version("HTTP/1.1"), _status_code("200"), _content_type("text/html"), _content_length(0), _req_file_fd(0),
      isdir(false), _sent_header(false) {

    // Define default headers
    this->set_header("Content-Type", "text/html");
    // Enabling CORS by default
    this->set_header("Access-Control-Allow-Origin", "*");
    this->set_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE");
    this->set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    this->set_header("Access-Control-Allow-Credentials", "true");
}

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

void Response::set_header(std::string key, std::string value) {
    std::map<std::string, std::string>::iterator it;

    // check if there is a key already in the header
    for (it = this->_headers.begin(); it != this->_headers.end(); ++it) {
        // keys can be case insensitive so we will check in insensitive mode
        if (ft_strcmp_insensitive(it->first, key)) {
            // found match, update it and stop here
            it->second = value;
            return;
        }
    }

    // if reaches here means its a new entry and must be inserted
    this->_headers.insert(std::pair<std::string, std::string>(key, value));
}

void Response::set_status_code(std::string code, Server *server) {
    // update status code
    this->_status_code = code;

    // TODO check if is a error code
    if (code[0] == '4' || code[0] == '5') {
        // check if has custom error page
        if (server->get_error_page(code) != "") {
            this->set_error_page_fd(server->root + server->get_error_page(code));
        } else {
            // else use the default error page
            this->set_error_page_fd(server->get_default_error_page(code));
        }
    }

}

void Response::set_content_type(const std::string type) { this->_content_type = type; }
void Response::set_content_length(std::size_t length) { this->_content_length = length; }

std::string Response::get_status_code() const { return (this->_status_code); }

std::string Response::get_content_type() const { return (this->_content_type); }
std::size_t Response::get_content_length() const { return (this->_content_length); }

std::string Response::assemble_header() {
    std::ostringstream ss;

    // TODO missing status description after status code
    // TODO how to handle if we set an invalid status code in the header inside PHP

    ss << "HTTP/1.1 " << this->_status_code << "\r\n";

    std::map<std::string, std::string>::iterator it;
    for (it = this->_headers.begin(); it != this->_headers.end(); ++it) {
        ss << it->first << ": " << it->second << "\r\n";
    }

    ss << "\r\n";

    return ss.str();
}

/**
 * Use the stringstream from the CGI header and update the headers in the response object.
 *
 * @param ss - stringstream with the headers from CGI
 * @param server - pointer to the server that handle this request
 */
void Response::parse_cgi_headers(std::stringstream &ss, Server *server) {
    // Update all the headers in the response with the header from CGI
    std::string key_status = "Status: ";
    std::string line;

    while (1) {
        std::getline(ss, line);
        if (ss.fail())
            break;

        if (line == "\r" || line == "")
            break;

        if (line.find(key_status) == 0) {
            std::string value = line.substr(key_status.size(), 3); // extract error code (3 bytes size)
            // std::cout << "status is " << value << std::endl;
            this->set_status_code(value, server);
        } else {
            std::size_t colon_pos = line.find(":");

            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);

                // if first char is ' ' remove it
                if (value.at(0) == ' ') {
                    value.erase(0, 1);
                }
                // if last char is '\r' remove it
                if (value.size() && value.at(value.size() - 1) == '\r') {
                    value.erase(value.size() - 1);
                }

                // std::cout << "1) key is >" << key << "<" << std::endl;
                // std::cout << "2) value is >" << value << "<" << std::endl;

                this->set_header(key, value);
            }
        }
    }
}