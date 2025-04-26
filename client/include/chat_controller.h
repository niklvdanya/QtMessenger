#pragma once
#include "inetwork_client.h"
#include "network_client.h"
#include <memory>
#include <string>

class IChatView;

class ChatController {
public:
    ChatController(std::unique_ptr<INetworkClient> networkClient, IChatView* view);
    void sendMessage(const std::string& message);
    void setUsername(const std::string& username);
    std::string username() const;
    NetworkClient* getNetworkClient() const;
private:
    void setupNetworkCallbacks();
    std::unique_ptr<INetworkClient> m_networkClient;
    IChatView* m_view; 
    std::string m_username;
};