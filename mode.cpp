/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smuneer <smuneer@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/28 09:56:52 by smuneer           #+#    #+#             */
/*   Updated: 2025/11/28 09:56:53 by smuneer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/replies.hpp"
#include "../inc/Channel.hpp"

void Server::handleMode(std::vector<std::string> &args, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
        return;
        
    if (args.empty())
    {
        _sendResponse(ERR_NEEDMOREPARAMS(serverName, cli->getNickname(), "MODE"), fd);
        return;
    }


    std::string target = args[0];
    Channel* ch = NULL;

    // --- CHANNEL MODE ---
    if (!target.empty() && target[0] == '#')
    {
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
    }

    // ---- QUERY CURRENT CHANNEL MODES ----
    if (args.size() == 1)
    {
         std::string currentModes = "";
         if (ch->isInviteOnly())
             currentModes += "i";
         if (ch->isTopicRestricted())
            currentModes += "t";
        if (ch->hasKey())
            currentModes += "k";
        if (ch->hasUserLimit())
            currentModes += "l";
        _sendResponse(":" + serverName + " 324 " +
                      cli->getNickname() + " " +
                      ch->getName() + " +" + currentModes + "\r\n", fd);
        return;
    }

    // ---- PERMISSION CHECK ----
    if (!ch->isOperator(cli))
    {
        _sendResponse(":" + serverName + " 482 " +
                      cli->getNickname() + " " + target +
                      " :You're not channel operator\r\n", fd);
        return;
    }

    // ---- MODE PROCESSING ----
    std::string modes = args[1];
    std::vector<std::string> params(args.begin() + 2, args.end());

    bool add = true;
    size_t paramIndex = 0;
    std::string appliedModes;
    std::vector<std::string> appliedParams;

    for (size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];

        if (c == '+') { add = true; continue; }
        if (c == '-') { add = false; continue; }

        switch (c)
        {
            case 'i':
                ch->setInviteOnly(add);
                appliedModes += c;
                break;

            case 't':
                ch->setTopicRestricted(add);
                appliedModes += c;
                break;

            case 'k':
                if (add)
                {
                    if (paramIndex >= params.size()) continue;
                    ch->setKey(params[paramIndex]);
                    appliedParams.push_back(params[paramIndex]);
                    paramIndex++;
                }
                else
                {
                    ch->removeKey();
                }
                appliedModes += c;
                break;

            case 'l':
                if (add)
                {
                    if (paramIndex >= params.size()) continue;
                    ch->setUserLimit(std::atoi(params[paramIndex].c_str()));
                    appliedParams.push_back(params[paramIndex]);
                    paramIndex++;
                }
                else
                {
                    ch->removeUserLimit();
                }
                appliedModes += c;
                break;

            case 'o':
                if (paramIndex >= params.size()) continue;

                if (Client* targetUser = GetClientNick(params[paramIndex]))
                {
                    if (add) ch->addOperator(targetUser);
                    else     ch->removeOperator(targetUser);

                    appliedModes += c;
                    appliedParams.push_back(params[paramIndex]);
                }
                else
                {
                    _sendResponse(":" + serverName + " 401 " +
                                  cli->getNickname() + " " + params[paramIndex] +
                                  " :No such nick\r\n", fd);
                }

                paramIndex++;
                break;

            default:
                _sendResponse(ERR_UNKNOWNMODE(cli->getNickname(),
                                              ch->getName(),
                                              std::string(1, c)),
                              fd);
                break;
        }
    }

    // ---- BROADCAST APPLIED MODES ----
    if (!appliedModes.empty())
    {
        std::string msg = ":" + cli->getPrefix() + " MODE " + ch->getName() + " +";
        msg += appliedModes;

        for (size_t i = 0; i < appliedParams.size(); i++)
            msg += " " + appliedParams[i];

        msg += "\r\n";
        ch->broadcast(msg, NULL);
    }


    return;
}
