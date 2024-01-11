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
std::string Request::getBody() const { return (this->_body); }
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
        if (line.str().find("\r\n") != std::string::npos)
            break;
    }

    // std::string tmp = line.str();
    // tmp.erase(tmp.size() - 2); // remove \r\n from string

    return line.str();
}

std::string Request::extract_filename_from_body(size_t &bytes_read) {

    std::string filename;
    while (1) {
        std::string line = getline_from_body(bytes_read);
        if (line == "\r\n" || line == "")
            break;

        if (line.find("Content-Disposition:") == 0) {
            std::string key = "filename=\"";
            std::size_t pos;
            pos = line.find(key);
            if (pos != std::string::npos) {
                filename = line.substr(pos + key.size());
                // std::cout << "####filename: " << filename << std::endl;
                filename = filename.erase(filename.size() - 3); // remove \"\n\r
                // std::cout << "####final filename: " << filename << std::endl;
            }
        }

        // !warning must read all lines until \r\n
    }
    return (filename);
}

std::string Request::upload_files(Server *server) {
    std::cout << RED << "WORK TO BE DONE" << RESET << std::endl;

    //  content_type = "multipart/form-data;
    //  boundary=--------------------------314581073725974381613932"

    std::size_t boundary_pos = this->get_content_type().find("boundary=");
    std::string boundary = "--" + this->get_content_type().substr(boundary_pos + 9);

    std::cout << "Boundary is: " << boundary << std::endl;

    std::size_t body_pos = this->getRaw().find("\r\n\r\n") + 4;

    std::size_t bytes_read = 0;

    // TODO stop reading until reaching this->get_content_length() for binary reading

    // advance the position of the body
    this->_raw.seekg(body_pos);

    // first line must be a boundary line
    std::string first_line = getline_from_body(bytes_read);

    if (first_line != boundary + "\r\n")
        return "400"; // bad request

    std::string filename = extract_filename_from_body(bytes_read);
    if (filename == "")
        return "400";

    std::string upload_folder = server->root + this->getUrl();
    // TODO must check if url has sufix /

    // std::cout << "folder to upload: " << upload_folder << std::endl;
    // std::cout << "filename: " << filename << std::endl;

    // extract binary data from raw until next boundary

    std::stringstream ss;
    std::size_t bytes_read2 = 0;
    while (1) { // loop getting single characters
        char buf[1];
        this->_raw.read(buf, 1);
        if (this->_raw.fail()) {
            print_error("Failed to extract line from request body");
            this->_raw.clear();
            break;
        }

        bytes_read2++;
        ss.write(buf, 1);
        // ss << buf;

        // std::cout << "current: " << ss.str() << std::endl;

        if (ss.str().find(boundary) != std::string::npos) {
            std::cout << "break!" << std::endl;
            break;
        }
    }

    std::cout << "+++++++++++++++++++++++++++" << std::endl;
    std::cout << "data from first file1: " << std::endl;
    std::cout << ss.str() << std::endl;
    std::cout << "+++++++++++++++++++++++++++" << std::endl;

    std::size_t total_bytes = bytes_read2 - boundary.size();
    std::string file_path = upload_folder + "/" + filename;
    std::ofstream ofs(file_path.c_str(), std::ios::binary);

    while (total_bytes) { // loop getting single characters
        char buf[1];
        ss.read(buf, 1);
        if (ss.fail()) {
            print_error("Failed to extract line from request body WHAT THE HELL");
            ss.clear();
            break;
        }

        total_bytes--;
        ofs.write(buf, 1); // binary output
    }

    ofs.close();

    // write to file

    // std::string line;
    // getline(this->_raw, line);
    // std::cout << "********2 " << std::string(buf) << std::endl;
    // std::string x;
    // ss >> x

    // std::stringstream ss{line};
    // std::string z; // switch later to char
    // ss >> z;
    // if (ss.good()) {
    //     std::cout << "Stream good\n";
    // } else {
    //     std::cout << "Not good\n";
    // }

    //     while (!ss.eof()) {
    //     if (ss.peek() != '.') {
    //         ss >> number;
    //         if (ss.tellg() == -1) {
    //             std::cout << "Last one: " << number;
    //         } else {
    //             std::cout << number;
    //         }
    //     } else { ss.get(); }
    // }

    // skip the header

    // while (this->_raw.read(buf, header_size) != at) {
    //     /* code */
    // }

    // std::cout << "[Request Raw]" << std::endl;
    // std::cout << this->getRaw() << std::endl;

    // response.set_status_code("200", conn->server);

    std::cout << GREEN << "WORK DONE" << RESET << std::endl;

    return "201";
}

void Request::process_post_request(Connection *conn) {
    // int cfd = ev.data.fd;
    Request &request = conn->request;
    Response &response = conn->response;

    std::cout << "[Request Header]" << request.getRaw() << std::endl;

    std::string full_path = conn->server->root + request.getUrl();

    // check if is a file or dir
    file_types curr_type = get_file_type(full_path.c_str());

    if (curr_type == TYPE_UNKOWN) {
        print_error("failed to check if is a dir");
        response.set_status_code("404", conn->server);
    } else if (curr_type == TYPE_FILE) {
        std::cout << "------- file -------- must handle TODO" << std::endl;
        // TODO check if the file is a suported script file (php, python?)
    } else if (curr_type == TYPE_DIR) {
        std::cout << "------- dir --------" << std::endl;

        // if not multipart form data stop here
        if (request.get_content_type().find("multipart/form-data") != 0) {
            response.set_status_code("404", conn->server);
            return;
        }

        // write permissions for Others
        bool has_write_permision = has_permissions(full_path.c_str(), S_IWOTH);

        // std::cout << "has_perm: " << has_write_permision << std::endl;

        if (!has_write_permision) {
            response.set_status_code("403", conn->server);
        } else {
            std::string status = this->upload_files(conn->server);
            response.set_status_code(status, conn->server);
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