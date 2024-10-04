#ifndef IRC_HPP
# define IRC_HPP

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define PORT 6667
#define BACKLOG 5
#define MAX_CLIENTS 3

#endif