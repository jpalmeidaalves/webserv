#include "../headers/HTTP.hpp"
#include "../headers/Connection.hpp"
#include "../headers/utils.hpp"

bool g_stop = false;

void sighandler(int signum) {
    (void)signum;
    std::cout << "control-c triggered!!" << std::endl;
    g_stop = true;
}

HTTP::HTTP() {}

HTTP::HTTP(std::vector<Server> &servers) : _epoll_fd(0), _servers(servers) {
    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);
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
        std::cout << "Server listening on " << BOLDGREEN
                  << convert_uint32_to_str(ntohl(it->sin_addr.s_addr)) << ":" << ntohs(it->sin_port)
                  << RESET << ", sockfd: " << curr_sockfd << std::endl;

        this->_listening_sockets.push_back(curr_sockfd);
    }

    return 0;
}

int HTTP::add_listening_socket_to_poll(struct epoll_event &ev, int sockfd) {

    // std::cout << "adding listening socket " << sockfd << " to epoll" << std::endl;
    int ret;             // store the stat us of the epoll instance accross the program
    ev.events = EPOLLIN; // monitors file descriptors ready to read
    ev.data.fd = sockfd; // the fd we are listening on the network

    /* epoll ctl -> control interface for an epoll file descriptor
                    add, modify, or remove entries in the interest list
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
        int epfd -> instance createde by epoll_create()
        int op -> EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
        int fd -> socket file descriptor
    */

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

    // accept connection
    /* extracts the first connection request on the queue of pending connections for
        the listening socket, sockfd, creates a new connected socket, and returns a
        new file descriptor referring to that socket. */
    int accepted_fd = accept(sockfd, (struct sockaddr *)&cur_sockin, &socklen);
    if (accepted_fd == -1) {
        print_error("failed to accept connection");
        return 1;
    }

    std::cout << "accepted connection in listing socket: " << sockfd
              << " for incoming socket: " << accepted_fd << std::endl;

    // add to epoll modifiyng the flags to allow read and write operations
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
    // extract the ip number and port from accepted socket, store in Connecetion struct
    get_port_host_from_sockfd(accepted_fd, this->_active_connects[accepted_fd]);

    return 0;
}

int HTTP::handle_connections() {

    this->_epoll_fd = epoll_create(MAXEPOLLSIZE);
    if (this->_epoll_fd == -1)
        return 1;

    struct epoll_event ev;
    /* buffer pointed to by events is used to return information from  the  ready
    list, about file descriptors in the interest list that have some events available. */
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
            } else {
                if (evlist[i].events & EPOLLIN) {
                    // Ready for read
                    this->read_socket(evlist[i]);
                } else if (evlist[i].events & EPOLLOUT) {
                    // Ready for write
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

// when a readable event is detected on a socket
int HTTP::read_socket(struct epoll_event &ev) {

    int cfd = ev.data.fd;
    char buf[BUFFERSIZE];
    int buflen;

    buflen = read(cfd, buf, BUFFERSIZE - 1);
    buf[buflen] = '\0';

    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;

    if (buflen == 0 && request.getRaw().size() == 0) {
        print_error("---- read 0 bytes ----");
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 1;
    }
    if (buflen == -1) {
        print_error("failed to read socket");
        if (this->close_connection(cfd, this->_epoll_fd, ev)) {

            return 1;
        }
    }

    request.append_raw(buf);

    if (std::string(buf).find("\r\n") != std::string::npos) {
        request.parse_request(); // extract header info

        if (request.getMethod() == "GET") {
            std::cout << "processing GET request" << std::endl;
            if (request.process_request(this->_epoll_fd, ev, this->_active_connects[cfd]) == -1) {
                this->send_header(cfd, response);
                this->close_connection(cfd, this->_epoll_fd, ev);
            }
        } else if (request.getMethod() == "POST") {
            // TODO post => getbody()
            std::cout << "processing POST request" << std::endl;
        } else if (request.getMethod() == "DELETE") {
            // TODO delete
            std::cout << "processing DELETE request" << std::endl;
        }
    }

    return 0;
}

// when a writable event is detected on a socket
int HTTP::write_socket(struct epoll_event &ev) {

    int cfd = ev.data.fd;
    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;

    if (!response._sent_header) {
        this->send_header(cfd, response);
    } 

    if (response.isdir) {
        if (send(cfd, response.dir_data.c_str(), response.get_content_length(), MSG_NOSIGNAL) == -1) {
            print_error("failed to write");
        }
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 1;
    }

    int file_fd = request.get_requested_fd();
    if (!file_fd) {
        this->close_connection(cfd, this->_epoll_fd, ev);
    }

    int bytes_read = 0;
    char buff[BUFFERSIZE];

    bytes_read = read(file_fd, buff, BUFFERSIZE);
    if (bytes_read == -1) {
        print_error("Failed read file");
        close(file_fd);
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 1;
    }

    if (bytes_read == 0) {
        close(file_fd);
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 0;
    }

    if (write(cfd, buff, bytes_read) == -1) {
        print_error("failed to write");
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 1;
    }

    return 0;
}

int HTTP::send_header(int &cfd, Response &response) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << response.get_status_code() << "\n"
       << "Content-Type: " << response.get_content_type() << "\n"
       << "Content-Length: " << response.get_content_length() << "\n"
       << "Access-Control-Allow-Origin: *\n"
       << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\n"
       << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\n"
       << "Access-Control-Allow-Credentials: true\n"
       << "\n";

    response._sent_header = true;

    if (write(cfd, ss.str().c_str(), ss.str().size()) == -1)
        return 1;
    return 0;
}

int HTTP::close_connection(int cfd, int &epfd, epoll_event &ev) {
    int ret;

    // free and delete from active connections
    delete this->_active_connects[cfd];
    this->_active_connects.erase(cfd);

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
