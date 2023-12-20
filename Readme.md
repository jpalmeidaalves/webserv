## Functions Notes

https://www.youtube.com/watch?v=XXfdzwEsxFk

### Used
- **socket** - creates a socket and return a socket fd.
- **bind** - binds a socket to an ip and port.
- **listen** - set a socket to passive mode, listening for incomming requests.
- **accept** - accepts an incomming connection.

- **htons** - convert host to network short. 
- **htonl** - convert host to network long.
- **ntohs** - convert network to host short.
- **ntohl** - convert network to host long.


### Known but will not be used
- **kqueue** (kqueue, kevent) - handle multiple connections in a queue (nonblocking). **Reason**: not available in Linux.
- **connect** - connect to a server (used in a client socket)
- **fcntl** - can be used to set fd to non-blocking. Maybe only allowed for MAC


### Unsed

#### Previously used in C
- **execve** - 
- **dup** - 
- **dup2** - 
- **pipe** - 
- **fork** - 
- **close** - 
- **read** - 
- **write** - 
- **waitpid** - 
- **kill** - 
- **signal** - 
- **errno** - 
- **strerror** - 
- **open** - opens a file.
- **chdir** - change directory.
- **access** - check users permissions about a file.
- **stat** - informations about a file.


#### Others

- **select** - handle multiple requests (nonblocking)
- **poll** - handle multiple requests (nonblocking)
- **epoll (epoll_create, epoll_ctl, epoll_wait)** - handle multiple requests (nonblocking)

- getaddrinfo - 
- **gai_strerror** - returns getaddrinfor error message
- freeaddrinfo - 

- **opendir** - opens a directory.
- **readdir** - returns a pointer to a dirent structure representing the next directory entry.
- **closedir** - closes a dir.

- socketpair - 
- send - 
- recv - 
- setsockopt - 
- getsockname - 
- getprotobyname - 