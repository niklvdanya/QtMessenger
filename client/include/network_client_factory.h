#pragma once
#include <memory>

#include "inetwork_client.h"

class QObject;

class NetworkClientFactory
{
public:
    static std::unique_ptr<INetworkClient> createTcpClient(QObject* parent);
};