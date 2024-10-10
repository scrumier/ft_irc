#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# include <iostream>
# include <vector>
# include <stdexcept>
# include <netinet/in.h>
# include <sys/types.h>       // Added for socket types
# include <sys/socket.h>
# include <arpa/inet.h>       // Added for htonl, htons, ntohl, ntohs functions
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
// include for RPL_msg
# define RPL_WELCOME 001
# define RPL_YOURHOST 002
# define RPL_CREATED 003
# define RPL_MYINFO 004
# define RPL_ISUPPORT 005

# define BACKLOG 5
# define MAX_CLIENTS 10

# include "Server.hpp"

std::string intToString(int number);
bool is_valid_nickname_char(char c);
bool is_valid_realname_char(char c);

#endif // FT_IRC_HPP
