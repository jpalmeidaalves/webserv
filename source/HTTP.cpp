#include "../headers/HTTP.hpp"
#include "../headers/Connection.hpp"
#include "../headers/utils.hpp"

bool g_stop = false;

std::map<int, int> HTTP::cgi_sockets;

void sighandler(int signum) {
    (void)signum;
    std::cout << "control-c triggered!!" << std::endl;
    g_stop = true;
}

HTTP::HTTP() {}

HTTP::HTTP(std::vector<Server> &servers) : _epoll_fd(0), _servers(servers) {
    signal(SIGINT, sighandler);
    // signal(SIGQUIT, sighandler);
}

HTTP::~HTTP() {}

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

int HTTP::open_listening_sockets(std::vector<struct sockaddr_in> addresses) {
    std::vector<struct sockaddr_in>::iterator it;

    for (it = addresses.begin(); it != addresses.end(); ++it) {
        unsigned long address_len = sizeof(*it);
        int curr_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (curr_sockfd < 0) {
            print_error("Failed to create socket");
            return 1;
        }
        int optval = 1;
        if ((setsockopt(curr_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) != 0)
            return 1;
        if (bind(curr_sockfd, (struct sockaddr *)&*it, address_len) < 0) {
            print_error("Failed to bind socket to the address");
            return 1;
        }
        if (listen(curr_sockfd, SOMAXCONN) < 0) {
            print_error("Failed to listen");
            return 1;
        }
        // show message
        std::cout << "Server listening on " << BOLDGREEN << convert_uint32_to_str(ntohl(it->sin_addr.s_addr)) << ":"
                  << ntohs(it->sin_port) << RESET << ", sockfd: " << curr_sockfd << std::endl;
        this->_listening_sockets.push_back(curr_sockfd);
    }

    return 0;
}

int HTTP::add_listening_socket_to_poll(struct epoll_event &ev, int sockfd) {

    // std::cout << "adding listening socket " << sockfd << " to epoll" << std::endl;
    int ret;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    ret = epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret == -1) {
        print_error("failed add to epoll");
        return 1;
    }
    return 0;
}

int HTTP::accept_and_add_to_poll(struct epoll_event &ev, int &epfd, int sockfd) {
    int ret;
    struct sockaddr_in cur_sockin;
    socklen_t socklen = sizeof(struct sockaddr);

    int accepted_fd = accept(sockfd, (struct sockaddr *)&cur_sockin, &socklen);
    if (accepted_fd == -1) {
        print_error("failed to accept connection");
        return 1;
    }
    std::cout << RED << "accepted connection in listing socket: " << sockfd << " for incoming socket: " << accepted_fd
              << RESET << std::endl;

    ev.events = EPOLLIN;
    ev.data.fd = accepted_fd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, accepted_fd, &ev);
    if (ret == -1) {
        close(accepted_fd);
        print_error("failed epoll_ctl");
        return 1;
    }
    // create a key with accepted_fd, whose value is a struct Connection
    this->_active_connects[accepted_fd] = new Connection();
    this->_active_connects[accepted_fd]->fd = accepted_fd;
    this->_active_connects[accepted_fd]->cgi_pid = 0;
    // extract the ip number and port from accepted socket, store in Connecetion struct
    get_port_host_from_sockfd(accepted_fd, this->_active_connects[accepted_fd]);
    return 0;
}

/**
 * Checks if sock is in the HTTP::cgi_sockets map
 *
 * @note HTTP::cgi_sockets contains a map with the cgi socket and the associated socket
 * for the client.
 */
bool HTTP::is_cgi_socket(int sock) {
    std::map<int, int>::iterator it;
    for (it = HTTP::cgi_sockets.begin(); it != HTTP::cgi_sockets.end(); it++) {
        if (it->first == sock) {
            return true;
        }
    }
    return false;
}

