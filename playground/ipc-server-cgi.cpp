#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
        std::cout << "opening stream socket pair" << std::endl;
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) { // child 1
        close(sockets[1]);

        dup2(sockets[0], STDIN_FILENO);
        dup2(sockets[0], STDOUT_FILENO);
        dup2(sockets[0], STDERR_FILENO);

        close(sockets[0]);

        char *cmd[] = {(char *)"/usr/bin/php-cgi", (char *)"test.php", NULL};

        // ! Basically content type was wrong!!!

        char *env[] = {(char *)"REDIRECT_STATUS=200",      (char *)"PATH_TRANSLATED=./test.php",
                       (char *)"SERVER_PROTOCOL=HTTP/1.1", (char *)"CONTENT_LENGTH=17",
                       (char *)"REQUEST_METHOD=POST",      (char *)"SCRIPT_FILENAME=./test.php",
                       (char *)"PATH_INFO=./test.php",     (char *)"CONTENT_TYPE=application/x-www-form-urlencoded",
                       (char *)"QUERY_STRING=page=35"};

        if (execve(cmd[0], cmd, env) == -1)
            std::cout << "execve ls failed" << std::endl;

    } else if (pid > 0) { // parent
        close(sockets[0]);

        char body[] = "user=nuno&age=35";

        if (write(sockets[1], body, 17) <= 0) {
            std::cout << "failed to send the body to the CGI" << std::endl;
        }

        std::cout << "[parent process] wrote to child process: " << body << std::endl;

        char res[1000];
        memset(res, 0, 1000);
        if (read(sockets[1], res, 1000) <= 0) {
            std::cout << "failed to read data from the CGI" << std::endl;
        }

        std::cout << "[parent process] read: " << res << std::endl;
    }
}