#include "ft_irc.hpp"


/*
 * @brief Init all the data and start the server
 * @param port The port to listen on
 * @param password The password to require for clients to connect
 * @return void
*/
Server::Server(int port, const std::string& password) : password(password) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Socket creation failed");
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(server_fd);
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd, BACKLOG) == -1) {
        close(server_fd);
        throw std::runtime_error("Listen failed");
    }

    struct pollfd server_poll_fd;
    server_poll_fd.fd = server_fd;
    server_poll_fd.events = POLLIN;
    poll_fds.push_back(server_poll_fd);

    server_name = "localhost:" + intToString(port);
    server_version = "1.0";
    time_t now = time(0);
    char time[64];
    strftime(time, sizeof(time), "%a %b %d %H:%M:%S %Y", gmtime(&now));
    server_creation_date = time;
    requires_password = !password.empty();

    initialize_command_map();
}

Server::~Server() {
    close(server_fd);
}

/*
 * @brief Start the server and handle incoming connections
 * @return void
*/
void Server::run() {
    std::cout << "Server is running..." << std::endl;

    while (true) {
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);
        if (poll_count == -1) {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); i++) {
            try {
                if (poll_fds[i].fd == server_fd && (poll_fds[i].revents & POLLIN)) {
                    handle_new_connection();
                } else if (poll_fds[i].revents & POLLIN) {
                    handle_client_data(i);
                }
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                close_client(i);
            }
        }
    }
}

/*
 * @brief Add a new client to the server (in the map of clients)
 * @return void
*/
void Server::handle_new_connection() {
    if (poll_fds.size() - 1 >= MAX_CLIENTS) {
        std::cerr << "Max clients reached. Refusing connection." << std::endl;
        int temp_fd = accept(server_fd, NULL, NULL);
        if (temp_fd != -1) {
            const char *reject_message = "Server full, cannot accept more clients.\r\n";
            send(temp_fd, reject_message, strlen(reject_message), 0);
            close(temp_fd);
        }
        return;
    }

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        std::cerr << "Accept failed" << std::endl;
    } else {
        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
            std::cerr << "Failed to set client socket to non-blocking" << std::endl;
            close(client_fd);
            return;
        }
        struct pollfd client_poll_fd;
        client_poll_fd.fd = client_fd;
        client_poll_fd.events = POLLIN;
        poll_fds.push_back(client_poll_fd);

        std::string name = "Guest" + intToString(client_fd);
        clients[client_fd] = Client(client_fd);
        clients[client_fd].setNickname(name);

        std::cout << "New client connected: " << client_fd << std::endl;

        std::string welcome_msg = "Welcome to the server, " + name + "\r\n";
        if (send(client_fd, welcome_msg.c_str(), welcome_msg.size(), 0) == -1) {
            std::cerr << "Failed to send welcome message to client " << client_fd << std::endl;
        }
    }
}

/*
 * @brief Trim whitespace and newlines from a string
 * @param str The string to trim
 * @return The trimmed string
*/
std::string Server::my_trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \r\n");
    return str.substr(first, last - first + 1);
}

/*
 * @brief Check if a command is valid
 * @param command The command to check
 * @return True if the command is valid, false otherwise
*/
bool Server::is_command(const std::string& command) {
    return command_map.find(command) != command_map.end();
}

/*
 * @brief Receive data from a client
 * @param client_fd The client file descriptor
 * @return The received data
*/
std::string Server::receive_data(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        return "";
    }

    buffer[bytes_received] = '\0';
    return std::string(buffer);
}

/*
 * @brief Parse a command into a command and arguments
 * @param input The input string
 * @param command The command
 * @param args The arguments
 * @return void
*/
void Server::parse_command(const std::string& input, std::string& command, std::string& args) {
    std::string trimmed_input = my_trim(input);
    size_t space_pos = input.find(' ');

    if (trimmed_input[0] == '/') {
        trimmed_input = trimmed_input.substr(1);
    }    

    if (space_pos != std::string::npos) {
        command = trimmed_input.substr(0, space_pos);
        args = trimmed_input.substr(space_pos + 1);
    } else {
        command = trimmed_input;
        args = "";
    }

    std::cout << "Command: |" << command << "|" << std::endl;
    std::cout << "Args: |" << args << "|" << std::endl;

    command = my_trim(command);
    if (!args.empty()) {
        args = my_trim(args);
    }

    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
}

