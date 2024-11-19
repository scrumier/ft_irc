#include "ft_irc.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    try {
        setup_signal_handling();

        g_server_instance = new Server(port, password); // Assign global pointer
        g_server_instance->run();

        delete g_server_instance;  // Cleanup after server stops
        g_server_instance = NULL;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (g_server_instance) {
            delete g_server_instance;
        }
        return 1;
    }

    return 0;
}
