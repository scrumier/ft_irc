#include "bot/IRCBot.hpp"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <server> <port> <nickname> <password> <channel>\n";
        return 1;
    }

    std::string server = argv[1];
    int port = atoi(argv[2]);
    std::string nickname = argv[3];
    std::string password = argv[4];
    std::string channel_tmp = argv[5];
    std::string channel = "#" + channel_tmp;

    Bot bot(server, port, nickname, password, channel);

    if (bot.connect_to_server()) {
        bot.run();
    } else {
        std::cerr << "Failed to connect to the server.\n";
        return 1;
    }

    return 0;
}
