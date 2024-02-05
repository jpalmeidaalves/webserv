#include <iostream>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
// #include <fcntl.h>
#include <unistd.h>



int main() {
    int sockets[2];
    
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets) < 0) {
        std::cout << "opening stream socket pair" << std::endl;
        exit(1);
    }

    std::cout << " sockets in use : " << sockets[0] << " - " << sockets[1] << std::endl;

    close(sockets[0]);
    close(sockets[1]);

     int sockets2[2];
    
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets2) < 0) {
        std::cout << "opening stream socket pair" << std::endl;
        exit(1);
    }

    std::cout << " sockets2 in use : " << sockets2[0] << " - " << sockets2[1] << std::endl;
}