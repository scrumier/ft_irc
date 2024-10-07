#include "Server.hpp"

Server::Server(int port) {
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

void Server::run() {
    std::cout << "Server is running..." << std::endl;

    while (true) {
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);
        if (poll_count == -1) {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); i++) {
            if (poll_fds[i].fd == server_fd && (poll_fds[i].revents & POLLIN)) {
                handle_new_connection();
            } else if (poll_fds[i].revents & POLLIN) {
                handle_client_data(i);
            }
        }
    }
}

void Server::handle_new_connection() {
    if (poll_fds.size() - 1 >= MAX_CLIENTS) {
        std::cerr << "Max clients reached. Refusing connection." << std::endl;
        int temp_fd = accept(server_fd, NULL, NULL);
        if (temp_fd != -1) {
            const char *reject_message = "Server full, cannot accept more clients.\n";
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
        std::cout << "New client connected: " << client_fd << std::endl;
    }
}

void Server::handle_nick(int client_fd, const std::string& args) {
    std::cout << "Client " << client_fd << " set nickname to " << args << std::endl;
}

void Server::handle_user(int client_fd, const std::string& args) {
    std::cout << "Client " << client_fd << " set username to " << args << std::endl;
}

void Server::handle_join(int client_fd, const std::string& args) {
    std::cout << "Client " << client_fd << " joined channel " << args << std::endl;
}

void Server::handle_privmsg(int client_fd, const std::string& args) {
    std::cout << "Client " << client_fd << " sent message: " << args << std::endl;
}

void Server::initialize_command_map() {
    command_map["NICK"] = &Server::handle_nick;
    command_map["USER"] = &Server::handle_user;
    command_map["JOIN"] = &Server::handle_join;
    command_map["PRIVMSG"] = &Server::handle_privmsg;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}


void Server::handle_client_data(size_t i) {
    char buffer[1024];
    int bytes_received = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        close_client(i);
    } else {
        buffer[bytes_received] = '\0';
        std::string input(buffer);

        std::string command;
        std::string args;
        size_t space_pos = input.find(' ');

        if (input[0] == '/') {
            input = input.substr(1);
        }

        if (space_pos != std::string::npos) {
            command = input.substr(0, space_pos);
            args = input.substr(space_pos);
        } else {
            command = input;
        }

        command = trim(command);
        if (args.size() > 0)
            args = trim(args);

        for (size_t j = 0; j < command.size(); ++j) {
            command[j] = std::toupper(command[j]);
        }

        std::map<std::string, CommandHandler>::iterator it = command_map.find(command);
        if (it != command_map.end()) {
            std::cout << "Executing command handler for: " << command << std::endl;
            (this->*(it->second))(poll_fds[i].fd, args);
        } else {
            if (command == "QUIT") {
                close_client(i);
            } else if (args.size() <= 0) {
                std::string error_msg = "No arguments\n";
                send(poll_fds[i].fd, error_msg.c_str(), error_msg.size(), 0);
            } else {
                std::string error_msg = "Command not recognized\n";
                send(poll_fds[i].fd, error_msg.c_str(), error_msg.size(), 0);
            }
        }
    }
}

void Server::close_client(size_t i) {
    std::cout << "Closing client: " << poll_fds[i].fd << std::endl;
    close(poll_fds[i].fd);
    poll_fds.erase(poll_fds.begin() + i);
    std::cout << "Client disconnected, fd " << poll_fds[i].fd << " removed" << std::endl;
}