// TODO remove the part where we delete from epoll from this function
void HTTP::close_connection(int cfd, int &epfd, epoll_event &ev) {

    std::cout << "Closing connection for socket " << cfd << " ..." << std::endl;

    // check if the client socket has an associated CGI socket
    int assoc_cgi_socket = this->_active_connects[cfd]->request.cgi_socket;
    if (assoc_cgi_socket) {
        HTTP::remove_cgi_socket(assoc_cgi_socket);
    }

    delete this->_active_connects[cfd];
    this->_active_connects.erase(cfd);

    // Removes this FD from the EPOLL
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);
    if (ret == -1) {
        std::cerr << "failed to remove fd " << cfd << " from EPOLL" << std::endl;
        print_error(strerror(errno));
    }

    // close fd
    if (close(cfd) == -1) {
        print_error("failed to close fd");
    } else {
        std::cout << "Connection closed for socket " << cfd << "!" << std::endl;
    }
}

/**
 * Update the type of monitoring in the current event
 *
 * @param ev the event we want to modify
 * @param flag the mode we want to change (EPOLLIN | EPOLLOUT)
 */
int HTTP::epoll_mod(struct epoll_event &ev, uint32_t flag) {
    int cfd = ev.data.fd;

    // update to the new flags
    ev.events = flag;

    int ret = epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, cfd, &ev);
    if (ret == -1) {
        print_error("failed to change mode");
        return ret;
    }

    std::cout << CYAN << "Changed fd " << ev.data.fd << " to " << ((flag & EPOLLIN) ? "[EPOLLIN] " : "")
              << ((flag & EPOLLOUT) ? "[EPOLLOUT] " : "") << RESET << std::endl;

    return 0;
}

int HTTP::handle_connections() {

    this->_epoll_fd = epoll_create(MAXEPOLLSIZE);
    if (this->_epoll_fd == -1)
        return 1;

    struct epoll_event ev;
    ft_memset(&ev, 0, sizeof(ev));

    struct epoll_event evlist[MAXEPOLLSIZE];

    // add each sockfd to the epoll list of listening sockets
    std::vector<int>::iterator it;
    for (it = this->_listening_sockets.begin(); it != this->_listening_sockets.end(); ++it) {
        if (this->add_listening_socket_to_poll(ev, *it))
            return 1;
    }
    // program is hanging, waiting for events
    while (!g_stop) {
        // put the epoll instance waiting for events(requests) until a fd delivers an event
        int nfds = epoll_wait(this->_epoll_fd, evlist, MAXEPOLLSIZE, -1);
        if (nfds == -1) {
            print_error("epoll_wait failed");
            continue;
        }
        //
        for (int i = 0; i < nfds; i++) {

            if (is_listening_socket(evlist[i].data.fd, this->_listening_sockets)) {

                this->accept_and_add_to_poll(ev, this->_epoll_fd, evlist[i].data.fd);

            } else if (evlist[i].events & EPOLLIN) {
                // Ready for read
                std::cout << YELLOW << "READING" << RESET << std::endl;

                if (HTTP::is_cgi_socket(evlist[i].data.fd)) {
                    std::cout << "CGI read in fd:" << evlist[i].data.fd << std::endl;
                    Connection *associated_conn = this->get_associated_conn(evlist[i].data.fd);
                    this->read_cgi_socket(evlist[i].data.fd, associated_conn, evlist[i], evlist[associated_conn->fd]);
                } else {
                    std::cout << "NORMAL read in fd: " << evlist[i].data.fd << std::endl;
                    this->read_socket(evlist[i]);
                }
            } else if (evlist[i].events & EPOLLOUT) {
                // Ready for write
                std::cout << BLUE << "WRITING" << RESET << std::endl;

                if (HTTP::is_cgi_socket(evlist[i].data.fd)) {
                    std::cout << "CGI write in fd:" << evlist[i].data.fd << std::endl;
                    std::cout << "TODO handle write CGI" << std::endl;
                } else {
                    std::cout << "NORMAL write in fd: " << evlist[i].data.fd << std::endl;
                    this->write_socket(evlist[i]);
                }
            }
        }
    }

    // Free data from active connections
    connects_map::iterator ite;
    for (ite = this->_active_connects.begin(); ite != this->_active_connects.end(); ++ite) {
        delete (*ite).second;
    }

    return 0;
}

