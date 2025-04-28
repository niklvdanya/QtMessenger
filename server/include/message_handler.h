#pragma once
#include <memory>

#include "iserver.h"

class MessageHandler : public IMessageHandler
{
public:
    explicit MessageHandler(std::shared_ptr<IClientManager> clientManager);
    ~MessageHandler() override = default;

    void handleMessage(const Message& message, QUuid senderId) override;

private:
    void handleUserListRequest(QUuid senderId);
    void handleChatMessage(const Message& message);

    std::shared_ptr<IClientManager> m_clientManager;
};