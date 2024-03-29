#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <sys/types.h>

class Server;
class HTTP;
struct Connection;
class Request;

class Response {
  private:
    std::string _version;
    std::string _status_code;
    std::string _content_type;
    std::size_t _content_length;
    std::map<std::string, std::string> _headers;

    Response(const Response &src);
    Response &operator=(const Response &rhs);

  public:
    Response();
    ~Response();

    std::stringstream _response_buffer;
    std::ifstream inputfilestream;
    bool isdir;
    std::string dir_data;
    bool _sent_header;
    mode_t permissions;
    std::string last_modified;
    bool _cgi_header_parsed;
    int buffer_writes;

    void set_header(std::string key, std::string value);
    void set_status_code(std::string code, Server *server, Request &request);
    void set_content_type(const std::string type);
    void set_content_length(std::size_t length);
    void set_error_page_fd(std::string full_path);

    std::string get_status_code() const;
    std::size_t get_content_length() const;
    std::string assemble_header();
    void parse_cgi_headers(Connection *conn);
    void write_buffer(char *str, std::size_t len);
    int read_buffer(char *buf, std::size_t size);
    bool has_header();
    int bytes_in_buffer();
};

#endif/* RESPONSE_HPP */
