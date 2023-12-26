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