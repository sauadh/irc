/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRC.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asmafrid <asmafrid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:20:31 by asmafrid          #+#    #+#             */
/*   Updated: 2025/10/28 15:20:34 by asmafrid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef IRC_HPP
#define IRC_HPP

// ===== STANDARD C++ LIBRARIES =====
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

// ===== SYSTEM LIBRARIES (C++98 Compatible) =====
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>

// ===== PROJECT HEADERS =====
#include "Client.hpp"
#include "Channel.hpp"

// ===== IRC PROTOCOL CONSTANTS =====

// Standard IRC Port
#define IRC_DEFAULT_PORT 6667

// IRC Message Limits (RFC 1459)
#define IRC_MAX_MESSAGE_LENGTH 512
#define IRC_MAX_NICKNAME_LENGTH 9
#define IRC_MAX_CHANNEL_NAME_LENGTH 50

// IRC Numeric Replies (Most Common)
// Connection Registration
#define RPL_WELCOME             "001"
#define RPL_YOURHOST            "002"
#define RPL_CREATED             "003"
#define RPL_MYINFO              "004"

// Channel Operations
#define RPL_NOTOPIC             "331"
#define RPL_TOPIC               "332"
#define RPL_NAMREPLY            "353"
#define RPL_ENDOFNAMES          "366"

// Command Responses
#define RPL_INVITING            "341"

// Errors
#define ERR_NOSUCHNICK          "401"
#define ERR_NOSUCHCHANNEL       "403"
#define ERR_CANNOTSENDTOCHAN    "404"
#define ERR_NORECIPIENT         "411"
#define ERR_NOTEXTTOSEND        "412"
#define ERR_UNKNOWNCOMMAND      "421"
#define ERR_NONICKNAMEGIVEN     "431"
#define ERR_ERRONEUSNICKNAME    "432"
#define ERR_NICKNAMEINUSE       "433"
#define ERR_USERNOTINCHANNEL    "441"
#define ERR_NOTONCHANNEL        "442"
#define ERR_USERONCHANNEL       "443"
#define ERR_NOTREGISTERED       "451"
#define ERR_NEEDMOREPARAMS      "461"
#define ERR_ALREADYREGISTERED   "462"
#define ERR_PASSWDMISMATCH      "464"
#define ERR_CHANNELISFULL       "471"
#define ERR_INVITEONLYCHAN      "473"
#define ERR_BADCHANNELKEY       "475"
#define ERR_CHANOPRIVSNEEDED    "482"

// ===== COLOR CODES (For Console Output) =====
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// ===== UTILITY MACROS =====
#define LOG(msg) std::cout << COLOR_CYAN << "[LOG] " << COLOR_RESET << msg << std::endl
#define ERROR(msg) std::cerr << COLOR_RED << "[ERROR] " << COLOR_RESET << msg << std::endl
#define SUCCESS(msg) std::cout << COLOR_GREEN << "[SUCCESS] " << COLOR_RESET << msg << std::endl
#define WARNING(msg) std::cout << COLOR_YELLOW << "[WARNING] " << COLOR_RESET << msg << std::endl

// ===== HELPER FUNCTIONS (Declarations) =====

// String utilities
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);

// IRC message formatting
std::string formatReply(const std::string& code, 
                       const std::string& client, 
                       const std::string& message);

std::string formatMessage(const std::string& prefix,
                          const std::string& command,
                          const std::string& params);

// Validation
bool isValidNickname(const std::string& nick);
bool isValidChannelName(const std::string& name);

#endif