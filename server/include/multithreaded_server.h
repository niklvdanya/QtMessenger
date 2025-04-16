#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <QUuid>
#include <thread>
#include <vector>
#include <mutex>
#include <QObject>
#include <string> // Добавлено для std::string
#include "message.h"
#include "quuid_hash.h"

class MultithreadedServer : public QObject {
    Q_OBJECT
public:
    explicit MultithreadedServer(unsigned short port, int thread_count = 4, QObject* parent = nullptr);
    ~MultithreadedServer() override;

    void start();
    void stop();

signals:
    void newConnection(QUuid clientId);
    void clientDisconnected(QUuid clientId);
    void messageReceived(const std::string& message, QUuid senderId, const std::string& username);

private:
    void accept_connections();
    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handle_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket, 
                     std::shared_ptr<std::vector<char>> buffer);
    void handle_write(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                      const std::string& message);
    void broadcast_message(std::string_view message, QUuid senderId, std::string_view username);

    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::vector<std::thread> m_threads;
    std::unordered_map<QUuid, std::shared_ptr<boost::asio::ip::tcp::socket>> m_clients;
    std::unordered_map<QUuid, std::string> m_usernames; // Изменено с QString на std::string
    std::mutex m_clients_mutex;
    bool m_running;
};