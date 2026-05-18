#pragma once
#include <string>
#include <vector>
#include "../common/Event.hpp"

class RuleEngine {
public:
    RuleEngine();
    std::vector<Event> check_rules(const Event& event);

private:
    bool is_auth_failure(const std::string& message);
    bool is_kernel_error(const std::string& message);
    bool is_oom(const std::string& message);
    Event create_alert(const Event& original, const std::string& alert_type, const std::string& description);
};
