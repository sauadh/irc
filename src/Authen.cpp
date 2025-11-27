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

    // === Not enough parameters ===
    if (args.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, cli->getNickname(), "PASS"), fd);
        return;
    }

    // === PASS after registration ===
    if (cli->isRegistered() || cli->getisAuthenticated())
    {
        _sendResponse(ERR_ALREADYREGISTERED(serverName, cli->getNickname()), fd);
        return;
    }

    // === Check password ===
    std::string pass = args[0];

    if (pass == password)
    {
        cli->setPassword(true);
        std::cout << "Client " << fd << " provided correct PASS\n";
    }
    else
    {
        _sendResponse(ERR_PASSWDMISMATCH(serverName, cli->getNickname()), fd);
        // optionally disconnect wrong password: check if needed
        //closeClient(fd);
    }
    if (cli->getisAuthenticated() && !cli->getNickname().empty() && !cli->getUsername().empty())
    {
        cli->checkRegistration();
        if (cli->isRegistered())
        {
            _sendResponse(RPL_WELCOME(cli->getNickname(), cli->getUsername(), cli->getHostname()), fd);
        }
    }
}



bool Server::is_validNickname(std::string& nickname)
{
    if (nickname.empty())
        return false;

    // First char must be alphabetic per RFC
    if (!std::isalpha(nickname[0]))
        return false;

    // Ban leading special chars
    if (nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':')
        return false;

    // Remaining chars must be alnum or _
    for (size_t i = 1; i < nickname.size(); i++)
    {
        if (!std::isalnum(nickname[i]) && nickname[i] != '_')
            return false;
    }
    return true;
}



bool Server::nickNameInUse(std::string& nickname)
{
    std::string nickLower = nickname;
    std::transform(nickLower.begin(), nickLower.end(), nickLower.begin(), ::tolower);

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (!it->second)
            continue;

        std::string exist = it->second->getNickname();
        std::transform(exist.begin(), exist.end(), exist.begin(), ::tolower);

        if (exist == nickLower)
            return true;
    }
    return false;
}


void Server::handleNick(std::vector<std::string> &args, int fd)
{
    Client *cli = GetClient(fd);
    if (!cli)
        return;

    if (args.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, cli->getNickname(), "NICK"), fd);
        return;
    }

    std::string newNick = args[0];

    if (!is_validNickname(newNick))
    {
        _sendResponse(ERR_ERRONEUSNICK(newNick), fd);
        return;
    }

    // Case-insensitive collision check
    if (nickNameInUse(newNick))
    {
        std::string old = cli->getNickname();
        std::string lowerOld = old; 
        std::string lowerNew = newNick;
        std::transform(lowerOld.begin(), lowerOld.end(), lowerOld.begin(), ::tolower);
        std::transform(lowerNew.begin(), lowerNew.end(), lowerNew.begin(), ::tolower);

        if (lowerOld != lowerNew)
        {
            _sendResponse(ERR_NICKINUSE(newNick), fd);
            return;
        }
    }

    std::string oldNick = cli->getNickname();
    cli->setNickname(newNick);
    cli->checkRegistration();


    // --- BROADCAST nickname change ---
    if (!oldNick.empty() && oldNick != newNick)
    {
        std::string msg =
            ":" + oldNick + "!" + cli->getUsername() + "@" +
            cli->getHostname() + " NICK :" + newNick + "\r\n";

        broadcastToAll(msg);
    }
     if (cli->getisAuthenticated() && !cli->getUsername().empty() && oldNick.empty())
    {
        if (cli->isRegistered())
        {
            _sendResponse(RPL_WELCOME(newNick, cli->getUsername(), cli->getHostname()), fd);
        }
    }
}

void Server::handleUser(std::vector<std::string> &args, int fd)
{
    Client *cli = GetClient(fd);
    if (!cli)
        return;
    std::cout << cli->getRealname() <<std::endl;
    // USER <username> <mode> <unused> <realname>
    // Minimum needed: USER username 0 * :real name

    if (args.size() < 4)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, cli->getNickname(), "USER"), fd);
        return;
    }

    if (cli->isRegistered())
    {
       _sendResponse(ERR_ALREADYREGISTERED(serverName, cli->getNickname()), fd);
        return;
    }
    std::string realname;
    bool colonFind = false;
    for (size_t i = 0; i < args.size(); i++) {
        if (!colonFind && args[i][0] == ':') {
            colonFind = true;
        realname = args[i].substr(1); // remove ':'
    } else if (colonFind) {
        realname += " " + args[i];
    }
}

    std::string username = args[0];

    // Extract realname from the last argument
    // Realname always starts with ':'
    // Store username + realname
    cli->setUsername(username, realname);

    std::cout << "Client " << fd << " set USER = " << username
              << ", realname = " << realname << std::endl;

    // ========= COMPLETE REGISTRATION =========
    // Must have:
    // PASS (valid)
    // NICK
    // USER

    if (cli->getisAuthenticated() && !cli->getNickname().empty())
    {
        cli->checkRegistration();
        if (cli->isRegistered())
        {
            _sendResponse(RPL_WELCOME(cli->getNickname(), cli->getUsername(), cli->getHostname()), fd);
        }
    }
}


void Server::handleQuit(std::vector<std::string> &args, int fd)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    std::string reason = "Client Quit";
    if (!args.empty())
    {
        reason.clear();
        for (size_t i = 0; i < args.size(); i++)
        {
            if (i > 0)
                reason += " ";
            reason += args[i];
        }
        if (!reason.empty() && reason[0] == ':')
            reason.erase(0, 1);
    }

    std::string quitMsg =
        ":" + client->getNickname() + "!" +
        client->getUsername() + "@" +
        client->getHostname() + " QUIT :" +
        reason + "\r\n";

    std::cout << "[QUIT] " << quitMsg;

    // Broadcast to all channels the client is in
    const std::vector<Channel*> &channels = client->getChannels();
    for (size_t i = 0; i < channels.size(); i++)
    {
        channels[i]->broadcast(quitMsg, client);
        channels[i]->removeMember(client);
    }

    // Send quit message to the client (optional)
    _sendResponse(quitMsg, fd);

    // Finally remove client from server (closes fd, erases from maps, etc.)
    removeClient(fd);
}
