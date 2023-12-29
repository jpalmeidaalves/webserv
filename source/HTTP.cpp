#include "../headers/HTTP.hpp"

bool g_stop = false;

void sighandler(int signum) {
    (void)signum;
    g_stop = true;
}

HTTP::HTTP() : _epfd(0) {

    signal(SIGINT, sighandler);

    Server *srv1 = new Server(8084); // TODO check if other args are required
    if (srv1->create_server()) {
        delete srv1;
        throw HTTP::FailedToCreateServer();
    }
    this->_servers.push_back(srv1);

    Server *srv2 = new Server(8085); // TODO check if other args are required
    if (srv2->create_server()) {
        delete srv2;
        throw HTTP::FailedToCreateServer();
    }
    this->_servers.push_back(srv2);

    // create new epoll instance
    this->_epfd = epoll_create(MAXEPOLLSIZE);
    if (0 > this->_epfd)
        throw HTTP::FailedToInit();

    this->handle_connections();
}

HTTP::~HTTP() {
    std::vector<Server *>::iterator it;
    for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
        delete *it;
    }
}

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

int HTTP::add_listening_socket_to_poll(struct epoll_event &ev, Server *server) {

    if (!server) {
        return 1;
    }

    int ret;             // store the status of the epoll instance accross the program
    ev.events = EPOLLIN; // monitors file descriptors ready to read
    // ev.data.fd = listening_socket; // the fd we are listening on the network
    server->connection = new Connection();
    ev.data.ptr = server->connection;
    static_cast<Connection *>(ev.data.ptr)->fd = server->get_sockfd();

    /* epoll ctl -> control interface for an epoll file descriptor
                    add, modify, or remove entries in the interest list
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
        int epfd -> instance createde by epoll_create()
        int op -> EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
        int fd -> socket file descriptor
    */

    ret = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, server->get_sockfd(), &ev);
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

    // accept connection
    /* extracts the first connection request on the queue of pending connections for
        the listening socket, sockfd, creates a new connected socket, and returns a
        new file descriptor referring to that socket. */
    int cfd = accept(sockfd, (struct sockaddr *)&cur_sockin, &socklen);
    if (cfd == -1) {
        print_error("failed to accept connection");
        return 1;
    }

    std::cout << "accepted connection in listing socket: " << sockfd
              << " for incoming socket: " << cfd << std::endl;

    // add to epoll
    ev.events = EPOLLIN | EPOLLOUT;
    // ev.data.fd = cfd;
    ev.data.ptr = new Connection();
    static_cast<Connection *>(ev.data.ptr)->fd = cfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        close(cfd);
        print_error("failed epoll_ctl");
        return 1;
    }

    // this->_inc_connects[cfd] = ""; // TODO check if necessary
    std::cout << "added success" << std::endl;
    return 0;
}

int HTTP::close_connection(int &cfd, int &epfd, epoll_event &ev) {
    int ret;

    Connection *conn_ptr = static_cast<Connection *>(ev.data.ptr);

    delete conn_ptr;

    // Removes cfd from the EPOLL
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);
    if (ret == -1) {
        std::cerr << "fd: " << cfd << std::endl;
        print_error(strerror(errno));
        close(cfd);
        return 1;
    }

    std::cout << "closed connection for fd(socket) " << cfd << std::endl;

    // close fd
    if (close(cfd) == -1) {
        print_error("failed to close fd");
        return 1;
    }

    return 0;
}

bool HTTP::is_listening_socket(int sockfd) {
    std::vector<Server *>::iterator it;

    for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
        if ((*it)->get_sockfd() == sockfd) {
            return true;
        }
    }
    return false;
}

int HTTP::read_socket(struct epoll_event &ev) {

    Connection *conn_ptr = static_cast<Connection *>(ev.data.ptr);

    int cfd = conn_ptr->fd;
    char buf[BUFFERSIZE];
    int buflen;

    buflen = read(cfd, buf, BUFFERSIZE - 1);
    buf[buflen] = '\0';

    if (buflen == 0 && conn_ptr->request.getRaw().size() == 0) {
        print_error("---- read 0 bytes ----");
        this->close_connection(cfd, this->_epfd, ev);
        return 1;
    }

    if (buflen == -1) {
        print_error("failed to read socket");
        if (this->close_connection(cfd, this->_epfd, ev)) {

            return 1;
        }
    }

    conn_ptr->request.append_raw(buf);

    return 0;
}

