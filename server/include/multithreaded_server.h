#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <unordered_map>
#include <QUuid>
#include <thread>
#include <vector>
#include <mutex>
#include <QObject>
#include <string>
#include "message.h"
#include "quuid_hash.h"
#include "iserver.h"
#include "client_manager.h"
#include "message_handler.h"

class MultithreadedServer : public QObject, public IServer {
    Q_OBJECT
public:
    explicit MultithreadedServer(unsigned short port, int thread_count = 4, QObject* parent = nullptr);
    ~MultithreadedServer() override;

    void start(uint16_t port) override;
    void stop() override;

signals:
    void newConnection(QUuid clientId);
    void clientDisconnected(QUuid clientId);
    void messageReceived(const std::string& message, QUuid senderId, const std::string& username);

private:
    void accept_connections();
    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket, 
                     std::shared_ptr<std::vector<char>> buffer);

    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::vector<std::thread> m_threads;
    std::shared_ptr<ClientManager> m_clientManager;
    std::shared_ptr<MessageHandler> m_messageHandler;
    std::mutex m_clients_mutex;
    bool m_running;
};