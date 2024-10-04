#include "irc.hpp"

int main(int ac, char *av[]) {
    if (ac != 3) {
        std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
        return 1;
    }
    
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    std::vector<struct pollfd> poll_fds;  // List of file descriptors to monitor

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK); // Set non-blocking mode for server socket

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(av[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, BACKLOG) == -1) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return 1;
    }

    // Add server socket to poll list
    struct pollfd server_poll_fd;
    server_poll_fd.fd = server_fd;
    server_poll_fd.events = POLLIN;
    poll_fds.push_back(server_poll_fd);

    std::cout << "Server is listening on port " << av[1] << std::endl;

    // Main loop
    while (true) {
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);  // Block indefinitely
        if (poll_count == -1) {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); i++) {
            // Check if there's a new connection
            if (poll_fds[i].fd == server_fd && (poll_fds[i].revents & POLLIN)) {
                // Accept a new connection if MAX_CLIENTS is not reached
				if (poll_fds.size() - 1 >= MAX_CLIENTS) {
					std::cerr << "Max clients reached. Refusing connection." << std::endl;
					int temp_fd = accept(server_fd, NULL, NULL);
					if (temp_fd != -1) {
						const char *reject_message = "Server full, cannot accept more clients.\n";
						send(temp_fd, reject_message, strlen(reject_message), 0);
						close(temp_fd);  // Immediately close the connection
					}
					poll_fds[i].revents = 0;  // Reset revents
					continue;
				}
                client_fd = accept(server_fd, NULL, NULL);
                if (client_fd == -1) {
                    std::cerr << "Accept failed" << std::endl;
                } else {
                    // Set client socket to non-blocking mode
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    
                    // Add the new client to poll list
                    struct pollfd client_poll_fd;
                    client_poll_fd.fd = client_fd;
                    client_poll_fd.events = POLLIN;
                    poll_fds.push_back(client_poll_fd);

                    std::cout << "New client connected: " << client_fd << std::endl;
                }
            }
            // Check if a client has sent data
            else if (poll_fds[i].revents & POLLIN) {
                char buffer[1024];
                int bytes_received = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received <= 0) {
                    std::cout << "Client disconnected: " << poll_fds[i].fd << std::endl;
                    close(poll_fds[i].fd);  // Close the socket
                    poll_fds.erase(poll_fds.begin() + i);  // Remove from poll list
                } else {
                    buffer[bytes_received] = '\0';
                    std::cout << "Received: " << buffer << " from " << poll_fds[i].fd << std::endl;
                    // Process commands here
                }
            }
        }
    }

    close(server_fd);
    return 0;
}