int HTTP::handle_connections() {
    struct epoll_event ev;
    /* buffer pointed to by events is used to return information from  the  ready
    list  about  file  descriptors in the interest list that have some events available. */
    struct epoll_event evlist[MAXEPOLLSIZE];

    // add each server to the epoll
    std::vector<Server *>::iterator it;
    for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
        if (this->add_listening_socket_to_poll(ev, *it))
            return 1;
    }

    while (!g_stop) {
        // epoll
        int nfds = epoll_wait(this->_epfd, evlist, MAXEPOLLSIZE, -1);
        if (nfds == -1) {
            print_error("epoll_wait failed");
            continue;
        }

        std::cout << "epoll_wait completed" << std::endl;

        for (int i = 0; i < nfds; i++) {

            Connection *conn_ptr = static_cast<Connection *>(evlist[i].data.ptr);

            std::cout << "epoll cfd: " << conn_ptr->fd << std::endl;

            // TODO add flag to connection indicating listening socket
            if (this->is_listening_socket(conn_ptr->fd)) {
                std::cout << "is listening socket" << std::endl;
                this->accept_and_add_to_poll(ev, this->_epfd, conn_ptr->fd);
            } else {
                std::cout << "not listening socket" << std::endl;

                if (evlist[i].events & EPOLLIN) {
                    // Ready for read
                    std::cout << " inside EPOLLIN" << std::endl;

                    if (this->read_socket(evlist[i]))
                        break;
                } else if (evlist[i].events & EPOLLOUT) {
                    // Ready for write

                    // accepted socket
                    int cfd = conn_ptr->fd; // TODO maybe remove this

                    conn_ptr->request.parse_request();

                    // TODO remove DEBUG
                    if (conn_ptr->request.getRaw().find("\r\n") != std::string::npos) {

                        std::cout << "[Request object]: \n"
                                  << conn_ptr->request.getRaw() << std::endl;

                        std::cout << "MimeType: " << MimeTypes::identify(conn_ptr->request.getUrl())
                                  << std::endl;

                        std::string root_folder = "./www";

                        DIR *dir = opendir(root_folder.c_str());

                        if (dir == NULL) {
                            print_error(strerror(errno));
                            // response.set_status_code(401);
                        }

                        if (closedir(dir) == -1) {
                            print_error(strerror(errno));
                        }

                        std::string full_path = root_folder + conn_ptr->request.getUrl();

                        std::cout << "full_path: " << full_path << std::endl;

                        conn_ptr->response.set_status_code("200");
                        conn_ptr->response.set_content_type(
                            MimeTypes::identify(conn_ptr->request.getUrl()));

                        struct stat struc_st;
                        if (stat(full_path.c_str(), &struc_st) == -1) {
                            print_error("failed to get file information");
                            // TODO early response
                        }

                        conn_ptr->response.set_content_length(struc_st.st_size);

                        int file_fd = open(full_path.c_str(), O_RDONLY);
                        if (!file_fd) {
                            print_error("Error opening file");
                            conn_ptr->response.set_status_code("500");
                            this->send_header(cfd, conn_ptr->response);
                        }

                        // TODO check permission
                        // TODO check for invalid read

                        int bytes_read = 0;
                        char buff[BUFFERSIZE];

                        // send header first
                        this->send_header(cfd, conn_ptr->response);

                        // TODO send body
                        std::cout << "reading file.........." << std::endl;
                        while (1) {
                            bytes_read = read(file_fd, buff, BUFFERSIZE);
                            // buff[bytes_read] = '\0';
                            std::cout << "read " << bytes_read << " bytes" << std::endl;
                            if (bytes_read == -1) {
                                print_error("Failed read file");
                                break;
                            }
                            if (bytes_read == 0) {
                                close(file_fd);
                                break;
                            }

                            if (write(cfd, buff, bytes_read) == -1) {
                                print_error("failed to write");
                                break;
                            }
                        }

                        // std::cout << "end reading file.........." << std::endl;
                        // std::cout << ">>>>> SIZES <<<<<<" << std::endl;
                        // std::cout << "bytes read: " << response.get_content_length() <<
                        // std::endl; std::cout << "from stat: " << struc_st.st_size << std::endl;

                        this->close_connection(cfd, this->_epfd, evlist[i]);
                        // this->_inc_connects.erase(cfd);
                    }
                }
            }
            // AQUI
        }
    }

    // TODO detele connection structs for listening sockets

    std::vector<Server *>::iterator ite;
    for (ite = this->_servers.begin(); ite != this->_servers.end(); ++ite) {
        delete (*ite)->connection;
    }

    return 0;
}

const char *HTTP::FailedToInit::what() const throw() { return ("Failed to initialize fd"); }

const char *HTTP::FailedToCreateServer::what() const throw() { return ("Failed to Create Server"); }

int HTTP::send_header(int &cfd, const Response &response) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << response.get_status_code() << "\n"
       << "Content-Type: " << response.get_content_type() << "\n"
       << "Content-Length: " << response.get_content_length() << "\n"
       << "Access-Control-Allow-Origin: *"
       << "\n"
       << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS"
       << "\n"
       << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With"
       << "\n"
       << "Access-Control-Allow-Credentials: true"
       << "\n\n";

    if (write(cfd, ss.str().c_str(), ss.str().size()) == -1)
        return 1;
    return 0;
}

/*

* input operation
- output operation

Request A |*****************--------------------------------------------------------------]
Request B |***-----]

*/