#include "Gateway.hpp"
#include "RuleEngine.hpp"
#include "KafkaProducer.hpp"
#include "../../external/json.hpp"
#include <algorithm>

using json = nlohmann::json;

Gateway::Gateway(std::string kafka_brokers)
    : rule_engine(std::make_unique<RuleEngine>())
    , kafka_producer(std::make_unique<KafkaProducer>(std::move(kafka_brokers))) {
}

Gateway::~Gateway() {
}

void Gateway::set_event_callback(std::function<void(const Event&)> callback) {
    on_event = std::move(callback);
}

void Gateway::set_alert_callback(std::function<void(const Event&)> callback) {
    on_alert = std::move(callback);
}

void Gateway::process_message(const std::string& json_message) {
    auto events = parse_json(json_message);

    for (auto& event : events) {
        if (!validate_event(event)) {
            continue;
        }

        normalize_event(event);
        classify_event(event);

        std::string event_json = serialize_event(event);
        kafka_producer->send("netstream.events", event_json);

        if (on_event) {
            on_event(event);
        }

        auto alerts = rule_engine->check_rules(event);
        for (const auto& alert : alerts) {
            std::string alert_json = serialize_event(alert);
            kafka_producer->send("netstream.alerts", alert_json);

            if (on_alert) {
                on_alert(alert);
            }
        }
    }
}

std::vector<Event> Gateway::parse_json(const std::string& json_str) {
    std::vector<Event> events;

    auto j = json::parse(json_str);

    if (!j.contains("events") || !j["events"].is_array()) {
        throw std::runtime_error("Invalid JSON: missing 'events' array");
    }

    for (const auto& item : j["events"]) {
        Event event;
        event.agent_id = item.value("agent_id", "");
        event.timestamp = item.value("timestamp", 0);
        event.source = item.value("source", "");
        event.source_name = item.value("source_name", "");
        event.raw_message = item.value("raw_message", "");
        event.severity = item.value("severity", "");
        event.category = item.value("category", "");

        if (item.contains("metadata") && item["metadata"].is_object()) {
            for (auto& [key, value] : item["metadata"].items()) {
                if (value.is_string()) {
                    event.metadata[key] = value.get<std::string>();
                }
            }
        }

        events.push_back(event);
    }

    return events;
}

bool Gateway::validate_event(const Event& event) {
    return !event.agent_id.empty() &&
           event.timestamp != 0 &&
           !event.source.empty() &&
           !event.raw_message.empty();
}

void Gateway::normalize_event(Event& event) {
    auto trim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    };

    trim(event.agent_id);
    trim(event.source);
    trim(event.source_name);
    trim(event.raw_message);

    std::transform(event.source.begin(), event.source.end(), event.source.begin(), ::tolower);

    if (event.severity.empty()) {
        event.severity = "info";
    }
}

void Gateway::classify_event(Event& event) {
    if (!event.category.empty()) {
        return;
    }

    std::string lower_msg = event.raw_message;
    std::transform(lower_msg.begin(), lower_msg.end(), lower_msg.begin(), ::tolower);

    if (lower_msg.find("sudo") != std::string::npos ||
        lower_msg.find("authentication") != std::string::npos ||
        lower_msg.find("login") != std::string::npos ||
        lower_msg.find("password") != std::string::npos) {
        event.category = "auth";
    } else if (lower_msg.find("kernel") != std::string::npos) {
        event.category = "kernel";
    } else if (lower_msg.find("network") != std::string::npos ||
               lower_msg.find("connection") != std::string::npos) {
        event.category = "network";
    } else if (lower_msg.find("disk") != std::string::npos ||
               lower_msg.find("filesystem") != std::string::npos) {
        event.category = "storage";
    } else {
        event.category = "system";
    }
}

std::string Gateway::serialize_event(const Event& event) {
    json j;
    j["agent_id"] = event.agent_id;
    j["timestamp"] = event.timestamp;
    j["source"] = event.source;
    j["source_name"] = event.source_name;
    j["raw_message"] = event.raw_message;
    j["severity"] = event.severity;
    j["category"] = event.category;
    j["metadata"] = event.metadata;
    return j.dump();
}
