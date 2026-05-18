#include "TcpServer.hpp"
#include <memory>

using boost::asio::ip::tcp;

TcpServer::TcpServer(int port, std::function<void(std::string)> on_message)
    : port(port)
    , on_message(std::move(on_message))
    , acceptor(io_context, tcp::endpoint(tcp::v4(), port))
    , running(false) {
}

TcpServer::~TcpServer() {
    shutdown();
}

void TcpServer::run() {
    running = true;
    start_accept();
    io_context.run();
}

void TcpServer::shutdown() {
    running = false;
    boost::system::error_code ec;
    acceptor.close(ec);
    io_context.stop();
}

void TcpServer::start_accept() {
    acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec && running) {
            handle_client(std::move(socket));
        }

        if (running) {
            start_accept();
        }
    });
}

void TcpServer::handle_client(tcp::socket socket) {
    auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
    async_read_length(socket_ptr);
}

void TcpServer::async_read_length(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<std::array<char, 4>>();

    boost::asio::async_read(*socket, boost::asio::buffer(*buffer),
        [this, socket, buffer](boost::system::error_code ec, size_t) {
            if (ec) {
                return;
            }

            uint32_t length = ntohl(*reinterpret_cast<uint32_t*>(buffer->data()));
            async_read_data(socket, length);
        });
}

void TcpServer::async_read_data(std::shared_ptr<tcp::socket> socket, uint32_t length) {
    auto buffer = std::make_shared<std::vector<char>>(length);

    boost::asio::async_read(*socket, boost::asio::buffer(*buffer),
        [this, socket, buffer](boost::system::error_code ec, size_t) {
            if (ec) {
                return;
            }

            std::string json(buffer->begin(), buffer->end());
            on_message(json);
            async_read_length(socket);
        });
}
