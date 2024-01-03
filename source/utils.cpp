#include "../headers/utils.hpp"

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

int is_file(const char *name) {
    DIR *directory = opendir(name);

    if (directory != NULL) {
        closedir(directory);
        return 0;
    }

    if (errno == ENOTDIR)
        return 1;

    return -1;
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
