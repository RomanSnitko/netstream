#include "KafkaProducer.hpp"
#include <librdkafka/rdkafkacpp.h>
#include <stdexcept>

KafkaProducer::KafkaProducer(std::string brokers)
    : brokers(std::move(brokers))
    , producer(nullptr) {

    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    if (conf->set("bootstrap.servers", this->brokers, errstr) != RdKafka::Conf::CONF_OK) {
        delete conf;
        throw std::runtime_error("Kafka configuration failed");
    }

    producer = RdKafka::Producer::create(conf, errstr);
    delete conf;

    if (!producer) {
        throw std::runtime_error("Kafka producer creation failed");
    }
}

KafkaProducer::~KafkaProducer() {
    if (producer) {
        producer->flush(5000);
        delete producer;
    }
}

void KafkaProducer::send(const std::string& topic, const std::string& message) {
    producer->produce(
        topic,
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(message.c_str()),
        message.size(),
        nullptr,
        0,
        0,
        nullptr
    );

    producer->poll(0);
}

void KafkaProducer::reset() {
    if (producer) {
        producer->flush(5000);
    }
}
