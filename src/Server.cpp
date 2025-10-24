#include "../inc/Server.hpp"

Server::Server(int port, const std::string &password)
    : port(port), password(password), server_fd(-1), running(false)
{}

Server::~Server()
{
    if (server_fd != -1)
        close(server_fd);
    std::cout << YEL << "[Server] Closed successfully" << WHI << std::endl;
}

void Server::errorExit(const std::string &msg)
{
    std::cerr << RED << "[Error] " << msg << WHI << std::endl;
    if (server_fd != -1)
        close(server_fd);
    std::exit(EXIT_FAILURE);
}

void Server::initSocket()
{
    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        errorExit("socket() failed");

    // 2. Allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        errorExit("setsockopt() failed");

    // 3. Set non-blocking
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
        errorExit("fcntl() failed");

    // 4. Configure address
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 5. Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
        errorExit("bind() failed");

    // 6. Start listening
    if (listen(server_fd, 20) == -1)
        errorExit("listen() failed");

    // 7. Add to pollfd list
    struct pollfd pd;
    pd.fd = server_fd;
    pd.events = POLLIN;
    pd.revents = 0;
    poll_fds.push_back(pd);

    running = true;
    std::cout << GRE << "[Server] Listening on port " << port << WHI << std::endl;
}