/*
 * @brief Process a command from a client (Execute what command can the client do)
 * @param client_fd The client file descriptor
 * @param command The command
 * @param args The arguments
 * @return void
*/
void Server::process_command(int client_fd, const std::string& command, const std::string& args) {
    if (is_command(command)) {
        if (!clients[client_fd].isRegistered()) {
            if (command == "PASS" || command == "NICK" || command == "USER" || command == "CAP" || command == "PING" || command == "PONG" || command == "QUIT") {
                std::cout << "Executing command handler for: " << command << std::endl;
                CommandHandler handler = command_map[command];
                (this->*handler)(client_fd, args);
            } else {
                std::string error_msg = ":" + server_name + " 451 " + clients[client_fd].getNickname() + " :You have not registered. Please complete registration.\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            }
        } else {
            std::cout << "Executing command handler for: " << command << std::endl;
            CommandHandler handler = command_map[command];
            (this->*handler)(client_fd, args);
        }
    } else {
        if (!command.empty()) {
            std::string error_msg = ":" + server_name + " 421 " + clients[client_fd].getNickname() + " " + command + " :Unknown command\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        }
    }
}

/*
 * @brief Check if all information is provided and complete the registration
 * @param client_fd The client file descriptor
 * @return void
*/
void Server::complete_registration(int client_fd) {
    Client& client = clients[client_fd];

    if (!client.isRegistered() && client.hasNick() && client.hasUser()) {
        if (requires_password && !client.isAuthenticated()) {
            std::string error_msg = ":" + server_name + " 464 " + client.getNickname() + " :Password incorrect\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            close_client(client_fd);
            return;
        }
        client.setRegistered(true);
        std::string nickname = client.getNickname();

        std::string welcome_msg = ":" + server_name + " 001 " + nickname + " :Welcome to the IRC network, " + nickname + "\r\n";
        send(client_fd, welcome_msg.c_str(), welcome_msg.size(), 0);

        std::string yourhost_msg = ":" + server_name + " 002 " + nickname + " :Your host is " + server_name + ", running version " + server_version + "\r\n";
        send(client_fd, yourhost_msg.c_str(), yourhost_msg.size(), 0);

        std::string created_msg = ":" + server_name + " 003 " + nickname + " :This server was created " + server_creation_date + "\r\n";
        send(client_fd, created_msg.c_str(), created_msg.size(), 0);

        std::string myinfo_msg = ":" + server_name + " 004 " + nickname + " " + server_name + " " + server_version + " o o\r\n";
        send(client_fd, myinfo_msg.c_str(), myinfo_msg.size(), 0);

        std::string isupport_msg = ":" + server_name + " 005 " + nickname + " :are supported by this server\r\n";
        send(client_fd, isupport_msg.c_str(), isupport_msg.size(), 0);

        std::cout << "Client " << client_fd << " registered as " << nickname << std::endl;
    }
}

/*
 * @brief handle when a new client connects
 * @param client_fd The client file descriptor
*/
void Server::handle_client_data(size_t i) {
    std::string input = receive_data(poll_fds[i].fd);

    if (input.empty()) {
        close_client(i);
    } else {
        std::string command, args;
        parse_command(input, command, args);
        process_command(poll_fds[i].fd, command, args);
    }
}

void Server::handle_topic(int client_fd, const std::string& args) {
    std::string channel_name;
    std::string topic;

    // Sépare le nom du canal des arguments (sujet)
    std::istringstream iss(args);
    iss >> channel_name;  // Le premier mot est le nom du canal
    std::getline(iss, topic);  // Le reste est le sujet

    // Vérifie si le canal existe
    if (channels.find(channel_name) == channels.end()) {
        std::string msg = "No such channel: " + channel_name + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    Client& client = clients[client_fd];

    // Si aucun sujet n'est fourni, afficher le sujet actuel
    if (topic.empty()) {
        std::string msg = "Current topic for " + channel_name + ": " + channel.getTopic() + "\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    // Vérifie si le client est opérateur du canal
    if (!channel.isOperator(client.getNickname())) {
        std::string msg = "You are not an operator of this channel.\r\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        return;
    }

    // Met à jour le sujet du canal
    channel.setTopic(topic);
    std::string msg = "Topic for " + channel_name + " set to: " + topic + "\r\n";
    send(client_fd, msg.c_str(), msg.size(), 0);
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

    // Vérifier la taille du string avant d'utiliser substr (sécurité supplémentaire)
    if (args.size() < target_nickname.size() + channel_name.size()) {
        std::string error_msg = "Error: invalid command format.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Vérification de l'existence du canal
    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = "Channel " + channel_name + " does not exist.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    Channel& channel = channels[channel_name];
    std::string sender_nickname = clients[client_fd].getNickname();

    // Vérification que l'utilisateur est un opérateur
    if (!channel.isOperator(sender_nickname)) {
        std::string error_msg = "You must be a channel operator to invite users.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Vérification que l'utilisateur à inviter existe et est connecté
    Client* target_client = NULL;  // Utilisation de NULL au lieu de nullptr
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

    // Vérification que la cible n'est pas déjà dans le canal
    if (channel.isClient(target_nickname)) {
        std::string error_msg = target_nickname + " is already in the channel.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Ajout de l'invité à la liste d'invités du canal
    channel.inviteClient(target_nickname, target_client);

    // Notifier l'utilisateur invité
    std::string invite_msg = "You have been invited to join channel " + channel_name + " by " + sender_nickname + "\r\n";
    send(target_client->getFd(), invite_msg.c_str(), invite_msg.size(), 0);

    // Notifier l'opérateur qui a invité
    std::string confirm_msg = "You have invited " + target_nickname + " to " + channel_name + ".\r\n";
    send(client_fd, confirm_msg.c_str(), confirm_msg.size(), 0);

    std::cout << "Client " << sender_nickname << " invited " << target_nickname << " to " << channel_name << std::endl;
}

/*
 * @brief Close a client connection
 * @param i The index of the client in the poll_fds vector
*/
void Server::close_client(size_t i) {
    int client_fd = poll_fds[i].fd;

    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        it->second.removeClient(clients[client_fd].getNickname());
    }
    poll_fds.erase(poll_fds.begin() + i);
    clients.erase(client_fd);
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        it->second.removeOperator(clients[client_fd].getNickname());
    }

    std::cout << "Client " << client_fd << " disconnected" << std::endl;

    close(client_fd);
}

bool Server::is_valid_channel_name(const std::string& name) {
    return !name.empty() && name[0] == '#';
}


