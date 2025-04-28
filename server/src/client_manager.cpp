#include "client_manager.h"
#include <QDebug>
#include <thread>
#include <chrono>

ClientManager::ClientManager() {
    m_chatHistory.reserve(MAX_HISTORY_SIZE);
}

void ClientManager::addClient(QUuid clientId, std::unique_ptr<IClientSession> client) {
    qDebug() << "Client added, UUID:" << clientId;
    m_clients.emplace(clientId, std::move(client));
    auto* sessionPtr = m_clients.at(clientId).get();
    
    if (m_chatHistory.empty()) {
        qDebug() << "No chat history to send";
        return;
    }

    for (const auto& msg : m_chatHistory) {
        sessionPtr->sendMessage(msg);
        qDebug() << "Sent history message from" << QString::fromStdString(msg.username) 
                 << " at " << msg.timestamp.toString("hh:mm:ss")
                 << " - Text: " << QString::fromStdString(msg.text);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    qDebug() << "Finished sending chat history to client" << clientId;
}

void ClientManager::removeClient(QUuid clientId) {
    qDebug() << "Removing client, UUID:" << clientId;
    m_clients.erase(clientId);
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

std::vector<std::string> ClientManager::getUsernames() const {
    std::vector<std::string> usernames;
    for (const auto& [id, client] : m_clients) {
        usernames.push_back(client->username());
    }
    return usernames;
}

void ClientManager::sendMessageToClient(QUuid clientId, const Message& message) {
    auto it = m_clients.find(clientId);
    if (it != m_clients.end()) {
        it->second->sendMessage(message);
    }
}