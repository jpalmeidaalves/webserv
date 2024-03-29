#include "../headers/HTTP.hpp"
#include "../headers/Connection.hpp"
#include "../headers/utils.hpp"

bool g_stop = false;

void sighandler(int signum) {
    (void)signum;
    std::cout << " Shutting down webserv..." << std::endl;

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
        std::cout << "Server listening on " << BOLDGREEN << convert_uint32_to_str(ntohl(it->sin_addr.s_addr)) << ":"
                  << ntohs(it->sin_port) << RESET << ", sockfd: " << curr_sockfd << std::endl;
        this->_listening_sockets.push_back(curr_sockfd);
    }

    return 0;
}

int HTTP::add_listening_socket_to_poll(struct epoll_event &ev, int sockfd) {

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
    std::cout << "New connection on socket: " << BOLDBLUE << sockfd << RESET
        << " from watched file descriptor: " << BOLDBLUE << accepted_fd << RESET << std::endl;

    ev.events = EPOLLIN | EPOLLRDHUP;
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
    this->_active_connects[accepted_fd]->cgi_fd = 0;
    this->_active_connects[accepted_fd]->last_operation = get_timestamp();
    this->_active_connects[accepted_fd]->ev_ptr = &ev;
    this->_active_connects[accepted_fd]->timedout = false;
    
    // extract the ip number and port from accepted socket, store in Connecetion struct
    get_port_host_from_sockfd(accepted_fd, this->_active_connects[accepted_fd]);
    return 0;
}

/**
 * Will search the server blocks in config file and match the request header `Host:` with the
 * server block `server_name`. If not found, use the first server block with the same
 * ip and port as the default server.
 */
