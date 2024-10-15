# include "ft_irc.hpp"

/*
 * @brief Convert an int to a string
 * @param number The number to convert
 * @return The string representation of the number
*/
std::string intToString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

/*
 * @brief Check if a character is a valid nickname character
 * @param c The character to check
 * @return True if the character is a valid nickname character, false otherwise
*/
bool is_valid_nickname_char(char c) {
    return std::isalnum(c) || c == '-' || c == '_' || c == '[' || c == ']' || c == '\\' || c == '^' || c == '{' || c == '}';
}

/*
 * @brief Check if a character is a valid realname character
 * @param c The character to check
 * @return True if the character is a valid realname character, false otherwise
*/
bool is_valid_realname_char(char c) {
    return std::isprint(c);
}

/*
 * @brief Check if a user is in a channel
 * @param clients_in_channel The clients in the channel
 * @param nickname The nickname to check
 * @return True if the user is in the channel, false otherwise
*/
bool user_in_channel(const std::map<std::string, Client*>& clients_in_channel, const std::string& nickname) {
    return clients_in_channel.find(nickname) != clients_in_channel.end();
}