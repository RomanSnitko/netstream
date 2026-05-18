#include "Agent.hpp"
#include "TcpSender.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

Agent::Agent(std::string agent_id, std::string gateway_host, int gateway_port)
    : agent_id(std::move(agent_id))
    , tcp_sender(std::make_unique<TCPSender>(std::move(gateway_host), gateway_port)) {
}

Agent::~Agent() {
    shutdown();
}

void Agent::run() {
    running = true;

    source_threads.emplace_back([this]() {
        read_syslog();
    });

    batch_thread = std::thread([this]() {
        batch_timer_loop();
    });
}

void Agent::shutdown() {
    running = false;

    for (auto& t : source_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    if (batch_thread.joinable()) {
        batch_thread.join();
    }

    reset_batch();
    tcp_sender->close();
}

void Agent::read_syslog() {
    std::ifstream logfile("/var/log/syslog");
    if (!logfile.is_open()) {
        return;
    }

    logfile.seekg(0, std::ios::end);

    std::string line;
    while (running) {
        if (std::getline(logfile, line)) {
            Event event;
            event.agent_id = agent_id;
            event.timestamp = std::time(nullptr);
            event.source = "syslog";
            event.raw_message = line;
            event.severity = "info";
            event.source_name = "system";

            add_event(std::move(event));
        } else {
            logfile.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Agent::add_event(Event event) {
    std::lock_guard<std::mutex> lock(batch_mutex);
    batch_buffer.push_back(std::move(event));

    if (batch_buffer.size() >= max_batch_size) {
        reset_batch_locked();
    }
}

void Agent::reset_batch() {
    std::lock_guard<std::mutex> lock(batch_mutex);
    reset_batch_locked();
}

void Agent::reset_batch_locked() {
    if (batch_buffer.empty()) {
        return;
    }

    std::string json = serialize_to_json(batch_buffer);
    tcp_sender->send(json);
    batch_buffer.clear();
}

std::string Agent::serialize_to_json(const std::vector<Event>& batch) {
    auto escape_json = [](const std::string& str) {
        std::string result;
        result.reserve(str.size());
        for (char c : str) {
            switch (c) {
                case '"':  result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:   result += c;
            }
        }
        return result;
    };

    std::ostringstream oss;
    oss << "{\"events\":[";

    for (size_t i = 0; i < batch.size(); ++i) {
        const auto& e = batch[i];

        oss << "{";
        oss << "\"agent_id\":\"" << escape_json(e.agent_id) << "\",";
        oss << "\"timestamp\":" << e.timestamp << ",";
        oss << "\"source\":\"" << escape_json(e.source) << "\",";
        oss << "\"source_name\":\"" << escape_json(e.source_name) << "\",";
        oss << "\"raw_message\":\"" << escape_json(e.raw_message) << "\",";
        oss << "\"severity\":\"" << escape_json(e.severity) << "\",";
        oss << "\"category\":\"" << escape_json(e.category) << "\"";
        oss << "}";

        if (i < batch.size() - 1) {
            oss << ",";
        }
    }

    oss << "]}";
    return oss.str();
}

void Agent::batch_timer_loop() {
    while (running) {
        std::this_thread::sleep_for(max_wait_time);
        reset_batch();
    }
}
