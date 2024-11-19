#include "ft_irc.hpp"

Channel::Channel() : channelLimit(100), clientNumber(0), tmode(false), inviteOnly(false), name(""), topic(""), channel_password("") {}

Channel::~Channel() {}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        name = other.name;
        topic = other.topic;
        channel_password = other.channel_password;
        clients = other.clients;
        operators = other.operators;
    }
    return *this;
}

void Channel::broadcast(const std::string& send_msg) {
    for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client* client = it->second;
        if (client) {
            int client_fd = client->getFd();
            send(client_fd, send_msg.c_str(), send_msg.size(), 0);
        }
    }
    std::cout << send_msg << std::endl;
}

std::string Channel::getNamesList() {
    std::string result;
    for (std::map<std::string, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (isOperator(it->first)) {
            result += "@"; // Prefix with @ if the user is an operator
        }
        result += it->first; // Add the nickname
        result += " "; // Separate nicknames with a space
    }
    if (!result.empty()) {
        result.erase(result.size() - 1); // Remove the trailing space
    }
    return result;
}


uint16_t Channel::getClientNumber() const {
    return clientNumber;
}

void Channel::setClientNumber(uint16_t clientNumber) {
    this->clientNumber = clientNumber;
}

void Channel::removeIfInvitedClient(std::string clientName) {
    if (invited_clients.find(clientName) != invited_clients.end())
        invited_clients.erase(clientName);
}

bool Channel::getInviteOnly() const {
    return this->inviteOnly;
}

void Channel::setInviteOnly(bool inviteOnly) {
    this->inviteOnly = inviteOnly;
}

bool Channel::getTmode() const {
    return this->tmode;
}

void Channel::setTmode(bool tmode) {
    this->tmode = tmode;
}

uint16_t Channel::getChannelLimit() const {
    return this->channelLimit;
}

void Channel::setChannelLimit(uint16_t channelLimit) {
    this->channelLimit = channelLimit;
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

const std::map<std::string, Client*>& Channel::getInvitedClients() const {
    return invited_clients;
}

void Channel::inviteClient(const std::string& nickname, Client* client) {
    invited_clients[nickname] = client;
}

bool Channel::isInvited(const std::string& nickname) const {
    return invited_clients.find(nickname) != invited_clients.end();
}

void Channel::setTopic(const std::string& new_topic) {
    this->topic = new_topic;
}

const std::string& Channel::getPassword() const {
    return channel_password;
}

void Channel::setPassword(const std::string& password) {
    this->channel_password = password;
}

const std::map<std::string, Client*>& Channel::getClients() const {
    return clients;
}

std::map<std::string, Client*>& Channel::getClients() {
    return clients;
}

const std::vector<std::string>& Channel::getOperators() const {
    return operators;
}

std::vector<std::string>& Channel::getOperators() {
    return operators;
}

void Channel::addClient(const std::string& nickname, Client* client) {
    if (clientNumber < 1000)
    {
        clients[nickname] = client;
        clientNumber++;
    }
}

void Channel::removeClient(const std::string& nickname) {
    if (clientNumber > 0)
    {
        clients.erase(nickname);
        clientNumber--;
    }
}

void Channel::addOperator(const std::string& nickname) {
    if (isOperator(nickname) == false && clients.find(nickname) != clients.end()) {
        operators.push_back(nickname);
        std::string promotedOperator = ":localhost:6669 MODE " + getName() + " +o " + nickname + "\r\n";
        broadcast(promotedOperator);
        std::cout << "Client " << nickname << " promoted to operator in channel " << getName() << std::endl;
    }
}


void Channel::removeOperator(const std::string& nickname) {
    std::vector<std::string>::iterator pos = std::find(operators.begin(), operators.end(), nickname);
    if (pos != operators.end()) {
        operators.erase(pos);

        std::string demotedOperator = ":localhost:6669 MODE " + getName() + " -o " + nickname + "\r\n";
        broadcast(demotedOperator);

        std::cout << "Client " << nickname << " removed as operator from channel " << getName() << std::endl;
    }
}

bool Channel::isOperator(const std::string& nickname) const {
    return std::find(operators.begin(), operators.end(), nickname) != operators.end();
}

bool Channel::isClient(const std::string& nickname) const {
    return clients.find(nickname) != clients.end();
}
