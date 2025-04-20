#pragma once
#include <unordered_map>
#include <memory>
#include <QUuid>
#include "client_session_interface.h"
#include "iserver.h"
#include "quuid_hash.h"

class ClientManager : public IClientManager {
public:
    ClientManager();
    void addClient(QUuid clientId, std::unique_ptr<IClientSession> client) override;
    void removeClient(QUuid clientId) override;
    void broadcastMessage(const std::string& message, QUuid senderId, const std::string& username) override;
    void clear();
    const std::unordered_map<QUuid, std::unique_ptr<IClientSession>, QUuidHash>& getClients() const;

private:
    std::unordered_map<QUuid, std::unique_ptr<IClientSession>, QUuidHash> m_clients;
};