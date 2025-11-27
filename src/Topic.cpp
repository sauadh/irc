#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"

void Server::handleTopic(std::vector<std::string> &args, int fd)
{
    Client *client = GetClient(fd);
    if (!client)
        return;

    if (args.empty())
    {
        _sendResponse("461 " + client->getNickname() + " TOPIC :Not enough parameters\r\n", fd);
        return;
    }

    std::string channelName = args[0];

    Channel *channel = GetChannel(channelName);
    if (!channel)
    {
        _sendResponse("403 " + client->getNickname() + " " + channelName + " :No such channel\r\n", fd);
        return;
    }

    if (!channel->hasMember(client))
    {
        _sendResponse("442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n", fd);
        return;
    }

    // Query topic
    if (args.size() == 1)
    {
        std::string topic = channel->getTopic();
        if (topic.empty())
            _sendResponse("331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n", fd);
        else
            _sendResponse("332 " + client->getNickname() + " " + channelName + " :" + topic + "\r\n", fd);
        return;
    }

    // Set new topic
    std::string newTopic = args[1];
    for (size_t i = 2; i < args.size(); ++i)
        newTopic += " " + args[i];
    if (!newTopic.empty() && newTopic[0] == ':')
        newTopic.erase(0, 1);

    if (channel->isTopicRestricted() && !channel->isOperator(client))
    {
        _sendResponse("482 " + client->getNickname() + " " + channelName +
                      " :You're not channel operator\r\n", fd);
        return;
    }

    channel->setTopic(newTopic);

    std::string msg =
        ":" + client->getNickname() + "!" +
        client->getUsername() + "@" +
        client->getHostname() + " TOPIC " +
        channelName + " :" + newTopic + "\r\n";

    channel->broadcast(msg);
}
