#include "DatabaseWriter.hpp"
#include <pqxx/pqxx>

DatabaseWriter::DatabaseWriter(std::string connection_string)
    : connection_string(std::move(connection_string))
    , conn(nullptr) {

    conn = std::make_unique<pqxx::connection>(this->connection_string);

    if (!conn->is_open()) {
        throw std::runtime_error("Failed to open database connection");
    }
}

DatabaseWriter::~DatabaseWriter() {
    if (conn && conn->is_open()) {
        conn->close();
    }
}

void DatabaseWriter::write_event(const Event& event) {
    pqxx::work txn(*conn);

    std::string metadata_json = "{";
    bool first = true;
    for (const auto& [key, value] : event.metadata) {
        if (!first) metadata_json += ",";
        metadata_json += "\"" + key + "\":\"" + value + "\"";
        first = false;
    }
    metadata_json += "}";

    txn.exec_params(
        "INSERT INTO events (agent_id, timestamp, source, source_name, raw_message, severity, category, metadata) "
        "VALUES ($1, to_timestamp($2), $3, $4, $5, $6, $7, $8::jsonb)",
        event.agent_id,
        event.timestamp,
        event.source,
        event.source_name,
        event.raw_message,
        event.severity,
        event.category,
        metadata_json
    );

    txn.commit();
}

void DatabaseWriter::write_alert(const Event& alert) {
    pqxx::work txn(*conn);

    std::string alert_type = alert.metadata.count("alert_type") ? alert.metadata.at("alert_type") : "";
    std::string description = alert.metadata.count("description") ? alert.metadata.at("description") : "";

    std::string metadata_json = "{";
    bool first = true;
    for (const auto& [key, value] : alert.metadata) {
        if (!first) metadata_json += ",";
        metadata_json += "\"" + key + "\":\"" + value + "\"";
        first = false;
    }
    metadata_json += "}";

    txn.exec_params(
        "INSERT INTO alerts (agent_id, timestamp, alert_type, description, source, raw_message, severity, category, metadata) "
        "VALUES ($1, to_timestamp($2), $3, $4, $5, $6, $7, $8, $9::jsonb)",
        alert.agent_id,
        alert.timestamp,
        alert_type,
        description,
        alert.source,
        alert.raw_message,
        alert.severity,
        alert.category,
        metadata_json
    );

    txn.commit();
}
