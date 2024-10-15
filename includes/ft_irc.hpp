#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# include <iostream>
# include <vector>
# include <stdexcept>
# include <netinet/in.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>
# include <poll.h>
# include <cstdlib>
# include <cstring>
# include <map>
# include <string>
# include <algorithm>
# include <cctype>
# include <sstream>
# include <ctime>

# define BACKLOG 5
# define MAX_CLIENTS 10

# include "Server.hpp"

std::string intToString(int number);
bool is_valid_nickname_char(char c);
bool is_valid_realname_char(char c);
bool user_in_channel(const std::map<std::string, Client*>& clients_in_channel, const std::string& nickname);

#endif
