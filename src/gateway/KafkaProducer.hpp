#pragma once
#include <string>

namespace RdKafka {
    class Producer;
}

class KafkaProducer {
public:
    KafkaProducer(std::string brokers);
    ~KafkaProducer();

    void send(const std::string& topic, const std::string& message);
    void reset();

private:
    std::string brokers;
    RdKafka::Producer* producer;
};
