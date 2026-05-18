#include "TcpServer.hpp"
#include "Gateway.hpp"
#include <csignal>
#include <atomic>
#include <thread>

std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int port = 8080;
    std::string kafka_brokers = "localhost:9092";

    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    if (argc > 2) {
        kafka_brokers = argv[2];
    }

    Gateway gateway(kafka_brokers);

    TcpServer server(port, [&gateway](std::string json) {
        gateway.process_message(json);
    });

    std::thread server_thread([&server]() {
        server.run();
    });

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.shutdown();
    server_thread.join();

    return 0;
}
