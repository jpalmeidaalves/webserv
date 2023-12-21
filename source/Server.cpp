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
        print_error("failed to create epoll");
        return 1;
    }

    // handle epoll ctl
    ev.events = EPOLLIN;        // monitors file descriptors ready to read
    ev.data.fd = this->_sockfd; // the fd we are listening on the network

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, this->_sockfd, &ev);
    if (0 > ret) {
        print_error("failed add to epoll");
        return 1;
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

    // fcntl(cfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

    std::cout << "accepted connection for fd " << cfd << std::endl;

    // add to epoll
    ev.events = EPOLLIN | EPOLLONESHOT;
    ev.data.fd = cfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        print_error("failed epoll_ctl");
        return 1;
    }

    this->_inc_msgs[cfd] = "";

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
        if (ret == -1 || nfds == -1) {
            print_error("epoll_wait failed");
            break;
        }

        for (i = 0; i < nfds; i++) {
            if (evlist[i].data.fd == this->_sockfd) {
                if (this->accept_and_add_to_poll(ev, ret, epfd))
                    break;
            } else {

                int cfd = evlist[i].data.fd;

                while (1) {
                    // read
                    std::cout << "reading ..." << std::endl;
                    buflen = read(cfd, buf, BUFSIZ - 1);
                    std::cout << buflen << std::endl;
                    buf[buflen] = '\0';

                    if (buflen == -1) {
                        print_error("failed to read socket");
                        if (this->read_complete(cfd, epfd, evlist[i], ret))
                            break;
                    }

                    if (buflen < (BUFSIZ - 1)) {
                        // has read everything
                        std::cout << "last read" << std::endl;
                        this->_inc_msgs[cfd] += buf;
                        std::cout << "Message from Socket\n" << this->_inc_msgs[cfd] << std::endl;
                        this->send_response(cfd);

                        this->read_complete(cfd, epfd, evlist[i], ret);
                        this->_inc_msgs.erase(cfd);
                        break;

                    } else {
                        // append request to list of requests
                        this->_inc_msgs[cfd] += buf;
                    }
                }
            }
        }
    }

    return 0;
}

// Event loop
// epoll_event events[MAX_EVENTS];
// while (true) {
//     int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
//     if (numEvents == -1) {
//         perror("Error waiting for events");
//         break;
//     }

//     for (int i = 0; i < numEvents; ++i) {
//         // Handle incoming connections
//         if (events[i].data.fd == serverSocket) {
//             // ... (same as before)
//         } else {
//             // Handle data from connected clients
//             int clientSocket = events[i].data.fd;

//             auto it = std::find_if(clients.begin(), clients.end(), [clientSocket](const Client
//             &c) {
//                 return c.socket == clientSocket;
//             });

//             if (it != clients.end()) {
//                 Client &client = *it;
//                 ssize_t bytesRead =
//                     recv(clientSocket, client.buffer.data(), client.buffer.size(), 0);

//                 if (bytesRead == 0) {
//                     // Connection closed by the client
//                     std::cout << "Connection closed by client" << std::endl;
//                     epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
//                     close(clientSocket);
//                     clients.erase(it);
//                 } else if (bytesRead == -1) {
//                     perror("Error reading from client");
//                     epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
//                     close(clientSocket);
//                     clients.erase(it);
//                 } else {
//                     // Process the received data (you may implement your HTTP request handling
//                     logic
//                     // here)
//                     std::cout << "Received data: " << std::string(client.buffer.data(),
//                     bytesRead)
//                               << std::endl;

//                     // Echo the data back to the client
//                     send(clientSocket, client.buffer.data(), bytesRead, 0);
//                 }
//             }
//         }
//     }
// }

// Cleanup
// close(serverSocket);
// close(epollFd);

// return 0;
// }

// fcntl(cfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

// if (errno == EWOULDBLOCK)
//     std::cout << "would block" << std::endl;

// read
// buflen = read(cfd, buf, BUFSIZ - 1);
// // received EOF
// if (buflen == 0) {
//     if (this->read_complete(cfd, epfd, evlist[i], ret))
//         break;
// } else {
//     if (still_reading(buf, buflen))
//         continue;
// }

// int Server::still_reading(char *buf, int buflen) {
//     buf[buflen] = '\0';
//     std::string msgPrefix = "some-prefix-to-prevent-arbitrary-connection";
//     std::string msg = buf;
//     std::cout << "inc msg: " << std::string(buf) << std::endl;
//     if (msgPrefix.length() > msg.length())
//         return 1;
//     std::cout << "test" << std::endl;
//     if (!strcmp(msgPrefix.c_str(), msg.substr(0, msgPrefix.length()).c_str())) {
//         msg = msg.substr(msgPrefix.length(), msg.length());
//         std::cout << "XXXXX" << msg << std::endl;
//         // write(cfd, buf, buflen);  // write back to client if
//         // needed
//     } // else, reject
//     return 0;
// }

/* ------------------------------------ / ----------------------------------- */
// int cfd = evlist[i].data.fd;

//                 // read
//                 std::cout << "reading ..." << std::endl;
//                 buflen = read(cfd, buf, BUFSIZ - 1);
//                 std::cout << buflen << std::endl;

//                 if (buflen == -1) {
//                     print_error("failed to read socket");
//                     close(cfd);
//                     break;
//                 }

//                 if (buflen < (BUFSIZ - 1)) {
//                     // Has all the data from the request
//                     std::cout << "last read" << std::endl;
//                     std::cout << "Message from Socket\n" << this->_inc_msgs[cfd] << std::endl;
//                     this->send_response(cfd);

//                     if (this->read_complete(cfd, epfd, evlist[i], ret))
//                         break;
//                     this->_inc_msgs.erase(cfd);

//                     close(cfd);
//                     break;
//                 }

//                 buf[buflen] = '\0';
//                 this->_inc_msgs[cfd] += buf;
//                 std::cout << this->_inc_msgs[cfd] << std::endl;