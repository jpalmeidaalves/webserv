#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <dirent.h>
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#define WRITE_END 1
#define READ_END 0
#define STD_IN 1
#define STD_OUT 0

// class HTTP;
class Connection;
class Response;
class Server;

class Request {
  private:
    std::string _method;
    std::string _url;
    std::string _host;
    std::size_t _content_length;
    std::string _content_type;

    Request(const Request &src);
    Request &operator=(const Request &rhs);

  public:
    std::stringstream _buffer;
    std::ofstream request_body;
    std::size_t request_body_writes;
    bool is_cgi;
    bool cgi_complete;
    bool read_complete;
    std::string query; // TODO make pivate?
    std::string short_url;
    std::string body_file_name;
    Request();
    ~Request();
    std::string getMethod() const;
    std::string getUrl() const;
    std::string getBody() const;
    std::string getHost() const;
    std::string getRaw() const;
    bool not_parsed();
    void setUrl(std::string url);
    void parse_request_header();
    void process_request(Connection *conn);
    void process_requested_file(Connection *conn, std::string full_path);
    int list_directory(std::string full_path, Connection *conn);
    bool has_cgi();
    int prepare_file_to_save_body(int fd, Connection *conn, int epfd);
    void set_content_type(const std::string type);
    void set_content_length(std::size_t length);
    std::string get_content_type() const;
    std::size_t get_content_length() const;

    void process_cgi(Connection *conn, int epfd);
    // std::string upload_files(Server *server);
    std::string getline_from_body(std::size_t &bytes_read);
    // std::string extract_filename_from_body(size_t &bytes_read);
    // std::string upload_single_file(size_t &bytes_read, std::string boundary, Server *server);
    void append_buffer(const char *buf, int len);
};
std::ostream &operator<<(std::ostream &out, const Request &obj);

#endif/* REQUEST_HPP */
