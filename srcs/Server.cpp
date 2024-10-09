#include "Server.hpp"

Server::Server() {}

/*
* @brief Initialize the class members
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

    initialize_command_map();
}

Server::~Server() {
    close(server_fd);
}

const Server& Server::operator=(const Server& other) {
    if (this != &other) {
        server_fd = other.server_fd;
        poll_fds = other.poll_fds;
        command_map = other.command_map;
    }
    return *this;
}

/*
* @brief The main server loop
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
* @brief Handle a new connection (Accept or refuse)
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
    if (client_fd == -1) {
        std::cerr << "Accept failed" << std::endl;
    } else {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        struct pollfd client_poll_fd;
        client_poll_fd.fd = client_fd;
        client_poll_fd.events = POLLIN;
        poll_fds.push_back(client_poll_fd);

        std::string name = "Guest" + intToString(client_fd);
        clients[client_fd] = Client(client_fd);
        clients[client_fd].setNickname(name);

        std::cout << "New client connected: " << client_fd << std::endl;
        std::string welcome_msg = "Welcome to the server, " + name + "\r\n";
    }
}

/*
* @brief Trim the " " and "\r\n" characters from the string
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
* @brief check if the command is in the command map
* @param command The command to check
*/
bool Server::is_command(std::string command)
{
    for (std::map<std::string, CommandHandler>::iterator it = command_map.begin();
                    it != command_map.end(); ++it) {
        if (it->first == command) {
            return true;
        }
    }
    return false;
}

/*
* @brief Receive data from the client
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
* @brief Take the input string and split it into command and arguments
* @param input The input string
* @param command The command to process
* @param args The arguments of the command
* @return void
*/
void Server::parse_command(const std::string& input, std::string& command, std::string& args) {
    std::string trimmed_input = my_trim(input);
    size_t space_pos = trimmed_input.find(' ');

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

    command = my_trim(command);
    if (!args.empty()) {
        args = my_trim(args);
    }

    for (size_t j = 0; j < command.size(); ++j) {
        command[j] = std::toupper(command[j]);
    }
}


/*
* @brief Execute the command if the command is valid
* @param client_fd The client file descriptor
* @param command The command to process
* @param args The arguments of the command
* @return void
*/
void Server::process_command(int client_fd, const std::string& command, const std::string& args) {
    //std::cout << "Received command: |" << command << "|" << std::endl;
    //std::cout << "Received args: |" << args << "|" << std::endl;

    if (is_command(command)) {
        std::cout << "Executing command handler for: " << command << std::endl;
        std::map<std::string, CommandHandler>::iterator it = command_map.find(command);
        (this->*(it->second))(client_fd, args);
    } else {
        if (!command.empty()) {
            std::string error_msg = "Command not recognized\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        }
    }
}

/*
* @brief Check if there is a "\r\n" at the end of the buffer
* @param buffer The buffer to check
* @return true if there is no "\r\n" in the buffer, false otherwise
*/
bool notRNInBuffer(std::string buffer) {
    if (buffer.find("\r\n") == std::string::npos) {
        return true;
    }
    return false;
}

/*
* @brief Handle the client data
* @param i The index of the client
* @return void
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

/*
* @brief Close the client connection
* @param i The index of the client
* @return void
*/
void Server::close_client(size_t i) {
    std::cout << "Client " << poll_fds[i].fd << " disconnected" << std::endl;
    close(poll_fds[i].fd);
    poll_fds.erase(poll_fds.begin() + i);
    clients.erase(poll_fds[i].fd);
}
