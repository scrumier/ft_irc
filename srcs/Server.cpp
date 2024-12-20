#include "ft_irc.hpp"

Server* g_server_instance = NULL;

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
    for (size_t i = 0; i < poll_fds.size(); ++i) {
        if (poll_fds[i].fd != server_fd) {
            close(poll_fds[i].fd);
        }
    }
    close(server_fd);
}

// Signal handler function
void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGQUIT) {
        std::cout << "\nSignal received (" << signum << "), shutting down..." << std::endl;

        if (g_server_instance != NULL) {
            delete g_server_instance;
            g_server_instance = NULL;
        }

        exit(0);
    }
}


void setup_signal_handling() {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));

    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        std::cerr << "Error setting SIGPIPE handler" << std::endl;
        exit(1);
    }

    sa.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "Error setting SIGINT handler" << std::endl;
        exit(1);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        std::cerr << "Error setting SIGQUIT handler" << std::endl;
        exit(1);
    }
}

/*
 * @brief Start the server and handle incoming connections
 * @return void
*/
void Server::run() {
    std::cout << "Server is running..." << std::endl;

    setup_signal_handling();

    while (true) {
        for (size_t i = 0; i < poll_fds.size(); ++i) {
            poll_fds[i].revents = 0;
        }

        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);
        if (poll_count == -1) {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ) {
            if (poll_fds[i].fd < 0) {
                ++i;
                continue;
            }
            
            try {
                if (poll_fds[i].fd == server_fd && (poll_fds[i].revents & POLLIN)) {
                    handle_new_connection();
                    ++i;
                } else if (poll_fds[i].revents & POLLIN) {
                    handle_client_data(i);
                    ++i;
                } else {
                    ++i;
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

    if (bytes_received == 0) {
        throw std::runtime_error("Client disconnected");
    } else if (bytes_received < 0) {
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
		std::cout << "Client is registered: " << std::boolalpha << clients[client_fd].isRegistered() << std::endl;

        if (clients[client_fd].isRegistered() == false) {
            if (command == "PASS" || command == "NICK" || command == "USER" || command == "CAP" || command == "PING" || command == "PONG" || command == "QUIT") {
                std::cout << "Executing command handler for: " << command << std::endl;
                CommandHandler handler = command_map[command];
                (this->*handler)(client_fd, args);
            } else {
                std::string error_msg = ":" + server_name + " 451 " + clients[client_fd].getNickname() + " :You have not registered. Please complete registration.\r\n";
                send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            }
			complete_registration(client_fd);
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

    if (client.isRegistered() == false && client.hasNick() && client.hasUser() && client.isAuthenticated()) {
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

void Server::send_ping(int client_fd) {
    std::string pingMessage = "PING :ServerCheck\r\n";
    send(client_fd, pingMessage.c_str(), pingMessage.length(), 0);
}

/*
 * @brief handle when a new client connects
 * @param client_fd The client file descriptor
*/
void Server::handle_client_data(size_t i) {
    int client_fd = poll_fds[i].fd;
    try {
        this->clientInput += receive_data(client_fd);

        size_t pos;
        while ((pos = this->clientInput.find("\n")) != std::string::npos) {
            std::string input = this->clientInput.substr(0, pos);
            this->clientInput.erase(0, pos + 1);

            std::string command, args;
            parse_command(input, command, args);
            process_command(client_fd, command, args);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client " << client_fd << ": " << e.what() << std::endl;
        close_client(i);
    }
}

/*
 * @brief Close a client connection
 * @param i The index of the client in the poll_fds vector
*/
void Server::close_client(int i) {
    if (i < 0 || static_cast<size_t>(i) >= poll_fds.size()) {
        return;
    }

    int client_fd = poll_fds[i].fd;

    std::map<int, Client>::iterator it = clients.find(client_fd);
    if (it == clients.end()) {
        std::cerr << "Error: Client FD " << client_fd << " not found in clients map" << std::endl;
        return;
    }

    std::string nickname = it->second.getNickname();

    for (std::map<std::string, Channel>::iterator ch_it = channels.begin(); ch_it != channels.end(); ++ch_it) {
        ch_it->second.removeClient(nickname);
        ch_it->second.removeOperator(nickname, server_name);
    }

    close(client_fd);

    clients.erase(client_fd);

    poll_fds.erase(poll_fds.begin() + i);

    std::cout << "Client " << client_fd << " (" << nickname << ") disconnected" << std::endl;
}



bool Server::is_valid_channel_name(const std::string& name) {
    return !name.empty() && name[0] == '#';
}


