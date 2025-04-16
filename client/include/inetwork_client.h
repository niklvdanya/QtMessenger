#pragma once
#include <cstdint>      // Добавлено для uint16_t
#include <functional>
#include <string_view>

class INetworkClient {
public:
    virtual ~INetworkClient() = default;
    virtual void connectToServer(std::string_view host, std::uint16_t port, std::string_view username) = 0;
    virtual void sendMessage(std::string_view message) = 0;

    // Callback для получения сообщений
    using MessageCallback = std::function<void(const std::string&, const std::string&)>;
    virtual void setMessageCallback(const MessageCallback& callback) = 0;

    // Callback для уведомления об отключении
    using DisconnectedCallback = std::function<void()>;
    virtual void setDisconnectedCallback(const DisconnectedCallback& callback) = 0;
};