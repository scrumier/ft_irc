#include "Server.hpp"

void Server::handle_nick(int client_fd, const std::string& args) {
    if (args.size() == 0) {
        std::string name = clients[client_fd].nickname;
        std::string msg = "Your nickname is: " + name + "\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }
    else if (args.size() > 9) {
        std::string error_msg = "Nickname too long. Max 9 characters.\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    clients[client_fd].nickname = args;
    std::cout << "Client " << client_fd << " set nickname to " << args << std::endl;
}

void Server::handle_user(int client_fd, const std::string& args) {
    if (args.size() == 0) {
        std::string name = clients[client_fd].username;
        std::string msg = "Your username is: " + name + "\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }
    else if (args.size() > 9) {
        std::string error_msg = "Username too long. Max 9 characters.\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    clients[client_fd].username = args;
    std::cout << "Client " << client_fd << " set username to " << args << std::endl;
}

void Server::handle_join(int client_fd, const std::string& args) {
    std::string channel_name = args;
    if (channels.find(channel_name) == channels.end()) {
        channels[channel_name] = std::vector<int>();
    }
    channels[channel_name].push_back(client_fd);
    std::cout << "Client " << client_fd << " joined channel " << channel_name << std::endl;
}


void Server::handle_privmsg(int client_fd, const std::string& args) {
    std::cout << "Client " << client_fd << " sent message: " << args << std::endl;
}

void Server::handle_pass(int client_fd, const std::string& args) {
    if (clients[client_fd].authenticated) {
        std::string error_msg = "Already authenticated.\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    if (args == password) {
        clients[client_fd].authenticated = true;
        std::string msg = "Password accepted.\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        std::cout << "Client " << client_fd << " authenticated.\n";
    } else {
        std::string error_msg = "Invalid password.\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        close_client(client_fd);
    }
}

void Server::handle_quit(int i, const std::string& args) {
    std::string quit_msg;

    if (args.size() > 0) {
        quit_msg = "Goodbye! Reason: " + args + "\n";
    } else {
        quit_msg = "Goodbye!\n";
    }

    send(poll_fds[i].fd, quit_msg.c_str(), quit_msg.size(), 0);

    throw std::runtime_error("Client quit");
}

void Server::initialize_command_map() {
    command_map["NICK"] = &Server::handle_nick;
    command_map["USER"] = &Server::handle_user;
    command_map["JOIN"] = &Server::handle_join;
    command_map["PRIVMSG"] = &Server::handle_privmsg;
    command_map["PASS"] = &Server::handle_pass;
    command_map["QUIT"] = &Server::handle_quit;
}
