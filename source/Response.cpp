#include "../headers/StatusCode.hpp"
#include "../headers/Response.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"
#include "../headers/Connection.hpp"

Response::Response()
    : _version("HTTP/1.1"), _status_code("200"), _content_type("text/html"), _content_length(0),
      isdir(false), _sent_header(false), _cgi_header_parsed(false), buffer_writes(0) {

    // Define default headers
    this->set_header("Content-Type", "text/html");
    // Enabling CORS by default
    this->set_header("Access-Control-Allow-Origin", "*");
    this->set_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE");
    this->set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    this->set_header("Access-Control-Allow-Credentials", "true");
    this->set_header("Connection", "close");
}

Response::~Response() {}

Response &Response::operator=(const Response &rhs) {
    (void)rhs;
    return *this;
}

Response::Response(const Response &src) { *this = src; }

void Response::set_error_page_fd(std::string full_path) {
    this->set_content_type(MimeTypes::identify(full_path));

    if (this->inputfilestream.is_open())
        this->inputfilestream.close();

    this->inputfilestream.open(full_path.c_str());
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

void Response::set_status_code(std::string code, Server *server, Request &request) {
    // update status code
    this->_status_code = code;

    // check if is a error code
    if (code[0] == '4' || code[0] == '5') {
        // check if has custom error page

        if (server->get_error_page(code) != "") {
            // update request url_path
            request.url_path = server->root + server->get_error_page(code);
            std::cout << BLUE << "using server error page..." << request.url_path << RESET << std::endl;
            this->set_error_page_fd(request.url_path);
        } else {
            std::cout << RED << "using default error page..." << RESET << std::endl;
            // update request url_path
            request.url_path = server->get_default_error_page(code);
            // else use the default error page
            this->set_error_page_fd(server->get_default_error_page(code));
        }
    }
}

std::string Response::get_status_code() const { return (this->_status_code); }

void Response::set_content_type(const std::string type) { this->set_header("Content-Type", type); }
void Response::set_content_length(std::size_t length) {
    this->_content_length = length;
    this->set_header("Content-Length", ft_ultos(length));
}

std::size_t Response::get_content_length() const { return (this->_content_length); }

/**
 * Create a stringstream based on the previously defined headers with set_header function
*/
std::string Response::assemble_header() {
    std::ostringstream ss;

    ss << "HTTP/1.1 " << StatusCode::get_code(this->_status_code) << "\r\n";

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
void Response::parse_cgi_headers(Connection *conn) {

    std::stringstream &ss = conn->response._response_buffer;
    Server *server = conn->server;

    // Show python cgi errors
    if (ss.str().find("  File") == 0) {
        this->_cgi_header_parsed = true;
        return;
    }

    // Update all the headers in the response with the header from CGI
    bool php_notices_in_header = false;

    if (ss.str().find("PHP") == 0) {
        php_notices_in_header = true;
    }

    std::string key_status = "Status: ";
    std::string status_code;
    std::string line;


    bool has_status_header = ss.str().find(key_status) != std::string::npos;

    int i = 0;
    while (1) {

        std::getline(ss, line, '\n');
        i++;
        if (ss.bad()) {
            break;
        }

        if (line == "\r" || line == "") {
            break;
        }

        // If has Status header line inside the header
        // ignore all lines until it encounters that specific line
        if (has_status_header && line.find(key_status) != 0 && status_code == "") {
            continue;
        }

        if (line.find(key_status) == 0) {
            status_code = line.substr(key_status.size(), 3); // extract error code (3 bytes size)
            this->set_status_code(status_code, server, conn->request);
        } else {

            if (!php_notices_in_header) {
                std::size_t colon_pos = line.find(":");

                if (colon_pos != std::string::npos) {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(colon_pos + 1);

                    // if first char is ' ' remove it
                    if (value.size() && value.at(0) == ' ') {
                        value.erase(0, 1);
                    }
                    // if last char is '\r' remove it
                    if (value.size() && value.at(value.size() - 1) == '\r') {
                        value.erase(value.size() - 1);
                    }

                    this->set_header(key, value);
                }
            }
        }

        line = "";
    }
    this->_cgi_header_parsed = true;
}

void Response::write_buffer(char *str, std::size_t len) {
    if (this->_response_buffer.write(str, len)) {
        if (this->_response_buffer.fail()) {
            std::cerr << "_response_buffer error FAILED TO WRITE" << std::endl;
        } else {
            this->buffer_writes++;
        }
    }
}

bool Response::has_header() {
    if (this->_response_buffer.str().find("\r\n\r\n") != std::string::npos)
        return true;
    return false;
}

int Response::bytes_in_buffer() { return (this->_response_buffer.tellp()); }

int Response::read_buffer(char *buf, std::size_t size) {
    this->_response_buffer.read(buf, size);
    if (this->_response_buffer.fail())
        return 0;
    return (0);
}