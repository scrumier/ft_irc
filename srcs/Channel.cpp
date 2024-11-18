#include "ft_irc.hpp"

Channel::Channel() : channelLimit(1000), clientNumber(0), tmode(false), inviteOnly(false), name(""), topic(""), channel_password("") {}

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
    if (isOperator(nickname) == false && clients.find(nickname) != clients.end())
    {
        operators.push_back(nickname);
    }
}

void Channel::removeOperator(const std::string& nickname) {
    std::vector<std::string>::iterator pos = std::find(operators.begin(), operators.end(), nickname);
    if (pos != operators.end()) {
        operators.erase(pos);
    }
}

bool Channel::isOperator(const std::string& nickname) const {
    return std::find(operators.begin(), operators.end(), nickname) != operators.end();
}

bool Channel::isClient(const std::string& nickname) const {
    return clients.find(nickname) != clients.end();
}
