#include "../inc/Server.hpp"

bool Server::Serversignal = false;

Server::Server(int port, const std::string &password)
    : port(port), server_fd(-1), password(password) {}

Server::~Server()
{
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    clients.clear();

    if (server_fd != -1)
        close(server_fd);
    std::cout << "[Server] Closed successfully" << std::endl;
}


void Server::errorExit(const std::string &msg)
{
    std::cerr << "[Error] " << msg  << std::endl;
    if (server_fd != -1)
        close(server_fd);
    std::exit(EXIT_FAILURE);
}

void Server::removeClient(int fd)
{
    std::cout << "[Server] Client is disconnected (fd =" << fd << ")" << std::endl;

    close(fd);

    // Delete the client object first
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it != clients.end())
    {
        delete it->second;
        clients.erase(it);
    }

    // Remove from poll_fds
    for (size_t i = 0; i < poll_fds.size(); i++)
    {
        if (poll_fds[i].fd == fd)
        {
            poll_fds.erase(poll_fds.begin() + i);
            break;
        }
    }
}


void Server::signalHandler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Signal Received!" << std::endl;
    Server::Serversignal = true;
}

void Server::non_block(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        errorExit("fcntl() failed");
}

void Server::initSocket()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        errorExit("socket() failed");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        errorExit("setsockopt() failed");

    non_block(server_fd);

    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
        errorExit("bind() failed");

    if (listen(server_fd, 20) == -1)
        errorExit("listen() failed");

    struct pollfd pd;
    pd.fd = server_fd;
    pd.events = POLLIN;
    pd.revents = 0;
    poll_fds.push_back(pd);

    std::cout  << "[Server] Listening on port " << port  << std::endl;
}

void Server::run()
{
    while (Server::Serversignal == false)
    {
        if ((poll(&poll_fds[0], poll_fds.size(), -1) == -1) && Server::Serversignal == false)
            errorExit("poll() failed");

        for (int i = poll_fds.size() - 1; i >= 0; --i)
        {
            int fd = poll_fds[i].fd;
            if (poll_fds[i].revents & POLLIN)
            {
                if (fd == server_fd)
                    this->acceptNewClient();
                else
                    this->receiveClientData(fd);
            }
            else if (poll_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                removeClient(fd);
        }
    }
}

void Server::acceptNewClient()
{
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    int clfd = accept(server_fd, (sockaddr *)&cliaddr, &len);
    if (clfd == -1)
    {
        std::cout << "accept() failed" << std::endl;
        return;
    }

    non_block(clfd);

    struct pollfd pd;
    pd.fd = clfd;
    pd.events = POLLIN;
    pd.revents = 0;
    poll_fds.push_back(pd);

    // ✅ Step 1: create the Client directly in-place
    clients[clfd] = new Client(clfd);

    // ✅ Step 2: log connection
    std::cout << "[Server] Client <" << clfd << "> Connected from "
              << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port)
              << std::endl;
}


void Server::receiveClientData(int fd)
{
    char buff[1024];
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    if (bytes <= 0)
    {
        removeClient(fd);
        return;
    }

    buff[bytes] = '\0';

    // Use .at() to avoid default construction
    Client* client = clients.at(fd);
client->appendToBuffer(buff);

while (client->hasCompleteMessage())
{
    std::string msg = client->extractMessage();
    std::cout << "[Server] Received from fd=" << fd << ": " << msg << std::endl;
    //parseCommand(msg, fd);
}

 }

 Client* Server::GetClient(int fd)
{
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it != clients.end())
        return it->second;  // Found → return the Client pointer
    return NULL;            // Not found → return null
}


//  bool Server::isRegistered(int fd)
// {
//     Client* client = GetClient(fd);
//     if (!client)
//         return false;

//     if (client->getNickname().empty() ||
//         client->getUsername().empty() ||
//         client->getNickname() == "*" ||
//         !client->isFullyAuthenticated())
//         return false;

//     return true;
// }


//  void Server::parseCommand(std::string &line, int fd)
// {
//     if (line.empty())
//         return;

//     // --- Trim leading/trailing spaces ---
//     size_t start = line.find_first_not_of(" \t\r\n");
//     size_t end = line.find_last_not_of(" \t\r\n");
//     if (start == std::string::npos)
//         return;
//     line = line.substr(start, end - start + 1);

//     // --- Split into command and args ---
//     size_t spacePos = line.find(' ');
//     std::string command;
//     std::vector<std::string> args;

//     if (spacePos != std::string::npos)
//     {
//         command = line.substr(0, spacePos);
//         std::string rest = line.substr(spacePos + 1);

//         // split args by spaces
//         size_t pos = 0;
//         while ((pos = rest.find(' ')) != std::string::npos)
//         {
//             if (pos > 0)
//                 args.push_back(rest.substr(0, pos));
//             rest.erase(0, pos + 1);
//         }
//         if (!rest.empty())
//             args.push_back(rest);
//     }
//     else
//         command = line;

//     // --- Convert command to uppercase (IRC is case-insensitive) ---
//     for (size_t i = 0; i < command.size(); ++i)
//         command[i] = toupper(command[i]);

//     // --- Registration commands ---
//     if (command == "PASS")
//         handlePass(args, fd);
//     else if (command == "NICK")
//         handleNick(args, fd);
//     else if (command == "USER")
//         handleUser(args, fd);
//     else if (command == "QUIT")
//         handleQuit(args, fd);

//     // --- Only allow other commands if registered ---
//     else if (isRegistered(fd)) // renamed from notregistered()
//     {
//         if (command == "JOIN")
//             handleJoin(args, fd);
//         else if (command == "PART")
//             handlePart(args, fd);
//         else if (command == "PRIVMSG")
//             handlePrivmsg(args, fd);
//         else if (command == "TOPIC")
//             handleTopic(args, fd);
//         else if (command == "KICK")
//             handleKick(args, fd);
//         else if (command == "MODE")
//             handleMode(args, fd);
//         else if (command == "INVITE")
//             handleInvite(args, fd);
//         else
//             sendClientMessage(fd, "421 " + command + " :Unknown command");
//     }
//     else
//     {
//         // Not yet registered
//         sendClientMessage(fd, "451 * :You have not registered");
//     }
// }

