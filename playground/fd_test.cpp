
#include <iostream>
// #include <sstream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>


int main() {
    std::ofstream ofs;
    ofs.open("test.cpp");

    int fd = open("utils.cpp", O_RDONLY);

    std::cout << "fd is: " << fd << std::endl;
    ofs.close();

    close(fd);

    
}