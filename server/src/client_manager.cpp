#include "client_manager.h"
#include <QDebug>

ClientManager::ClientManager() {
    m_chatHistory.reserve(MAX_HISTORY_SIZE);
}

void ClientManager::addClient(QUuid clientId, std::unique_ptr<IClientSession> client) {
    m_clients.emplace(clientId, std::move(client));
    qDebug() << "Client added, UUID:" << clientId;
}

void ClientManager::removeClient(QUuid clientId) {
    m_clients.erase(clientId);
    qDebug() << "Client removed, UUID:" << clientId;
}

void ClientManager::broadcastMessage(const std::string& message, QUuid senderId, const std::string& username) {
    Message msg;
    msg.senderId = senderId;
    msg.username = username;
    msg.text = message;
    msg.timestamp = QDateTime::currentDateTime();
    addMessage(msg);
    for (const auto& pair : m_clients) {
        pair.second->sendMessage(msg);
    }
}

void ClientManager::addMessage(const Message& message) {
    if (m_chatHistory.size() >= MAX_HISTORY_SIZE) {
        m_chatHistory.erase(m_chatHistory.begin());
    }
    m_chatHistory.push_back(message);
}

const std::vector<Message>& ClientManager::getChatHistory() const {
    return m_chatHistory;
}

void ClientManager::clear() {
    m_clients.clear();
    m_chatHistory.clear();
}

const std::unordered_map<QUuid, std::unique_ptr<IClientSession>, QUuidHash>& ClientManager::getClients() const {
    return m_clients;
}