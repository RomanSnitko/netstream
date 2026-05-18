#pragma once
#include <string>
#include <ctime>
#include <map>

struct Event {
    std::string agent_id;
    std::time_t timestamp;
    std::string source;
    std::string source_name;
    std::string raw_message;
    std::string severity;
    std::string category;
    std::map<std::string, std::string> metadata;
};
