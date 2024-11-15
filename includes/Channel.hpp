#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
class Channel {
private:
    size_t channelLimit;
    size_t clientNumber;
    bool inviteOnly;
    std::string name;
    std::string topic;
    std::string channel_password;
    std::map<std::string, Client*> clients;
    std::map<std::string, Client*> invited_clients;
    std::vector<std::string> operators;

public:
    Channel();
    ~Channel();

    Channel& operator=(const Channel& other);

    size_t getClientNumber() const;
    void setClientNumber(size_t channelLimit);

    bool getInviteOnly() const;
    void setInviteOnly(bool inviteOnly);

    const std::string& getName() const;
    void setName(const std::string& name);

    size_t getChannelLimit() const;
    void setChannelLimit(size_t channelLimit);

    const std::string& getTopic() const;
    void setTopic(const std::string& topic);

    const std::string& getPassword() const;
    void setPassword(const std::string& password);

    const std::map<std::string, Client*>& getClients() const;
    std::map<std::string, Client*>& getClients();
    void removeIfInvitedClient(std::string clientName);


    const std::vector<std::string>& getOperators() const;
    std::vector<std::string>& getOperators();

    const std::map<std::string, Client*>& getInvitedClients() const;
    void inviteClient(const std::string& nickname, Client* client);
    bool isInvited(const std::string& nickname) const;

    void addClient(const std::string& nickname, Client* client);
    void removeClient(const std::string& nickname);

    void addOperator(const std::string& nickname);
    void removeOperator(const std::string& nickname);

    bool isOperator(const std::string& nickname) const;
    bool isClient(const std::string& nickname) const;
};

#endif
