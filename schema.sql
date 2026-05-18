CREATE DATABASE netstream;

\c netstream

CREATE TABLE IF NOT EXISTS events (
    id SERIAL PRIMARY KEY,
    agent_id VARCHAR(255) NOT NULL,
    timestamp TIMESTAMP NOT NULL,
    source VARCHAR(255) NOT NULL,
    source_name VARCHAR(255),
    raw_message TEXT NOT NULL,
    severity VARCHAR(50),
    category VARCHAR(100),
    metadata JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS alerts (
    id SERIAL PRIMARY KEY,
    agent_id VARCHAR(255) NOT NULL,
    timestamp TIMESTAMP NOT NULL,
    alert_type VARCHAR(255) NOT NULL,
    description TEXT,
    source VARCHAR(255) NOT NULL,
    raw_message TEXT NOT NULL,
    severity VARCHAR(50),
    category VARCHAR(100),
    metadata JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_events_agent_id ON events(agent_id);
CREATE INDEX idx_events_timestamp ON events(timestamp);
CREATE INDEX idx_events_category ON events(category);
CREATE INDEX idx_events_severity ON events(severity);

CREATE INDEX idx_alerts_agent_id ON alerts(agent_id);
CREATE INDEX idx_alerts_timestamp ON alerts(timestamp);
CREATE INDEX idx_alerts_type ON alerts(alert_type);
CREATE INDEX idx_alerts_severity ON alerts(severity);
