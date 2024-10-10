#include "ft_irc.hpp"

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

    initialize_command_map();
}

Server::~Server() {
    close(server_fd);
}

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

std::string Server::my_trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \r\n");
    return str.substr(first, last - first + 1);
}

bool Server::is_command(const std::string& command) {
    return command_map.find(command) != command_map.end();
}

std::string Server::receive_data(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        return "";
    }

    buffer[bytes_received] = '\0';
    return std::string(buffer);
}

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

    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
}

void Server::process_command(int client_fd, const std::string& command, const std::string& args) {
    if (is_command(command)) {
        std::cout << "Executing command handler for: " << command << std::endl;
        CommandHandler handler = command_map[command];
        (this->*handler)(client_fd, args);
    } else {
        if (!command.empty()) {
            std::string error_msg = "Command not recognized\r\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        }
    }
}

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
#include <stdio.h>
void Server::close_client(size_t i) {
    int client_fd = poll_fds[i].fd;

    close(client_fd);
}


bool Server::is_valid_channel_name(const std::string& name) {
    return !name.empty() && name[0] == '#';
}
