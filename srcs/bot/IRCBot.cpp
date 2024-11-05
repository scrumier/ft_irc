#include "IRCBot.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

#define BUFFER_SIZE 512

Bot::Bot(const std::string& server, int port, const std::string& nickname, const std::string& password, const std::string& channel)
    : _sockfd(-1), _server(server), _port(port), _nickname(nickname), _channel(channel), _password(password), _current_question_index(-1) {
    memset(&_pfd, 0, sizeof(_pfd));
    initialize_trivia();
}

Bot::Bot() : _sockfd(-1) {
    memset(&_pfd, 0, sizeof(_pfd));
}

Bot::~Bot() {
    if (_sockfd >= 0) {
        close(_sockfd);
    }
}

bool Bot::connect_to_server() {
    // in want to connect to the IRC server using IRC protocol with numerics replies
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(_server.c_str(), std::to_string(_port).c_str(), &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return false;
    }

    _sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (_sockfd < 0) {
        std::cerr << "socket: " << strerror(errno) << std::endl;
        return false;
    }

    if (connect(_sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "connect: " << strerror(errno) << std::endl;
        return false;
    }

    _pfd.fd = _sockfd;
    _pfd.events = POLLIN;

    send_msg("PASS " + _password);
    sleep(1);
    send_msg("NICK " + _nickname);
    sleep(1);
    send_msg("USER " + _nickname + " 0 * :" + _nickname);
    sleep(1);
    return true;
}

void Bot::send_msg(const std::string& msg) {
    std::string final_msg = msg + "\r\n";
    send(_sockfd, final_msg.c_str(), final_msg.length(), 0);
}

void Bot::run() {
    char buffer[BUFFER_SIZE];
    std::string msg_from_server;

    while (true) {
        int ret = poll(&_pfd, 1, -1);
        if (ret < 0) {
            std::cerr << "poll: " << strerror(errno) << std::endl;
            break;
        }

        if (_pfd.revents & POLLIN) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(_sockfd, buffer, BUFFER_SIZE, 0);
            if (bytes_received < 0) {
                std::cerr << "recv: " << strerror(errno) << std::endl;
                continue;
            }

            msg_from_server += std::string(buffer, bytes_received);
            size_t pos = msg_from_server.find("\r\n");
            while (pos != std::string::npos) {
                std::string message = msg_from_server.substr(0, pos);
                handle_server_message(message);
                msg_from_server.erase(0, pos + 2);
                pos = msg_from_server.find("\r\n");
            }
        }
    }
}
