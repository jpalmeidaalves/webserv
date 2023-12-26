#include "../headers/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                         Constructors & Desctructor                         */
/* -------------------------------------------------------------------------- */

/*  Constructor: Initialize the members of the struct sockaddr_in and its adress_len
    a atribute. Also intalls a signal handler  */
Server::Server(int port): _sockfd(0), _port(port) {


    // Define address struct
    // this->_address.sin_family = AF_INET;
    // this->_address.sin_port = htons(port);       // host to network short
    // this->_address.sin_addr.s_addr = INADDR_ANY; // TODO change this
    // // used in bind and accept
    // this->_address_len = sizeof(this->_address);

    

    ft_memset(&(this->_hints), 0, sizeof(this->_hints));

    this->_hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    this->_hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    this->_hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    this->_hints.ai_protocol = 0;          /* Any protocol */
    this->_hints.ai_canonname = NULL;
    this->_hints.ai_addr = NULL;
    this->_hints.ai_next = NULL;

}

Server::~Server() {
    if (this->_sockfd)
        close(this->_sockfd);

    if (this->result)
        freeaddrinfo(this->result);  
}

/* -------------------------------------------------------------------------- */
/*                              Member Functions                              */
/* -------------------------------------------------------------------------- */

int Server::create_server() {
    int                      s;
    struct addrinfo          *rp;

    std::stringstream ss;
    ss << this->_port;

    s = getaddrinfo(NULL, ss.str().c_str(), &this->_hints, &this->result);
    if (s != 0) {
        std::cerr << "getaddrinfo: "<< gai_strerror(s);
        return 1;
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

     int optval = 1;

     int n = 0;

    for (rp = this->result; rp != NULL; rp = rp->ai_next) {
        std::cout << "hints: " << n << std::endl;
        n++;
        this->_sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (this->_sockfd == -1)
            continue;

        if ((setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR,
                        &optval, sizeof (optval))) != 0)
            continue;

        if (bind(this->_sockfd, rp->ai_addr, rp->ai_addrlen) == 0) 
            break;                  /* Success */
        else
            print_error(strerror(errno));

        close(this->_sockfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        std::cerr << "Could not bind" << std::endl;
        return 1;
    }





    /*  int socket(int domain, int type, int protocol);                          
    domain -> protocol family - AF_INET = ip
    type -> specifies the communication semantics - SOCK_STREAM = sequenced, 
            reliable, two-way, connection-based
    protocol -> a particular protocol to be used - 0 = a single protocol exists */
    // this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (this->_sockfd < 0) {
    //     print_error("Failed to create socket");
    //     return 1;
    // }

    /*  int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        Assigns the address specified by addr to the socket referred to by the file
        descriptor sockfd(“assigning a name to a socket”).
        sockfd -> file descriptor number generated by socket()
        const struct sockaddr *addr -> depend on the address family for ip connection
                                       struct sockaddr_in is used(defined in ip(7))
        addrlen -> size, in bytes, of the address structure pointed to by addr    */
    // if (bind(this->_sockfd, (struct sockaddr *)&this->_address, this->_address_len) < 0) {
    //     print_error("Failed to bind socket to the address");
    //     return 1;
    // }
    

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
              << "http://127.0.0.1:" << this->_port << RESET << std::endl;

    return 0;
}

int Server::get_sockfd() const
{
    return this->_sockfd;
}
