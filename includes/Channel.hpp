#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "ft_irc.hpp"
# include "Client.hpp"

class Channel {
private:
	std::string name;
	std::string topic;
	std::string password;
	std::map<std::string, Client> clients;
	std::vector<std::string> operators;
public:
	Channel();
	~Channel();
	const Channel& operator=(const Channel& other);
	const std::string& getName() const;
	void setName(const std::string& name);
	const std::string& getTopic() const;
	void setTopic(const std::string& topic);
	const std::string& getPassword() const;
	void setPassword(const std::string& password);
	const std::map<std::string, Client>& getClients() const;
	std::map<std::string, Client>& getClients();
	const std::vector<std::string>& getOperators() const;
	std::vector<std::string>& getOperators();
	void addClient(const std::string& nickname, const Client& client);
	void removeClient(const std::string& nickname);
	void addOperator(const std::string& nickname);
	void removeOperator(const std::string& nickname);
	bool isOperator(const std::string& nickname);
	bool isClient(const std::string& nickname);
};

#endif