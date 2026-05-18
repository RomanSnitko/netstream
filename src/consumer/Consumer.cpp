#include "Consumer.hpp"
#include "KafkaConsumer.hpp"
#include "DatabaseWriter.hpp"
#include "../../external/json.hpp"
#include <thread>
#include <chrono>

using json = nlohmann::json;

Consumer::Consumer(std::string kafka_brokers, std::string group_id, std::string db_connection)
    : running(false) {

    db_writer = std::make_unique<DatabaseWriter>(std::move(db_connection));

    events_consumer = std::make_unique<KafkaConsumer>(
        kafka_brokers,
        group_id,
        std::vector<std::string>{"netstream.events"}
    );

    alerts_consumer = std::make_unique<KafkaConsumer>(
        kafka_brokers,
        group_id + "_alerts",
        std::vector<std::string>{"netstream.alerts"}
    );
}

Consumer::~Consumer() {
}

void Consumer::run() {
    running = true;

    while (running) {
        int events_count = events_consumer->poll([this](const std::string& msg) {
            process_event(msg);
        }, 100);

        int alerts_count = alerts_consumer->poll([this](const std::string& msg) {
            process_alert(msg);
        }, 100);

        if (events_count > 0) {
            events_consumer->commit();
        }

        if (alerts_count > 0) {
            alerts_consumer->commit();
        }

        if (events_count == 0 && alerts_count == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Consumer::shutdown() {
    running = false;
}

void Consumer::process_event(const std::string& json_message) {
    Event event = deserialize_event(json_message);
    db_writer->write_event(event);
}

void Consumer::process_alert(const std::string& json_message) {
    Event alert = deserialize_event(json_message);
    db_writer->write_alert(alert);
}

Event Consumer::deserialize_event(const std::string& json_str) {
    auto j = json::parse(json_str);

    Event event;
    event.agent_id = j.value("agent_id", "");
    event.timestamp = j.value("timestamp", 0);
    event.source = j.value("source", "");
    event.source_name = j.value("source_name", "");
    event.raw_message = j.value("raw_message", "");
    event.severity = j.value("severity", "");
    event.category = j.value("category", "");

    if (j.contains("metadata") && j["metadata"].is_object()) {
        for (auto& [key, value] : j["metadata"].items()) {
            if (value.is_string()) {
                event.metadata[key] = value.get<std::string>();
            }
        }
    }

    return event;
}
