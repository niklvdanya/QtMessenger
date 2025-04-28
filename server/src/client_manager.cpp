#include "client_manager.h"

#include <chrono>
#include <thread>

#include <QDebug>

ClientManager::ClientManager()
{
    m_chatHistory.reserve(MAX_HISTORY_SIZE);
}

void ClientManager::addClient(QUuid clientId, std::unique_ptr<IClientSession> client)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    qDebug() << "Client added, UUID:" << clientId;
    m_clients.emplace(clientId, std::move(client));
    auto* sessionPtr = m_clients.at(clientId).get();

    if (m_chatHistory.empty()) {
        qDebug() << "No chat history to send";
        return;
    }

    for (const auto& msg : m_chatHistory) {
        sessionPtr->sendMessage(msg);
        qDebug() << "Sent history message from" << QString::fromStdString(msg.username) << " at "
                 << msg.timestamp.toString("hh:mm:ss")
                 << " - Text: " << QString::fromStdString(msg.text);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    qDebug() << "Finished sending chat history to client" << clientId;
}

void ClientManager::removeClient(QUuid clientId)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    qDebug() << "Removing client, UUID:" << clientId;
    m_clients.erase(clientId);
}

void ClientManager::broadcastMessage(const Message& message)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    addMessage(message);
    for (const auto& [id, client] : m_clients) {
        if (client->isAuthenticated()) {
            client->sendMessage(message);
        }
    }
}

std::vector<std::string> ClientManager::getUsernames() const
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    std::vector<std::string> usernames;
    usernames.reserve(m_clients.size());

    for (const auto& [id, client] : m_clients) {
        if (client->isAuthenticated()) {
            usernames.push_back(client->username());
        }
    }

    return usernames;
}

void ClientManager::sendMessageToClient(QUuid clientId, const Message& message)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    auto it = m_clients.find(clientId);
    if (it != m_clients.end() && it->second->isAuthenticated()) {
        it->second->sendMessage(message);
    }
}

size_t ClientManager::getClientCount() const
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return m_clients.size();
}

void ClientManager::addMessage(const Message& message)
{
    if (m_chatHistory.size() >= MAX_HISTORY_SIZE) {
        m_chatHistory.erase(m_chatHistory.begin());
    }
    m_chatHistory.push_back(message);
}

const std::vector<Message>& ClientManager::getChatHistory() const
{
    return m_chatHistory;
}

void ClientManager::clear()
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    m_clients.clear();
    m_chatHistory.clear();
}