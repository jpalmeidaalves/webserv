#include "../headers/Request.hpp"
#include "../headers/Connection.hpp"
#include "../headers/HTTP.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Response.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

Request::Request() : _content_length(0), request_body_writes(0), is_cgi(false), cgi_complete(false), read_complete(false), is_dir(false), chunked(false), chunked_complete(false) {}

Request::~Request() {}

Request &Request::operator=(const Request &rhs) {
    (void)rhs;
    return *this;
}
Request::Request(const Request &src) { *this = src; }

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

void Request::parse_request_header() {
    // parse request
    std::string line;

    // process first line
    getline(this->_buffer, this->_method, ' ');
    getline(this->_buffer, this->_url, ' ');
    getline(this->_buffer, line); // ignore the rest of the line

    this->_url = urlDecode(this->_url);

    while (getline(this->_buffer, line)) {

        // has reach the end of the header
        if (line == "\r")
            break;

        if (line.find("Host:") == 0) {
            remove_char_from_string(line, '\r');
            this->_host = line.substr(line.find(" ") + 1);
        } else if (line.find("Content-Length:") == 0) {
            remove_char_from_string(line, '\r');
            this->_content_length = ft_stoi(line.substr(line.find(" ") + 1));
        } else if (line.find("Content-Type: ") == 0) {
            remove_char_from_string(line, '\r');
            this->_content_type = line.substr(line.find(" ") + 1);
        } else if (line.find("Transfer-Encoding: chunked") == 0) {
            this->chunked = true;
        }
    }

    std::stringstream tmp;
    tmp << _buffer.rdbuf();
    _buffer.str(std::string()); // clear the underlying buffer
    _buffer << tmp.str();

    if (_buffer.str().find( "\r\n\r\n") != std::string::npos) {
        this->chunked_complete = true;
    }

    // all subsequent reads on that client socket will be written in request_body too
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
}

std::ostream &operator<<(std::ostream &out, const Request &obj) {
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Host => " << obj.getHost() << std::endl;
    out << "url => " << obj.getUrl() << std::endl;
    out << "body => " << obj.getBody() << std::endl;
    return out;
}

void Request::setUrl(std::string url) { this->_url = url; }

void Request::process_url(void) {

    // /upload.php?name=nuno&other=stuff
    // /upload.html?name=.php&other=stuff
    // /upload.html#section

    std::string base_url;


    std::string url = this->getUrl();
    std::size_t has_query = url.find("?");
    

    if (has_query != std::string::npos) {
        base_url = this->getUrl().substr(0, has_query);
        this->url_query = this->getUrl().substr(has_query + 1);
    } else {
        base_url = this->getUrl();
    }

    std::size_t has_fragment = base_url.find("#");
    if (has_fragment != std::string::npos) {
        base_url = base_url.substr(0, has_fragment);
        this->url_fragment = this->getUrl().substr(has_fragment + 1);
    } 


    this->url_path = base_url;
}

bool Request::has_cgi(Connection *conn) {

    if (this->is_cgi)
        return true;

    std::map<std::string, struct LocationOptions>::iterator it;

    for (it = conn->server->locations.begin(); it != conn->server->locations.end(); it++) {

        // *.php | *.bla | *.py

        if (it->first.find("*.") ==  0 && it->first.size() > 2 ) {
            std::string ext = it->first.substr(1);

            if (has_suffix(conn->request.url_path, ext) && it->second.cgi_pass != "") {
                this->is_cgi = true;
                this->cgi_path = it->second.cgi_pass;
                return true;
            }
        }

        if (it->first == conn->request.url_path) {
            return it->second.autoindex;

            if (it->second.cgi_pass != "") {
                this->is_cgi = true;
                this->cgi_path = it->second.cgi_pass;
                return true;
            }

        }
    }
    
    return false;
}

void Request::process_requested_file(Connection *conn, std::string full_path) {
    Response &response = conn->response;

    if (get_stat_info(full_path, response)) {
        response.set_status_code("500", conn->server, conn->request);
        return;
    }

    // make sure the filestream is closed before opening
    if (!response.inputfilestream.is_open()){
        response.inputfilestream.open(full_path.c_str(), std::ifstream::binary);
        if (!response.inputfilestream) {
            std::cout << "Error opening file" << std::endl;
            response.set_status_code("403", conn->server, conn->request);
            return;
        }
    }
    std::string file_type = MimeTypes::identify(full_path);
    response.set_content_type(file_type);
}

