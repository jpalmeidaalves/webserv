#ifndef SERVER_HPP
#define SERVER_HPP


#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

class Server{

	private:
		int _socket;
		std::string _host;
		int _port;
		Server();
		struct socketinfo;
	public:
		Server(const std::string &configfile, const std::string &host, int port);
		~Server();

};




#endif/* SERVER_HPP */
