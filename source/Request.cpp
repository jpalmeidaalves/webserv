#include "../headers/Request.hpp"
#include "../headers/Connection.hpp"
#include "../headers/HTTP.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Response.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

Request::Request() : _content_length(0), request_body_writes(0), is_cgi(false), cgi_complete(false), read_complete(false), is_dir(false) {}

Request::~Request() {}

/* -------------------------------- Disabled -------------------------------- */
Request &Request::operator=(const Request &rhs) {
    (void)rhs;
    return *this;
}
Request::Request(const Request &src) { *this = src; }

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

void Request::parse_request_header() {
    std::cout << RED << "parsed request" << RESET << std::endl;
    std::cout << CYAN << this->_buffer.str() << RESET << std::endl;
    // parse request
    // std::stringstream ss(this->_buffer.str());
    std::string line;

    // process first line
    getline(this->_buffer, this->_method, ' ');
    getline(this->_buffer, this->_url, ' ');
    getline(this->_buffer, line); // ignore the rest of the line

    // std::cout << "---------- parsing header -----------" << std::endl;

    while (getline(this->_buffer, line)) {

        // has reach the end of the header
        if (line == "\r")
            break;

        // std::cout << line << std::endl;
        // print_ascii(line.c_str());

        if (line.find("Host:") == 0) {
            remove_char_from_string(line, '\r');
            this->_host = line.substr(line.find(" ") + 1);
        } else if (line.find("Content-Length:") == 0) {
            remove_char_from_string(line, '\r');
            this->_content_length = ft_stoi(line.substr(line.find(" ") + 1));
        } else if (line.find("Content-Type: ") == 0) {
            remove_char_from_string(line, '\r');
            this->_content_type = line.substr(line.find(" ") + 1);
        }
    }
    // std::cout << "---------- end parsing header -----------" << std::endl;
}

std::string Request::getMethod() const { return (this->_method); }
std::string Request::getUrl() const { return (this->_url); }
std::string Request::getBody() const { return ""; }
std::string Request::getHost() const { return (this->_host); }
std::string Request::getRaw() const { return (this->_buffer.str()); }

/**
 * Checks if the request has been parsed.
 *
 * @note If the request has a method defined (GET, POST, ...) means that it's parsed.
 */
bool Request::not_parsed() {
    if (this->_method == "")
        return true;
    return false;
}

std::string Request::get_content_type() const { return (this->_content_type); }
std::size_t Request::get_content_length() const { return (this->_content_length); }

void Request::set_content_type(const std::string type) { this->_content_type = type; }
void Request::set_content_length(std::size_t length) { this->_content_length = length; }

void Request::append_buffer(const char *buf, int len) {
    this->_buffer.write(buf, len);
    std::cout << "appended data in the request buffer" << std::endl;
}

std::ostream &operator<<(std::ostream &out, const Request &obj) {
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Host => " << obj.getHost() << std::endl;
    out << "url => " << obj.getUrl() << std::endl;
    out << "body => " << obj.getBody() << std::endl;
    return out;
}

void Request::setUrl(std::string url) { this->_url = url; }

void Request::process_url() {

    // /upload.php?name=nuno&other=stuff
    // /upload.html?name=.php&other=stuff
    // /upload.html#section

    std::string base_url;


    std::string url = this->getUrl();
    std::size_t has_query = url.find("?");
    

    if (has_query != std::string::npos) {
        base_url = this->getUrl().substr(0, has_query);
        this->url_query = this->getUrl().substr(has_query + 1); // TODO may segfault "index.html?"
    } else {
        base_url = this->getUrl();
    }

    std::size_t has_fragment = base_url.find("#");
    if (has_fragment != std::string::npos) {
        base_url = base_url.substr(0, has_fragment);
        this->url_fragment = this->getUrl().substr(has_fragment + 1); // TODO may segfault "index.html#"
    } else {
        base_url = base_url;
    }

    this->url_path = base_url;

}

