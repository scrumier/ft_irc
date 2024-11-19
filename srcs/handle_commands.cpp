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
        std::string error_msg = ":" + server_name + " 431 * :No nickname given\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
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
    Client& client = clients[client_fd];

    if (client.getUsername().empty() == false  && client.getNickname().empty() == false) {
        std::string error_msg = ":" + server_name + " 462 :You may not reregister\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

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
        std::string realname = my_trim(args.substr(colon + 1));

        if (username.empty() || realname.empty()) {
            std::string error_msg = ":" + server_name + " 461 USER :Not enough parameters\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        client.setUsername(username);
        client.setRealname(realname);
        client.setHasUser(true);

        std::cout << "Client " << client_fd << " set username to " << username
                  << " and real name to " << realname << std::endl;

        if (client.hasNick()) {
            std::string welcome_msg = ":" + server_name + " 001 " + client.getNickname() +
                                      " :Welcome to the Internet Relay Network " +
                                      client.getNickname() + "!" + username + "@<host>\r\n";
            send(client_fd, welcome_msg.c_str(), welcome_msg.size(), 0);
        }

    } else {
        std::string error_msg = ":" + server_name + " 461 USER :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
    }
}


/*
 * @brief Part a channel
 * @param client_fd The client file descriptor
 * @param args The channel name to part
 * @return void
*/
void Server::handle_part(int client_fd, const std::string& args) {
    size_t space_pos = args.find(' ');
    std::string channel_name;
    std::string reason = "Leaving"; // Default reason if none is provided.

    if (space_pos != std::string::npos) {
        channel_name = my_trim(args.substr(0, space_pos));
        std::string rest = my_trim(args.substr(space_pos + 1));
        if (!rest.empty() && rest[0] == ':') {
            reason = rest.substr(1); // Extract reason without the leading ':'
        }
    } else {
        channel_name = my_trim(args);
    }

    if (channel_name.empty()) {
        std::string error_msg = ":" + server_name + " 461 " + clients[client_fd].getNickname() + " PART :Not enough parameters\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (!is_valid_channel_name(channel_name)) {
        std::string error_msg = ":" + server_name + " 403 " + clients[client_fd].getNickname() + " " + channel_name + " :No such channel\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = ":" + server_name + " 403 " + clients[client_fd].getNickname() + " " + channel_name + " :No such channel\r\n";
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

    channel.removeClient(client_nickname);
    std::string part_msg = ":" + client_nickname + " PART " + channel_name + " :" + reason + "\r\n";
    channel.broadcast(part_msg);
    send(client_fd, part_msg.c_str(), part_msg.size(), 0);
    std::cout << "Client " << clients[client_fd].getNickname() << " left channel " << channel_name << " with reason: " << reason << std::endl;
    channel.updateList(client_fd, server_name, client_nickname);

    if (channel.getClientNumber() == 0) {
        channels.erase(channel_name);
        std::cout << "Channel " << channel_name << " deleted because it is empty.\n";
    }
}


void Channel::sendNumericRepliesToJoiner(Client& joiner, const std::string& server_name) {
    int client_fd = joiner.getFd();
    std::string nickname = joiner.getNickname();

    std::string join_msg = ":" + nickname + " JOIN " + name + "\r\n";
    send(client_fd, join_msg.c_str(), join_msg.size(), 0);

    if (!topic.empty()) {
        std::string topic_msg = ":" + server_name + " 332 " + nickname + " " + name + " :" + topic + "\r\n";
        send(client_fd, topic_msg.c_str(), topic_msg.size(), 0);
    }

    std::string names_msg = ":" + server_name + " 353 " + nickname + " = " + name + " :";
    for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it != clients.begin()) {
            names_msg += " ";
        }
        names_msg += it->first;
    }
    names_msg += "\r\n";
    send(client_fd, names_msg.c_str(), names_msg.size(), 0);

    std::string end_of_names_msg = ":" + server_name + " 366 " + nickname + " " + name + " :End of /NAMES list\r\n";
    send(client_fd, end_of_names_msg.c_str(), end_of_names_msg.size(), 0);
}

