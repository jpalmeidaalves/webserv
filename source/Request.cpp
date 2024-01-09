#include "../headers/Request.hpp"
#include "../headers/Connection.hpp"
#include "../headers/HTTP.hpp"
#include "../headers/MimeTypes.hpp"
#include "../headers/Response.hpp"
#include "../headers/utils.hpp"

Request::Request() : _req_file_fd(0) {}

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
    std::stringstream ss(this->_raw);
    std::string line;

    // process first line
    getline(ss, this->_method, ' ');
    getline(ss, this->_url, ' ');
    getline(ss, line); // ignore the rest of the line

    // get host
    getline(ss, line);
    // std::cout << line << std::endl;
    if (line.find("Host:") == 0) {
        this->_host = line.substr(line.find(" ") + 1);
        remove_char_from_string(this->_host, '\r');
    }
}

std::string Request::getMethod() const { return (this->_method); }
std::string Request::getUrl() const { return (this->_url); }
std::string Request::getBody() const { return (this->_body); }
std::string Request::getHost() const { return (this->_host); }
std::string Request::getRaw() const { return (this->_raw); }

void Request::append_raw(std::string buf) { this->_raw += buf; }

std::ostream &operator<<(std::ostream &out, const Request &obj) {
    out << "Method => " << obj.getMethod() << std::endl;
    out << "Host => " << obj.getHost() << std::endl;
    out << "url => " << obj.getUrl() << std::endl;
    out << "body => " << obj.getBody() << std::endl;
    return out;
}

void Request::setUrl(std::string url) { this->_url = url; }

int Request::get_requested_fd() { return (this->_req_file_fd); }

void Request::set_req_file_fd(int ffd) { this->_req_file_fd = ffd; }

void Request::process_requested_file(Connection *conn) {
    Request &request = conn->request;
    Response &response = conn->response;

    std::string full_path = conn->server->root + request.getUrl();

    if (get_stat_info(full_path, response)) {
        response.set_status_code("500");
        return;
    }

    // TODO check permission, done for read
    if (!(response.permissions & S_IROTH)) {
        response.set_status_code("403");
        return;
    }

    int file_fd = open(full_path.c_str(), O_RDONLY);
    if (!file_fd) {
        print_error("Error opening file");
        response.set_status_code("500");
        return;
    }

    response.set_status_code("200");
    response.set_content_type(MimeTypes::identify(full_path));
    request.set_req_file_fd(file_fd);
}

void Request::process_request(Connection *conn) {
    // int cfd = ev.data.fd;
    Request &request = conn->request;
    Response &response = conn->response;

    // std::cout << "[Request Header]" << request.getRaw() << std::endl;

    // TODO implement root folder based on server
    std::string full_path = conn->server->root + request.getUrl();

    // check if is a file or dir
    file_types curr_type = get_file_type(full_path.c_str());

    if (curr_type == TYPE_UNKOWN) {
        print_error("failed to check if is a dir");
        response.set_status_code("404");
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
                response.set_status_code("403");
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
        response.set_status_code("500");
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
        response.set_status_code("500");
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

    response.set_status_code("200");
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