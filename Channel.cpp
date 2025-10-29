/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asmafrid <asmafrid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 10:51:08 by asmafrid          #+#    #+#             */
/*   Updated: 2025/10/28 15:05:32 by asmafrid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Channel.hpp"
#include "Client.hpp"  
#include <algorithm>
#include <iostream>

// ===== CONSTRUCTOR =====
Channel::Channel(const std::string& name)
    : _name(name),            
      _topic(""),              
      _inviteOnly(false),      
      _topicRestricted(true),  
      _hasKey(false),  
      _key(""),         
      _hasUserLimit(false),     
      _userLimit(0)
{
    std::cout << "Channel created: " << _name << std::endl;
}

// ===== DESTRUCTOR =====
Channel::~Channel() 
{
    std::cout << "Channel destroyed: " << _name << std::endl;
}

// ===== ADD MEMBER =====
void Channel::addMember(Client* client) 
{
    if (hasMember(client)) 
    {
        return;
    }
    
    _members.push_back(client);
    
    client->joinChannel(this);
    
    // IMPORTANT: First member becomes operator!
    if (_members.size() == 1) 
    {
        addOperator(client);
        std::cout << client->getNickname() << " is now operator of " 
                  << _name << std::endl;
    }
    
    std::cout << client->getNickname() << " joined " << _name << std::endl;
}

// ===== REMOVE MEMBER =====
void Channel::removeMember(Client* client) 
{
    std::vector<Client*>::iterator it = 
        std::find(_members.begin(), _members.end(), client);
    
    if (it != _members.end()) 
    {
        _members.erase(it);
        
        client->leaveChannel(this);
        
        removeOperator(client);
        
        removeFromInviteList(client);
        
        std::cout << client->getNickname() << " left " << _name << std::endl;
    }
}

// ===== CHECK IF MEMBER =====
bool Channel::hasMember(Client* client) const 
{
    return std::find(_members.begin(), _members.end(), client) 
           != _members.end();
}

// ===== GET MEMBER COUNT =====
size_t Channel::getMemberCount() const 
{
    return _members.size();
}

// ===== GET ALL MEMBERS =====
std::vector<Client*> Channel::getMembers() const 
{
    return _members;
}

// ===== ADD OPERATOR =====
void Channel::addOperator(Client* client) 
{
    if (!hasMember(client))
    {
        return;
    }
    
    if (isOperator(client)) 
    {
        return;
    }
    
    _operators.push_back(client);
    
    std::cout << client->getNickname() << " is now operator of " 
              << _name << std::endl;
}

// ===== REMOVE OPERATOR =====
void Channel::removeOperator(Client* client) {
    std::vector<Client*>::iterator it = 
        std::find(_operators.begin(), _operators.end(), client);
    
    if (it != _operators.end()) {
        _operators.erase(it);
        std::cout << client->getNickname() << " is no longer operator of " 
                  << _name << std::endl;
    }
}

// ===== CHECK IF OPERATOR =====
bool Channel::isOperator(Client* client) const 
{
    return std::find(_operators.begin(), _operators.end(), client) 
           != _operators.end();
}

// ===== BROADCAST MESSAGE =====
// This is the HEART of IRC channels!
void Channel::broadcast(const std::string& message, Client* exclude) 
{
    for (size_t i = 0; i < _members.size(); i++) 
    {
        if (_members[i] != exclude) 
        {
            _members[i]->sendMessage(message);
        }
    }
}

// ===== SET TOPIC =====
void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
    std::cout << "Topic for " << _name << " set to: " << topic << std::endl;
}

// ===== GET TOPIC =====
std::string Channel::getTopic() const
{
    return _topic;
}

// ===== MODE: INVITE-ONLY (i) =====
void Channel::setInviteOnly(bool val)
{
    _inviteOnly = val;
    if (val)
        std::cout << _name << " is now invite-only" << std::endl;
    else
        std::cout << _name << " is no longer invite-only" << std::endl;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

// ===== MODE: TOPIC RESTRICTED (t) =====
void Channel::setTopicRestricted(bool val)
{
    _topicRestricted = val;
}

bool Channel::isTopicRestricted() const 
{
    return _topicRestricted;
}

// ===== MODE: KEY/PASSWORD (k) =====
void Channel::setKey(const std::string& key)
{
    _key = key;
    _hasKey = true;
    std::cout << _name << " now has a password" << std::endl;
}

void Channel::removeKey() 
{
    _key = "";
    _hasKey = false;
    std::cout << _name << " password removed" << std::endl;
}

bool Channel::hasKey() const 
{
    return _hasKey;
}

std::string Channel::getKey() const 
{
    return _key;
}

bool Channel::checkKey(const std::string& key) const 
{
    // If no key set, any key is OK
    if (!_hasKey)
        return true;
    
    // Check if provided key matches
    return (_key == key);
}

// ===== MODE: USER LIMIT (l) =====
void Channel::setUserLimit(int limit) 
{
    _userLimit = limit;
    _hasUserLimit = true;
    std::cout << _name << " user limit set to " << limit << std::endl;
}

void Channel::removeUserLimit() 
{
    _userLimit = 0;
    _hasUserLimit = false;
    std::cout << _name << " user limit removed" << std::endl;
}

bool Channel::hasUserLimit() const 
{
    return _hasUserLimit;
}

int Channel::getUserLimit() const 
{
    return _userLimit;
}

bool Channel::isFull() const 
{
    if (!_hasUserLimit)
        return false;  // No limit = never full
    
    return ((int)_members.size() >= _userLimit);
}

// ===== ADD TO INVITE LIST =====
void Channel::addToInviteList(Client* client) 
{
    if (!isInvited(client)) 
    {
        _inviteList.push_back(client);
        std::cout << client->getNickname() << " invited to " << _name << std::endl;
    }
}

// ===== REMOVE FROM INVITE LIST =====
void Channel::removeFromInviteList(Client* client) 
{
    std::vector<Client*>::iterator it = 
        std::find(_inviteList.begin(), _inviteList.end(), client);
    
    if (it != _inviteList.end()) 
    {
        _inviteList.erase(it);
    }
}

// ===== CHECK IF INVITED =====
bool Channel::isInvited(Client* client) const 
{
    return std::find(_inviteList.begin(), _inviteList.end(), client) 
           != _inviteList.end();
}

// ===== CAN CLIENT JOIN? =====
bool Channel::canJoin(Client* client, const std::string& key) const 
{
    if (hasMember(client)) 
    {
        return false;
    }
    
    if (_inviteOnly && !isInvited(client)) 
    {
        return false;  
    }
    
    if (isFull()) 
    {
        return false;
    }
    
    if (!checkKey(key)) 
    {
        return false; 
    }
    return true;
}


// ===== GET NAME =====
std::string Channel::getName() const 
{
    return _name;
}