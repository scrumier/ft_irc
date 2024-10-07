#include "Server.hpp"

int main(int ac, char *av[]) {
    if (ac != 3) {
        std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
        return 1;
    }

    try {
        int port = atoi(av[1]);
        Server server(port, av[2]);
        server.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
