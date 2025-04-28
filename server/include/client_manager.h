#pragma once
#include <unordered_map>
#include <memory>
#include <QUuid>
#include <vector>
#include <mutex>
#include "client_session_interface.h"
#include "iserver.h"
#include "quuid_hash.h"
#include "message.h"

class ClientManager : public IClientManager {
public:
    ClientManager();
    ~ClientManager() override = default;
    
    ClientManager(const ClientManager&) = delete;
    ClientManager& operator=(const ClientManager&) = delete;
    ClientManager(ClientManager&&) = delete;
    ClientManager& operator=(ClientManager&&) = delete;
    
    void addClient(QUuid clientId, std::unique_ptr<IClientSession> client) override;
    void removeClient(QUuid clientId) override;
    void broadcastMessage(const Message& message) override;
    std::vector<std::string> getUsernames() const override;
    void sendMessageToClient(QUuid clientId, const Message& message) override;
    size_t getClientCount() const override;
    
    void clear();
    const std::vector<Message>& getChatHistory() const;
    void addMessage(const Message& message);

private:
    std::unordered_map<QUuid, std::unique_ptr<IClientSession>, QUuidHash> m_clients;
    std::vector<Message> m_chatHistory;
    mutable std::mutex m_clientsMutex;
    static constexpr size_t MAX_HISTORY_SIZE = 100;
};