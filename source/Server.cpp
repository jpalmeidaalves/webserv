#include "../headers/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                         Constructors & Desctructor                         */
/* -------------------------------------------------------------------------- */

/*  Constructor: Initialize the members of the struct sockaddr_in and its adress_len
    a atribute. Also intalls a signal handler  */
Server::Server(int port): _sockfd(0) {

    ft_memset(&(this->_address), 0, sizeof(this->_address));

    // Define address struct
    this->_address.sin_family = AF_INET;
    this->_address.sin_port = htons(port);       // host to network short
    this->_address.sin_addr.s_addr = INADDR_ANY; // TODO change this
    // used in bind and accept
    this->_address_len = sizeof(this->_address);
}

Server::~Server() {
    if (this->_sockfd)
        close(this->_sockfd);
}

/* -------------------------------------------------------------------------- */
/*                              Member Functions                              */
/* -------------------------------------------------------------------------- */

int Server::create_server() {

    /*  int socket(int domain, int type, int protocol);                          
    domain -> protocol family - AF_INET = ip
    type -> specifies the communication semantics - SOCK_STREAM = sequenced, 
            reliable, two-way, connection-based
    protocol -> a particular protocol to be used - 0 = a single protocol exists */
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd < 0) {
        print_error("Failed to create socket");
        return 1;
    }

    /*  int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        Assigns the address specified by addr to the socket referred to by the file
        descriptor sockfd(“assigning a name to a socket”).
        sockfd -> file descriptor number generated by socket()
        const struct sockaddr *addr -> depend on the address family for ip connection
                                       struct sockaddr_in is used(defined in ip(7))
        addrlen -> size, in bytes, of the address structure pointed to by addr    */
    if (bind(this->_sockfd, (struct sockaddr *)&this->_address, this->_address_len) < 0) {
        print_error("Failed to bind socket to the address");
        return 1;
    }

    /*  int listen(int sockfd, int backlog);
        mark sockfd as a passive socket -> will be used to accept incoming connection
                                           requests using accept(2)
        int backlog -> maximum length to which the queue of pending  connections
    */
    if (listen(this->_sockfd, SOMAXCONN) < 0) {
        print_error("Failed to listen");
        return 1;
    }

    // show message
    std::cout << "Server listening on " << BOLDGREEN
              << "http://127.0.0.1:" << ntohs(this->_address.sin_port) << RESET << std::endl;

    return 0;
}

int Server::get_sockfd() const
{
    return this->_sockfd;
}
