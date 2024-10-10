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
};

#endif // CLIENT_HPP
