#include "client_manager.h"
#include <QDebug>

ClientManager::ClientManager() = default;

void ClientManager::addClient(QUuid clientId, std::unique_ptr<IClientSession> client) {
    m_clients.emplace(clientId, std::move(client));
    qDebug() << "Client added, UUID:" << clientId;
}

void ClientManager::removeClient(QUuid clientId) {
    m_clients.erase(clientId);
    qDebug() << "Client removed, UUID:" << clientId;
}

void ClientManager::broadcastMessage(const std::string& message, QUuid senderId, const std::string& username) {
    for (const auto& pair : m_clients) {
        if (pair.first != senderId) {
            pair.second->sendMessage(username + ":" + message);
        }
    }
}

void ClientManager::clear() {
    m_clients.clear();
}

const std::unordered_map<QUuid, std::unique_ptr<IClientSession>, QUuidHash>& ClientManager::getClients() const {
    return m_clients;
}