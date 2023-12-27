#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

class Response {
    private:
        std::string _version;
        int _status;
        std::string _content_type;
        std::size_t _content_length;

        Response(const Response &src);
        Response & operator=(const Response &rhs);

    public:
        Response();
        ~Response();
};

#endif/* RESPONSE_HPP */
