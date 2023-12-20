#include "../headers/Server.hpp"

bool g_stop = false;

void sighandler(int signum) {
    (void)signum;
    g_stop = true;
}

/* -------------------------------------------------------------------------- */
/*                         Constructers & Desctructer                         */
/* -------------------------------------------------------------------------- */

Server::Server() {} // disabled default constructer

Server::~Server() {
    if (this->_sockfd)
        close(this->_sockfd);
}

Server::Server(const std::string &configfile, int port) {
    (void)configfile;

    signal(SIGINT, sighandler);

    // Define address struct
    this->_address.sin_family = AF_INET;
    this->_address.sin_port = htons(port);       // host to network short
    this->_address.sin_addr.s_addr = INADDR_ANY; // TODO change this
    // used in bind and accept
    this->_address_len = sizeof(this->_address);
}

/* -------------------------------------------------------------------------- */
/*                              Member Functions                              */
/* -------------------------------------------------------------------------- */

int Server::create_server() {

    // Create socket
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd < 0) {
        print_error("Failed to create socket");
        return 1;
    }

    // Bind adress and port
    if (bind(this->_sockfd, (struct sockaddr *)&this->_address, this->_address_len) < 0) {
        print_error("Failed to bind socket to the address");
        return 1;
    }

    // Start listen for incomming requests
    if (listen(this->_sockfd, SOMAXCONN) < 0) {
        print_error("Failed to listen");
        return 1;
    }

    // show message
    std::cout << "Server listening on " << BOLDGREEN
              << "http://127.0.0.1:" << ntohs(this->_address.sin_port) << RESET << std::endl;

    return 0;
}

int Server::setup_epoll(struct epoll_event &ev, int &ret, int &epfd) {

    // create epoll
    epfd = epoll_create(MAXEPOLLSIZE);
    if (0 > epfd) {
        std::cout << 1 << std::endl;
    }

    // handle epoll ctl
    ev.events = EPOLLIN;

    ev.data.fd = this->_sockfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, this->_sockfd, &ev);
    if (0 > ret) {
        std::cout << 3 << std::endl;
    }
    return 0;
}

int Server::send_response(int &cfd) {

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

int Server::accept_and_add_to_poll(struct epoll_event &ev, int &ret, int &epfd) {

    struct sockaddr_in cur_sockin;
    socklen_t socklen = sizeof(struct sockaddr);

    // accept connection
    int cfd = accept(this->_sockfd, (struct sockaddr *)&cur_sockin, &socklen);
    if (cfd == -1) {
        print_error("failed to accept connection");
        return 1;
    }

    std::cout << "accepted connection for fd " << cfd << std::endl;

    // add to epoll
    ev.events = EPOLLIN;
    ev.data.fd = cfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        print_error("failed epoll_ctl");
        return 1;
    }
    return 0;
}

int Server::read_complete(int &cfd, int &epfd, epoll_event &ev, int &ret) {
    // close
    close(cfd);
    if (cfd == -1) {
        print_error("failed to close fd");
        return 1;
    }

    std::cout << "closed connection for fd " << cfd << std::endl;

    // Removes cfd from the EPOLL
    epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);
    if (ret == -1) {
        print_error("failed to delele from epoll_ctl");
        return 1;
    }
    return 0;
}

int Server::still_reading(char *buf, int buflen) {

    buf[buflen] = '\0';
    std::string msgPrefix = "some-prefix-to-prevent-arbitrary-connection";
    std::string msg = buf;

    std::cout << "inc msg: " << std::string(buf) << std::endl;

    if (msgPrefix.length() > msg.length())
        return 1;

    std::cout << "test" << std::endl;
    if (!strcmp(msgPrefix.c_str(), msg.substr(0, msgPrefix.length()).c_str())) {
        msg = msg.substr(msgPrefix.length(), msg.length());
        std::cout << "XXXXX" << msg << std::endl;

        // write(cfd, buf, buflen);  // write back to client if
        // needed
    } // else, reject

    return 0;
}

