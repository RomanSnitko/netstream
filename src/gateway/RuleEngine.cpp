#include "RuleEngine.hpp"
#include <algorithm>

RuleEngine::RuleEngine() {
}

std::vector<Event> RuleEngine::check_rules(const Event& event) {
    std::vector<Event> alerts;

    if (is_auth_failure(event.raw_message)) {
        alerts.push_back(create_alert(event, "auth_failure", "Failed authentication attempt detected"));
    }

    if (is_kernel_error(event.raw_message)) {
        alerts.push_back(create_alert(event, "kernel_error", "Kernel error detected"));
    }

    if (is_oom(event.raw_message)) {
        alerts.push_back(create_alert(event, "oom", "Out of memory condition detected"));
    }

    return alerts;
}

bool RuleEngine::is_auth_failure(const std::string& message) {
    std::string lower = message;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower.find("failed password") != std::string::npos ||
           lower.find("authentication failure") != std::string::npos ||
           lower.find("invalid user") != std::string::npos ||
           lower.find("failed login") != std::string::npos;
}

bool RuleEngine::is_kernel_error(const std::string& message) {
    std::string lower = message;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower.find("kernel:") != std::string::npos &&
           (lower.find("error") != std::string::npos ||
            lower.find("panic") != std::string::npos ||
            lower.find("bug") != std::string::npos);
}

bool RuleEngine::is_oom(const std::string& message) {
    std::string lower = message;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower.find("out of memory") != std::string::npos ||
           lower.find("oom") != std::string::npos ||
           lower.find("killed process") != std::string::npos;
}

Event RuleEngine::create_alert(const Event& original, const std::string& alert_type, const std::string& description) {
    Event alert = original;
    alert.category = "alert";
    alert.severity = "critical";
    alert.metadata["alert_type"] = alert_type;
    alert.metadata["description"] = description;
    alert.metadata["original_message"] = original.raw_message;
    return alert;
}
