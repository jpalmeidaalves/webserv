#include "../headers/HTTP.hpp"


bool g_stop = false;

void sighandler(int signum) {
    (void)signum;
    g_stop = true;
}


HTTP::HTTP(): _epfd(0) {

     signal(SIGINT, sighandler);

    Server *srv1 = new Server(8080); //TODO check if other args are required
    if(srv1->create_server())
        throw HTTP::FailedToCreateServer();
    this->_servers.push_back(srv1);

    Server *srv2 = new Server(8081); //TODO check if other args are required
    if(srv2->create_server())
        throw HTTP::FailedToCreateServer();
    this->_servers.push_back(srv2);

     // create new epoll instance
    this->_epfd = epoll_create(MAXEPOLLSIZE);
    if (0 > this->_epfd)
        throw HTTP::FailedToInit();

    this->monitor_multiple_fds();

}

HTTP::~HTTP() {

}


/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

int HTTP::add_to_poll(struct epoll_event &ev, int &ret, int listening_socket) {
    ev.events = EPOLLIN;            // monitors file descriptors ready to read
    ev.data.fd = listening_socket;  // the fd we are listening on the network
    
    /* epoll ctl -> control interface for an epoll file descriptor
                    add, modify, or remove entries in the interest list
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
        int epfd -> instance createde by epoll_create()
        int op -> EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
        int fd -> socket file descriptor
    */

    ret = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, listening_socket, &ev);
    if (0 > ret) {
        print_error("failed add to epoll");
        return 1;
    }
    return 0;
}






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

int HTTP::accept_and_add_to_poll(struct epoll_event &ev, int &ret, int &epfd, int sockfd) {

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

    // fcntl(cfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

    std::cout << "accepted connection for fd " << cfd << std::endl;

    // add to epoll
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = cfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        print_error("failed epoll_ctl");
        return 1;
    }

    this->_inc_msgs[cfd] = "";
    std::cout << "added success" << std::endl;
    return 0;
}

int HTTP::end_connection(int &cfd, int &epfd, epoll_event &ev, int &ret) {
    // close
    close(cfd);
    if (cfd == -1) {
        print_error("failed to close fd");
        return 1;
    }

    std::cout << "closed connection for fd(socket) " << cfd << std::endl;

    // Removes cfd from the EPOLL
    epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);
    if (ret == -1) {
        print_error("failed to delele from epoll_ctl");
        return 1;
    }
    return 0;
}

bool HTTP::is_listening_socket(int sockfd) {
    std::vector<Server *>::iterator it;
    
    for (it = this->_servers.begin() ; it != this->_servers.end(); ++it) {
        if ((*it)->get_sockfd() == sockfd) {
            return true;
        }
    }
    
    return false;
}

int HTTP::monitor_multiple_fds() {

    int ret; //store the status of the epoll instance accross the program
    struct epoll_event ev;
     /* buffer pointed to by events is used to return information from  the  ready
     list  about  file  descriptors in the interest list that have some events available. */
    struct epoll_event evlist[MAXEPOLLSIZE];
    char buf[BUFSIZ]; // TODO is ok using this buff?


    // add each server to the epoll
    std::vector<Server *>::iterator it;
    
    for (it = this->_servers.begin() ; it != this->_servers.end(); ++it)
        this->add_to_poll(ev, ret, (*it)->get_sockfd());
        
   
    while (!g_stop) {
        // epoll
        int nfds = epoll_wait(this->_epfd, evlist, MAXEPOLLSIZE, -1);
        if (ret == -1 || nfds == -1) {
            print_error("epoll_wait failed");
            break;
        }

        for (int i = 0; i < nfds; i++) {

            // listning socket
            if (this->is_listening_socket(evlist[i].data.fd)) {
            // std::cout << "listening socket " << this->_sockfd << std::endl;
                if (this->accept_and_add_to_poll(ev, ret, this->_epfd, evlist[i].data.fd))
                    break;
                continue;
            }

            if (evlist[i].events & EPOLLIN) {
                // Ready for read

                // accepted socket
                int cfd = evlist[i].data.fd;

                int buflen;
                std::cout << "reading socket " << cfd << std::endl; // TODO remove DEBUG
                buflen = read(cfd, buf, BUFSIZ - 1);
                buf[buflen] = '\0';
                std::cout << buflen << std::endl;// TODO remove DEBUG

                if (buflen == 0 && this->_inc_msgs[cfd].size() == 0) {
                    this->end_connection(cfd, this->_epfd, evlist[i], ret);
                    this->_inc_msgs.erase(cfd);
                    break;
                }

                if (buflen == -1) {
                    print_error("failed to read socket");
                    if (this->end_connection(cfd, this->_epfd, evlist[i], ret)) {
                        this->_inc_msgs.erase(cfd);
                        break;
                    }
                }

                this->_inc_msgs[cfd] += buf;

                continue;
            }

             if (evlist[i].events & EPOLLOUT) { 
                // Ready for write
                // std::cout << "inside EPOLLOUT" << std::endl;

                int cfd = evlist[i].data.fd;

                if (has_suffix(this->_inc_msgs[cfd], "\r\n\r\n")) {
                    std::cout << "has the complete request" << std::endl;
                    std::cout << "Message from Socket\n" << this->_inc_msgs[cfd] << std::endl; // TODO remove DEBUG
                    this->send_response(cfd);
                    this->end_connection(cfd, this->_epfd, evlist[i], ret);
                    this->_inc_msgs.erase(cfd);
                }
             }
        }
    }

    return 0;
}



const char *HTTP::FailedToInit::what() const throw() {
    return ("Failed to initialize fd");
}

const char *HTTP::FailedToCreateServer::what() const throw() {
    return ("Failed to Create Server");
} 


/*

* input operation
- output operation

Request A |*****************--------------------------------------------------------------]
Request B |***-----]

*/