bool Request::has_cgi() {

    // HAS CGI
    // /upload.php
    // /upload.php?name=nuno&other=stuff

    // DOES NOT HAVE CGI
    // /upload.html?name=.php&other=stuff
    // /upload.html

    // std::string url = this->getUrl();
    // std::size_t has_query = url.find("?");

    // if (has_query != std::string::npos) {
    //     this->url_path = this->getUrl().substr(0, has_query);
    //     // std::cout << RED << url_path << RESET << std::endl;
    //     this->url_query = this->getUrl().substr(has_query + 1);
    // } else {
    //     this->url_path = this->getUrl();
    // }

    std::cout << "checking if has CGI: url_path = " << this->url_path << std::endl;

    if (has_suffix(this->url_path, ".php")) {
        this->is_cgi = true;
        return true;
    }
    
    return false;
}

void Request::process_requested_file(Connection *conn, std::string full_path) {
    Response &response = conn->response;

    if (get_stat_info(full_path, response)) {
        response.set_status_code("500", conn->server);
        return;
    }

    response.inputfilestream.open(full_path.c_str(), std::ifstream::binary);
    if (!response.inputfilestream) {
        std::cout << "Error opening file" << std::endl;
        response.set_status_code("403", conn->server);
        return;
    }

    std::string file_type = MimeTypes::identify(full_path);
    // std::cout << RED << "file mimetype: " << file_type << RESET << std::endl;
    response.set_content_type(file_type);

    // response.set_req_file_fd(file_fd);
}

void Request::process_request(Connection *conn, int epfd) {
    (void)epfd;
    // int cfd = ev.data.fd;
    Request &request = conn->request;
    Response &response = conn->response;

    std::string full_path = conn->server->root + request.url_path;

    // std::cout << "[Request Header]" << request.getRaw() << std::endl;
    // std::cout << GREEN << "processing request full_path: " << full_path << RESET << std::endl;

    // check if is a file or dir
    file_types curr_type = get_file_type(full_path.c_str());

    if (curr_type == TYPE_UNKOWN) {
        print_error("failed to check if is a dir");
        response.set_status_code("404", conn->server);
    } else if (curr_type == TYPE_FILE) {
        std::cout << "------- file --------" << std::endl;
        this->process_requested_file(conn, full_path);
    } else if (request.is_dir) {
        std::cout << "------- dir --------" << std::endl;

        bool is_dir_listing = conn->server->server_dir_listing(conn);

        if (!is_dir_listing) {
            response.set_status_code("403", conn->server);
        } else {
            response.isdir = true;
            this->list_directory(full_path, conn);
        }
    }
}

int Request::list_directory(std::string full_path, Connection *conn) {
    std::map<std::string, struct dir_entry> dir_entries;
    // find items inside folder
    struct dirent *dp;
    bool has_error = false;

    Request &request = conn->request;
    Response &response = conn->response;

    // remove trailing slash /
    if ((full_path.at(full_path.size() - 1)) == '/') {
        full_path.erase(full_path.end() - 1);
    }

    DIR *dir = opendir(full_path.c_str());

    if (dir == NULL) {
        print_error(strerror(errno));
        response.set_status_code("500", conn->server);
        return -1;
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

        std::cout << "full_path: " << full_path << std::endl;
        std::cout << "dp->d_name: " << dp->d_name << std::endl;
        std::cout << "item_path: " << item_path << std::endl;

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
        response.set_status_code("500", conn->server);
        return -1;
    }

    std::stringstream ss;

    ss << "<html><head><title>Index of " << request.url_path << "/</title></head><body><h1>Index of "
       << request.url_path << "</h1><hr><pre>";

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

    response.set_status_code("200", conn->server);
    response.set_content_length(ss.str().size());

    response.dir_data = ss.str();

    return 0;

    // TODO maybe change all writes to send and read to recv
    // https://stackoverflow.com/questions/21687695/getting-sigpipe-with-non-blocking-sockets-is-this-normal
    // if (send(cfd, ss.str().c_str(), ss.str().size(), MSG_NOSIGNAL) == -1) {
    //     print_error("failed to write");
    // }

    // this->close_connection(cfd, this->_epoll_fd, ev);
}


