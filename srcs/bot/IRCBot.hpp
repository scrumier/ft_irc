#ifndef BOT_HPP
#define BOT_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

class Bot {
private:
    int _sockfd;
    std::string _server;
    int _port;
    std::string _nickname;
    std::string _channel;
    std::string _password;
    struct pollfd _pfd;

    void send_msg(const std::string& msg);
    void handle_server_message(const std::string& message);

public:
    Bot(const std::string& server, int port, const std::string& nickname, const std::string& password, const std::string& channel);
    Bot();
    ~Bot();

    bool connect_to_server();
    void run();
};

#endif // BOT_HPP