#include "ft_irc.hpp"

Client::Client() : nickname(""), username(""), realname(""), authenticated(false), admin(false), fd(-1),
                   registered(false), has_nick(false), has_user(false), time_to_connect(time(NULL)), 
                   last_activity_time(time(NULL)) {}

Client::Client(int fd) : nickname(""), username(""), realname(""), authenticated(false), admin(false), fd(fd), 
                         registered(false), has_nick(false), has_user(false), time_to_connect(time(NULL)), 
                         last_activity_time(time(NULL)) {}

Client::~Client() {}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        nickname = other.nickname;
        username = other.username;
        realname = other.realname;
        authenticated = other.authenticated;
        admin = other.admin;
        fd = other.fd;
        buffer = other.buffer;
    }
    return *this;
}

std::string Client::getNickname() const {
    return nickname;
}

void Client::setNickname(const std::string& nickname) {
    if (nickname.length() < 1 || nickname.length() > 9) {
        return;
    }

    for (size_t i = 0; i < nickname.length(); ++i) {
        if (!std::isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_') {
            return;
        }
    }
    this->nickname = nickname;
}

std::string Client::getUsername() const {
    return username;
}

void Client::setUsername(const std::string& username) {
    for (size_t i = 0; i < username.length(); ++i) {
        if (!std::isalnum(username[i]) && username[i] != '-' && username[i] != '_') {
            return;
        }
    }
    this->username = username;
}

void Client::setRealname(const std::string& realname) {
    for (size_t i = 0; i < realname.length(); ++i) {
        if (!std::isprint(realname[i])) {
            return;
        }
    }
    this->realname = realname;
}

const std::string& Client::getRealname() const {
    return realname;
}

bool Client::isAuthenticated() const {
    return authenticated;
}

void Client::setAuthenticated(bool authenticated) {
    this->authenticated = authenticated;
}

bool Client::isAdmin() const {
    return admin;
}

void Client::setAdmin(bool admin) {
    this->admin = admin;
}

const std::string& Client::getBuffer() const {
    return buffer;
}

void Client::appendToBuffer(const std::string& data) {
    buffer += data;
}

void Client::setBuffer(const std::string& buffer) {
    this->buffer = buffer;
}

int Client::getFd() const {
    return fd;
}

void Client::setFd(int fd) {
    this->fd = fd;
}

bool Client::isRegistered() const {
    return registered;
}

void Client::setRegistered(bool registered) {
    this->registered = registered;
}

bool Client::hasNick() const {
    return has_nick;
}

void Client::setHasNick(bool has_nick) {
    this->has_nick = has_nick;
}

bool Client::hasUser() const {
    return has_user;
}

void Client::setHasUser(bool has_user) {
    this->has_user = has_user;
}

time_t Client::getTimeToConnect() const {
	return time_to_connect;
}

void Client::setTimeToConnect(time_t time_to_connect) {
	this->time_to_connect = time_to_connect;
}

time_t Client::getLastActivityTime() const {
    return last_activity_time;
}

void Client::setLastActivityTime(time_t lastActivityTime) {
    last_activity_time = lastActivityTime;
}