void Request::process_request(Connection *conn) {
    Request &request = conn->request;
    Response &response = conn->response;

    // check if is a file or dir
    file_types curr_type = get_file_type(request.url_path.c_str());

    if (curr_type == TYPE_UNKOWN) {
        response.set_status_code("404", conn->server, request);
    } else if (curr_type == TYPE_FILE) {
        this->process_requested_file(conn, request.url_path);
    } else if (request.is_dir) {
        bool is_dir_listing = conn->server->server_dir_listing(conn);

        if (!is_dir_listing) {
            response.set_status_code("403", conn->server, request);
        } else {
            response.isdir = true;
            this->list_directory(request.url_path, conn);
        }
    }
}

std::string Request::create_html_dir(std::map<std::string, struct dir_entry> &dir_entries) {
    std::stringstream ss;

    ss << "<html><head><title>Index of " << this->url_path << "/</title></head><body><h1>Index of "
       << this->url_path << "</h1><hr><pre>";

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

    return ss.str();
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
        response.set_status_code("500", conn->server, request);
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
        response.set_status_code("500", conn->server, request);
        return -1;
    }

    response.dir_data = create_html_dir(dir_entries);

    response.set_status_code("200", conn->server, conn->request);
    response.set_content_length(response.dir_data.size());

    return 0;
}


int Request::prepare_file_to_save_body(int fd, Connection *conn, int epfd) {
    if (this->getMethod() == "GET"){
        if (process_cgi(conn, epfd) == -1)
            return -1;
        return 0;
    }
    this->body_file_name = "./tmp/.tmp-req-body-" + ft_itos(fd);
    this->request_body.open(this->body_file_name.c_str());

    if (!this->request_body.is_open()) {
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

    return 0;
}

int Request::process_cgi(Connection *conn, int epfd) {

    if (this->request_body.is_open())
        this->request_body.close();

    int sockets[2];
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets) < 0) {
        print_error("opening stream socket pair");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        this->cgi_complete = true;
        return -1;
    }

    if (pid == 0) { // child 1

        close(sockets[0]);
        dup2(sockets[1], STDOUT_FILENO);
        dup2(sockets[1], STDERR_FILENO);
        close(sockets[1]);

        int fd = 0;

        if (this->request_body_writes) {
            fd = open(this->body_file_name.c_str(), O_RDONLY);
            if (!fd) {
                std::cout << "error openning body_file_name" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        if (fd) {
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // /usr/bin/php-cgi
        // cgi_pass /usr/bin/php8.1-cgi;
        
        char *cmd[] = {(char *)this->cgi_path.c_str(), (char *)this->url_path.c_str(), NULL};
      
        std::string server_port = "SERVER_PORT=" + conn->server->port;
        std::string remote_host = "REMOTE_HOST=" + conn->server->host;
        std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
        std::string content_length = "CONTENT_LENGTH=" + ft_itos((int)(this->get_content_length()));
        std::string request_method = "REQUEST_METHOD=" + conn->request.getMethod();
        std::string script_filename = "SCRIPT_FILENAME=" + this->url_path;
        std::string script_name = "SCRIPT_NAME=" + this->url_path;
        std::string path_info = "PATH_INFO=" + this->url_path;
        std::string content_type = "CONTENT_TYPE=" + this->get_content_type();
        
        std::string url_query;

        if (conn->request.upload_path != "")
            url_query = "QUERY_STRING=upload_path=" + conn->request.upload_path; // pass upload path here inside the query
        else
            url_query = "QUERY_STRING=" + this->url_query;

        char *custom_envp[] = {
            (char *)script_filename.c_str(),
            (char *)server_protocol.c_str(), (char *)"REDIRECT_STATUS=200", //(char *)path_translated.c_str(),
            (char *)server_port.c_str(),     (char *)request_method.c_str(), (char *)script_name.c_str(),
            (char *)path_info.c_str(),       (char *)url_query.c_str(),          (char *)content_length.c_str(),
            (char *)content_type.c_str(),    (char *)remote_host.c_str(),    NULL};

        if (execve(cmd[0], cmd, custom_envp) == -1) {
            perror(strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {               // parent

        if (!(close(sockets[1]) == 0)) {
            print_error("Close: ");
        }
        conn->cgi_pid = pid;

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

        ev.events = EPOLLIN | EPOLLHUP;
        ev.data.fd = conn->cgi_fd;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, conn->cgi_fd, &ev);
        if (ret == -1) {
            close(conn->cgi_fd);
            print_error("failed epoll_ctl ------");
            return -1;
        }
    }

    return 0;
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
