#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../common/Event.hpp"

class RuleEngine;
class KafkaProducer;

class Gateway {
public:
    Gateway(std::string kafka_brokers);
    ~Gateway();

    void process_message(const std::string& json_message);
    void set_event_callback(std::function<void(const Event&)> callback);
    void set_alert_callback(std::function<void(const Event&)> callback);

private:
    std::vector<Event> parse_json(const std::string& json);
    bool validate_event(const Event& event);
    void normalize_event(Event& event);
    void classify_event(Event& event);
    std::string serialize_event(const Event& event);

    std::function<void(const Event&)> on_event;
    std::function<void(const Event&)> on_alert;
    std::unique_ptr<RuleEngine> rule_engine;
    std::unique_ptr<KafkaProducer> kafka_producer;
};
