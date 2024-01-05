#ifndef PARSERCONFIGFILE_HPP
#define PARSERCONFIGFILE_HPP

#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct SServer {
    std::string host;
    std::string port;
    uint16_t sin_port;
    uint32_t s_addr;
    std::vector<std::string> server_names_vector;
    int client_max_body_size;
    std::string root;
    // std::vector<std::string> index;
};

class ParserConfFile {
  private:
    int fd;
    std::string _path;
    std::vector<SServer> servers;
    int servers_count;
    ParserConfFile(const ParserConfFile &src);
    ParserConfFile &operator=(const ParserConfFile &src);
    ParserConfFile();
    std::vector<std::string> tokens;

  public:
    ParserConfFile(std::string path);
    ~ParserConfFile();
    int open_config_file();
    int extract();
    std::vector<struct sockaddr_in> get_unique_addresses();
    std::vector<std::string>::iterator get_serv_data(std::vector<std::string>::iterator it,
                                                     struct SServer &s);

    void printMembers(void) const;
    void print_server_data();
    std::vector<SServer> &get_servers();

    // template <typename T>
    // void printVector(std::vector<T> v);

    class FailedToOpenConfFile : public std::exception {
        virtual const char *what() const throw();
    };
};
std::ostream &operator<<(std::ostream &out, const SServer &obj);

#endif /* PARSERCONFIGFILE_HPP */

/*
    [X] Choose the port and host of each 'server'.
    [X] Setup the server_names or not.
    [X] The first server for a host:port will be the default for this host:port (that means it will
   answer to all the requests that don't belong to an other server). [X] Setup default error pages.
    [X] Limit client body size.
    [ ] Setup routes with one or multiple of the following rules/configuration (routes wont be using
   regexp): [ ] Define a list of accepted HTTP methods for the route. [ ] Define a HTTP redirection.
        [ ] Define a directory or a file from where the file should be searched (for example, if url
   /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet). [ ]
   Turn on or off directory listing. [ ] Set a default file to answer if the request is a directory.
        [ ] Execute CGI based on certain file extension (for example .php).
        [ ] Make it work with POST and GET methods.
        [ ] Make the route able to accept uploaded files and configure where they should be saved.

*/

// Server 1
//  http://server1.com
//  http://www.server1.com
//  http://api.server1.com
//  http://myserver1.com

/*

!Note default_server is a flag to indicate the default server. Will not implement since subject asks
us to use the first server as the default.


!Note client_max_body_size default size is 1M;

http {

    client_max_body_size 100M;

    server {
        listen 80 default_server;
        listen [::]:80 default_server;
        server_names_vector example.com www.example.com;
        root /var/www/example.com;
        index index.html;
        try_files $uri /index.html;
    }

    server {
        listen       80;
        server_names_vector  example.org  www.example.org;
        client_max_body_size 50M;

        location /uploads {
            ...
            client_max_body_size 1000M;
        }

        error_page 404 /custom_404.html;
        location = /custom_404.html {
            root /usr/share/nginx/html;
            internal;
        }

        error_page 500 502 503 504 /custom_50x.html;
        location = /custom_50x.html {
            root /usr/share/nginx/html;
            internal;
        }
    }
}
*/