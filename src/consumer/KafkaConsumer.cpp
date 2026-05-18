#include "KafkaConsumer.hpp"
#include <librdkafka/rdkafkacpp.h>
#include <stdexcept>

KafkaConsumer::KafkaConsumer(std::string brokers, std::string group_id, std::vector<std::string> topics)
    : brokers(std::move(brokers))
    , group_id(std::move(group_id))
    , topics(std::move(topics))
    , consumer(nullptr) {

    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    if (conf->set("bootstrap.servers", this->brokers, errstr) != RdKafka::Conf::CONF_OK) {
        delete conf;
        throw std::runtime_error("Kafka configuration failed");
    }

    if (conf->set("group.id", this->group_id, errstr) != RdKafka::Conf::CONF_OK) {
        delete conf;
        throw std::runtime_error("Kafka configuration failed");
    }

    if (conf->set("enable.auto.commit", "false", errstr) != RdKafka::Conf::CONF_OK) {
        delete conf;
        throw std::runtime_error("Kafka configuration failed");
    }

    consumer = RdKafka::KafkaConsumer::create(conf, errstr);
    delete conf;

    if (!consumer) {
        throw std::runtime_error("Kafka consumer creation failed");
    }

    RdKafka::ErrorCode err = consumer->subscribe(this->topics);
    if (err != RdKafka::ERR_NO_ERROR) {
        delete consumer;
        throw std::runtime_error("Kafka subscription failed");
    }
}

KafkaConsumer::~KafkaConsumer() {
    if (consumer) {
        consumer->close();
        delete consumer;
    }
}

int KafkaConsumer::poll(std::function<void(const std::string&)> callback, int timeout_ms) {
    RdKafka::Message* message = consumer->consume(timeout_ms);

    if (!message) {
        return 0;
    }

    if (message->err() == RdKafka::ERR_NO_ERROR) {
        std::string payload(static_cast<const char*>(message->payload()), message->len());
        callback(payload);
        delete message;
        return 1;
    }

    delete message;
    return 0;
}

void KafkaConsumer::commit() {
    consumer->commitSync();
}
