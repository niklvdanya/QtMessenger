#include "network_client_factory.h"

#include "network_client.h"

std::unique_ptr<INetworkClient> NetworkClientFactory::createTcpClient(QObject* parent)
{
    return std::make_unique<NetworkClient>(parent);
}