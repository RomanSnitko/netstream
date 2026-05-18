#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <boost/asio.hpp>

class TcpServer {
public:
    TcpServer(int port, std::function<void(std::string)> on_message);
    ~TcpServer();

    void run();
    void shutdown();

private:
    void start_accept();
    void handle_client(boost::asio::ip::tcp::socket socket);
    void async_read_length(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void async_read_data(std::shared_ptr<boost::asio::ip::tcp::socket> socket, uint32_t length);

    int port;
    std::function<void(std::string)> on_message;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
    std::atomic<bool> running;
};
