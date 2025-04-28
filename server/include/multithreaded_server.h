#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <unordered_map>
#include <QUuid>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <QObject>
#include <string>
#include "message.h"
#include "quuid_hash.h"
#include "iserver.h"
#include "client_manager.h"
#include "message_handler.h"
#include "database_manager.h"

class MultithreadedServer : public QObject, public IServer {
    Q_OBJECT
public:
    explicit MultithreadedServer(unsigned short port, int thread_count = 4, 
                                IDatabase* dbManager = nullptr, QObject* parent = nullptr);
    ~MultithreadedServer() override;

    void start(uint16_t port) override;
    void stop() override;
    bool isRunning() const override;

signals:
    void newConnection(QUuid clientId);
    void clientDisconnected(QUuid clientId);
    void messageReceived(const Message& message);

private slots:
    bool authenticate_client_impl(const std::string& username, const std::string& password);

private:
    void accept_connections();
    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    bool authenticate_client(const std::string& username, const std::string& password);

    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    IDatabase* m_dbManager;
    std::vector<std::thread> m_threads;
    std::shared_ptr<ClientManager> m_clientManager;
    std::shared_ptr<MessageHandler> m_messageHandler;
    std::mutex m_mutex;
    std::atomic<bool> m_running;
};