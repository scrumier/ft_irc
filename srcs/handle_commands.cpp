#include "ft_irc.hpp"

bool Server::already_taken_nickname(const std::string& nickname) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == nickname) {
            return true;
        }
    }
    return false;
}

/*
 * @brief Set the nickname of a client only if its valid
 * @param client_fd The client file descriptor
 * @param args The nickname to set
 * @return void
*/
void Server::handle_nick(int client_fd, const std::string& args) {
    std::string nickname = my_trim(args);

    if (nickname.empty()) {
        std::string current_nick = clients[client_fd].getNickname();
        std::string msg = "Your nickname is: " + current_nick + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    } else if (nickname.size() > 9) {
        std::string error_msg = ":" + server_name + " 432 * " + nickname + " :Erroneous nickname (too long, max 9 characters)\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    } else if (already_taken_nickname(nickname)) {
        std::string error_msg = ":" + server_name + " 433 * " + nickname + " :Nickname is already in use\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    clients[client_fd].setNickname(nickname);
    clients[client_fd].setHasNick(true);
    std::cout << "Client " << client_fd << " set nickname to " << nickname << std::endl;
}

/*
 * @brief Set the username and realname of a client
 * @param client_fd The client file descriptor
 * @param args The username and realname to set
 * @return void
*/
void Server::handle_user(int client_fd, const std::string& args) {
    if (!args.empty()) {
        size_t first_space = args.find(' ');
        size_t second_space = args.find(' ', first_space + 1);
        size_t third_space = args.find(' ', second_space + 1);
        size_t colon = args.find(':', third_space + 1);

        if (first_space == std::string::npos || second_space == std::string::npos ||
            third_space == std::string::npos || colon == std::string::npos) {
            std::string error_msg = ":" + server_name + " 461 USER :Not enough parameters\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        std::string username = my_trim(args.substr(0, first_space));
        std::string realname = my_trim(args.substr(colon + 1)); // Everything after the colon
        if (username.empty() || realname.empty()) {
            std::string error_msg = ":" + server_name + " 461 USER :Not enough parameters\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        clients[client_fd].setUsername(username);
        clients[client_fd].setRealname(realname);
        clients[client_fd].setHasUser(true);
        std::cout << "Client " << client_fd << " set username to " << username << " and real name to " << realname << std::endl;
    } else {
        std::string username = clients[client_fd].getUsername();
        std::string realname = clients[client_fd].getRealname();
        std::string msg = "Your username is: " + username + "\r\n";
        msg += "Your real name is: " + realname + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
    }
}

/*
 * @brief Part a channel
 * @param client_fd The client file descriptor
 * @param args The channel name to part
 * @return void
*/
void Server::handle_part(int client_fd, const std::string& args) {
    std::string channel_name = my_trim(args);

    if (channel_name.empty()) {
        std::string error_msg = ":" + server_name + " 461 PART :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (!is_valid_channel_name(channel_name)) {
        std::string error_msg = ":" + server_name + " 403 " + clients[client_fd].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = ":" + server_name + " 403 " + clients[client_fd].getNickname() + " " + channel_name + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    std::map<std::string, Client*>& clients_in_channel = channel.getClients();
    std::string client_nickname = clients[client_fd].getNickname();

    if (clients_in_channel.find(client_nickname) == clients_in_channel.end()) {
        std::string error_msg = ":" + server_name + " 442 " + client_nickname + " " + channel_name + " :You're not on that channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    clients_in_channel.erase(client_nickname);
    std::string part_msg = ":" + client_nickname + " PART " + channel_name + "\r\n";
    send(client_fd, part_msg.c_str(), part_msg.size(), 0);
    std::cout << "Client " << clients[client_fd].getNickname() << " left channel " << channel_name << std::endl;
}

/*
 * @brief Join a channel
 * @param client_fd The client file descriptor
 * @param args The channel name to join
 * @return void
*/
void Server::handle_join(int client_fd, const std::string& args) {
    std::string channel_name = my_trim(args);

    if (channel_name.empty()) {
        std::string error_msg = ":" + server_name + " 461 JOIN :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (!is_valid_channel_name(channel_name)) {
        std::string error_msg = ":" + server_name + " 403 " + clients[client_fd].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channels.find(channel_name) == channels.end()) {
        channels[channel_name] = Channel();
    }

    Channel& channel = channels[channel_name];
    std::map<std::string, Client*>& clients_in_channel = channel.getClients();
    std::string client_nickname = clients[client_fd].getNickname();

    if (clients_in_channel.find(client_nickname) == clients_in_channel.end()) {
        clients_in_channel[client_nickname] = &clients[client_fd];
        std::string join_msg = ":" + client_nickname + " JOIN " + channel_name + "\r\n";
        if (clients_in_channel.size() == 1) {
            channel.addOperator(client_nickname);
            std::string op_msg = client_nickname + " is now an operator in channel " + channel_name + ".\r\n";
            send(client_fd, op_msg.c_str(), op_msg.size(), 0);
            std::cout << "Client " << client_fd << " joined channel " << channel_name << " and became operator." << std::endl;
        }
        send(client_fd, join_msg.c_str(), join_msg.size(), 0);
        std::cout << "Client " << clients[client_fd].getNickname() << " joined channel " << channel_name << std::endl;
    } else {
        std::string already_joined_msg = "You are already in channel " + channel_name + ".\r\n";
        send(client_fd, already_joined_msg.c_str(), already_joined_msg.size(), 0);
    }
}

/*
 * @brief Send a message to a client or channel
 * @param client_fd The client file descriptor
 * @param args The target and message
 * @return void
*/
void Server::handle_privmsg(int client_fd, const std::string& args) {
    std::string target;
    std::string message;

    size_t colon = args.find(':');
    if (colon != std::string::npos) {
        target = my_trim(args.substr(0, colon));
        message = my_trim(args.substr(colon));
    }
    
    if (target.empty() || message.empty() || message[0] != ':') {
        std::string error_msg = ":" + server_name + " 411 " + clients[client_fd].getNickname() + " :No recipient or text to send\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    message = message.substr(1);

    std::string sender_nickname = clients[client_fd].getNickname();
    
    if (target[0] == '#') {
        if (channels.find(target) == channels.end()) {
            std::string error_msg = ":" + server_name + " 403 " + sender_nickname + " " + target + " :No such channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        Channel& channel = channels[target];
        std::map<std::string, Client*>& clients_in_channel = channel.getClients();
        if (clients_in_channel.find(sender_nickname) == clients_in_channel.end()) {
            std::string error_msg = ":" + server_name + " 442 " + sender_nickname + " " + target + " :You're not on that channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }
        for (std::map<std::string, Client*>::iterator it = clients_in_channel.begin(); it != clients_in_channel.end(); ++it) {
            std::string msg = ":" + sender_nickname + " PRIVMSG " + target + " :" + message + "\r\n";
            send(it->second->getFd(), msg.c_str(), msg.size(), 0);
        }
    } else {
        bool target_found = false;
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                target_found = true;
                break;
            }
        }
        if (!target_found) {
            std::string error_msg = ":" + server_name + " 401 " + sender_nickname + " " + target + " :No such nick/channel\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        if (target == sender_nickname) {
            std::string error_msg = ":" + server_name + " 401 " + sender_nickname + " :You cannot send a message to yourself\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                std::string msg = ":" + sender_nickname + " PRIVMSG " + target + " :" + message + "\r\n";

                send(it->first, msg.c_str(), msg.size(), 0);
                std::cout << "Client " << clients[client_fd].getNickname() << " sent message to " << target << ": " << message << std::endl;
                return;
            }
        }
    }
}

/*
 * @brief Authenticate a client
 * @param client_fd The client file descriptor
 * @param args The password
 * @return void
*/
void Server::handle_pass(int client_fd, const std::string& args) {
    std::string pass = my_trim(args);
    if (clients[client_fd].isAuthenticated()) {
        std::string error_msg = ":" + server_name + " 462 " + clients[client_fd].getNickname() + " :You may not reregister\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    if (pass == password) {
        clients[client_fd].setAuthenticated(true);
        std::string msg = "Password accepted.\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        std::cout << "Client " << client_fd << " authenticated." << std::endl;
    } else {
        std::string error_msg = ":" + server_name + " 464 " + clients[client_fd].getNickname() + " :Password incorrect\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
    }
}

/*
 * @brief Close a client connection
 * @param client_fd The client file descriptor
 * @param args The quit message
 * @return void
*/
void Server::handle_quit(int client_fd, const std::string& args) {
    std::string quit_msg = clients[client_fd].getNickname() + " has quit";
    
    if (!args.empty()) {
        quit_msg += " (" + args + ")";
    }
    quit_msg += ".\r\n";
    
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        Channel& channel = it->second;
        std::map<std::string, Client*>& clients_in_channel = channel.getClients();
        
        std::map<std::string, Client*>::iterator client_it = clients_in_channel.find(clients[client_fd].getNickname());
        if (client_it != clients_in_channel.end()) {
            clients_in_channel.erase(client_it);
        }
    }

    std::cout << "Client " << client_fd << " quit with message: " << quit_msg << std::endl;
    close_client(client_fd);
}

/*
 * @brief Handle the CAP command
 * @param client_fd The client file descriptor
 * @param args The arguments
 * @return void
*/
void Server::handle_cap(int client_fd, const std::string& args) {
    (void)args;
    std::string response = "CAP * NAK :No supported capabilities\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

/*
 * @brief Handle the PING command
 * @param client_fd The client file descriptor
 * @param args The arguments
 * @return void
*/
void Server::handle_ping(int client_fd, const std::string& args) {
    std::string response = "PONG :" + args + "\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

/*
 * @brief Handle the PONG command
 * @param client_fd The client file descriptor
 * @param args The arguments
 * @return void
*/
void Server::handle_pong(int client_fd, const std::string& args) {
    (void)client_fd;
    (void)args;
}

void Server::handle_kick(int client_fd, const std::string& args) {
    // Parsing the command args
    size_t first_space = args.find(' ');
    size_t second_space = args.find(' ', first_space + 1);
    
    if (first_space == std::string::npos || second_space == std::string::npos) {
        std::string error_msg = "Usage: /KICK <channel> <user> :<reason>\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    
    std::string channel_name = my_trim(args.substr(0, first_space));
    std::string target_nickname = my_trim(args.substr(first_space + 1, second_space - first_space - 1));
    std::string reason = my_trim(args.substr(second_space + 1));
    
    if (reason.empty() || reason[0] != ':') {
        std::string error_msg = "Invalid KICK format. Reason must start with ':'\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    reason = reason.substr(1);  // Remove ':' from reason

    // Check if the channel exists
    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = "Channel " + channel_name + " does not exist.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    
    // Verify if the client issuing the KICK is an operator
    std::string kicker_nickname = clients[client_fd].getNickname();
    if (!channel.isOperator(kicker_nickname)) {
        std::string error_msg = "You are not an operator in channel " + channel_name + ".\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    
    // Check if the target user is in the channel
    if (!channel.isClient(target_nickname)) {
        std::string error_msg = "User " + target_nickname + " is not in channel " + channel_name + ".\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Get the target client's file descriptor
    Client* target_client = channel.getClients()[target_nickname];
    int target_fd = target_client->getFd();

    // Send a message to all users in the channel that the user was kicked
    std::string kick_msg = kicker_nickname + " KICK " + channel_name + " " + target_nickname + " :" + reason + "\r\n";
    std::map<std::string, Client*>& clients_in_channel = channel.getClients();
    for (std::map<std::string, Client*>::iterator it = clients_in_channel.begin(); it != clients_in_channel.end(); ++it) {
        send(it->second->getFd(), kick_msg.c_str(), kick_msg.size(), 0);
    }

    // Remove the client from the channel
    channel.removeClient(target_nickname);

    // Inform the kicked user
    std::string user_kicked_msg = "You have been kicked from " + channel_name + " by " + kicker_nickname + " :" + reason + "\r\n";
    send(target_fd, user_kicked_msg.c_str(), user_kicked_msg.size(), 0);
}

void Server::initialize_command_map() {
    command_map["NICK"] = &Server::handle_nick;
    command_map["USER"] = &Server::handle_user;
    command_map["JOIN"] = &Server::handle_join;
    command_map["PRIVMSG"] = &Server::handle_privmsg;
    command_map["PASS"] = &Server::handle_pass;
    command_map["PART"] = &Server::handle_part;
    command_map["QUIT"] = &Server::handle_quit;
    command_map["CAP"] = &Server::handle_cap;
    command_map["PING"] = &Server::handle_ping;
    command_map["PONG"] = &Server::handle_pong;
    command_map["KICK"] = &Server::handle_kick;
    command_map["INVITE"] = &Server::handle_invite;
    command_map["TOPIC"] = &Server::handle_topic;
}