/*
 * @brief Join a channel
 * @param client_fd The client file descriptor
 * @param args The channel name to join
 * @return void
*/
void Server::handle_join(int client_fd, const std::string& args) {
    size_t space_pos = args.find(' ');
    std::string channel_name;
    std::string password;

    if (space_pos != std::string::npos) {
        channel_name = my_trim(args.substr(0, space_pos));
        password = my_trim(args.substr(space_pos + 1));
    } else {
        channel_name = my_trim(args);
    }

    if (channel_name.empty() || channel_name == "#") {
        std::string error_msg = ":" + server_name + " 461 " + clients[client_fd].getNickname() + " JOIN :Not enough parameters\r\n";
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
        channels[channel_name].setPassword(password);
        channels[channel_name].setName(channel_name);
    }

    Channel& channel = channels[channel_name];
    std::string client_nickname = clients[client_fd].getNickname();

    if (channel.getClients().find(client_nickname) != channel.getClients().end()) {
        std::string already_joined_msg = ":" + server_name + " 443 " + client_nickname + " " + channel_name + " :is already on channel\r\n";
        send(client_fd, already_joined_msg.c_str(), already_joined_msg.size(), 0);
        return;
    }

    if (!channel.getPassword().empty() && password != channel.getPassword()) {
        std::string error_msg = ":" + server_name + " 475 " + channel_name + " :Cannot join channel (bad key)\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channel.getInviteOnly() && !channel.isInvited(client_nickname)) {
        std::string error_msg = ":" + server_name + " 473 " + channel_name + " :Cannot join channel (invite only)\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channel.getClientNumber() >= channel.getChannelLimit()) {
        std::string error_msg = ":" + server_name + " 471 " + channel_name + " :Cannot join channel (channel is full)\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Add client to the channel
    channel.addClient(client_nickname, &clients[client_fd]);
    channel.removeIfInvitedClient(client_nickname);

    // Notify the client of a successful join
    std::string join_msg = ":" + client_nickname + " JOIN :" + channel_name + "\r\n";
    channel.broadcast(join_msg);

    // Make the first client an operator
    if (channel.getClientNumber() == 1) {
        channel.addOperator(client_nickname);
    }


    // Send topic if it exists
    if (!channel.getTopic().empty()) {
        std::string topic_msg = ":" + server_name + " 332 " + client_nickname + " " + channel_name + " :" + channel.getTopic() + "\r\n";
        send(client_fd, topic_msg.c_str(), topic_msg.size(), 0);
    }

    std::cout << "Client " << client_nickname << " joined channel " << channel_name << std::endl;
    channel.updateList(client_fd, server_name, client_nickname);
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
            if (it->second->getFd() == client_fd)
                continue;
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
    std::string client_nickname = clients[client_fd].getNickname();
    std::string quit_msg = client_nickname + " has quit";
    if (!args.empty()) {
        quit_msg += " (" + args + ")";
    }
    quit_msg += ".\r\n";
    
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        Channel& channel = it->second;
        std::map<std::string, Client*>& clients_in_channel = channel.getClients();
        
        channel.broadcast(quit_msg);
        std::map<std::string, Client*>::iterator client_it = clients_in_channel.find(clients[client_fd].getNickname());
        if (client_it != clients_in_channel.end()) {
            clients_in_channel.erase(client_it);
        }
        channel.updateList(client_fd, server_name, client_nickname);
    }

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

    reason = reason.substr(1);

    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = "Channel " + channel_name + " does not exist.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    std::string sender_nickname = clients[client_fd].getNickname();

    if (!channel.isOperator(sender_nickname)) {
        std::string notOperator = ":" + server_name + " 482 " + sender_nickname + " " + channel.getName() + " :You're not channel operator\r\n";
        send(client_fd, notOperator.c_str(), notOperator.size(), 0);
        return;
    }
    
    if (!channel.isClient(target_nickname)) {
        std::string error_msg = "User " + target_nickname + " is not in channel " + channel_name + ".\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    Client* target_client = channel.getClients()[target_nickname];
    int target_fd = target_client->getFd();

    std::string kick_msg = ":" + sender_nickname + " KICK " + channel_name + " " + target_nickname + " :" + reason + "\r\n";
    std::map<std::string, Client*>& clients_in_channel = channel.getClients();
    for (std::map<std::string, Client*>::iterator it = clients_in_channel.begin(); it != clients_in_channel.end(); ++it) {
        if (send(it->second->getFd(), kick_msg.c_str(), kick_msg.size(), 0) < 0) {
            perror("send");
        }
    }

    channel.removeClient(target_nickname);

    std::string user_kicked_msg = "You have been kicked from " + channel_name + " by " + sender_nickname + " :" + reason + "\r\n";
    if (send(target_fd, user_kicked_msg.c_str(), user_kicked_msg.size(), 0) < 0) {
        perror("send");
    }

    std::cout << "Client " << target_nickname << " was kicked from " << channel_name << " by " << sender_nickname << " with reason: " << reason << std::endl;
}


void Server::handle_topic(int client_fd, const std::string& args) {
    std::string channel_name;
    std::string topic;

    std::istringstream iss(args);
    iss >> channel_name;
    std::getline(iss, topic);

    if (channels.find(channel_name) == channels.end()) {
        std::string msg = "No such channel: " + channel_name + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    Client& client = clients[client_fd];

    if (topic.empty()) {
        std::string msg = "Current topic for " + channel_name + ": " + channel.getTopic() + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    std::string sender_nickname = clients[client_fd].getNickname();

    if (!channel.isOperator(sender_nickname)) {
        std::string notOperator = ":" + server_name + " 482 " + sender_nickname + " " + channel.getName() + " :You're not channel operator\r\n";
        send(client_fd, notOperator.c_str(), notOperator.size(), 0);
        return;
    }

    if (channel.isOperator(client.getNickname()) || channel.getTmode() == false)
    {
        channel.setTopic(topic);
        std::string msg = "Topic for " + channel_name + " set to: " + topic + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
    }
}

void Server::handle_invite(int client_fd, const std::string& args) {
    std::stringstream ss(args);
    std::string target_nickname, channel_name;

    ss >> target_nickname >> channel_name;

    if (target_nickname.empty() || channel_name.empty()) {
        std::string error_msg = "Usage: INVITE <nickname> <channel>\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (args.size() < target_nickname.size() + channel_name.size()) {
        std::string error_msg = "Error: invalid command format.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = "Channel " + channel_name + " does not exist.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    std::string sender_nickname = clients[client_fd].getNickname();

    if (!channel.isOperator(sender_nickname)) {
        std::string notOperator = ":" + server_name + " 482 " + sender_nickname + " " + channel.getName() + " :You're not channel operator\r\n";
        send(client_fd, notOperator.c_str(), notOperator.size(), 0);
        return;
    }

    Client* target_client = NULL;
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == target_nickname) {
            target_client = &it->second;
            break;
        }
    }

    if (!target_client) {
        std::string error_msg = "User " + target_nickname + " not found.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (channel.isClient(target_nickname)) {
        std::string error_msg = target_nickname + " is already in the channel.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    channel.inviteClient(target_nickname, target_client);

    std::string invite_msg = "You have been invited to join channel " + channel_name + " by " + sender_nickname + "\r\n";
    send(target_client->getFd(), invite_msg.c_str(), invite_msg.size(), 0);

    std::string confirm_msg = "You have invited " + target_nickname + " to " + channel_name + ".\r\n";
    send(client_fd, confirm_msg.c_str(), confirm_msg.size(), 0);

    std::cout << "Client " << sender_nickname << " invited " << target_nickname << " to " << channel_name << std::endl;
}

void Server::handle_who(int client_fd, const std::string& args) {
    std::cout << client_fd << " :hello " << args << std::endl;
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
    command_map["MODE"] = &Server::handle_mode;
    command_map["WHO"] = &Server::handle_who;
}
