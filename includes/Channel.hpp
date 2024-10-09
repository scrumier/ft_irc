#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "ft_irc.hpp"

class Channel {
private:
	std::string name;
	std::string topic;
	std::string password;
	std::map<std::string, Client> clients;
	std::vector<std::string> operators;
};

#endif