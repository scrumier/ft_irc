#include "ft_irc.hpp"

void Server::handle_invite_only_mode(int client_fd, Channel& channel, bool adding_mode) {
    std::string client_nickname = clients[client_fd].getNickname();

    if (adding_mode) {
        if (channel.getInviteOnly()) {
            std::string already_enabled_msg = ":" + server_name + " 324 " + client_nickname + " " + channel.getName() + " +i :Invite-only mode is already enabled\r\n";
            send(client_fd, already_enabled_msg.c_str(), already_enabled_msg.size(), 0);
            return;
        }

        channel.setInviteOnly(true);
        std::string success_msg = ":" + server_name + " MODE " + channel.getName() + " +i\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
        std::cout << "Invite-only mode enabled for channel " << channel.getName() << std::endl;

    } else {
        if (!channel.getInviteOnly()) {
            std::string already_disabled_msg = ":" + server_name + " 324 " + client_nickname + " " + channel.getName() + " -i :Invite-only mode is already disabled\r\n";
            send(client_fd, already_disabled_msg.c_str(), already_disabled_msg.size(), 0);
            return;
        }

        channel.setInviteOnly(false);
        std::string success_msg = ":" + server_name + " MODE " + channel.getName() + " -i\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
        std::cout << "Invite-only mode disabled for channel " << channel.getName() << std::endl;
    }
}

void Server::handle_channel_key_mode(int client_fd, Channel& channel, bool adding_mode, const std::string& parameters) {
    std::string client_nickname = clients[client_fd].getNickname();

    if (adding_mode) {
        channel.setPassword(parameters);
    } else {
        channel.setPassword("");
    }
}