/**
 * Will perform a read operation in the client socket and write to request stringstream.
 *
 * @warning Call only ONE time after `epoll_wait`
 *
 * @note End of the Header will have "\\r\\n\\r\\n" CRLF
 * @eg:
 * header_line\r\n
 * header_line\r\n
 * \r\n
 * bodycontent here
 */
void HTTP::read_socket(struct epoll_event &ev) {
    int cfd = ev.data.fd;

    Request &request = this->_active_connects[cfd]->request;

    char buf[BUFFERSIZE];
    ft_memset(&buf, 0, BUFFERSIZE);

    int bytes_read = read(cfd, buf, BUFFERSIZE); // TODO maybe change to recv to handle SIGPIPE

    if (bytes_read <= 0 && request.getRaw().size() == 0) {
        print_error("failed to read socket");
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }

    request.append_buffer(buf, bytes_read);

    if (request.not_parsed()) {
        std::size_t end_header_pos = std::string(request.getRaw()).find("\r\n\r\n");

        if (end_header_pos != std::string::npos) {
            process_request(ev);
        }
    }
}

/**
 * Write to the client socket. First write will send the header, and then the body of the response.
 */
void HTTP::write_socket(struct epoll_event &ev) {
    int cfd = ev.data.fd;

    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;

    std::cout << "writing to socket: " << cfd << std::endl;
    std::cout << "active connects: " << this->_active_connects.size() << std::endl;
    std::cout << "request url is: " << request.getUrl() << std::endl;

    if (!response._sent_header) {
        this->send_header(cfd, ev, response);
        return;
    }

    if (response.isdir) {
        // TODO use the response buffer instead of dir_data
        if (send(cfd, response.dir_data.c_str(), response.get_content_length(), MSG_NOSIGNAL) == -1) {
            print_error("failed to send directory response");
        }
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }

    // TODO maybe improve this to use a boolean instead of inspecting the extension
    if (request.has_cgi()) {
        std::cout << "CGI OUTPUT TO CLIENT" << std::endl;
        char buff[BUFFERSIZE];
        ft_memset(&buff, 0, BUFFERSIZE);

        response._response_buffer.read(buff, BUFFERSIZE);
        int bytes_read = response._response_buffer.gcount();

        if (!bytes_read && request.cgi_complete) {
            this->close_connection(cfd, this->_epoll_fd, ev);
        } else {
            std::cout << "read sucessfully from inputfilestream" << std::endl;
            if (send(cfd, buff, bytes_read, MSG_NOSIGNAL) == -1) {
                print_error("failed to write in write_socket");
                this->close_connection(cfd, this->_epoll_fd, ev);
            } else {
                std::cout << BLUE << "wrote to socket " << cfd << " " << bytes_read << " bytes" << RESET << std::endl;
            }
        }

        return;
    }

    if (!response.inputfilestream) {
        std::cout << "No inputfilestream, will close this connection" << std::endl;
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }

    char buff[BUFFERSIZE];
    ft_memset(&buff, 0, BUFFERSIZE);

    response.inputfilestream.read(buff, BUFFERSIZE);
    int bytes_read = response.inputfilestream.gcount();

    if (bytes_read) {
        std::cout << "read sucessfully from inputfilestream" << std::endl;

        if (send(cfd, buff, bytes_read, MSG_NOSIGNAL) == -1) {
            print_error("failed to write in write_socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
        } else {
            std::cout << BLUE << "wrote to socket " << cfd << " " << bytes_read << " bytes" << RESET << std::endl;
        }

    } else {
        response.inputfilestream.close();
        this->close_connection(cfd, this->_epoll_fd, ev);
    }
}

void HTTP::process_request(struct epoll_event &ev) {
    int cfd = ev.data.fd;
    Connection *conn = this->_active_connects[cfd];

    // print_ascii(conn->request.getRaw().c_str());

    conn->request.parse_request_header(); // extract header info
    this->redirect_to_server(conn);

    if (conn->request.has_cgi()) {
        std::cout << GREEN << "cgi request" << RESET << std::endl;
        conn->request.process_cgi(conn, this->_epoll_fd);
        // TODO add here boolean to CGI
    } else {

        std::cout << GREEN << "normal request" << RESET << std::endl;

        if (conn->request.getMethod() == "GET") {
            conn->request.process_request(conn);
        } else {
            conn->response.set_status_code("404", conn->server);
        }

        if (epoll_mod(ev, EPOLLOUT) == -1) {
            print_error("failed to set write mode in incomming socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
        }
    }
}

/**
 * Will search the server blocks in config file and match the request header `Host:` with the
 * server block `server_name`. If not found, use the first server block with the same
 * ip and port as the default server.
 */
void HTTP::redirect_to_server(Connection *conn) {
    std::cout << "redirecting to server " << conn->host << ":" << conn->port << std::endl;
    std::cout << "host from header " << conn->request.getHost() << std::endl;

    Server *default_server = NULL;

    std::vector<Server>::iterator ite;
    for (ite = this->_servers.begin(); ite != this->_servers.end(); ite++) {
        if (ite->host == conn->host && ite->port == conn->port) {
            // if is the first match and default server is not defined, define it now
            if (!default_server) {
                default_server = &(*ite);
                // std::cout << "default server updated " << std::endl;
                // printVector(default_server->server_names);
            }

            std::vector<std::string>::iterator it;
            for (it = ite->server_names.begin(); it != ite->server_names.end(); it++) {
                if (*it == conn->request.getHost()) {
                    // found the correct server name (host from header)
                    conn->server = &(*ite);
                    return;
                }
            }
        }
    }

    // no server name (host from header) found, use the default server
    conn->server = default_server;

    if (!conn->server) {
        // TODO this should never happen but if somehow did, close the connection
        print_error("FAILED TO REDIRECT TO SERVER");
    } else {
        std::cout << "redirected sucessfuly to " << conn->server->host << ":" << conn->server->port << std::endl;
    }
}

/**
 * Assemble response Header and send it to the client.
 * @note If fail close connection and print error.
 */
int HTTP::send_header(int &cfd, struct epoll_event &ev, Response &response) {
    std::string header = response.assemble_header();
    // response.set_content_type("application/zip");

    std::cout << BLUE << "Will send this header" << RESET << std::endl;
    std::cout << BLUE << header << RESET << std::endl;

    if (send(cfd, header.c_str(), header.size(), MSG_NOSIGNAL) == -1) {
        print_error("failed to write in write_socket");
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 1;
    }

    response._sent_header = true;
    return 0;
}

// TODO BEFORE MOVING ON TO THE CGI, USE IFSTREAM IN FILES TO AVOID USING AN FD

/* -------------------------------------------------------------------------- */
/*                         REFACTOR BELLOW THIS POINT                         */
/* -------------------------------------------------------------------------- */

void HTTP::read_cgi_socket(int fd, Connection *conn, struct epoll_event &cgi_ev, struct epoll_event &conn_ev) {
    (void)conn_ev;
    char buffer[BUFFERSIZE];
    ft_memset(&buffer, 0, sizeof(buffer));

    std::size_t bytes_read = recv(fd, buffer, BUFFERSIZE, MSG_NOSIGNAL);
    if (bytes_read <= 0) {
        std::cout << RED << "ALERT" << RESET << std::endl;

        // will helps identify when the cgi ended
        conn->request.cgi_complete = true;

        // remove CGI socket from the EPOLL
        int ret = epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, &cgi_ev);
        if (ret == -1) {
            print_error(strerror(errno)); // TODO check this case
        }

        // remove from cgi_sockets map
        HTTP::cgi_sockets.erase(fd);

        close(fd);
        return;
    }

    if (conn->response.buffer_writes == 0) {
        /**
         * Update the interest in this FD to reads only. We dont want to get notified
         * for writes anymore because the CGI is already returning a response
         */
        epoll_mod(cgi_ev, EPOLLIN);

        // Update Client socket to EPOLLOUT
        epoll_event new_ev;
        ft_memset(&new_ev, 0, sizeof(new_ev));
        new_ev.data.fd = conn->fd;
        new_ev.events = EPOLLOUT;
        int ret = epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, conn->fd, &new_ev);
        if (ret == -1) {
            print_error(strerror(errno)); // TODO check this case
            std::cout << "failed to modify from EPOLL" << std::endl;
            close(conn->fd);
        }
    }

    conn->response.write_buffer(buffer, bytes_read);

    if (!conn->response._cgi_header_parsed && conn->response.has_header()) {
        /**
         * CGI Header example
         *
         * Status: 201 Created\r\n
         * Content-type: text/html; charset=UTF-8\r\n
         * \r\n
         */
        conn->response.parse_cgi_headers(conn->request._cgi_header, conn->server);
    }
}

