#pragma once
#include <string>
#include <boost/asio.hpp>
#include <atomic>

class TCPSender {
public:
    TCPSender(std::string host, int port);
    ~TCPSender();

    void send(const std::string& json_data);
    void close();

private:
    void connect();

    std::string gateway_host;

    int gateway_port;

    boost::asio::io_context io_context;

    boost::asio::ip::tcp::socket socket;

    std::atomic<bool> connected{false};
};
