#include "../headers/utils.hpp"
#include "../headers/Connection.hpp"
#include "../headers/Request.hpp"
#include "../headers/Response.hpp"

#include <signal.h>

void print_error(const std::string &error_msg) { std::cerr << "Error: " << error_msg << std::endl; }

bool has_suffix(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void *ft_memset(void *s, int c, std::size_t n) {
    unsigned char *f;

    f = (unsigned char *)s;
    while (n--)
        *f++ = c;
    return (s);
}

int ft_stoi(std::string str) {
    std::stringstream ss(str);
    int nb = 0;
    ss >> nb;
    return nb;
}

std::string ft_itos(int nb) {
    std::stringstream ss;
    ss << nb;
    return ss.str();
}

file_types get_file_type(const char *name) {
    DIR *directory = opendir(name);

    if (directory == NULL) {
        if (errno == ENOTDIR)
            return TYPE_FILE;
        return TYPE_UNKOWN;
    }

    closedir(directory);
    return TYPE_DIR;
}

std::string get_formated_time(long rawtime, const std::string &format) {
    char buffer[80];

    strftime(buffer, 80, format.c_str(), gmtime(&rawtime));
    return (std::string(buffer));
}

int get_stat_info(int cfd, Request &request, Response &response) {
    struct stat struc_st;
    (void)cfd; // TODO remove

    std::string root_folder = "./www";
    std::string full_path = root_folder + request.getUrl();

    ft_memset(&struc_st, 0, sizeof(struc_st));
    if (stat(full_path.c_str(), &struc_st) == -1) {
        print_error("failed to get file information");
        return 1;
    }

    response.isdir = S_ISDIR(struc_st.st_mode);
    response.permissions = struc_st.st_mode;
    response.set_content_length(struc_st.st_size);
    response.last_modified = get_formated_time(struc_st.st_mtim.tv_sec, "%d-%h-%Y %H:%M");

    return 0;
}

std::string convert_uint32_to_str(uint32_t nb) {
    std::stringstream ss;
    ss << ((nb >> 24) & 0xFF) << '.' << ((nb >> 16) & 0xFF) << '.' << ((nb >> 8) & 0xFF) << '.'
       << (nb & 0xFF);

    return (ss.str().c_str());
}

uint32_t convert_str_to_uint32(const std::string &str) {
    std::istringstream iss(str);
    uint32_t result = 0;
    unsigned int octet;

    for (int i = 0; i < 4; ++i) {
        char dot;
        if (i > 0) {
            if (!(iss >> dot) || dot != '.') {
                // Invalid format
                return 0;
            }
        }

        if (!(iss >> octet || octet > 255)) {
            // Invalid format or out of range
            return 0;
        }

        result = (result << 8) | octet;
    }

    return result;
}

void get_port_host_from_sockfd(int sockfd, Connection *conn) {
    if (!conn)
        return;

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(sockaddr_in);

    ft_memset(&addr, 0, sizeof(sockaddr_in));
    // recovers the data and stores in addr structure
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addrlen) == -1) {
        print_error(strerror(errno));
        return;
    }

    conn->s_addr = addr.sin_addr.s_addr;
    conn->sin_port = addr.sin_port;
    conn->host = convert_uint32_to_str(ntohl(addr.sin_addr.s_addr));
    conn->port = ft_itos(ntohs(addr.sin_port));
}

int file_exists(std::string path) { return (access(path.c_str(), F_OK) == 0); }

bool is_listening_socket(int sockfd, std::vector<int> &_listening_sockets) {
    std::vector<int>::iterator it;

    for (it = _listening_sockets.begin(); it != _listening_sockets.end(); ++it) {
        if (*it == sockfd) {
            return true;
        }
    }
    return false;
}