int Request::prepare_file_to_save_body(int fd, Connection *conn, int epfd) {
    if (this->getMethod() == "GET"){
        process_cgi(conn, epfd);
        std::cout << "ENTROU AQUI!" << std::endl;
        return 0;
    }
    this->body_file_name = "./etc/cgi/tmp/.tmp-req-body-" + ft_itos(fd);
    std::cout << "will use this filename: " << this->body_file_name.c_str() << std::endl;
    this->request_body.open(this->body_file_name.c_str());

    if (!this->request_body.is_open()) {
        std::cout << "this" << std::endl;
        return -1;
    }

    // If the request buffer has read some part of the body of the request
    // write those bytes belonging to this file

    char buf[BUFFERSIZE + 1];
    std::memset(buf, 0, sizeof(buf));

    this->_buffer.read(buf, BUFFERSIZE);
    
    std::size_t bytes_read = this->_buffer.gcount();

    this->request_body.write(buf, bytes_read);

    if (this->request_body.bad()) {
        this->request_body.close();
        return -1;
    }

    this->request_body_writes = bytes_read;

    if (this->request_body_writes >= this->get_content_length()) {
        process_cgi(conn, epfd);
    }

    std::cout << YELLOW << "number of bytes left in request buffer: " << bytes_read  << std::endl << buf << RESET << std::endl;

    return 0;
}

void Request::process_cgi(Connection *conn, int epfd) {
    std::cout << "processing CGI" << std::endl;

    if (this->request_body.is_open())
        this->request_body.close();

    // TODO verify if request buffer has more content than just the header
    // TODO if it has, than we must write the remaining bytes to the CGI socket
    // TODO redirect the input from socket to the output of CGI socket (dup2 the incomming socket to the CGI)

    // std::size_t bytes_left = remaining_bytes(this->_buffer);
    // std::cout << "request buffer has " << bytes_left << " bytes left" << std::endl;

    // std::cout << "[request]" << std::endl;
    // std::cout << this->getRaw() << std::endl;
    // std::cout << "[end request]" << std::endl;

    std::cout << "content-length from request: " << this->get_content_length() << std::endl;
    std::cout << "content-type from request: " << this->get_content_type() << std::endl;

    int sockets[2];
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets) < 0) {
        print_error("opening stream socket pair");
        return;
    }

    std::cout << GREEN << "added new FDs from socketspair: " << sockets[0] << " " << sockets[1] << RESET << std::endl;

    pid_t pid = fork();
    // TODO handle fork failed

    if (pid == 0) { // child 1

        close(sockets[0]);
        dup2(sockets[1], STDOUT_FILENO);
        dup2(sockets[1], STDERR_FILENO);
        close(sockets[1]);

        int fd = 0;

        if (this->request_body_writes) {
            // Open with open() to get a fd
            fd = open(this->body_file_name.c_str(), O_RDONLY);

            if (!fd) {
                // TODO handle error
                std::cout << "error oping body_file_name" << std::endl;
                exit(1);
            }
        }

        if (fd) {
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        std::string file_path = conn->server->root + this->url_path;

        char *cmd[] = {(char *)"/usr/bin/php-cgi", (char *)file_path.c_str(), NULL};

        std::string path_translated = "PATH_TRANSLATED" + conn->server->root + this->url_path;
        std::string server_port = "SERVER_PORT" + conn->server->port;
        std::string remote_host = "REMOTE_HOST" + conn->server->host;
        std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
        std::string content_length = "CONTENT_LENGTH=" + ft_itos((int)(this->get_content_length()));
        std::string request_method = "REQUEST_METHOD=" + conn->request.getMethod();
        std::string script_name = "SCRIPT_FILENAME=" + conn->server->root + this->url_path;
        std::string path_info = "PATH_INFO=" + conn->server->root + this->url_path;
        std::string content_type = "CONTENT_TYPE=" + this->get_content_type();
        std::string url_query = "QUERY_STRING=" + this->url_query;

        char *custom_envp[] = {
            (char *)server_protocol.c_str(), (char *)"REDIRECT_STATUS=200",  (char *)path_translated.c_str(),
            (char *)server_port.c_str(),     (char *)request_method.c_str(), (char *)script_name.c_str(),
            (char *)path_info.c_str(),       (char *)url_query.c_str(),          (char *)content_length.c_str(),
            (char *)content_type.c_str(),    (char *)remote_host.c_str(),    NULL};

        if (execve(cmd[0], cmd, custom_envp) == -1)
            perror("execvp ls failed"); // TODO handle error and send it to client
    } else if (pid > 0) {               // parent

        if (close(sockets[1]) == 0) {
            std::cout << GREEN << "Removed unused socket " << sockets[1] << RESET << std::endl;

        } else {
                perror("Close: ");
        }
        conn->cgi_pid = pid;
        // std::cout << GREEN << "conn->cgi_pid: " << conn->cgi_pid << std::endl;

        // Add the socket in the parent end to the EPOLL
        /**
         * The socket in the parent end (resulted from socketpair) will be added
         * to the EPOLL and we'll be interested in read and write events for this FD.
         *
         * @note We set this to read and write because the CGI process may have all the data
         * necessary to handle the request and has began to send the response, even if we have more data
         * that we wish to send to it.
         */
        struct epoll_event ev;
        ft_memset(&ev, 0, sizeof(ev));

        conn->cgi_fd = sockets[0];

        ev.events = EPOLLIN | EPOLLHUP; // TODO check EPOLLHUP
        ev.data.fd = conn->cgi_fd;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, conn->cgi_fd, &ev);
        if (ret == -1) {
            close(conn->cgi_fd);
            print_error("failed epoll_ctl ------");
            return;
        }

        std::cout << GREEN << "added cgi socket to epoll " << conn->cgi_fd << RESET << std::endl;

    }
}

