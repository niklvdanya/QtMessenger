#include "chat_controller.h"
#include "chat_window.h"
#include "network_client.h"

ChatController::ChatController(std::unique_ptr<INetworkClient> networkClient, IChatView* view)
    : m_networkClient(std::move(networkClient)), m_view(view) {
    setupNetworkCallbacks();
}

void ChatController::sendMessage(const std::string& message) {
    if (!message.empty()) {
        m_networkClient->sendMessage(message);
    }
}

void ChatController::setUsername(const std::string& username) {
    m_username = username;
    m_networkClient->connectToServer("127.0.0.1", 12345, m_username);
}

std::string ChatController::username() const {
    return m_username;
}

void ChatController::setupNetworkCallbacks() {
    m_networkClient->setMessageCallback([this](const std::string& sender, const std::string& message) {
    });
    m_networkClient->setDisconnectedCallback([this]() {
        m_view->displaySystemMessage("Disconnected from server");
    });

    if (auto* networkClient = dynamic_cast<NetworkClient*>(m_networkClient.get())) {
        QObject::connect(networkClient, &NetworkClient::connectionStatusChanged,
                        dynamic_cast<ChatWindow*>(m_view), &ChatWindow::updateConnectionStatus);
    }
}

NetworkClient* ChatController::getNetworkClient() const {
    return dynamic_cast<NetworkClient*>(m_networkClient.get());
}