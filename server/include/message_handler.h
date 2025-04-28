#pragma once
#include "iserver.h"
#include <memory>

class MessageHandler : public IMessageHandler {
public:
    explicit MessageHandler(std::shared_ptr<IClientManager> clientManager);
    void handleMessage(const std::string& message, QUuid senderId, const std::string& username) override;
private:
    std::shared_ptr<IClientManager> m_clientManager;
};