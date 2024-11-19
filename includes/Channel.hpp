#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"

class Channel {
private:
    uint16_t channelLimit;
    uint16_t clientNumber;
    bool tmode;
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

    std::string getNamesList();
    void updateList(int client_fd, std::string server_name, std::string nickname);

    void broadcast(std::string const &send_msg);

    std::string getModes() const;

    void sendNumericRepliesToJoiner(Client& joiner, const std::string& server_name);

    uint16_t getClientNumber() const;
    void setClientNumber(uint16_t clientNumber);

    bool getInviteOnly() const;
    void setInviteOnly(bool inviteOnly);

    

    bool getTmode() const;
    void setTmode(bool tmode);

    const std::string& getName() const;
    void setName(const std::string& name);

    uint16_t getChannelLimit() const;
    void setChannelLimit(uint16_t channelLimit);

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