std::string Request::getline_from_body(std::size_t &bytes_read) {
    std::stringstream line;

    // stop reading when reach the end of content length
    while (bytes_read < this->get_content_length()) {
        char buf[1];
        this->_buffer.read(buf, 1);
        if (this->_buffer.fail()) {
            print_error("Failed to extract line from request body");
            this->_buffer.clear();
            return line.str();
        }
        bytes_read++;
        line << buf;
        if (*buf == '\n' && line.str().find("\r\n") != std::string::npos)
            break;
    }

    // std::string tmp = line.str();
    // tmp.erase(tmp.size() - 2); // remove \r\n from string

    return line.str();
}

// SERVER_PROTOCOL -The name and revision of the information protcol this request came
// in with. Format: protocol/revision

// SERVER_PORT - The port number to which the request was sent.

// REQUEST_METHOD - he method with which the request was made. For HTTP, this is "GET",
// "HEAD", "POST", etc.

// PATH_INFO

// The extra path information, as given by the client. In other words, scripts can be
// accessed by their virtual pathname, followed by extra information at the end of this
// path. The extra information is sent as PATH_INFO. This information should be decoded
// by the server if it comes from a URL before it is passed to the CGI script.

// PATH_TRANSLATED

// The server provides a translated version of PATH_INFO, which takes the path and does
// any virtual-to-physical mapping to it.

// SCRIPT_NAME

// A virtual path to the script being executed, used for self-referencing URLs.

// QUERY_STRING

// The information which follows the ? in the URL which referenced this script. This is
// the query information. It should not be decoded in any fashion. This variable should
// always be set when there is query information, regardless of command line decoding.

// REMOTE_HOST

// The hostname making the request. If the server does not have this information, it
// should set REMOTE_ADDR and leave this unset.

// REMOTE_ADDR

// The IP address of the remote host making the request.

// AUTH_TYPE

// If the server supports user authentication, and the script is protects, this is the
// protocol-specific authentication method used to validate the user.

// REMOTE_USER

// If the server supports user authentication, and the script is protected, this is the
// username they have authenticated as.

// REMOTE_IDENT

// If the HTTP server supports RFC 931 identification, then this variable will be set to
// the remote user name retrieved from the server. Usage of this variable should be
// limited to logging only.

// CONTENT_TYPE

// For queries which have attached information, such as HTTP POST and PUT, this is the
// content type of the data.

// CONTENT_LENGTH

// The length of the said content as given by the client.

// std::cout << "size of envp " << i << std::endl;

// export GATEWAY_INTERFACE="CGI/1.1"
// export SCRIPT_FILENAME="/home/xzhttpd/htdocs/test.php"
// export REQUEST_METHOD="POST"
// export REDIRECT_STATUS=200
// export SERVER_PROTOCOL="HTTP/1.1"
// export REMOTE_HOST="127.0.0.1"
// export CONTENT_LENGHT=3
// export HTTP_ACCEPT="text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
// export CONTENT_TYPE="application/x-www-form-urlencoded"
// export BODY="t=1"

// url: http:127.0.0.1:8084?name=testing&age=35