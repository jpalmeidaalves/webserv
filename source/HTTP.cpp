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

    this->monitor_multiple_fds();
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

int HTTP::add_listening_socket_to_poll(struct epoll_event &ev, int listening_socket) {
    int ret;                       // store the status of the epoll instance accross the program
    ev.events = EPOLLIN;           // monitors file descriptors ready to read
    ev.data.fd = listening_socket; // the fd we are listening on the network

    /* epoll ctl -> control interface for an epoll file descriptor
                    add, modify, or remove entries in the interest list
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
        int epfd -> instance createde by epoll_create()
        int op -> EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
        int fd -> socket file descriptor
    */

    ret = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, listening_socket, &ev);
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
    ev.data.fd = cfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        close(cfd);
        print_error("failed epoll_ctl");
        return 1;
    }

    this->_inc_msgs[cfd] = "";
    std::cout << "added success" << std::endl;
    return 0;
}

int HTTP::close_connection(int &cfd, int &epfd, epoll_event &ev) {
    int ret;

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
    int cfd = ev.data.fd;
    char buf[BUFSIZ];
    int buflen;

    buflen = read(cfd, buf, BUFSIZ - 1);
    buf[buflen] = '\0';

    if (buflen == 0 && this->_inc_msgs[cfd].size() == 0) {
        print_error("---- read 0 bytes ----");
        this->close_connection(cfd, this->_epfd, ev);
        this->_inc_msgs.erase(cfd);
        return 1;
    }

    if (buflen == -1) {
        print_error("failed to read socket");
        if (this->close_connection(cfd, this->_epfd, ev)) {
            this->_inc_msgs.erase(cfd);
            return 1;
        }
    }

    // std::cout << "read OK: " << buf << std::endl;

    // char *ptr = buf;
    // while (*ptr) {
    //     std::cout << (int)*ptr << std::endl;
    //     ptr++;
    // }

    this->_inc_msgs[cfd] += buf;

    return 0;
}
char buf[BUFSIZ];
int buflen;

int HTTP::monitor_multiple_fds() {
    struct epoll_event ev;
    /* buffer pointed to by events is used to return information from  the  ready
    list  about  file  descriptors in the interest list that have some events available. */
    struct epoll_event evlist[MAXEPOLLSIZE];

    // add each server to the epoll
    std::vector<Server *>::iterator it;
    for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
        if (this->add_listening_socket_to_poll(ev, (*it)->get_sockfd()))
            return 1;
    }

    while (!g_stop) {
        // epoll
        int nfds = epoll_wait(this->_epfd, evlist, MAXEPOLLSIZE, -1);
        if (nfds == -1) {
            print_error("epoll_wait failed");
            continue;
        }

        for (int i = 0; i < nfds; i++) {

            if (this->is_listening_socket(evlist[i].data.fd)) {
                this->accept_and_add_to_poll(ev, this->_epfd, evlist[i].data.fd);
            } else {

                // accepted socket
                if (evlist[i].events & EPOLLIN) {
                    // Ready for read

                    std::cout << " inside EPOLLIN" << std::endl;

                    if (this->read_socket(evlist[i]))
                        break;

                } else if (evlist[i].events & EPOLLOUT) {
                    // Ready for write
                    int cfd = evlist[i].data.fd;

                    // TODO remove DEBUG
                    if (this->_inc_msgs[cfd].find("\r\n") != std::string::npos) {
                        std::cout << "Message from Socket\n" << this->_inc_msgs[cfd] << std::endl;

                        Request request(this->_inc_msgs[cfd]);

                        std::cout << "[Request object]: \n" << request << std::endl;

                        std::cout << "MimeType: " << MimeTypes::indentify(request.getUrl())
                                  << std::endl;

                        std::string root_folder = "./www";

                        Response response;

                        if (opendir(root_folder.c_str()) == NULL) {
                            print_error(strerror(errno));
                            // response.set_status_code(401);
                        }

                        std::string full_path = root_folder + request.getUrl();

                        std::cout << "full_path: " << full_path << std::endl;
                        int fd = open(full_path.c_str(), O_RDONLY);
                        if (fd == -1) {
                            print_error(strerror(errno));
                        }

                        // TODO check permission

                        // TODO check for invalid read

                        std::string response_data;
                        char buf[BUFSIZ];
                        int buflen = 0;

                        while (1) {
                            buflen = read(fd, buf, BUFSIZ - 1);
                            if (buflen <= 0)
                                break;
                            buf[buflen] = '\0';

                            response_data += buf;
                        }

                        std::cout << "response data: \n" << response_data << std::endl;

                        this->send_response(cfd);

                        this->close_connection(cfd, this->_epfd, evlist[i]);
                        this->_inc_msgs.erase(cfd);
                    }
                }
            }
        }
    }

    return 0;
}

const char *HTTP::FailedToInit::what() const throw() { return ("Failed to initialize fd"); }

const char *HTTP::FailedToCreateServer::what() const throw() { return ("Failed to Create Server"); }

int HTTP::send_response(int &cfd) {

    std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> "
                           "HOME </h1><p> Hello from "
                           "your Server :) </p></body></html>";
    std::ostringstream ss;
    ss << "HTTP/1.1 200 OK\nContent-Type: "
          "text/html\nContent-Length: "
       << htmlFile.size() << "\n\n"
       << htmlFile;

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

/*

Request

[header...]\r\n[body...]\r\n

*/
