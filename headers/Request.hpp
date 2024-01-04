#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>

class Request {
  private:
    std::string _raw;
    std::string _method;
    std::string _url;
    std::string _body;
    std::string _host;
    int _req_file_fd;
    bool _is_complete;

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
    void setUrl(std::string url);
    void parse_request();
    void setIsComplete();
    bool getIsComplete() const;
    int get_requested_fd();
    void set_req_file_fd(int ffd);

    void append_raw(std::string buf);
};
std::ostream &operator<<(std::ostream &out, const Request &obj);

#endif /* REQUEST_HPP */
