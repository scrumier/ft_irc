#include "ft_irc.hpp"

Client::Client() : nickname("nickname"), username("username"), realname("realname"), authenticated(false), admin(false) {}

Client::Client(int fd) : nickname("nickname"), username("username"), realname("realname"), authenticated(false), admin(false), fd(fd) {}

Client::~Client() {}

const Client& Client::operator=(const Client& other) {
	if (this != &other) {
		nickname = other.nickname;
		username = other.username;
		authenticated = other.authenticated;
		admin = other.admin;
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
        if (!std::isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_' &&
            nickname[i] != '[' && nickname[i] != ']' && nickname[i] != '\\' &&
            nickname[i] != '^' && nickname[i] != '{' && nickname[i] != '}') {
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
        if (!std::isalnum(username[i]) && username[i] != '-' && username[i] != '_' &&
            username[i] != '[' && username[i] != ']' && username[i] != '\\' &&
            username[i] != '^' && username[i] != '{' && username[i] != '}') {
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

std::string& Client::getRealname() {
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

std::string& Client::getBuffer() {
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