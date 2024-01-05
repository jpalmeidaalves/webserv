#include "../headers/HTTP.hpp"

bool g_stop = false;

void sighandler(int signum) {
    (void)signum;
    std::cout << "control-c triggered!!" << std::endl;
    g_stop = true;
}

HTTP::HTTP() : _epfd(0) {

    signal(SIGINT, sighandler);
    signal(SIGQUIT, sighandler);

    // Server *srv1 = new Server(8084); // TODO check if other args are required
    // if (srv1->create_server()) {
    //     delete srv1;
    //     throw HTTP::FailedToCreateServer();
    // }
    // this->_servers.push_back(srv1);

    // Server *srv2 = new Server(8085); // TODO check if other args are required
    // if (srv2->create_server()) {
    //     delete srv2;
    //     throw HTTP::FailedToCreateServer();
    // }
    // this->_servers.push_back(srv2);

    // // create new epoll instance

    // this->handle_connections();
}

HTTP::~HTTP() {
    // std::vector<int>::iterator it;
    // for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
    //     delete *it;
    // }
}

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

int HTTP::open_listening_sockets(std::vector<struct sockaddr_in> addresses) {
    std::vector<struct sockaddr_in>::iterator it;

    for (it = addresses.begin(); it != addresses.end(); ++it) {

        std::cout << "try to listening on " << BOLDGREEN
                  << convert_uint32_to_str(ntohl(it->sin_addr.s_addr)) << ":" << ntohs(it->sin_port)
                  << RESET << std::endl;

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

    std::cout << "adding listening socket " << sockfd << " to epoll" << std::endl;
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

    ret = epoll_ctl(this->_epfd, EPOLL_CTL_ADD, sockfd, &ev);
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

    // saved to active connections
    this->_active_connects[cfd] = new Connection();

    // get_port_host_from_sockfd(cfd);
    this->_active_connects[cfd]->host_port = get_port_host_from_sockfd(cfd);

    std::cout << "converted host_port: " << this->_active_connects[cfd]->host_port << std::endl;

    std::cout << "added success" << std::endl;
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

bool HTTP::is_listening_socket(int sockfd) {
    std::vector<int>::iterator it;

    for (it = this->_listening_sockets.begin(); it != this->_listening_sockets.end(); ++it) {
        if (*it == sockfd) {
            return true;
        }
    }
    return false;
}

int HTTP::read_socket(struct epoll_event &ev) {

    int cfd = ev.data.fd;
    char buf[BUFFERSIZE];
    int buflen;

    buflen = read(cfd, buf, BUFFERSIZE - 1);
    buf[buflen] = '\0';

    Request &request = this->_active_connects[cfd]->request;

    if (buflen == 0 && request.getRaw().size() == 0) {
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

    request.append_raw(buf);

    return 0;
}

void HTTP::list_directory(std::string full_path, struct epoll_event &ev) {
    std::map<std::string, struct dir_entry> dir_entries;
    // find items inside folder
    struct dirent *dp;
    bool has_error = false;

    int cfd = ev.data.fd;

    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;

    // remove trailing slash /
    if ((full_path.at(full_path.size() - 1)) == '/') {
        full_path.erase(full_path.end() - 1);
    }

    DIR *dir = opendir(full_path.c_str());

    if (dir == NULL) {
        print_error(strerror(errno));
        response.set_status_code("404");
        this->send_header(cfd, response);
        this->close_connection(cfd, this->_epfd, ev);
        return;
    }

    while (1) {
        dp = readdir(dir);
        if (dp == NULL)
            break;

        dir_entry new_entry;

        struct stat struc_st;

        // ignore current dir
        if (std::string(dp->d_name) == ".")
            continue;

        std::string item_path = full_path + "/" + dp->d_name;

        std::string href = item_path.substr(5);

        ft_memset(&struc_st, 0, sizeof(struc_st));
        if (stat(item_path.c_str(), &struc_st) == -1) {
            print_error("failed to get file information");

            has_error = true;
            break;
        }

        if (dp->d_type & DT_DIR) {
            new_entry.is_file = false;
        } else {
            new_entry.is_file = true;
        }

        new_entry.size = 0;
        new_entry.last_modified = get_formated_time(struc_st.st_mtim.tv_sec, "%d-%h-%Y %H:%M");
        new_entry.href = href;

        dir_entries[dp->d_name] = new_entry;
    }

    closedir(dir);

    if (has_error) {
        response.set_status_code("500");
        this->send_header(cfd, response);
    } else {
        std::stringstream ss;

        ss << "<html><head><title>Index of " << request.getUrl()
           << "/</title></head><body><h1>Index of " << request.getUrl() << "</h1><hr><pre>";

        std::map<std::string, struct dir_entry>::iterator it;
        {
            for (it = dir_entries.begin(); it != dir_entries.end(); it++) {
                if (it->second.is_file == false) {

                    std::string folder_name = it->first + "/";

                    ss << "<a href=\"" << it->second.href << "\">" << folder_name << "</a>"
                       << std::setw(51 - folder_name.size()) << " ";

                    // parent folder has no modified date and size
                    if (it->first == "..") {
                        ss << "\n";
                    } else {
                        ss << it->second.last_modified << " ";
                        ss << std::right << std::setw(20) << "-\n";
                    }
                }
            }
            for (it = dir_entries.begin(); it != dir_entries.end(); it++) {
                if (it->second.is_file == true) {

                    ss << "<a href=\"" << it->second.href << "\">" << it->first << "</a>"
                       << std::setw(51 - it->first.size()) << " ";

                    ss << it->second.last_modified << " ";
                    ss << std::right << std::setw(19) << it->second.size << "\n";
                }
            }
        }

        ss << "</pre><hr></body></html>";

        response.set_status_code("200");
        response.set_content_length(ss.str().size());
        this->send_header(cfd, response);

        // TODO maybe change all writes to send and read to recv
        // https://stackoverflow.com/questions/21687695/getting-sigpipe-with-non-blocking-sockets-is-this-normal
        if (send(cfd, ss.str().c_str(), ss.str().size(), MSG_NOSIGNAL) == -1) {
            print_error("failed to write");
        }
    }

    this->close_connection(cfd, this->_epfd, ev);
}

void HTTP::process_requested_file(struct epoll_event &ev) {
    int cfd = ev.data.fd;
    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;

    std::string root_folder = "./www";
    std::string full_path = root_folder + request.getUrl();

    if (get_stat_info(cfd, request, response)) {
        response.set_status_code("500");
        this->send_header(cfd, response);
        this->close_connection(cfd, this->_epfd, ev);
        return;
    }

    // TODO check permission, done for read
    if (!(response.permissions & S_IROTH)) {
        response.set_status_code("403");
        this->send_header(cfd, response);
        this->close_connection(cfd, this->_epfd, ev);
        return;
    }

    int file_fd = open(full_path.c_str(), O_RDONLY);
    if (!file_fd) {
        print_error("Error opening file");
        response.set_status_code("500");
        this->send_header(cfd, response);
        this->close_connection(cfd, this->_epfd, ev);
        return;
    }

    response.set_status_code("200");
    response.set_content_type(MimeTypes::identify(full_path));
    request.set_req_file_fd(file_fd);
    // std::cout << "requested file fd changed: " << request.get_requested_fd() << std::endl;
    this->send_header(cfd, response);
    // body info will go in another subsequent write
}

int HTTP::send_subsequent_write(struct epoll_event &ev) {
    int cfd = ev.data.fd;
    Request &request = this->_active_connects[cfd]->request;
    // Response &response = this->_active_connects[cfd]->response;

    int file_fd = request.get_requested_fd();

    int bytes_read = 0;
    char buff[BUFFERSIZE];

    // std::cout << "subsequent write.........." << std::endl;

    bytes_read = read(file_fd, buff, BUFFERSIZE);
    // std::cout << "read " << bytes_read << " bytes" << std::endl;
    if (bytes_read == -1) {
        print_error("Failed read file");
        close(file_fd);
        this->close_connection(cfd, this->_epfd, ev);
        return 1;
    }

    if (bytes_read == 0) {
        close(file_fd);
        this->close_connection(cfd, this->_epfd, ev);
        return 1;
    }

    if (write(cfd, buff, bytes_read) == -1) {
        print_error("failed to write");
        this->close_connection(cfd, this->_epfd, ev);
        return 1;
    }
    return 0;
}

int HTTP::write_socket(struct epoll_event &ev) {

    int cfd = ev.data.fd;
    Request &request = this->_active_connects[cfd]->request;
    Response &response = this->_active_connects[cfd]->response;

    // check if has the complete header
    if (!request.getIsComplete()) {
        if (request.getRaw().find("\r\n") != std::string::npos)
            request.setIsComplete();
    } else {

        // Flow of the function after incomming msg is complete

        // first time to write
        if (!request.get_requested_fd()) {
            request.parse_request(); // extract header info

            std::string root_folder = "./www";
            std::string full_path = root_folder + request.getUrl();

            // std::cout << "[Request Header]" << request.getRaw() << std::endl;

            // check if is a file or dir
            int isfile = is_file(full_path.c_str());
            if (isfile == -1) {
                // TODO error checking
                print_error("failed to check if is a dir");
                response.set_status_code("500");
                this->close_connection(cfd, this->_epfd, ev);
            } else if (isfile == 1) {
                std::cout << "------- file --------" << std::endl;
                this->process_requested_file(ev);
                // send file (must check permissions)
            } else {
                std::cout << "------- dir --------" << std::endl;

                // if index file is present
                // TODO must check all index files defined in the configfile
                if (file_exists(full_path + "/" + "index.html")) {
                    // send file (must check permissions)
                    request.setUrl(request.getUrl() + "/" + "index.html"); // update url
                    this->process_requested_file(ev);
                } else {
                    // send list dir (must check permissions)

                    // TODO if dir listing is active, from config file
                    bool is_dir_listing = true;

                    // TODO we must check the index files in the configfile and show them instead of
                    // listing dir

                    if (!is_dir_listing) {
                        response.set_status_code("403");
                        this->send_header(cfd, response);
                        this->close_connection(cfd, this->_epfd, ev);
                    } else {
                        this->list_directory(full_path, ev);
                    }
                }
            }

        } else {
            // subsequent writes
            this->send_subsequent_write(ev);
        }
    }

    return 0;
}

int HTTP::handle_connections() {

    this->_epfd = epoll_create(MAXEPOLLSIZE);
    if (this->_epfd == -1)
        return 1;

    struct epoll_event ev;
    /* buffer pointed to by events is used to return information from  the  ready
    list  about  file  descriptors in the interest list that have some events available. */
    struct epoll_event evlist[MAXEPOLLSIZE];

    // add each server to the epoll
    std::vector<int>::iterator it;
    for (it = this->_listening_sockets.begin(); it != this->_listening_sockets.end(); ++it) {
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

        for (int i = 0; i < nfds; i++) {
            if (this->is_listening_socket(evlist[i].data.fd)) {
                this->accept_and_add_to_poll(ev, this->_epfd, evlist[i].data.fd);
            } else {
                if (evlist[i].events & EPOLLIN) {
                    // Ready for read
                    if (this->read_socket(evlist[i]))
                        break; // TODO check this
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

const char *HTTP::FailedToInit::what() const throw() { return ("Failed to initialize fd"); }

const char *HTTP::FailedToCreateServer::what() const throw() { return ("Failed to Create Server"); }

int HTTP::send_header(int &cfd, const Response &response) {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << response.get_status_code() << "\n"
       << "Content-Type: " << response.get_content_type() << "\n"
       << "Content-Length: " << response.get_content_length() << "\n"
       << "Access-Control-Allow-Origin: *\n"
       << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\n"
       << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\n"
       << "Access-Control-Allow-Credentials: true\n"
       << "\n";

    if (write(cfd, ss.str().c_str(), ss.str().size()) == -1)
        return 1;
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