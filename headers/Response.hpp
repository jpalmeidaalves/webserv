#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>

class Response {
    private:
        std::string _version;
        std::string _status;
        std::string _content_type;
        std::size_t _content_length;

        Response(const Response &src);
        Response & operator=(const Response &rhs);

    public:
        Response();
        ~Response();

        void set_status_code(std::string code);
        // void set_content_data(unsigned char * data);
        void set_content_type(const std::string type);
        void set_content_length(std::size_t length);

        std::string get_status_code() const;
        // std::string get_content_data() const;
        std::string get_content_type() const;
        std::size_t get_content_length() const;
};

#endif/* RESPONSE_HPP */
