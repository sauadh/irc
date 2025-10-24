#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include "Client.hpp"

#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Server
{
private:
	int port;
	int server_fd;
	std::string password;
	std::vector<struct pollfd> poll_fds;
	std::map<int, Client> clients;
	struct sockaddr_in address;
	void errorExit(const std::string &msg);

public:
//check if cannonical form needed
	Server();
	Server(int port, const std::string &password);
	~Server();

	void initSocket();
	void run();

	// signal + cleanup
	static void signalHandler(int signum);
	void closeAll();

	// client mgmt
	void acceptNewClient();
	void receiveClientData(int fd);
	void removeClient(int fd);

	// command handling
	void parseCommand(std::string &line, int fd);
	void handlePASS(std::vector<std::string> &args, int fd);
	void handleNICK(std::vector<std::string> &args, int fd);
	void handleUSER(std::vector<std::string> &args, int fd);
	void handleQUIT(std::vector<std::string> &args, int fd);

	const std::string &getPassword() const;
};

#endif
