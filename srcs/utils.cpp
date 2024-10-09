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

bool Server::is_valid_channel_name(const std::string& channel) {
    return !channel.empty() && channel[0] == '#' && channel.size() > 1;
}