void HTTP::redirect_to_server(Connection *conn) {

    Server *default_server = NULL;

    std::vector<Server>::iterator ite;
    for (ite = this->_servers.begin(); ite != this->_servers.end(); ite++) {
        if (ite->host == conn->host && ite->port == conn->port) {
            // if is the first match and default server is not defined, define it now
            if (!default_server) {
                default_server = &(*ite);
            }

            std::vector<std::string>::iterator it;
            for (it = ite->server_names.begin(); it != ite->server_names.end(); it++) {
                size_t pos = conn->request.getHost().find(":");
                std::string curr_host = conn->request.getHost().substr(0, pos);
                if (*it == curr_host) {
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
        print_error("FAILED TO REDIRECT TO SERVER");
    } 
}

/**
 * Assemble response Header and send it to the client.
 * @note If fail close connection and print error.
 */
int HTTP::send_header(int &cfd, struct epoll_event &ev, Response &response) {
    std::string header = response.assemble_header();

    if (send(cfd, header.c_str(), header.size(), MSG_NOSIGNAL) <= 0) {
        print_error("failed to write in write_socket");
        this->close_connection(cfd, this->_epoll_fd, ev);
        return 1;
    }

    response._sent_header = true;
    return 0;
}

void HTTP::close_connection(int cfd, int &epfd, epoll_event &ev) {

    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;
    Connection *conn = this->_active_connects[cfd];

    if (conn && response.inputfilestream.is_open())
        response.inputfilestream.close();

    if (conn && request.request_body.is_open())
        request.request_body.close();

    // check if the client socket has a CGI socket active
    if (conn && request.is_cgi) {

        if (conn->cgi_fd) {

            // Remove CGI socket from EPOLL
            epoll_event tmp;
            ft_memset(&tmp, 0, sizeof(tmp));
            tmp.data.fd = conn->cgi_fd;
            int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, conn->cgi_fd, &tmp);
            if (ret == -1) {
                std::cerr << "failed to remove fd " << conn->cgi_fd << " from EPOLL" << std::endl;
                print_error(strerror(errno));
            }

            kill(this->_active_connects[cfd]->cgi_pid, SIGKILL);

            close(conn->cgi_fd);
        }
    }

    if (conn && request.body_file_name != "")
        std::remove(request.body_file_name.c_str());

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
        std::cout << "Connection closed for FD " << RED << cfd << RESET << std::endl;
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

    return 0;
}

void HTTP::handle_timeouts() {
    connects_map::iterator it;

    // fds = [5,6] 
    std::vector<Connection *> timedout_fds;

    // 5,6 - client socket
    for (it = this->_active_connects.begin(); it != this->_active_connects.end(); it++) {
        if ((get_timestamp() - it->second->last_operation) > TIMEOUT) {

            if (it->second->ev_ptr && it->second->ev_ptr->events & EPOLLIN) {
                // assing the server server to handle the timeout if the connection has no server yet
                if (!it->second->server)
                    it->second->server = &this->_servers[0];

                it->second->response.set_status_code("408", it->second->server, it->second->request);
                epoll_mod(*(it->second->ev_ptr), EPOLLOUT);
                it->second->last_operation = it->second->last_operation + 2;
                
                it->second->timedout = true;

            } else {
                std::cout << "connection with fd " << it->second->fd << " has timed out!! closing..." << std::endl;
                timedout_fds.push_back(it->second);
            }
        }
    }

    std::vector<Connection *>::iterator fd_it;
    for (fd_it = timedout_fds.begin(); fd_it != timedout_fds.end(); fd_it++) {
        this->close_connection((*fd_it)->fd, this->_epoll_fd, *((*fd_it)->ev_ptr));
    }
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

    // waiting for events
    while (!g_stop) {
        // put the epoll instance waiting for events(requests) until a fd delivers an event
        int nfds = epoll_wait(this->_epoll_fd, evlist, MAXEPOLLSIZE, 2000);

        // if epoll_wait failed move to the next iteration instead of closing the program
        if (nfds == -1)
            continue;
        
        // Check for timeouts
        this->handle_timeouts();
        
        for (int i = 0; i < nfds; i++) {

            if (is_listening_socket(evlist[i].data.fd, this->_listening_sockets)) {

                this->accept_and_add_to_poll(ev, this->_epoll_fd, evlist[i].data.fd);

            } else if (evlist[i].events & EPOLLIN) {
                // Ready for read

                Connection *associated_conn = this->get_associated_conn(evlist[i].data.fd);

                if (associated_conn) {
                    this->read_cgi_socket(evlist[i].data.fd, associated_conn, evlist[i]);
                } else {
                    this->read_socket(evlist[i]);
                }
            } else if (evlist[i].events & EPOLLOUT) {
                // Ready for write
                this->write_socket(evlist[i]);
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

    Connection *conn = this->_active_connects[cfd];
    Request &request = this->_active_connects[cfd]->request;

    char buf[BUFFERSIZE + 1];
    ft_memset(&buf, 0, BUFFERSIZE);

    int bytes_read = recv(cfd, buf, BUFFERSIZE, MSG_NOSIGNAL);

    if (bytes_read <= 0) {
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }

    // update time since last operation
    conn->last_operation = get_timestamp();

    std::string tmp(buf);

    if (request.is_cgi) {    
        request.request_body.write(buf, bytes_read);

        request.request_body_writes += bytes_read;

        if (request.chunked && tmp == "\r\n") {
            return;
        }

        // When we wrote all bytes from the request to the request body file, process CGI
        if (request.request_body_writes >= request.get_content_length()) {
            request.process_cgi(conn, this->_epoll_fd);
        }

        return;
    }
  
    request.append_buffer(buf, bytes_read);

    if (request.chunked && (request.getRaw().find( "\r\n\r\n") != std::string::npos )) {
        request.chunked_complete = true;
    }

    if (request.chunked_complete) {
        if (conn->request.getMethod() == "GET") {
            conn->request.process_request(conn);
        } else {
            conn->response.set_status_code("405", conn->server, conn->request);
        }

        if (epoll_mod(ev, EPOLLOUT) == -1) {
            print_error("failed to set write mode in incomming socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
        }
    }

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

    // if the connection has been removed in this happens to be in the list of FDs ready to read stop here
    if (!this->_active_connects[cfd]) {
        return;
    }

    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;
    Connection *conn = this->_active_connects[cfd];

    if (!response._sent_header) {
        this->send_header(cfd, ev, response);
        // update time since last operation
        conn->last_operation = get_timestamp();
        return;
    }

    // REQUEST TO FOLDER
    if (response.isdir) {
        // Use the response buffer instead of dir_data
        if (send(cfd, response.dir_data.c_str(), response.get_content_length(), MSG_NOSIGNAL) <= 0) {
            print_error("failed to send directory response");
        }
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }

    // REQUEST TO CGI
    if (request.is_cgi && !conn->timedout && !(response.get_status_code()[0] == '4' || response.get_status_code()[0] == '5')) {
        int bytes_read = 0;

        char buff[BUFFERSIZE];
        ft_memset(&buff, 0, BUFFERSIZE);

        response._response_buffer.read(buff, BUFFERSIZE - 1);
        bytes_read = response._response_buffer.gcount();

        if (bytes_read) {

            if (send(cfd, buff, bytes_read, MSG_NOSIGNAL) <= 0) {
                print_error("failed to write in write_socket");
                this->close_connection(cfd, this->_epoll_fd, ev);
                return;
            }

            // update time since last operation
            conn->last_operation = get_timestamp();
        }

        if (request.cgi_complete && response._response_buffer.peek() == EOF) {
            this->close_connection(cfd, this->_epoll_fd, ev);
            close(cfd);
        }

        return;
    }

    // NORMAL REQUEST
    if (!response.inputfilestream) {
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }

    char buff[BUFFERSIZE + 1];
    ft_memset(&buff, 0, BUFFERSIZE);

    response.inputfilestream.read(buff, BUFFERSIZE);
    int bytes_read = response.inputfilestream.gcount();

    if (bytes_read) {

        if (send(cfd, buff, bytes_read, MSG_NOSIGNAL) <= 0) {
            print_error("failed to write in write_socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
            return;
        } 
        // update time since last operation
        conn->last_operation = get_timestamp();

    } else {
        this->close_connection(cfd, this->_epoll_fd, ev);
        return;
    }
}

void HTTP::process_request(struct epoll_event &ev) {
    int cfd = ev.data.fd;
    Connection *conn = this->_active_connects[cfd];

    conn->request.parse_request_header(); // extract header info
    this->redirect_to_server(conn);

    // print request type
    std::cout << conn->request.getMethod() << " " 
    << conn->request.getUrl() << " " << conn->request.getHost() << std::endl;


    if (conn->request.chunked && !conn->request.chunked_complete)
        return;

    /* ----------------------------------- DEFINE FULL PATH ----------------------------------- */

    // extract url query and fragments from full url 
    conn->request.process_url();

    // update url_path depending on the server root or location root
    conn->server->set_full_path(conn);

    // If the request has been redirected or Method is not allowed, change to EPOLLOUT
    if (conn->response.get_status_code().find("3") == 0 || conn->response.get_status_code() == "405") {
         if (epoll_mod(ev, EPOLLOUT) == -1) {
            print_error("failed to set write mode in incomming socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
        }
        return;
    }

    conn->server->server_index_page_exists(conn);

    if (conn->request.url_path.find("/") == 0)
        conn->request.url_path = "." + conn->request.url_path;

    // check if is a file or dir
    file_types curr_type = get_file_type(conn->request.url_path.c_str());

    if (curr_type == TYPE_UNKOWN) {

        conn->response.set_status_code("404", conn->server, conn->request);

        if (epoll_mod(ev, EPOLLOUT) == -1) {
            print_error("failed to set write mode in incomming socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
        }

        return;
    } else if (curr_type == TYPE_DIR) {
        conn->server->update_url_with_index_page(conn);
    }

    /* ----------------------------------- DEFINE FULL PATH END ----------------------------------- */

    // Check client max body size limits
    if (conn->request.get_content_length() > conn->server->client_max_body_size){
        conn->response.set_status_code("413", conn->server, conn->request);
        conn->request.is_cgi = false;

        if (epoll_mod(ev, EPOLLOUT) == -1) {
            this->close_connection(cfd, this->_epoll_fd, ev);
        }

        return;
    }

    if (conn->request.has_cgi(conn)) {
        if (conn->request.prepare_file_to_save_body(cfd, conn, this->_epoll_fd) == -1) {
            std::cout << "Missing directory /tmp. Must be present to work.";
            std::cout << "If the directory is missing, create it manually." << std::endl;
            conn->response.set_status_code("500", conn->server, conn->request);
            conn->request.is_cgi = false;

            if (epoll_mod(ev, EPOLLOUT) == -1) {
                print_error("failed to set write mode in incomming socket");
                this->close_connection(cfd, this->_epoll_fd, ev);
            }
        }
    } else {
        if (conn->request.getMethod() == "GET") {
            conn->request.process_request(conn);
        } else {
            conn->response.set_status_code("400", conn->server, conn->request);
        }

        if (epoll_mod(ev, EPOLLOUT) == -1) {
            print_error("failed to set write mode in incomming socket");
            this->close_connection(cfd, this->_epoll_fd, ev);
        }
    }
}

void HTTP::read_cgi_socket(int fd, Connection *conn, struct epoll_event &cgi_ev) {
    char buffer[BUFFERSIZE + 1];
    ft_memset(&buffer, 0, sizeof(buffer));

    int bytes_read = recv(fd, buffer, BUFFERSIZE, MSG_NOSIGNAL);

    if (bytes_read <= 0) {
        conn->request.cgi_complete = true;

        // Remove CGI fd from Epoll
        if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, &cgi_ev) == -1) {
            print_error(strerror(errno));
        }

        close(fd);
        std::remove(conn->request.body_file_name.c_str());
        conn->cgi_fd = 0;
    }

    conn->response.write_buffer(buffer, bytes_read);

    if (conn->request.cgi_complete) {
        // Only parse the header when the buffer has the "\r\n\r\n"
        conn->response.parse_cgi_headers(conn);

        // Must wait until the cgi header is parsed to update client socket to EPOLLOUT
        // Update Client socket to EPOLLOUT
        epoll_event new_ev;
        ft_memset(&new_ev, 0, sizeof(new_ev));
        new_ev.data.fd = conn->fd;
        new_ev.events = EPOLLOUT;
        int ret = epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, conn->fd, &new_ev);
        if (ret == -1) {
            print_error(strerror(errno));
            std::cout << "failed to modify from EPOLL" << std::endl;
            close(conn->fd);
            return ;
        }
    }
}

Connection *HTTP::get_associated_conn(int sock) {
    if (sock <= 0)
        return NULL;
        
    connects_map::iterator it;
    for (it = this->_active_connects.begin(); it != this->_active_connects.end(); it++) {
        if (it->second->cgi_fd == sock) {
            return (it->second);
        }
    }
    return NULL;
}