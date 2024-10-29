#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ft_irc.hpp"

class Client {
private:
    std::string nickname;
    std::string username;
    std::string realname;
    bool authenticated;
    bool admin;
    std::string buffer;
    int fd;
    bool registered;
    bool has_nick;
    bool has_user;
	time_t time_to_connect;

public:
    Client();
    Client(int fd);
    ~Client();

    Client& operator=(const Client& other);

    std::string getNickname() const;
    void setNickname(const std::string& nickname);

    std::string getUsername() const;
    void setUsername(const std::string& username);

    const std::string& getRealname() const;
    void setRealname(const std::string& realname);

    bool isAuthenticated() const;
    void setAuthenticated(bool authenticated);

    bool isAdmin() const;
    void setAdmin(bool admin);

    const std::string& getBuffer() const;
    void appendToBuffer(const std::string& data);
    void setBuffer(const std::string& buffer);

    int getFd() const;
    void setFd(int fd);

    bool isRegistered() const;
    void setRegistered(bool registered);

    bool hasNick() const;
    void setHasNick(bool has_nick);

    bool hasUser() const;
    void setHasUser(bool has_user);

	time_t getTimeToConnect() const;
	void setTimeToConnect(time_t time_to_connect);
};

#endif
