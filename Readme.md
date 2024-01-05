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

- **epoll (epoll_create, epoll_ctl, epoll_wait)** - handle multiple requests (nonblocking)

- **setsockopt** - get and set options on sockets

- **read** - 
- **write** - 
- **close** - 
- **signal** - 
- **send** - similar to write


### Known but will not be used
- **kqueue** (kqueue, kevent) - handle multiple connections in a queue (nonblocking). **Reason**: not available in Linux.
- **connect** - connect to a server (used in a client socket)
- **fcntl** - can be used to set fd to non-blocking. Maybe only allowed for MAC
- **select** - handle multiple requests (nonblocking)
- **poll** - handle multiple requests (nonblocking)
- **recv** - similar to read


### Unsed

#### Previously used in C
- **execve** - 
- **dup** - 
- **dup2** - 
- **pipe** - 
- **fork** - 
- **waitpid** - 
- **kill** - 
- **errno** - 
- **strerror** - 
- **open** - opens a file.
- **chdir** - change directory.
- **access** - check users permissions about a file.
- **stat** - informations about a file.
- **opendir** - opens a directory.
- **readdir** - returns a pointer to a dirent structure representing the next directory entry.
- **closedir** - closes a dir.

#### Others

- getaddrinfo - 
- **gai_strerror** - returns getaddrinfor error message
- freeaddrinfo - 

- **socketpair** - creates an unnamed pair of connected sockets in the specified domain,
                    of the specified type, and using the  optionally specified  protocol;
- **getsockname** - returns the current address to which the socket sockfd is
                    bound, in the buffer pointed to by addr.
- **getprotobyname** - returns a protoent structure for the entry from the database that
                       matches the protocol name name.

<!-- 
The protoent structure is defined in <netdb.h> as follows:

struct protoent {
    char  *p_name;       /* official protocol name */
    char **p_aliases;    /* alias list */
    int    p_proto;      /* protocol number */
} -->
