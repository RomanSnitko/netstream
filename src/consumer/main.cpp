#include "Consumer.hpp"
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::string kafka_brokers = "localhost:9092";
    std::string group_id = "netstream-consumer";
    std::string db_connection = "host=localhost dbname=netstream user=postgres password=postgres";

    if (argc > 1) {
        kafka_brokers = argv[1];
    }
    if (argc > 2) {
        group_id = argv[2];
    }
    if (argc > 3) {
        db_connection = argv[3];
    }

    Consumer consumer(kafka_brokers, group_id, db_connection);

    std::thread consumer_thread([&consumer]() {
        consumer.run();
    });

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    consumer.shutdown();
    consumer_thread.join();

    return 0;
}
