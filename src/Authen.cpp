#include "../inc/Server.hpp"
#include "../inc/replies.hpp"


void Server::_sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}
void Server::handlePass(std::vector<std::string> &args, int fd)
{
    Client *cli = GetClient(fd);
    if (!cli)
        return;

    // 1️⃣ Check if enough arguments
    if (args.empty())
    {
        _sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), fd);
        return;
    }

    // 2️⃣ Check if client already registered
    if (cli->isFullyAuthenticated())
    {
        _sendResponse(ERR_ALREADYREGISTERED(GetClient(fd)->getNickname()), fd);
        return;
    }

    // 3️⃣ Validate password
    std::string pass = args[0];
    if (pass == password)
    {
        cli->setPassword(true);
        std::cout << "✅ Client " << fd << " registered successfully with PASS" << std::endl;
    }
    else
    {
        _sendResponse(ERR_INCORPASS(GetClient(fd)->getNickname()), fd);
    }
}
