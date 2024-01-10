#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <dirent.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>

// class HTTP;
class Connection;
class Response;

class Request {
  private:
    std::stringstream _raw;
    std::size_t _rawsize;
    std::string _method;
    std::string _url;
    std::string _body;
    std::string _host;
    std::size_t _content_length;
    std::string _content_type;
    std::string _content_body;

    Request(const Request &src);
    Request &operator=(const Request &rhs);

  public:
    Request();
    ~Request();
    std::string getMethod() const;
    std::string getUrl() const;
    std::string getBody() const;
    std::string getHost() const;
    std::string getRaw() const;
    bool not_parsed();
    void setUrl(std::string url);
    void parse_request();
    void process_request(Connection *conn);
    void process_requested_file(Connection *conn);
    int list_directory(std::string full_path, Connection *conn);

    void set_content_type(const std::string type);
    void set_content_body(const std::string type);
    void set_content_length(std::size_t length);
    std::string get_content_type() const;
    std::string get_content_body() const;
    std::size_t get_content_length() const;

    void append_raw(const char *buf, size_t len);
};
std::ostream &operator<<(std::ostream &out, const Request &obj);

#endif /* REQUEST_HPP */
