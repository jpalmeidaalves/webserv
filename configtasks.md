# Config file tasks

## [✅] Choose the port and host of each ’server’.
```bash    
...
server {
    listen 80; # can have only the port
    ...
}
server {
    listen 127.0.0.1:80; # or the ip and port
    ...
}
...
```

### Nginx:

Valid values:
- 80
- 127.0.0.1:80
- 127.0.0.2:80
- 127.254.254.254:80
- 0.0.0.0:80
- ...:80
- 0.0..:80

Invalid values:
- :80
- .:80
- ..:80
- 127..:80

### Our Implementation:

Valid:
- 80 (port only)
- 127.0.0.1:80 (ip and port)

Note: to check this, if ':' is present must have a valid ip on the left and a valid port on the right

## [❌] Setup the server_names or not.
```bash
...
server {
    ...
    host myserver.com www.myserver.com myserver.net;
    ...
}

...
server {
    # host testing.com
}
...

# Note: it can be a list of names or not have it at all
```


## [❌] The first server for a host:port will be the default for this host:port (that means it will answer to all the requests that don’t belong to an other server).
```bash
...
server {
    listen 127.0.0.1:8000;
    host first.com;
    ...
}

server {
    listen 127.0.0.1:8000;
    host second.com
    ...
}

server {
    listen 127.0.0.1:8001;
    host alt1.com
    ...
}

server {
    listen 127.0.0.1:8001;
    host alt2.com
    ...
}
...

# Note: nginx has a flag that can change the default server (default_server at the end of the listen line) but we'll not use this here because the subject dictates that we must use the first server as the default one.

```
curl eg: `curl -X GET http://127.0.0.1:8000 -H "Host: third"`

Notice that the page we'll get is the page from the first server because we have no host by that name.


## [❌] Setup default error pages.
```bash
...
server {
    ...
    error_page 404 /custom_404.html;
    # location = /custom_404.html {
        # root /usr/share/nginx/html;
        # internal;
    # }

    error_page 500 502 503 504 /custom_50x.html;
    # location = /custom_50x.html {
        # root /usr/share/nginx/html;
        # internal;
    # }
    ...
}
...
```

## [❌] Limit client body size.
## [❌] Setup routes with one or multiple of the following rules/configuration (routes wont be using regexp):
## [❌] Define a list of accepted HTTP methods for the route.
## [❌] Define a HTTP redirection.
## [❌] Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet).
## [❌] Turn on or off directory listing.
## [❌] Set a default file to answer if the request is a directory.
## [❌] Execute CGI based on certain file extension (for example .php).
## [❌] Make it work with POST and GET methods.
## [❌] Make the route able to accept uploaded files and configure where they should be saved.


header ex:
http GET /
Host: one.com



127.0.0.1:8000 one.com alt.com (root /www/a)
127.0.0.1:8000 two.com (root /www/b)
127.0.0.1:8001 three.com (root /www/c)
127.0.0.1:8001 four.com (root /www/d)

127.0.0.1:8000 one.com two.com alt.com
127.0.0.1:8001 three.com four.com


1) Get list of servers from config file ✅

Servers
listen -> std::string "127.0.0.1:8000";
hosts -> std::vector<std::string> _hosts;
root -> std::string root;
...

2) iterate over listen from each server and call bind and listen (must check if it is already bound)

Bound Addresses (vector)
127.0.0.1:8000
127.0.0.1:8001


3) after epoll_wait

- sockfd (changed)

    a) get details about sockfd (getsockname); ✅
    b) iterater over server and find server matching ip and port;
    c) check if a server has a requested host;
    d) if not use the first server;


4) add server pointer
Incomming Connections
Request &
Response &
Server *