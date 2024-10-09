#include "Server.hpp"

void Server::handle_nick(int client_fd, const std::string& args) {
    std::string nickname = my_trim(args);
    
    if (nickname.empty()) {
        std::string current_nick = clients[client_fd].getNickname();
        std::string msg = "Your nickname is: " + current_nick + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    } 
    else if (nickname.size() > 9) {
        std::string error_msg = "Nickname too long. Max 9 characters.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    
    clients[client_fd].setNickname(nickname);
    std::cout << "Client " << client_fd << " set nickname to " << nickname << std::endl;
}

void Server::handle_user(int client_fd, const std::string& args) {
    std::cout << "Raw args received: |" << args << "|" << std::endl;
    
    if (!args.empty()) {
        size_t first_space = args.find(' ');
        size_t second_space = args.find(' ', first_space + 1);
        size_t third_space = args.find(' ', second_space + 1);
        size_t colon = args.find(':', third_space + 1);

        if (first_space == std::string::npos || second_space == std::string::npos ||
            third_space == std::string::npos || colon == std::string::npos) {
            std::string error_msg = "Invalid USER command format. Missing fields or colon.\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        std::string username = my_trim(args.substr(0, first_space));

        std::string realname = my_trim(args.substr(colon + 1)); // Everything after the colon

        std::cout << "Parsed username: |" << username << "|" << std::endl;
        std::cout << "Parsed realname: |" << realname << "|" << std::endl;

        if (username.empty() || realname.empty()) {
            std::string error_msg = "Invalid USER command format. Empty username or realname.\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        clients[client_fd].setUsername(username);
        clients[client_fd].setRealname(realname);

        std::cout << "Client " << client_fd << " set username to " << username 
                  << " and real name to " << realname << std::endl;
    } else {
        std::string username = clients[client_fd].getUsername();
        std::string realname = clients[client_fd].getRealname();
        std::string msg = "Your username is: " + username + "\r\n";
        msg += "Your real name is: " + realname + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
    }
}

bool Server::is_valid_channel_name(const std::string& channel) {
    return !channel.empty() && channel[0] == '#' && channel.size() > 1;
}

void Server::handle_join(int client_fd, const std::string& args) {
    std::string channel_name = my_trim(args);

    if (channel_name.empty()) {
        std::string error_msg = "No channel name provided.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (!is_valid_channel_name(channel_name)) {
        std::string error_msg = "Invalid channel name. Channels must start with '#'.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channels.find(channel_name) == channels.end()) {
        channels[channel_name] = std::vector<int>();
    }

    std::vector<int>& clients_in_channel = channels[channel_name];
    if (std::find(clients_in_channel.begin(), clients_in_channel.end(), client_fd) == clients_in_channel.end()) {
        clients_in_channel.push_back(client_fd);
        std::string join_msg = "You have joined channel " + channel_name + ".\r\n";
        send(client_fd, join_msg.c_str(), join_msg.size(), 0);

        std::cout << "Client " << client_fd << " joined channel " << channel_name << std::endl;

    } else {
        std::string already_joined_msg = "You are already in channel " + channel_name + ".\r\n";
        send(client_fd, already_joined_msg.c_str(), already_joined_msg.size(), 0);
    }
}

void Server::handle_msg(int client_fd, const std::string& args) {
    std::istringstream iss(args);
    std::string target, message;
    
    iss >> target;
    std::getline(iss, message);
    
    target = my_trim(target);
    message = my_trim(message);
    
    if (target.empty() || message.empty()) {
        std::string error_msg = "Invalid message format. Use PRIVMSG <target> :<message>\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    
    if (channels.find(target) != channels.end()) {
        std::string msg = clients[client_fd].getNickname() + ": " + message + "\r\n";
        for (size_t i = 0; i < channels[target].size(); ++i) {
            send(channels[target][i], msg.c_str(), msg.size(), 0);
        }
        std::cout << "Client " << clients[client_fd].getNickname() << " sent message to channel " << target << std::endl;
    } else {
        bool user_found = false;
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                std::string msg = clients[client_fd].getNickname() + ": " + message + "\r\n";
                send(it->first, msg.c_str(), msg.size(), 0);
                user_found = true;
                std::cout << "Client " << clients[client_fd].getNickname() << " sent private message to " << target << std::endl;
                break;
            }
        }
        
        if (!user_found) {
            std::string error_msg = "User or channel not found.\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        }
    }
}

void Server::handle_pass(int client_fd, const std::string& args) {
    std::string pass = my_trim(args);
    
    if (clients[client_fd].isAuthenticated()) {
        std::string error_msg = "Already authenticated.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    if (pass == password) {
        clients[client_fd].setAuthenticated(true);
        std::string msg = "Password accepted.\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        std::cout << "Client " << client_fd << " authenticated." << std::endl;
    } else {
        std::string error_msg = "Invalid password.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        close_client(client_fd);
    }
}

void Server::handle_quit(int client_fd, const std::string& args) {
    std::string quit_msg = clients[client_fd].getNickname() + " has quit";
    
    if (!args.empty()) {
        quit_msg += " (" + args + ")";
    }
    quit_msg += ".\r\n";
    
    for (std::map<std::string, std::vector<int> >::iterator it = channels.begin(); it != channels.end(); ++it) {
        std::vector<int>& channel_clients = it->second;
        if (std::find(channel_clients.begin(), channel_clients.end(), client_fd) != channel_clients.end()) {
            for (size_t i = 0; i < channel_clients.size(); ++i) {
                send(channel_clients[i], quit_msg.c_str(), quit_msg.size(), 0);
            }
        }
    }
    
    std::cout << "Client " << client_fd << " quit with message: " << args << std::endl;
    close_client(client_fd);
}


void Server::initialize_command_map() {
    command_map["NICK"] = &Server::handle_nick;
    command_map["USER"] = &Server::handle_user;
    command_map["JOIN"] = &Server::handle_join;
    command_map["MSG"] = &Server::handle_msg;
    command_map["PASS"] = &Server::handle_pass;
    command_map["QUIT"] = &Server::handle_quit;
}
