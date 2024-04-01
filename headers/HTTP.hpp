#ifndef HTTP_HPP
#define HTTP_HPP

#include "MimeTypes.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"
#include "colors.hpp"
#include <bitset>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // close
#include <vector>

#define MAXEPOLLSIZE SOMAXCONN
#define BUFFERSIZE 8000
#define BACKLOG 100000    // how many pending connections queue will hold
#define TIMEOUT 30
#define EOT 4

struct Connection;
class Server;

typedef std::map<int, Connection *> connects_map;

struct dir_entry {
    bool is_file;
    long size;
    std::string last_modified;
    std::string href;
};

class HTTP {
  private:
    HTTP(const HTTP &src);
    HTTP &operator=(const HTTP &rhs);

    int _epoll_fd; // epoll fd
    std::vector<int> _listening_sockets;

    std::vector<Server> _servers;
    connects_map _active_connects;

    HTTP();

  public:
    HTTP(std::vector<Server> &servers);
    ~HTTP();
    int open_listening_sockets(std::vector<struct sockaddr_in> addresses);
    int handle_connections();
    int accept_and_add_to_poll(struct epoll_event &ev, int &epfd, int sockfd);
    void close_connection(int cfd, int &epfd, epoll_event &ev);
    int add_listening_socket_to_poll(struct epoll_event &ev, int sockfd);
    void read_socket(struct epoll_event &ev);
    int send_header(int &cfd, struct epoll_event &ev, Response &response);
    void write_socket(struct epoll_event &ev);
    int epoll_mod(struct epoll_event &ev, uint32_t flag);
    void redirect_to_server(Connection *conn);
    void read_cgi_socket(int fd, Connection *conn, struct epoll_event &cgi_ev);
    Connection *get_associated_conn(int sock);
    static void remove_cgi_socket(int sock);
    static void add_cgi_socket(int sock, int connection_socket);
    void process_request(struct epoll_event &ev);
    void handle_timeouts();
};

#endif/* HTTP_HPP */
