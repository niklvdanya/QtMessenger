#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "inetwork_client.h"
#include "network_client.h"

class IChatView
{
public:
    virtual ~IChatView() = default;
    virtual void displaySystemMessage(const std::string& message) = 0;
    virtual void displayChatMessage(const Message& message) = 0;
    virtual std::string getInputText() = 0;
    virtual void clearInput() = 0;
    virtual void updateConnectionStatus(bool connected) = 0;
    virtual void updateUserList(const std::vector<QString>& userList) = 0;
};

class ChatController
{
public:
    ChatController(std::unique_ptr<INetworkClient> networkClient, IChatView* view);

    void sendMessage(const std::string& message);
    void sendMessage();
    void requestUserList();
    void setUsername(const std::string& username);
    std::string username() const;
    NetworkClient* getNetworkClient() const;

    void connectToServer(const std::string& host, uint16_t port, const std::string& username,
                         const std::string& password);
    void disconnect();

private:
    void setupNetworkCallbacks();
    std::unique_ptr<INetworkClient> m_networkClient;
    IChatView* m_view;
    std::string m_username;
};