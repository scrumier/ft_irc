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

bool isValidModeString(const std::string& flags) {
    char last_sign = '\0';
    for (size_t i = 0; i < flags.size(); ++i) {
        char c = flags[i];
        
        if (c == ' ') {
            return true;
        } else if (c == '+' || c == '-') {
            if (last_sign == c) {
                return false;
            }
            last_sign = c;
        } else if (isalpha(c)) {
            if (c != 'i' && c != 't' && c != 'k' && c != 'o' && c != 'l') {
                return false;
            }
            last_sign = '\0';
        } else {
            return false;
        }
    }

    return last_sign == '\0';
}

#include <algorithm>

void Channel::updateList(int client_fd, std::string server_name, std::string client_nickname) {
    std::vector<std::string>::iterator it_ope = std::find(operators.begin(), operators.end(), client_nickname);
    std::map<std::string, Client*>::iterator it_client = clients.find(client_nickname);   
    if (it_ope != operators.end() && it_client == clients.end()) {
        operators.erase(it_ope);
        std::cout << "Removed " << client_nickname << " from operators for channel " << name << std::endl;
    }

    if (operators.empty() && !clients.empty()) {
        std::string new_operator = clients.begin()->first;
        operators.push_back(new_operator);
        std::cout << "Assigned " << new_operator << " as operator for channel " << name << std::endl;
    }

    std::string names_list = getNamesList();
    std::cout << "Generated names list for channel " << name << ": " << names_list << std::endl;

    std::string names_reply = ":" + server_name + " 353 " + client_nickname + " = " + name + " :" + names_list + "\r\n";
    send(client_fd, names_reply.c_str(), names_reply.size(), 0);
}

