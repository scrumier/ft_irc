#include "Channel.hpp"

Channel::Channel() : name("name"), topic("topic"), password("password") {}

Channel::~Channel() {}

const Channel& Channel::operator=(const Channel& other) {
	if (this != &other) {
		name = other.name;
		topic = other.topic;
		password = other.password;
		clients = other.clients;
		operators = other.operators;
	}
	return *this;
}

const std::string& Channel::getName() const {
	return name;
}

void Channel::setName(const std::string& name) {
	this->name = name;
}

const std::string& Channel::getTopic() const {
	return topic;
}

void Channel::setTopic(const std::string& topic) {
	this->topic = topic;
}

const std::string& Channel::getPassword() const {
	return password;
}

void Channel::setPassword(const std::string& password) {
	this->password = password;
}

const std::map<std::string, Client>& Channel::getClients() const {
	return clients;
}

std::map<std::string, Client>& Channel::getClients() {
	return clients;
}

const std::vector<std::string>& Channel::getOperators() const {
	return operators;
}

std::vector<std::string>& Channel::getOperators() {
	return operators;
}

void Channel::addClient(const std::string& nickname, const Client& client) {
	clients[nickname] = client;
}

void Channel::removeClient(const std::string& nickname) {
	clients.erase(nickname);
}

void Channel::addOperator(const std::string& nickname) {
	operators.push_back(nickname);
}

void Channel::removeOperator(const std::string& nickname) {
	std::vector<std::string>::iterator pos = std::find(operators.begin(), operators.end(), nickname);
	if (pos != operators.end()) {
		operators.erase(pos);
	}
}

bool Channel::isOperator(const std::string& nickname) {
	return std::find(operators.begin(), operators.end(), nickname) != operators.end();
}

bool Channel::isClient(const std::string& nickname) {
	return clients.find(nickname) != clients.end();
}

