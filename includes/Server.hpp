#ifndef IRC_HPP
# define IRC_HPP

#include <iostream>
#include <vector>
#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <algorithm>
#include <cctype>

#define BACKLOG 5
#define MAX_CLIENTS 10



class Server {
public:
    typedef void (Server::*CommandHandler)(int, const std::string&);
    Server(int port, const std::string& password);
    Server();
    ~Server();
    const Server& operator=(const Server& other);
    void run();

private:
    struct ClientInfo {
        std::string nickname;
        std::string username;
        bool authenticated;
    };
    std::string password;
    int server_fd;
    std::vector<struct pollfd> poll_fds;
    std::map<std::string, CommandHandler> command_map;
    std::map<int, ClientInfo> clients;
    std::map<std::string, std::vector<int> > channels;

    void process_command(int client_fd, const std::string& command, const std::string& args);
    void parse_command(const std::string& input, std::string& command, std::string& args);
    std::string receive_data(int client_fd);
    void handle_new_connection();
    void handle_client_data(size_t i);
    void close_client(size_t i);
	void handle_nick(int client_fd, const std::string& args);
	void handle_user(int client_fd, const std::string& args);
	void handle_join(int client_fd, const std::string& args);
	void handle_privmsg(int client_fd, const std::string& args);
    void handle_pass(int client_fd, const std::string& args);
    void handle_quit(int i, const std::string& args);
	void initialize_command_map();
    bool is_command(std::string command);
    std::string my_trim(const std::string& str);

};

#endif