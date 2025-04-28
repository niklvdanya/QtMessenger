#include "chat_controller.h"

#include <QObject>

#include "network_client.h"

ChatController::ChatController(std::unique_ptr<INetworkClient> networkClient, IChatView* view)
    : m_networkClient(std::move(networkClient)), m_view(view)
{
    setupNetworkCallbacks();
}

void ChatController::sendMessage(const std::string& message)
{
    if (!message.empty()) {
        m_networkClient->sendMessage(message);
    }
}

void ChatController::sendMessage()
{
    std::string message = m_view->getInputText();
    if (!message.empty()) {
        m_networkClient->sendMessage(message);
        m_view->clearInput();
    }
}

void ChatController::requestUserList()
{
    m_networkClient->requestUserList();
}

void ChatController::setUsername(const std::string& username)
{
    m_username = username;
}

std::string ChatController::username() const
{
    return m_username;
}

void ChatController::connectToServer(const std::string& host, uint16_t port,
                                     const std::string& username, const std::string& password)
{
    m_username = username;
    m_networkClient->connectToServer(host, port, username, password);
}

void ChatController::disconnect()
{
    m_networkClient->disconnect();
}

void ChatController::setupNetworkCallbacks()
{
    m_networkClient->setMessageCallback(
        [this](const Message& message) { m_view->displayChatMessage(message); });

    m_networkClient->setConnectionCallback(
        [this](bool connected) { m_view->updateConnectionStatus(connected); });

    m_networkClient->setDisconnectedCallback(
        [this]() { m_view->displaySystemMessage("Disconnected from server"); });

    m_networkClient->setUserListCallback(
        [this](const std::vector<QString>& userList) { m_view->updateUserList(userList); });
}

NetworkClient* ChatController::getNetworkClient() const
{
    return dynamic_cast<NetworkClient*>(m_networkClient.get());
}