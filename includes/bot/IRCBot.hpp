#ifndef BOT_HPP
#define BOT_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <map>
#include <cstdlib>

// Structure for trivia questions
struct TriviaQuestion {
    std::string question;
    std::string answer;
};

class Bot {
private:
    int _sockfd;
    std::string _server;
    int _port;
    std::string _nickname;
    std::string _channel;
    std::string _password;
    struct pollfd _pfd;
    // map of trivia questions
    std::map<std::string, std::string> _triviaQuestions;
    int _current_question_index;

    void send_msg(const std::string& msg);
    void handle_server_message(const std::string& message);

public:
    Bot(const std::string& server, int port, const std::string& nickname, const std::string& password, const std::string& channel);
    Bot();
    ~Bot();

    bool connect_to_server();
    void run();

    void initialize_trivia();
    void ask_trivia_question();
    void check_trivia_answer(const std::string& user_answer);

    void fetch_weather(const std::string& location);
    void fetch_crypto_price(const std::string& crypto_symbol);
    
};

#endif // BOT_HPP
