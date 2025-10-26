/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asmafrid <asmafrid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:01:28 by asmafrid          #+#    #+#             */
/*   Updated: 2025/10/22 11:54:24 by asmafrid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>       
#include <vector>      

class Channel;

class Client
{ 
        private: 
        
            int _fd;       
            std::string _nickname;
            std::string _username;
            std::string _realname;
            std::string _hostname;

            bool _hasPassword;                 
            bool _hasNick;
            bool _hasUser;
               
            std::string _inputBuffer;
            std::vector<Channel*> _joinedChannels;

        public:
                            //CONSTRUCTOR
            Client(int fd);
            ~Client();

                                        //authentication
                                        
                void    setPassword(bool val);
                void    setNickname(const std::string& nick);
                void    setUsername(const std::string& user, const std::string& real);
                bool    isFullyAuthenticated() const;

                                        //buffer managment
                void    appendToBuffer(const std::string& data);
                std::string extractMessage();
                bool    hasCompleteMessage() const;

                                        //channel management
                void    joinChannel(Channel* channel);
                void    leaveChannel(Channel* channel);
                bool    isInChannel(Channel* channel)  const;
                std::vector<Channel*> getChannels() const;

                                        //getters
                int getFd() const;
                std::string     getNickname() const;
                std::string     getUsername() const;
                std::string     getRealname() const;
                std::string     getPrefix()   const;

                                        //message
                void    sendMessage(const std::string& message);
};

#endif