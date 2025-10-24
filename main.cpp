#include inc/Server.hpp

int main(int av, char **argc)
{
    if(av != 3)
    {
        std::cout << "Format:" << argv[0] << "<port number>  <password>" << std::endl;
        return (1);
    }
    int port = std::atoi(argc[1]);
    //check if port limit is needed.
    std::string password = argc[2];
    Server server(port, password);
    server.initSocket();
    
    return (0);
}