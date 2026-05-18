#pragma once
#include <string>
#include <memory>
#include "../common/Event.hpp"

class KafkaConsumer;
class DatabaseWriter;

class Consumer {
public:
    Consumer(std::string kafka_brokers, std::string group_id, std::string db_connection);
    ~Consumer();

    void run();
    void shutdown();

private:
    void process_event(const std::string& json_message);
    void process_alert(const std::string& json_message);
    Event deserialize_event(const std::string& json_str);

    std::unique_ptr<KafkaConsumer> events_consumer;
    std::unique_ptr<KafkaConsumer> alerts_consumer;
    std::unique_ptr<DatabaseWriter> db_writer;
    bool running;
};
