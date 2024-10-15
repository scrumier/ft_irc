# include "ft_irc.hpp"

std::string intToString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

bool is_valid_nickname_char(char c) {
    return std::isalnum(c) || c == '-' || c == '_' || c == '[' || c == ']' || c == '\\' || c == '^' || c == '{' || c == '}';
}

bool is_valid_realname_char(char c) {
    return std::isprint(c);
}

bool user_in_channel(const std::map<std::string, Client*>& clients_in_channel, const std::string& nickname) {
    return clients_in_channel.find(nickname) != clients_in_channel.end();
}