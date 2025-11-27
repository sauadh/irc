#include "../inc/Server.hpp"
#include "../inc/replies.hpp"


void Server::handleJoin(std::vector<std::string> &args, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
        return;

    if (args.empty())
    {
        _sendResponse(ERR_NOTREGISTERED(cli->getNickname()), fd);
        return;
    }

    std::string channelName = args[0];
    std::string key = (args.size() > 1) ? args[1] : "";

    // Find channel
    Channel* channel = NULL;
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i].getName() == channelName)
        {
            channel = &channels[i];
            break;
        }
    }
    // If not found, create it
    if (!channel)
    {
        Channel newChannel(channelName);
        AddChannel(newChannel);
        channel = &channels.back();
    }

    // Check if client can join
    if (!channel->canJoin(cli, key))
    {
        _sendResponse(": " + cli->getNickname() + " cannot join " + channelName + CRLF, fd);
        return;
    }

    // Add client
    channel->addMember(cli);

    // Send JOIN to client
    _sendResponse(":" + cli->getPrefix() + " JOIN :" + channelName + CRLF, fd);

    // Optionally, broadcast to other members
    channel->broadcast(":" + cli->getPrefix() + " JOIN :" + channelName + CRLF, cli);
}


void Server::handleInvite(std::vector<std::string> &args, int fd)
{
    Client *inviter = GetClient(fd);
    if (!inviter)
        return;

    // === ERR_NEEDMOREPARAMS (461) ===
    if (args.size() < 2)
    {
        _sendResponse("461 " + inviter->getNickname() + " INVITE :Not enough parameters\r\n", fd);
        return;
    }

    std::string targetNick = args[0];
    if (!targetNick.empty() && targetNick[0] == ':')
        targetNick.erase(0, 1);
    std::string channelName = args[1];

    // === Validate channel format ===
    if (channelName.empty() || channelName[0] != '#')
    {
         _sendResponse(ERR_CHANNELNOTFOUND(inviter->getNickname(), channelName), fd);
    }

    // === Find channel ===
    Channel *channel = GetChannel(channelName);
    if (!channel)
    {
         _sendResponse(ERR_CHANNELNOTFOUND(inviter->getNickname(), channelName), fd);
        return;
    }

    // === Check inviter is in channel ===
    if (!channel->hasMember(inviter))
    {
        _sendResponse("442 " + inviter->getNickname() + " " + channelName + " :You're not on that channel\r\n", fd);
        return;
    }

    // === Find target user ===
    Client *target = GetClientNick(targetNick);
    if (!target)
    {
        _sendResponse("401 " + inviter->getNickname() + " " + targetNick + " :No such nick\r\n", fd);
        return;
    }

    // === Check if already in channel ===
    if (channel->hasMember(target))
    {
        _sendResponse("443 " + target->getNickname() + " " + channelName + " :is already on channel\r\n", fd);
        return;
    }

    // === Check invite-only mode ===
    if (channel->isInviteOnly() && !channel->isOperator(inviter))
    {
        _sendResponse("482 " + inviter->getNickname() + " " + channelName + " :You're not channel operator\r\n", fd);
        return;
    }

    // === Check user limit ===
    if (channel->isFull())
    {
        _sendResponse("471 " + channelName + " :Cannot join channel (+l)\r\n", fd);
        return;
    }

    // === Add target to invite list ===
    channel->addToInviteList(target);

    // === Reply to inviter (RPL_INVITING = 341) ===
    std::string rep1 = "341 " + inviter->getNickname() + " " +
                       target->getNickname() + " " + channelName + "\r\n";
    _sendResponse(rep1, fd);

    // === Notify the invited user ===
    std::string rep2 = ":" + inviter->getNickname() + "!" + inviter->getHostname() +
                       " INVITE " + target->getNickname() + " :" + channelName + "\r\n";

    _sendResponse(rep2, target->getFd());
}

