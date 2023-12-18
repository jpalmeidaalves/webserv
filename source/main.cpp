#include "../headers/Server.hpp"
#include <iostream>
#include <string>

struct sock

int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	std::cout << "hello world" << std::endl;
	return 0;
}

void log(const std::string &message)
{
    std::cout << message << std::endl;
}

void exitWithError(const std::string &errorMessage)
{
    log("ERROR: " + errorMessage);
    exit(1);
}