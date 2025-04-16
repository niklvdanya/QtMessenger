#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <QUuid>
#include <thread>
#include <vector>
#include <QObject>
#include "message.h"
#include "quuid_hash.h"

class MultithreadedServer : public QObject {
    Q_OBJECT
public:
    explicit MultithreadedServer(unsigned short port, int thread_count = 4, QObject* parent = nullptr);
    ~MultithreadedServer();
    void start();
    void stop();

signals:
    void newConnection(QUuid clientId);
    void clientDisconnected(QUuid clientId);
    void messageReceived(const QString& message, QUuid senderId, const QString& username);

private:
    void accept_connections();
    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket, 
                    std::shared_ptr<std::vector<char>> buffer);
    void handle_write(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                     const std::string& message);
    void broadcast_message(const std::string& message, QUuid senderId, const QString& username);

    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::vector<std::thread> m_threads;
    std::unordered_map<QUuid, std::shared_ptr<boost::asio::ip::tcp::socket>> m_clients;
    std::unordered_map<QUuid, QString> m_usernames;
    bool m_running;
}; 