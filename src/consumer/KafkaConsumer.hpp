#pragma once
#include <string>
#include <vector>
#include <functional>

namespace RdKafka {
    class KafkaConsumer;
}

class KafkaConsumer {
public:
    KafkaConsumer(std::string brokers, std::string group_id, std::vector<std::string> topics);
    ~KafkaConsumer();

    int poll(std::function<void(const std::string&)> callback, int timeout_ms);
    void commit();

private:
    std::string brokers;
    std::string group_id;
    std::vector<std::string> topics;
    RdKafka::KafkaConsumer* consumer;
};
