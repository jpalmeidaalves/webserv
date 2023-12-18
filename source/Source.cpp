#include "../headers/Server.hpp"

Server::Server(){
}

Server::~Server(){
	//handle Control signal
	close(this->_socket);
	exit 0;
}

Server::Server(const std::string &configfile, const std::string &host, int port) : _port(port), _host(host){
	(void)configfile;
	
}

Server::start()
{
	this->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		std::cout << "Failed to create socket" << std::endl;
		exit 1;
	}
	if (bind())
}