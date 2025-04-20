#pragma once
#include "inetwork_client.h"
#include <memory>

class QObject;

class NetworkClientFactory {
public:
    static std::unique_ptr<INetworkClient> createTcpClient(QObject* parent);
};
