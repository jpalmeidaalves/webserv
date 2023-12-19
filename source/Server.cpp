#include "../headers/Server.hpp"

int g_stop = 0;

void sighandler(int signum) {
    (void)signum;
    g_stop = 1;
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
    this->_address.sin_addr.s_addr = INADDR_ANY; // TODO maybe change this
    // used in bind and accept
    this->_address_len = sizeof(this->_address);

    this->create_server();
}

/* -------------------------------------------------------------------------- */
/*                              Member Functions                              */
/* -------------------------------------------------------------------------- */

void Server::create_server() {

    // Create socket
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd < 0)
        throw Server::SocketException();

    // Bind adress and port
    if (bind(this->_sockfd, (struct sockaddr *)&this->_address,
             this->_address_len) < 0)
        throw Server::BindException();
}

void Server::start_listen() {
    // Start listen for incomming requests
    if (listen(this->_sockfd, SOMAXCONN) < 0)
        throw Server::ListenException();

    std::cout << "Server listening on " << BOLDGREEN
              << "http://127.0.0.1:" << ntohs(this->_address.sin_port) << RESET
              << std::endl;

    // ---------------------------------------

    int MAX_EVENTS = 10;
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;

    /* Code to set up listening socket, 'listen_sock',
       (socket(), bind(), listen()) omitted. */

    epollfd = epoll_create(SOMAXCONN);
    if (epollfd == -1) {
        // perror("epoll_create1");
        return;
    }

    listen_sock = 0;

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        // perror("epoll_ctl: listen_sock");
        return;
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            // perror("epoll_wait");
            break;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                conn_sock =
                    accept(listen_sock, (struct sockaddr *)&this->_address,
                           (socklen_t *)&this->_address_len);
                if (conn_sock == -1) {
                    // perror("accept");
                    break;
                }
                // setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    // perror("epoll_ctl: conn_sock");
                    break;
                }
            } else {
                std::cout << "do use fd " << events[n].data.fd << std::endl;
            }
        }
    }

    // ---------------------------------------

    while (g_stop == 0) {
        std::cout << "  > Waiting for new connection\n";

        int in_sockfd = this->accept_connection();
        if (in_sockfd < 0)
            continue; // if connection refused continue to next request

        char buffer[30720] = {0};
        int bytesReceived = read(in_sockfd, buffer, 30720);
        if (bytesReceived < 0) {
            print_error("Failed to read bytes from client socket connection");
            continue;
        }

        std::cout << "  < Received Request from client \n" << std::endl;

        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> "
                               "HOME </h1><p> Hello from "
                               "your Server :) </p></body></html>";
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: "
           << htmlFile.size() << "\n\n"
           << htmlFile;

        write(in_sockfd, ss.str().c_str(), ss.str().size());

        close(in_sockfd);
    }
}

int Server::accept_connection() {
    int in_sockfd = accept(this->_sockfd, (sockaddr *)&this->_address,
                           (socklen_t *)&this->_address_len);
    if (in_sockfd < 0)
        print_error("Server failed to accept incoming connection");

    return in_sockfd;
}

/* -------------------------------------------------------------------------- */
/*                                 Exceptions                                 */
/* -------------------------------------------------------------------------- */

const char *Server::SocketException::what() const throw() {
    return ("Failed to create socket");
}

const char *Server::BindException::what() const throw() {
    return ("Failed to bind socket to the address");
}

const char *Server::ListenException::what() const throw() {
    return ("Failed to listen");
}