void Server::handle_operator_mode(int client_fd, Channel& channel, bool adding_mode, const std::string& parameters) {
    std::string client_nickname = clients[client_fd].getNickname();

    if (adding_mode) {
        if (channel.isOperator(parameters)) {
            std::string error_msg = ":" + server_name + " 481 " + client_nickname + " :User is already an operator";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        if (channel.getClients().find(parameters) == channel.getClients().end()) {
            std::string error_msg = ":" + server_name + " 441 " + client_nickname + " " + parameters + " :They aren't on the channel";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        channel.addOperator(parameters);

        std::string promotion_msg = ":" + server_name + " 381 " + client_nickname + " " + parameters + " :User is now an operator";
        send(client_fd, promotion_msg.c_str(), promotion_msg.size(), 0);

        std::cout << "User " << parameters << " has been promoted to operator by " << client_nickname << std::endl;

    } else {
        if (!channel.isOperator(parameters)) {
            std::string error_msg = ":" + server_name + " 481 " + client_nickname + " :User is not an operator";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            return;
        }

        channel.removeOperator(parameters);

        std::string unpromotion_msg = ":" + server_name + " 381 " + client_nickname + " " + parameters + " :User is no longer an operator";
        send(client_fd, unpromotion_msg.c_str(), unpromotion_msg.size(), 0);

        std::cout << "User " << parameters << " has been unpromoted from operator by " << client_nickname << std::endl;
    }
}


void Server::handle_topic_restriction_mode(int client_fd, Channel& channel, bool adding_mode) {
    std::string client_nickname = clients[client_fd].getNickname();
    if (adding_mode) {
        if (channel.getTmode()) {
            std::string already_enabled_msg = ":" + server_name + " 324 " + client_nickname + " " + channel.getName() + " +t :topic restriction mode is already enabled\r\n";
            send(client_fd, already_enabled_msg.c_str(), already_enabled_msg.size(), 0);
            return;
        }

        channel.setTmode(true);
        std::string success_msg = ":" + server_name + " MODE " + channel.getName() + " +t\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
        std::cout << "Topic-restriction mode enabled for channel " << channel.getName() << std::endl;

    } else {
        if (!channel.getTmode()) {
            std::string already_disabled_msg = ":" + server_name + " 324 " + client_nickname + " " + channel.getName() + " -t :Topic restriction mode is already disabled\r\n";
            send(client_fd, already_disabled_msg.c_str(), already_disabled_msg.size(), 0);
            return;
        }

        channel.setTmode(false);
        std::string success_msg = ":" + server_name + " MODE " + channel.getName() + " -i\r\n";
        send(client_fd, success_msg.c_str(), success_msg.size(), 0);
        std::cout << "Topic restriction mode disabled for channel " << channel.getName() << std::endl;
    }
}

void Server::handle_user_limit_mode(int client_fd, Channel& channel, bool adding_mode, const std::string& parameters) {
    std::string client_nickname = clients[client_fd].getNickname();

    int limit = atoi(parameters.c_str());

    std::string invalid_param_msg = ":" + server_name + " 467 " + client_nickname + " :Invalid channel limit\r\n";
    std::string already_disabled_msg = ":" + server_name + " 500 " + client_nickname + " :Channel limit already disabled\r\n";

    if (adding_mode) {
        if (limit > 0 && limit < 1000) {
            channel.setChannelLimit(static_cast<uint16_t>(limit));
            std::string success_msg = ":" + server_name + " MODE " + channel.getName() + " +l " + parameters + "\r\n";
            send(client_fd, success_msg.c_str(), success_msg.size(), 0);
        } else {
            send(client_fd, invalid_param_msg.c_str(), invalid_param_msg.size(), 0);
        }
    } else {
        if (channel.getChannelLimit() != 1000) {
            channel.setChannelLimit(1000);
            std::string limit_removed_msg = ":" + server_name + " MODE " + channel.getName() + " -l\r\n";
            send(client_fd, limit_removed_msg.c_str(), limit_removed_msg.size(), 0);
        } else {
            send(client_fd, already_disabled_msg.c_str(), already_disabled_msg.size(), 0);
        }
    }
}

std::string Channel::getModes() const {
    std::string modes = "+";
    if (inviteOnly)
        modes += "i";
    if (tmode)
        modes += "t";
    if (!channel_password.empty())
        modes += "k";
    if (channelLimit < 1000)
        modes += "l";
    return modes;
}

void Server::handle_mode(int client_fd, const std::string& args) {
    size_t first_space = args.find(' ');
    size_t second_space = args.find(' ', first_space + 1);
    
    if (first_space == std::string::npos) {
        std::string error_msg = "Usage: /MODE <channel> <flags> [parameters]\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    
    std::string channel_name = my_trim(args.substr(0, first_space));
    std::string flags = my_trim(args.substr(first_space + 1));
    std::string parameters;
    if (second_space != std::string::npos) {
        parameters = my_trim(args.substr(second_space + 1));
    } else {
        parameters = "";
    }

    if (channels.find(channel_name) == channels.end()) {
        std::string error_msg = "Channel " + channel_name + " does not exist.\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    if (!isValidModeString(flags)) {
        std::string error_msg = "Invalid mode string: " + flags + "\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }
    Channel& channel = channels[channel_name];

    if (flags.empty()) {
        std::string current_modes = channel.getModes(); // Assume this function returns a string like "+i +t -k"
        std::string response = ":" + server_name + " 324 " + clients[client_fd].getNickname() + " " + channel_name + " " + current_modes + "\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }

    if (!isValidModeString(flags)) {
        std::string error_msg = "Invalid mode string: " + flags + "\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    bool adding_mode = true;
    for (size_t i = 0; i < flags.size(); ++i) {
        char flag = flags[i];

        if (flag == ' ') {
            return;
        } else if (flag == '+') {
            adding_mode = true;
        } else if (flag == '-') {
            adding_mode = false;
        } else {
            switch (flag) {
                case 'i':
                    handle_invite_only_mode(client_fd, channel, adding_mode);
                    break;
                case 't':
                    handle_topic_restriction_mode(client_fd, channel, adding_mode);
                    break;
                case 'k':
                    handle_channel_key_mode(client_fd, channel, adding_mode, parameters);
                    break;
                case 'o':
                    handle_operator_mode(client_fd, channel, adding_mode, parameters);
                    break;
                case 'l':
                    handle_user_limit_mode(client_fd, channel, adding_mode, parameters);
                    break;
                default:
                    std::string error_msg = "Unknown mode flag: " + std::string(1, flag) + "\r\n";
                    send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                    break;
            }
        }
    }
}
