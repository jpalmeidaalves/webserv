#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sys/types.h>

class Server;
class HTTP;

class Response {
  private:
    std::string _version;
    std::string _status;
    std::string _content_type;
    std::size_t _content_length;
    int _req_file_fd;

    Response(const Response &src);
    Response &operator=(const Response &rhs);

  public:
    Response();
    ~Response();

    bool isdir;
    std::string dir_data;
    bool _sent_header;
    mode_t permissions;
    std::string last_modified;
    void set_status_code(std::string code, Server *server);
    // void set_content_data(unsigned char * data);
    void set_content_type(const std::string type);
    void set_content_length(std::size_t length);
    int get_requested_fd();
    void set_req_file_fd(int ffd);
    void set_error_page_fd(std::string full_path);

    std::string get_status_code() const;
    // std::string get_content_data() const;
    std::string get_content_type() const;
    std::size_t get_content_length() const;
};

#endif /* RESPONSE_HPP */