void HTTP::add_cgi_socket(int sock, int connection_socket) { HTTP::cgi_sockets[sock] = connection_socket; }

Connection *HTTP::get_associated_conn(int sock) {
    std::map<int, int>::iterator it;
    for (it = HTTP::cgi_sockets.begin(); it != HTTP::cgi_sockets.end(); it++) {
        if (it->first == sock) {
            return (HTTP::_active_connects[it->second]);
        }
    }
    return NULL;
}

void HTTP::remove_cgi_socket(int sock) { HTTP::cgi_sockets.erase(sock); } // TODO remove end done cgi

/*

* input operation
- output operation

Request A |*****************--------------------------------------------------------------]
Request B |***-----]


Logger Ex

timestamp [INFO] - incomming connection from x, GET /
timestamp [INFO] - finish send response to x
timestamp [INFO] - closed connection for x

timestamp [ERROR] - failed to read


    printf("File Permissions: \t");
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n\n");


*/

/*

// int nbytes = response.bytes_in_buffer();

    // std::cout << "****buffer nb of bytes: " << nbytes << std::endl;
    // if (nbytes > 0) {
    //     char buf[BUFFERSIZE];
    //     ft_memset(&buf, 0, BUFFERSIZE);
    //     if (response.read_buffer(buf, BUFFERSIZE) == -1) {
    //         print_error("failed to read response buffer");
    //         std::cout << "buf has: " << std::string(buf) << std::endl;
    //         if (conn->cgi_pid && !request.cgi_complete) {
    //             request.cgi_complete = true;
    //         }
    //         this->close_connection(cfd, this->_epoll_fd, ev);
    //         return;
    //     }

    //     if (send(cfd, buf, nbytes, MSG_NOSIGNAL) == -1) {
    //         print_error("failed to write2");
    //         this->close_connection(cfd, this->_epoll_fd, ev);
    //     }

    //     if (nbytes <= BUFFERSIZE) {
    //         // TODO check if the buffer can be empty at some point

    //         if (conn->cgi_pid && request.cgi_complete) {

    //             int ret = epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, cfd, &ev);
    //             std::cout << "removed from EPOLL and redirect cgi fd to the incomming socket" << std::endl;
    //             if (ret == -1) {
    //                 std::cerr << "fd: " << cfd << std::endl;
    //                 print_error(strerror(errno));
    //                 std::cout << "ANOTHER ?????????" << std::endl;
    //             }

    //             dup2(request.cgi_socket, cfd);
    //             close(request.cgi_socket);

    //             int cgi_socket = request.cgi_socket;

    //             // erase from active connection
    //             delete this->_active_connects[cfd];
    //             this->_active_connects.erase(cfd);
    //             // erase active cgi
    //             HTTP::cgi_sockets.erase(cgi_socket);

    //             std::cout << "checking active connects: " << this->_active_connects.size() << std::endl;
    //             std::cout << "checking active cgis: " << HTTP::cgi_sockets.size() << std::endl;
    //         }
    //     }

    //     return;

    //     // TODO send the remainder of the buffer (in case if it read more than the header)
    //     // TODO socketpair cgi socket with connection socket
    // }

    // if (HTTP::is_cgi_socket(cfd)) {
    //     return;
    // }
*/