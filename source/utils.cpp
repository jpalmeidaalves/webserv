#include "../headers/utils.hpp"
#include "../headers/Connection.hpp"
#include "../headers/Request.hpp"
#include "../headers/Response.hpp"

#include <signal.h>

void print_error(const std::string &error_msg) { std::cerr << "Error: " << error_msg << std::endl; }

bool has_suffix(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
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

std::string ft_ultos(unsigned long nb) {
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

/**
 * Get information of this file or dir and save the data the response object
 *
 * @return true if sucessfully
 * @return false if some error ocurred
 */
int get_stat_info(std::string full_path, Response &response) {
    struct stat struc_st;

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

bool has_permissions(std::string full_path, mode_t permissions) {
    struct stat struc_st;

    ft_memset(&struc_st, 0, sizeof(struc_st));
    if (stat(full_path.c_str(), &struc_st) == -1) {
        print_error("failed to get file information");
        return false;
    }

    // return true if has the specified permissions
    return (struc_st.st_mode & permissions);
}

std::string convert_uint32_to_str(uint32_t nb) {
    std::stringstream ss;
    ss << ((nb >> 24) & 0xFF) << '.' << ((nb >> 16) & 0xFF) << '.' << ((nb >> 8) & 0xFF) << '.' << (nb & 0xFF);

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

int file_exists(std::string path) { 
    return (access(path.c_str(), F_OK)); }

bool is_listening_socket(int sockfd, std::vector<int> &_listening_sockets) {
    std::vector<int>::iterator it;

    for (it = _listening_sockets.begin(); it != _listening_sockets.end(); ++it) {
        if (*it == sockfd) {
            return true;
        }
    }
    return false;
}

void remove_char_from_string(std::string &str, char to_remove) {
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (*it == to_remove) {
            str.erase(it);
            --it;
        }
    }
}

/**
 * Print ascii characters from a char *
 *
 * @note usefull for debuging
 */
void print_ascii(const char *str) {
    int i = 0;
    while (str[i]) {
        std::cout << str[i] << " - " << (int)str[i] << std::endl;
        i++;
    }
}

/**
 * Compares two strings in case insensitive mode
 */
bool ft_strcmp_insensitive(std::string str1, std::string str2) {
    std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
    return (str1 == str2);
}

struct timeval begin, end;

long get_timestamp() {
    struct timeval time_s;
    gettimeofday(&time_s, 0);

    return time_s.tv_sec;
}

void start_timer(struct timeval *begin) {
    std::cout << YELLOW << "Timer started" << std::endl;
    gettimeofday(begin, 0);
}

void end_timer(struct timeval *begin, struct timeval *end) {
    // Stop measuring time and calculate the elapsed time
    gettimeofday(end, 0);
    long seconds = end->tv_sec - begin->tv_sec;
    long microseconds = end->tv_usec - begin->tv_usec;
    double elapsed = seconds + microseconds * 1e-6;

    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout.precision(3);

    std::cout << YELLOW << "Time elapsed: " << elapsed << RESET << std::endl;
}
std::size_t remaining_bytes(const std::stringstream &s) {
    std::streambuf *buf = s.rdbuf();
    std::streampos pos = buf->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
    std::streampos end = buf->pubseekoff(0, std::ios_base::end, std::ios_base::in);
    buf->pubseekpos(pos, std::ios_base::in);
    return end - pos;
}

bool only_digits(std::string s)
{
    if (s.find_first_not_of("1234567890") != std::string::npos)
        return false;
    return true;
}

std::string urlDecode(const std::string& encoded) {
    std::ostringstream decoded;
    std::string::size_type i = 0;

    while (i < encoded.length()) {
        if (encoded[i] == '%') {
            // If we find a percent sign, we decode the following two characters
            if (i + 2 < encoded.length()) {
                std::istringstream iss(encoded.substr(i + 1, 2));
                int c;
                if (iss >> std::hex >> c) {
                    decoded << static_cast<char>(c);
                    i += 3; // Move to the next character after the decoded sequence
                } else {
                    decoded << encoded[i]; // If decoding fails, keep the '%'
                    ++i;
                }
            } else {
                // Incomplete percent-encoded sequence, keep the '%' as is
                decoded << encoded[i++];
            }
        } else if (encoded[i] == '+') {
            // Convert '+' to space
            decoded << ' ';
            ++i;
        } else {
            // Keep the character as is
            decoded << encoded[i++];
        }
    }

    return decoded.str();
}