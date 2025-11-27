#include "../inc/Server.hpp"
#include "../inc/replies.hpp"
#include "../inc/Channel.hpp"

void Server::handleKick(std::vector<std::string> &args, int fd)
{
    Client* kicker = GetClient(fd);
    if (!kicker || !kicker->isRegistered())
        return;

    // --- Check argument count ---
    if (args.size() < 2)
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, kicker->getNickname(), "KICK"), fd);
        return;
    }

    std::string channelName = args[0];
    std::string targetNick = args[1];
    std::string reason = (args.size() >= 3) ? args[2] : kicker->getNickname();

    // --- Find channel ---
    Channel* ch = GetChannel(channelName);
    if (!ch)
    {
        _sendResponse(ERR_NOSUCHCHANNEL(kicker->getNickname(), channelName), fd);
        return;
    }

    // --- Check if kicker is operator ---
    if (!ch->isOperator(kicker))
    {
        _sendResponse(ERR_CHANOPRIVSNEEDED(kicker->getNickname(), channelName), fd);
        return;
    }

    // --- Find target client ---
    Client* target = GetClientNick(targetNick);
    if (!target)
    {
        _sendResponse(ERR_NOSUCHNICK(kicker->getNickname(), targetNick), fd);
        return;
    }

    // --- Check if target is in the channel ---
    if (!ch->hasMember(target))
    {
        _sendResponse(ERR_USERNOTINCHANNEL(kicker->getNickname(), targetNick, channelName), fd);
        return;
    }

    // --- Remove target from channel ---
    ch->removeMember(target);

    // --- Construct KICK message ---
    std::string msg = ":" + kicker->getNickname() + "!" + kicker->getUsername() + "@" + kicker->getHostname() +
                      " KICK " + channelName + " " + target->getNickname() + " :" + reason + "\r\n";

    // --- Broadcast to channel members (excluding kicker) ---
    ch->broadcast(msg, nullptr); // Optional: exclude kicker if you want

    // --- Optionally send directly to target ---
    target->sendMessage(msg);
    kicker->sendMessage(msg); // Optional: send to kicker if you want to see confirmation
}