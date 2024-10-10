#include "ft_irc.hpp"

bool Server::taken_nickname(const std::string& nickname) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == nickname) {
            return true;
        }
    }
    return false;
}

void Server::handle_nick(int client_fd, const std::string& args) {
    std::string nickname = my_trim(args);

    // check if the nickname is already taken

    if (nickname.empty()) {
        std::string current_nick = clients[client_fd].getNickname();
        std::string msg = "Your nickname is: " + current_nick + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    } else if (nickname.size() > 9) {
        std::string error_msg = "Nickname too long. Max 9 characters.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    } else if (!taken_nickname(nickname)) {
        std::string error_msg = "Nickname already taken.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    clients[client_fd].setNickname(nickname);
    clients[client_fd].setHasNick(true);
    std::cout << "Client " << client_fd << " set nickname to " << nickname << std::endl;

    complete_registration(client_fd);
}

// Server.cpp

void Server::handle_user(int client_fd, const std::string& args) {
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
        if (username.empty() || realname.empty()) {
            std::string error_msg = "Invalid USER command format. Empty username or realname.\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        clients[client_fd].setUsername(username);
        clients[client_fd].setRealname(realname);
        clients[client_fd].setHasUser(true);
        std::cout << "Client " << client_fd << " set username to " << username
                  << " and real name to " << realname << std::endl;
        complete_registration(client_fd);
    } else {
        std::string username = clients[client_fd].getUsername();
        std::string realname = clients[client_fd].getRealname();
        std::string msg = "Your username is: " + username + "\r\n";
        msg += "Your real name is: " + realname + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
    }
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
        channels[channel_name] = Channel();
    }

    Channel& channel = channels[channel_name];
    std::map<std::string, Client*>& clients_in_channel = channel.getClients(); // Use Client* here
    std::string client_nickname = clients[client_fd].getNickname();

    if (clients_in_channel.find(client_nickname) == clients_in_channel.end()) {
        clients_in_channel[client_nickname] = &clients[client_fd]; // Store pointer to Client
        std::string join_msg = "You have joined channel " + channel_name + ".\r\n";
        send(client_fd, join_msg.c_str(), join_msg.size(), 0);
        std::cout << "Client " << client_fd << " joined channel " << channel_name << std::endl;
    } else {
        std::string already_joined_msg = "You are already in channel " + channel_name + ".\r\n";
        send(client_fd, already_joined_msg.c_str(), already_joined_msg.size(), 0);
    }
}

void Server::handle_privmsg(int client_fd, const std::string& args) {
    std::istringstream iss(args);
    std::string target, message;
    
    iss >> target;
    std::getline(iss, message);
    
    target = my_trim(target);
    message = my_trim(message);
    
    if (target.empty() || message.empty() || message[0] != ':') {
        std::string error_msg = "Invalid message format. Use /PRIVMSG <target> :<message>\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    message = message.substr(1);
    
    std::string sender_nickname = clients[client_fd].getNickname();
    
    if (channels.find(target) != channels.end()) {
        std::string msg = sender_nickname + ": " + message + "\r\n";
        Channel& channel = channels[target];
        std::map<std::string, Client*>& clients_in_channel = channel.getClients(); // Use Client* here
        
        for (std::map<std::string, Client*>::iterator it = clients_in_channel.begin(); it != clients_in_channel.end(); ++it) {
            if (it->first != sender_nickname) {
                int target_fd = it->second->getFd();
                if (target_fd < 0) {
                    std::cerr << "Error: Invalid file descriptor for client " << it->first << std::endl;
                    continue;
                }
                if (send(target_fd, msg.c_str(), msg.size(), 0) == -1) {
                    std::cerr << "Error sending message to " << it->first << std::endl;
                }
            }
        }
        std::cout << "Client " << sender_nickname << " sent message to channel " << target << std::endl;
    }
    else {
        bool user_found = false;
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                std::string msg = sender_nickname + ": " + message + "\r\n";
                send(it->first, msg.c_str(), msg.size(), 0);
                user_found = true;
                std::cout << "Client " << sender_nickname << " sent private message to " << target << std::endl;
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
    if (clients[client_fd].hasNick() || clients[client_fd].hasUser()) {
        std::string error_msg = "PASS command must be sent before NICK/USER\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
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
    
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        Channel& channel = it->second;
        std::map<std::string, Client*>& clients_in_channel = channel.getClients(); // Use Client* here
        
        std::map<std::string, Client*>::iterator client_it = clients_in_channel.find(clients[client_fd].getNickname());
        if (client_it != clients_in_channel.end()) {
            clients_in_channel.erase(client_it);
        }
    }

    std::cout << "Client " << client_fd << " quit with message: " << args << std::endl;
    close_client(client_fd);
}

void Server::handle_cap(int client_fd, const std::string& args) {
    (void)args;
    std::string response = "CAP * NAK :No supported capabilities\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handle_ping(int client_fd, const std::string& args) {
    std::string response = "PONG :" + args + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handle_pong(int client_fd, const std::string& args) {
    (void)client_fd;
    (void)args;
}

void Server::initialize_command_map() {
    command_map["NICK"] = &Server::handle_nick;
    command_map["USER"] = &Server::handle_user;
    command_map["JOIN"] = &Server::handle_join;
    command_map["PRIVMSG"] = &Server::handle_privmsg;
    command_map["PASS"] = &Server::handle_pass;
    command_map["QUIT"] = &Server::handle_quit;
    command_map["CAP"] = &Server::handle_cap;
    command_map["PING"] = &Server::handle_ping;
    command_map["PONG"] = &Server::handle_pong;
}
