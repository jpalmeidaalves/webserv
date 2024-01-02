#include "../headers/utils.hpp"

#include <signal.h>

void print_error(const std::string &error_msg) {
    std::cerr << "Error: " << error_msg << std::endl;
}

bool has_suffix(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void	*ft_memset(void *s, int c, std::size_t n)
{
	unsigned char	*f;

	f = (unsigned char *)s;
	while (n--)
		*f++ = c;
	return (s);
}

int is_file(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL) {
     closedir(directory);
     return 0;
    }

    if(errno == ENOTDIR)
        return 1;

    return -1;
}

std::string get_formated_time(long rawtime, const std::string &format) {
    char buffer [80];

    strftime (buffer, 80, format.c_str(), gmtime(&rawtime));
    return (std::string(buffer));
}
