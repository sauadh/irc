NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98

SRCS = src/main.cpp src/Server.cpp src/Client.cpp src/Authen.cpp src/Channel.cpp src/Invite.cpp src/Topic.cpp src/kick.cpp src/mode.cpp
OBJS = $(SRCS:.cpp=.o)   # src/main.o src/Server.o src/Client.o

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compile rule: src/*.cpp -> src/*.o
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
