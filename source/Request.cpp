#include "../headers/Request.hpp"
#include "../headers/Connection.hpp"
#include "../headers/HTTP.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Response.hpp"
#include "../headers/Server.hpp"
#include "../headers/utils.hpp"

Request::Request() : _rawsize(0), _content_length(0) {}

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

void Request::parse_request() {
    // std::cout << "data: " << data << std::endl;
    // parse request
    std::stringstream ss(this->_raw.str());
    std::string line;

    // process first line
    getline(ss, this->_method, ' ');
    getline(ss, this->_url, ' ');
    getline(ss, line); // ignore the rest of the line

    // std::cout << "---------- parsing header -----------" << std::endl;

    while (getline(ss, line)) {

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
std::string Request::getRaw() const { return (this->_raw.str()); }

bool Request::not_parsed() {
    if (this->_method == "")
        return true;
    return false;
}

std::string Request::get_content_type() const { return (this->_content_type); }
std::size_t Request::get_content_length() const { return (this->_content_length); }

void Request::set_content_type(const std::string type) { this->_content_type = type; }
void Request::set_content_length(std::size_t length) { this->_content_length = length; }

void Request::append_raw(const char *buf, size_t len) {
    this->_raw.write(buf, len);
    this->_rawsize += len;
}

std::ostream &operator<<(std::ostream &out, const Request &obj) {
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Host => " << obj.getHost() << std::endl;
    out << "url => " << obj.getUrl() << std::endl;
    out << "body => " << obj.getBody() << std::endl;
    return out;
}

void Request::setUrl(std::string url) { this->_url = url; }

bool Request::has_cgi() {

    // HAS CGI
    // /upload.php
    // /upload.php?name=nuno&other=stuff

    // DOES NOT HAVE CGI
    // /upload.html?name=.php&other=stuff
    // /upload.html

    std::size_t has_query = this->getUrl().find("?");

    if (has_query != std::string::npos) {
        this->short_url = this->getUrl().substr(0, has_query);
        // std::cout << RED << short_url << RESET << std::endl;
        this->query = this->getUrl().substr(has_query + 1);
    } else {
        this->short_url = this->getUrl();
    }

    return has_suffix(short_url, ".php");
}

void Request::process_requested_file(Connection *conn) {
    Request &request = conn->request;
    Response &response = conn->response;

    std::string full_path = conn->server->root + request.getUrl();

    if (get_stat_info(full_path, response)) {
        response.set_status_code("500", conn->server);
        return;
    }

    // TODO check permission, done for read
    if (!(response.permissions & S_IROTH)) {
        response.set_status_code("403", conn->server);
        return;
    }

    int file_fd = open(full_path.c_str(), O_RDONLY);
    if (!file_fd) {
        print_error("Error opening file");
        response.set_status_code("500", conn->server);
        return;
    }

    // response.set_status_code("200");
    response.set_content_type(MimeTypes::identify(full_path));
    response.set_req_file_fd(file_fd);
}

void Request::process_request(Connection *conn) {
    // int cfd = ev.data.fd;
    Request &request = conn->request;
    Response &response = conn->response;

    // std::cout << "[Request Header]" << request.getRaw() << std::endl;

    std::string full_path = conn->server->root + request.getUrl();

    // check if is a file or dir
    file_types curr_type = get_file_type(full_path.c_str());

    if (curr_type == TYPE_UNKOWN) {
        print_error("failed to check if is a dir");
        response.set_status_code("404", conn->server);
    } else if (curr_type == TYPE_FILE) {
        std::cout << "------- file --------" << std::endl;
        this->process_requested_file(conn);
    } else if (curr_type == TYPE_DIR) {
        std::cout << "------- dir --------" << std::endl;

        // if index file is present
        // TODO must check all index files defined in the configfile
        if (file_exists(full_path + "/" + "index.html")) {
            // send file (must check permissions)
            request.setUrl(request.getUrl() + "/" + "index.html"); // update url
            this->process_requested_file(conn);
        } else {
            // send list dir (must check permissions)

            // TODO if dir listing is active, from config file
            bool is_dir_listing = true; // TODO now

            // TODO we must check the index files in the configfile and show them instead of
            // listing dir

            if (!is_dir_listing) {
                response.set_status_code("403", conn->server);
            } else {
                response.isdir = true;
                this->list_directory(full_path, conn);
            }
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

    ss << "<html><head><title>Index of " << request.getUrl() << "/</title></head><body><h1>Index of "
       << request.getUrl() << "</h1><hr><pre>";

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

void Request::process_post_request(Connection *conn) {
    std::cout << "processing POST request" << std::endl;

    // std::cout << "[request]" << std::endl;
    // std::cout << this->getRaw() << std::endl;
    // std::cout << "[end request]" << std::endl;

    // TODO handle CGI
    print_error("TODO MUST HANDLE CGI");

    int first_pipefd[2];
    int second_pipefd[2];
    pipe(first_pipefd);
    pipe(second_pipefd);
    pid_t pid = fork();

    if (pid == 0) {              // child 1
        close(first_pipefd[1]);  // close the write end of the first pipe
        close(second_pipefd[0]); // close reading end in the child

        dup2(first_pipefd[0], STDIN_FILENO); // send stdin to the first pipe

        dup2(second_pipefd[1], 1); // send stdout to the pipe
        dup2(second_pipefd[1], 2); // send stderr to the pipe

        close(second_pipefd[1]); // this descriptor is no longer needed
        close(first_pipefd[0]);

        std::string file_path = conn->server->root + this->getUrl();

        (char *)this->_raw.rdbuf();
        char *cmd[] = {(char *)"/usr/bin/php-cgi", (char *)file_path.c_str(), NULL};

        std::string path_translated = "PATH_TRANSLATED" + conn->server->root + this->short_url;
        std::string server_port = "SERVER_PORT" + conn->server->port;
        std::string remote_host = "REMOTE_HOST" + conn->server->host;
        // std::string remote_addr = "REMOTE_ADDR" + conn->server->host;
        // std::string http_accept = "HTTP_ACCEPT";

        std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
        std::string content_length = "CONTENT_LENGTH=" + ft_itos((int)(this->get_content_length()));
        std::string request_method = "REQUEST_METHOD=" + conn->request.getMethod();
        std::string script_name = "SCRIPT_FILENAME=" + conn->server->root + this->short_url;
        std::string path_info = "PATH_INFO=" + conn->server->root + this->short_url;
        std::string content_type = "CONTENT_TYPE=text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
        std::string query = "QUERY_STRING=" + this->query;

        // char *custom_envp[] = build_custom_envp();

        char *custom_envp[] = {
            (char *)server_protocol.c_str(), (char *)"REDIRECT_STATUS=200",  (char *)path_translated.c_str(),
            (char *)server_port.c_str(),     (char *)request_method.c_str(), (char *)script_name.c_str(),
            (char *)path_info.c_str(),       (char *)query.c_str(),          (char *)content_length.c_str(),
            (char *)content_type.c_str(),    (char *)remote_host.c_str(),    NULL};

        // char *custom_envp[] = {
        //     (char *)server_protocol.c_str(),
        //     //    (char *)"REMOTE_HOST=127.0.0.1:8084",
        //     (char *)"REDIRECT_STATUS=200",
        //     (char *)request_method.c_str(),
        //     (char *)script_name.c_str(),
        //     (char *)path_info.c_str(),
        //     (char *)query.c_str(),
        //     (char *)content_length.c_str(),
        //     (char *)"CONTENT_TYPE=application/x-www-form-urlencoded",
        //     // (char *)"BODY=testing=42",
        //     NULL
        // };

        if (execve(cmd[0], cmd, custom_envp) == -1)
            perror("execvp ls failed");
    } else if (pid > 0) {        // parent
        close(second_pipefd[1]); // close the write end of the pipe in the parent
        close(first_pipefd[0]);  // close the read end of the first pipe in the parent

        std::size_t body_pos = this->getRaw().find("\r\n\r\n") + 4;
        this->_raw.ignore(body_pos);

        char body[this->get_content_length()];

        this->_raw.read(body, this->get_content_length());

        std::cout << "body is: " << body << std::endl;

        if (write(first_pipefd[1], body, this->get_content_length()) == -1) {
            std::cout << "failed to send the body to the CGI" << std::endl;
        }

        close(first_pipefd[1]);

        // std::cout << "---------------- CGI OUTPUT ----------------" << std::endl;

        // TODO read until end of the header
        std::stringstream ss;
        while (1) {
            char buffer[1];
            std::size_t byr = read(second_pipefd[0], buffer, 1);
            if (byr <= 0)
                break;
            ss << buffer;

            if (*buffer == '\n') {
                if (ss.str().find("\r\n\r\n") != std::string::npos)
                    break;
            }

        }

        // ss has the complete header from CGI

        // std::cout << "header from CGI: " << std::endl;
        // std::cout << ss.str() << std::endl;

        /*
        Status: 201 Created
        Content-type: text/html; charset=UTF-8

        */

        conn->response.parse_cgi_headers(ss, conn->server);

        conn->response.set_req_file_fd(second_pipefd[0]);

        // close(second_pipefd[0]);
        // waitpid(pid, NULL, 0);

        // TODO if takes too long send request timed out
        // std::cout << "---------------- END OUTPUT ----------------" << std::endl;
    }
}

std::string Request::getline_from_body(std::size_t &bytes_read) {
    std::stringstream line;

    // stop reading when reach the end of content length
    while (bytes_read < this->get_content_length()) {
        char buf[1];
        this->_raw.read(buf, 1);
        if (this->_raw.fail()) {
            print_error("Failed to extract line from request body");
            this->_raw.clear();
            return line.str();
        }
        bytes_read++;
        line << buf;
        // TODO optimize, on check this if current char is \r or \n
        if (line.str().find("\r\n") != std::string::npos)
            break;
    }

    // std::string tmp = line.str();
    // tmp.erase(tmp.size() - 2); // remove \r\n from string

    return line.str();
}

// std::string Request::extract_filename_from_body(size_t &bytes_read) {

//     std::string filename;
//     while (1) {
//         std::string line = getline_from_body(bytes_read);
//         if (line == "\r\n" || line == "")
//             break;

//         if (line.find("Content-Disposition:") == 0) {
//             std::string key = "filename=\"";
//             std::size_t pos;
//             pos = line.find(key);
//             if (pos != std::string::npos) {
//                 filename = line.substr(pos + key.size());
//                 // std::cout << "####filename: " << filename << std::endl;
//                 filename = filename.erase(filename.size() - 3); // remove \"\n\r
//                 // std::cout << "####final filename: " << filename << std::endl;
//             }
//         }

//         // !warning must read all lines until \r\n
//     }
//     return (filename);
// }

// std::string Request::upload_single_file(size_t &bytes_read, std::string boundary, Server *server)
// {

//     std::string filename = extract_filename_from_body(bytes_read);
//     if (filename == "") {
//         std::cout << "&&&&1" << std::endl;
//         return "400";
//     }

//     std::string upload_folder = server->root + this->getUrl();
//     if (upload_folder.at(upload_folder.size() - 1) != '/')
//         upload_folder += '/';
//     std::string file_path = upload_folder + filename;

//     std::cout << "&&&&2" << std::endl;

//     std::stringstream ss;
//     // bytes_read = 0;

//     // TODO improve this code
//     while (bytes_read < this->get_content_length()) { // loop getting single characters

//         char buf[186942];
//         this->_raw.read(buf, 186942);
//         if (this->_raw.fail()) {
//             print_error("Failed to extract line from request body");
//             this->_raw.clear();
//             break;
//         }

//         bytes_read++;
//         // std::cout << "bytes_read: " << bytes_read << "/" << this->get_content_length() <<
//         // std::endl;

//         ss.write(buf, 186942);
//         if (ss.fail()) {
//             std::cout << "failed to write" << std::endl;
//             break;
//         }

//         // "----------------------------776653983427372066393762"

//         // "isto e o conteudo------------------7766"
//         // if (boundary.find(buf[0]) != std::string::npos) {
//         //     if (ss.str().find(boundary) != std::string::npos) {
//         //         std::cout << CYAN << "break!" << RESET << std::endl;
//         //         break;
//         //     }
//         // }
//         getline_from_body(bytes_read); // advance boundary line
//     }

//     // std::cout << "+++++++++++++++++++++++++++" << std::endl;
//     // std::cout << "data from first file1: " << std::endl;
//     // std::cout << ss.str() << std::endl;
//     // std::cout << "+++++++++++++++++++++++++++" << std::endl;

//     std::size_t total_bytes = bytes_read - boundary.size();
//     std::ofstream ofs(file_path.c_str(), std::ios::binary);

//     std::cout << "&&&&5" << std::endl;

//     if (ofs.fail()) {
//         std::cout << RED << "FAILED AT 500" << RESET << std::endl;
//         return "500";
//     }

//     while (total_bytes) { // loop getting single characters
//         // std::cout << "&&&&6" << std::endl;
//         char buf[1];
//         ss.read(buf, 1);
//         if (ss.fail()) {
//             print_error("Failed to extract line from request body WHAT THE HELL");
//             ss.clear();
//             break;
//         }

//         total_bytes--;
//         ofs.write(buf, 1); // binary output
//     }
//     ofs.close();
//     std::cout << "&&&&7" << std::endl;
//     return "201";
// }

// std::string Request::upload_files(Server *server) {
//     std::cout << RED << "WORK TO BE DONE" << RESET << std::endl;

//     //  content_type = "multipart/form-data;
//     //  boundary=--------------------------314581073725974381613932"

//     std::size_t boundary_pos = this->get_content_type().find("boundary=");
//     std::string boundary = "--" + this->get_content_type().substr(boundary_pos + 9);

//     std::cout << "Boundary is: " << boundary << std::endl;

//     std::size_t body_pos = this->getRaw().find("\r\n\r\n") + 4;

//     std::cout << "body_pos " << body_pos << std::endl;

//     std::string status_code;
//     std::size_t bytes_read = 0;

//     // advance the position of the body
//     this->_raw.seekg(body_pos);

//     std::cout << YELLOW << "@@@@@@@@@@@@@@@@" << std::endl;

//     // first line must be a boundary line
//     std::string first_line = getline_from_body(bytes_read);

//     std::cout << YELLOW << "first line: " << first_line << RESET << std::endl;
//     std::cout << YELLOW << "content length: " << this->get_content_length() << RESET <<
//     std::endl;

//     print_ascii(first_line.c_str());

//     if (first_line != boundary + "\r\n")
//         return "400"; // bad request

//     // TODO stop reading until reaching this->get_content_length() for binary reading
//     while (bytes_read < this->get_content_length()) {
//         std::cout << RED << "bytes_read: " << bytes_read << "/" << this->get_content_length()
//                   << RESET << std::endl;
//         status_code = this->upload_single_file(bytes_read, boundary, server);
//         // read 2 bytes and check if are "\r\n" or "--"
//         char buf[2];
//         this->_raw.read(buf, 2);
//         if (this->_raw.fail()) {
//             print_error("error checking 2 character after boundary");
//             break;
//         }
//         bytes_read += 2;

//         if (buf[0] == '-' && buf[1] == '-') {
//             std::cout << GREEN << "done uploading files" << RESET << std::endl;
//             break;
//         }
//     }

//     std::cout << RED << "at the END bytes_read: " << bytes_read << "/" <<
//     this->get_content_length()
//               << RESET << std::endl;

//     // std::cout << "[Request Raw]" << std::endl;
//     // std::cout << this->getRaw() << std::endl;

//     // response.set_status_code("200", conn->server);

//     std::cout << GREEN << "WORK DONE" << RESET << std::endl;

//     return status_code;
// }

// void Request::process_post_request(Connection *conn) {
//     // int cfd = ev.data.fd;
//     Request &request = conn->request;
//     Response &response = conn->response;

//     std::cout << "[Request Header]" << request.getRaw() << std::endl;

//     std::string full_path = conn->server->root + request.getUrl();

//     // check if is a file or dir
//     file_types curr_type = get_file_type(full_path.c_str());

//     if (curr_type == TYPE_UNKOWN) {
//         print_error("failed to check if is a dir");
//         response.set_status_code("404", conn->server);
//     } else if (curr_type == TYPE_FILE) {
//         std::cout << "------- file -------- must handle TODO" << std::endl;
//         // TODO check if the file is a suported script file (php, python?)
//     } else if (curr_type == TYPE_DIR) {
//         std::cout << "------- dir --------" << std::endl;

//         // if not multipart form data stop here
//         if (request.get_content_type().find("multipart/form-data") != 0) {
//             response.set_status_code("404", conn->server);
//             return;
//         }

//         // write permissions for Others
//         bool has_write_permision = has_permissions(full_path.c_str(), S_IWOTH);

//         // std::cout << "has_perm: " << has_write_permision << std::endl;

//         if (!has_write_permision) {
//             response.set_status_code("403", conn->server);
//         } else {
//             std::string status = this->upload_files(conn->server);
//             response.set_status_code(status, conn->server);
//         }
//     }
// }

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