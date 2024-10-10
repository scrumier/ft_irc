#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"

class Server {
private:
    int server_fd;
    std::string password;
    std::vector<struct pollfd> poll_fds;
    std::map<int, Client> clients;
    std::map<std::string, Channel> channels;
    std::string server_name;
    std::string server_version;
    std::string server_creation_date;
    bool requires_password;

    void complete_registration(int client_fd);

    typedef void (Server::*CommandHandler)(int client_fd, const std::string& args);
    std::map<std::string, CommandHandler> command_map;

    void initialize_command_map();

    void handle_new_connection();
    void handle_client_data(size_t i);
    void close_client(size_t i);

    void parse_command(const std::string& input, std::string& command, std::string& args);
    void process_command(int client_fd, const std::string& command, const std::string& args);
    std::string receive_data(int client_fd);

    void handle_nick(int client_fd, const std::string& args);
    void handle_user(int client_fd, const std::string& args);
    void handle_join(int client_fd, const std::string& args);
    void handle_msg(int client_fd, const std::string& args);
    void handle_pass(int client_fd, const std::string& args);
    void handle_quit(int client_fd, const std::string& args);

    bool is_command(const std::string& command);
    std::string my_trim(const std::string& str);
    bool is_valid_channel_name(const std::string& name);

    void handle_cap(int client_fd, const std::string& args);
    void handle_ping(int client_fd, const std::string& args);
    void handle_pong(int client_fd, const std::string& args);

public:
    Server(int port, const std::string& password);
    ~Server();
    void run();
};

#endif // SERVER_HPP
