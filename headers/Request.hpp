#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <sstream>
#include <iostream>

class Request{
    private:
        std::string _method;
        std::string _url;
        std::string _content;

        Request();
        Request(const Request &src);
        Request & operator=(const Request &rhs);
        

    public:
        Request(const std::string &data);
        ~Request();
        std::string getMethod() const;
        std::string getUrl() const;
        std::string getContent() const;
};
std::ostream& operator<<(std::ostream & out,const Request &obj);

#endif/* REQUEST_HPP */
