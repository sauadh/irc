#include "../inc/Server.hpp"
#include "../inc/replies.hpp"
#include "../inc/Channel.hpp"

void Server::handleMode(std::vector<std::string> &args, int fd)
{
    if (args.empty())
        return;

    Client* cli = GetClient(fd);
    if (!cli)
        return;

    std::string target = args[0];

    // --- CHANNEL MODE ---
    if (!target.empty() && target[0] == '#')
    {
        Channel* ch = nullptr;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            if (channels[i].getName() == target)
            {
                ch = &channels[i];
                break;
            }
        }

        if (!ch)
        {
           _sendResponse(ERR_NOSUCHCHANNEL(cli->getNickname(), target), fd);
            return;
        }

        // Query current channel modes
        if (args.size() == 1)
        {
            std::string currentModes = "";
            if (ch->isInviteOnly()) currentModes += "i";
            if (ch->isTopicRestricted()) currentModes += "t";
            if (ch->hasKey()) currentModes += "k";
            if (ch->hasUserLimit()) currentModes += "l";

            sendReply(fd, "324", target + " +" + currentModes); // RPL_CHANNELMODEIS
            return;
        }

        // Only operators can change modes
        if (!ch->isOperator(cli))
        {
            std::string msg = ":" + serverName + " 482 " + cli->getNickname() + " " + target + " :You're not channel operator\r\n";
            _sendResponse(msg, fd); // ERR_CHANOPRIVSNEEDED
            return;
        }

        std::string modes = args[1];
        std::vector<std::string> params(args.begin() + 2, args.end());
        bool add = true;
        size_t paramIndex = 0;
        std::string broadcastModes = "";

        for (size_t i = 0; i < modes.size(); ++i)
        {
            char c = modes[i];
            if (c == '+')
                 add = true;
            else if (c == '-')
                 add = false;
            else
            {
                switch (c)
                {
                    case 'i': ch->setInviteOnly(add); broadcastModes += c; break;
                    case 't': ch->setTopicRestricted(add); broadcastModes += c; break;
                    case 'k':
                        if (add && paramIndex < params.size()) ch->setKey(params[paramIndex++]);
                        else if (!add) ch->removeKey();
                        broadcastModes += c;
                        break;
                    case 'l':
                        if (add && paramIndex < params.size())
                        {
                            int limit = std::atoi(params[paramIndex++].c_str());
                            ch->setUserLimit(limit);
                        }
                        else if (!add) ch->removeUserLimit();
                        broadcastModes += c;
                        break;
                    case 'o':
                        if (paramIndex < params.size())
                        {
                            Client* targetClient = GetClientNick(params[paramIndex++]);
                            if (targetClient)
                            {
                                if (add) ch->addOperator(targetClient);
                                else ch->removeOperator(targetClient);
                                broadcastModes += c;
                            }
                            else 
                                {
                                std::string msg = ":" + serverName + " 401 " + cli->getNickname() + " " + params[paramIndex - 1] + " :No such nick\r\n";
                                _sendResponse(msg, fd);
                                }
                        }
                        break;
                    default: _sendResponse(ERR_UNKNOWNMODE(cli->getNickname(), )fd, "472", std::string(1, c)); break;
                }
            }
        }

        if (!broadcastModes.empty())
        {
            std::string msg = ":" + cli->getNickname() + "!" + cli->getUsername() + "@" +
                              cli->getHostname() + " MODE " + ch->getName() + " " + modes;
            for (size_t i = 0; i < params.size(); ++i)
                msg += " " + params[i];
            msg += "\r\n";
            ch->broadcast(msg, nullptr);
        }
    }
    else // --- USER MODE ---
    {
        Client* user = GetClientNick(target);
        if (!user)
        {
            sendError(fd, "401", target); // ERR_NOSUCHNICK
            return;
        }

        if (user != cli)
        {
            sendError(fd, "502", target); // ERR_USERSDONTMATCH
            return;
        }

        if (args.size() == 1)
        {
            sendReply(fd, "221", "+" + user->getModes()); // RPL_UMODEIS
            return;
        }

        std::string modes = args[1];
        bool add = true;
        for (size_t i = 0; i < modes.size(); ++i)
        {
            char c = modes[i];
            if (c == '+') add = true;
            else if (c == '-') add = false;
            else
            {
                switch (c)
                {
                    case 'i': user->setMode(c, add); break;
                    default: sendError(fd, "501", std::string(1, c)); 
                        break; // ERR_UMODEUNKNOWNFLAG
                }
            }
        }

        std::string msg = ":" + serverName + " 221 " + user->getNickname() + " +" + user->getModes() + "\r\n";
        _sendResponse(msg, fd);

    }
}
