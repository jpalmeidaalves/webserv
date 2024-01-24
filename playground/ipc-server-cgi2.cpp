#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    int pipe_parent_to_child[2];
    int pipe_child_to_parent[2];
    int returnstatus1, returnstatus2;

    returnstatus1 = pipe(pipe_parent_to_child);
    if (returnstatus1 == -1) {
        printf("Unable to create pipe 1 \n");
        exit(1);
    }
    returnstatus2 = pipe(pipe_child_to_parent);
    if (returnstatus2 == -1) {
        printf("Unable to create pipe 2 \n");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {                     // child 1
        close(pipe_parent_to_child[1]); // Close the unwanted pipe1 write side
        close(pipe_child_to_parent[0]); // Close the unwanted pipe2 read side

        dup2(pipe_parent_to_child[0], STDIN_FILENO); // rediricet STD IN to pipe_parent_to_child

        dup2(pipe_child_to_parent[1], STDOUT_FILENO);
        dup2(pipe_child_to_parent[1], STDERR_FILENO);

        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);

        char *cmd[] = {(char *)"/usr/bin/php-cgi", (char *)"test.php", NULL};

        // ! Basically content type was wrong!!!

        char *env[] = {(char *)"REDIRECT_STATUS=200",      (char *)"PATH_TRANSLATED=./test.php",
                       (char *)"SERVER_PROTOCOL=HTTP/1.1", (char *)"CONTENT_LENGTH=17",
                       (char *)"REQUEST_METHOD=POST",      (char *)"SCRIPT_FILENAME=./test.php",
                       (char *)"PATH_INFO=./test.php",     (char *)"CONTENT_TYPE=application/x-www-form-urlencoded",
                       (char *)"QUERY_STRING=page=35"};

        if (execve(cmd[0], cmd, env) == -1)
            std::cout << "execve ls failed" << std::endl;

    } else if (pid > 0) {               // parent
        close(pipe_parent_to_child[0]); // Close the unwanted pipe1 write side
        close(pipe_child_to_parent[1]); // Close the unwanted pipe2 read side

        int fd = open("body.txt", O_RDONLY);
        if (fd < 0) {
            std::cerr << "failed to open" << std::endl;
            exit(1);
        }
        // char body[] = "user=nuno&age=35";

        // if (write(pipe_parent_to_child[1], body, 17) <= 0) {
        //     std::cout << "failed to send the body to the CGI" << std::endl;
        // }

        // std::cout << "[parent process] wrote to child process: " << body << std::endl;

        dup2(pipe_parent_to_child[1], fd);
        close(pipe_parent_to_child[1]);

        (fd);
        char res[1000];
        memset(res, 0, 1000);
        if (read(pipe_child_to_parent[0], res, 1000) <= 0) {
            std::cout << "failed to read data from the CGI" << std::endl;
        }
        std::cout << "TEST" << std::endl;

        close(pipe_child_to_parent[0]);

        std::cout << "[parent process] read: " << res << std::endl;
    }
}