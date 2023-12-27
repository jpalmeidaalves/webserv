#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>

class Request {
  private:
    std::string _method;
    std::string _url;
    std::string _body;
    std::string _host;

    Request();
    Request(const Request &src);
    Request &operator=(const Request &rhs);

  public:
    Request(const std::string &data);
    ~Request();
    std::string getMethod() const;
    std::string getUrl() const;
    std::string getBody() const;
    std::string getHost() const;
};
std::ostream &operator<<(std::ostream &out, const Request &obj);

#endif /* REQUEST_HPP */
