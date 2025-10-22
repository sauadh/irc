/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asmafrid <asmafrid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 11:24:02 by asmafrid          #+#    #+#             */
/*   Updated: 2025/10/22 11:55:34 by asmafrid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <sys/socket.h>

                //constructor

Client::Client(int fd)
        :   _fd(fd),
            _nickname(""),
            _username(""),
            _realname(""),
            _hostname("localhost"),
            _hasPassword(false),
            _hasNick(false),
            _hasUser(false),
            _inputBuffer("")
            {
                std::cout << "Client Created with fd: " << fd << std::endl;
            }


                    //destruvtor
Client::~Client()
            {
                std::cout << "Client destroyed: " << _nickname << std::endl;
            }


                    //authentication
void Client::setPassword(bool val)
        {
                _hasPassword = val;
        }

        void Client::setNickname(const std::string& nick)
        {
            _nickname = nick;
            _hasNick  = true;
        }

        void Client::setUsername(const std::string& user, const std::string& real)
        {
            _username = user;
            _realname = real;
            _hasUser  = true;
        }

        bool Client::isFullyAuthenticated() const
        {
            return _hasPassword && _hasNick && _hasUser;
        }

                //buffer management
        
void Client::appendToBuffer(const std::string& data)
        {
            _inputBuffer += data;
        }

        bool Client::hasCompleteMessage() const  //irc message ends with \r\n or \n
        {
            return  (_inputBuffer.find("\r\n") != std::string::npos || _inputBuffer.find("\n") != std::string::npos);
        }

        std::string Client::extractMessage()    //first complete message
        {
            size_t pos = _inputBuffer.find("\r\n");
            if (pos == std::string::npos)
            {
                pos = _inputBuffer.find("\n");
                if(pos == std::string::npos)
                return "";
            }
                    //extract the message
            std::string message = _inputBuffer.substr(0, pos);

                    //remove from buffer including the delimiter
            if(_inputBuffer[pos] == '\r')
               _inputBuffer.erase(0, pos + 2); // remove \r\n
            else
              _inputBuffer.erase(0, pos + 1);  //remove \name

              return message;
        }

                                //channel management
        
void Client::joinChannel(Channel* channel)
        {
            if(!isInChannel(channel))
            {
                _joinedChannels.push_back(channel);
            }
        }

        void Client::leaveChannel(Channel* channel)
        {
            std::vector<Channel*>::iterator it = std::find(_joinedChannels.begin(), _joinedChannels.end(), channel);

            if(it != _joinedChannels.end())
            {
                _joinedChannels.erase(it);
            }
        }

        bool Client::isInChannel(Channel* channel) const
        {
            return std::find(_joinedChannels.begin(),
                             _joinedChannels.end(),
                            channel) != _joinedChannels.end();
        }

        std::vector<Channel*> Client::getChannels() const
        {
            return _joinedChannels;
        }

                            //getters

int Client::getFd() const
    {
        return _fd;
    }

    std::string Client::getNickname() const
    {
        return _nickname;
    }

    std::string Client::getUsername() const
    {
        return _username;
    }

    std::string Client::getRealname() const
    {
        return _realname;
    }

    std::string Client::getPrefix() const   //Irc format: :nickname!username@hostname
    {
        return ":" + _nickname + "!" + _username + "@" + _hostname;
    }

                            //  messaging

void Client::sendMessage(const std::string& message)
    {
        std::string fullMessage = message;

            //ensure message ends with \r\n
        if (fullMessage.size() < 2 || fullMessage.substr(fullMessage.size() - 2) != "\r\n")
        {
            fullMessage += "\r\n";
        }
            //send via socket
        send(_fd, fullMessage.c_str(), fullMessage.length(), 0);
    }