int Server::monitor_multiple_fds() {

    int ret;
    int epfd; // epoll fd
    struct epoll_event ev;
    struct epoll_event evlist[MAXEPOLLSIZE];

    if (this->setup_epoll(ev, ret, epfd)) {
        return 1;
    }

    int i;
    char buf[BUFSIZ]; // TODO is ok using this buff?
    int buflen;

    while (!g_stop) {
        // epoll
        int nfds = epoll_wait(epfd, evlist, MAXEPOLLSIZE, -1);
        if (ret == -1) {
            print_error("epoll_wait failed");
            break;
        }

        for (i = 0; i < nfds; i++) {
            if (evlist[i].data.fd == this->_sockfd) {
                if (this->accept_and_add_to_poll(ev, ret, epfd))
                    break;
            } else {

                int cfd = evlist[i].data.fd;

                // read
                buflen = read(cfd, buf, BUFSIZ - 1);

                // received EOF
                if (buflen == 0) {
                    if (this->read_complete(cfd, epfd, evlist[i], ret))
                        break;
                } else {
                    if (still_reading(buf, buflen))
                        continue;
                }

                // means it read the hole socket and we must responde
                this->send_response(cfd);

                close(cfd);
            }
        }
    }

    return 0;
}

void Server::start_listen() {
    // -------------------
    // struct epoll_event ev;
    //
    // int kdpfd = epoll_create(MAXEPOLLSIZE);
    // ev.events = EPOLLIN | EPOLLET;
    // ev.data.fd = this->_sockfd;
    // if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, this->_sockfd, &ev) < 0) {
    //     std::cerr << strerror(errno) << " epoll set insert error." << std::endl;
    //     return;
    // } else {
    //     std::cout << "success insert listening socket into epoll.\n";
    // }
    // struct epoll_event events[MAXEPOLLSIZE];
    // int curfds = 1;
    // while (1) { // loop for accept incoming connection

    //     int nfds = epoll_wait(kdpfd, events, curfds, -1);
    //     if (nfds == -1) {
    //         std::cerr << strerror(errno) << " epoll_wait" << std::endl;
    //         break;
    //     }
    //     for (int n = 0; n < nfds; ++n) {
    //         struct sockaddr_storage client_addr;
    //         if (events[n].data.fd == this->_sockfd) {
    //             socklen_t addr_size = sizeof(client_addr);
    //             int new_fd = accept(events[n].data.fd, (struct sockaddr *)&client_addr,
    //             &addr_size); if (new_fd == -1) {
    //                 if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
    //                     break;
    //                 } else {
    //                     std::cerr << strerror(errno) << " accept" << std::endl;
    //                     break;
    //                 }
    //             }
    //             std::cout << "server: connection established...\n";
    //             // set_non_blocking(new_fd);
    //             ev.events = EPOLLIN | EPOLLET;
    //             ev.data.fd = new_fd;
    //             if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, new_fd, &ev) < 0) {
    //                 std::cerr << strerror(errno) << " Failed to insert socket into epoll"
    //                           << std::endl;
    //             }
    //             curfds++;
    //         } else {
    //             if (send(events[n].data.fd, "Hello, world!", 13, 0) == -1) {
    //                 std::cerr << strerror(errno) << " send" << std::endl;
    //                 break;
    //             }
    //             epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
    //             curfds--;
    //             close(events[n].data.fd);
    //         }
    //     }
    // }

    // -------------------

    // while (g_stop == 0) {
    //     std::cout << "  > Waiting for new connection\n";

    //     int in_sockfd = this->accept_connection();
    //     if (in_sockfd < 0)
    //         continue; // if connection refused continue to next request

    //     char buffer[30720] = {0};
    //     int bytesReceived = read(in_sockfd, buffer, 30720);
    //     if (bytesReceived < 0) {
    //         print_error("Failed to read bytes from client socket
    //         connection"); continue;
    //     }

    //     std::cout << "  < Received Request from client \n" << std::endl;

    //     std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> "
    //                            "HOME </h1><p> Hello from "
    //                            "your Server :) </p></body></html>";
    //     std::ostringstream ss;
    //     ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: "
    //        << htmlFile.size() << "\n\n"
    //        << htmlFile;

    //     write(in_sockfd, ss.str().c_str(), ss.str().size());

    //     close(in_sockfd);
    // }
}

int Server::accept_connection() {
    int in_sockfd =
        accept(this->_sockfd, (sockaddr *)&this->_address, (socklen_t *)&this->_address_len);
    if (in_sockfd < 0)
        print_error("Server failed to accept incoming connection");

    return in_sockfd;
}
