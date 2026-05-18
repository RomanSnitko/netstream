#include "TcpSender.hpp"
#include <boost/system/error_code.hpp>

TCPSender::TCPSender(std::string host, int port)
    : gateway_host(std::move(host))
    , gateway_port(port)
    , socket(io_context) {}

TCPSender::~TCPSender() {
    close();
}

void TCPSender::connect() {
    boost::asio::ip::tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(gateway_host, std::to_string(gateway_port));
    boost::asio::connect(socket, endpoints);
    connected = true;
}

void TCPSender::send(const std::string& json_data) {
    if (!connected) {
        connect();
    }

    uint32_t length = htonl(static_cast<uint32_t>(json_data.size()));
    boost::asio::write(socket, boost::asio::buffer(&length, 4));
    boost::asio::write(socket, boost::asio::buffer(json_data));
}

void TCPSender::close() {
    if (connected) {
        boost::system::error_code ec;
        socket.close(ec);
        connected = false;
    }
}
