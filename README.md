# Netstream is a distributed event ingestion and processing pipeline for collecting system events from Linux endpoints and delivering them to a centralized processing layer.

Each endpoint runs an agent that reads events from the system syslog, normalizes them into a common representation, batches them by size or time threshold, and sends them to the central gateway over TCP. The transport uses a custom length-prefixed framing protocol consisting of a 4-byte network-order payload length followed by a JSON message. This allows the receiver to correctly handle partial reads, multiple messages within a single read, and TCP's byte-stream semantics.

The central Gateway is implemented around asynchronous I/O with Boost.Asio. It accepts TCP connections, reconstructs application-level messages from the byte stream, validates and normalizes incoming events, classifies them, and passes them through a rule engine. Rules can transform specific system events, such as failed authentication attempts, kernel errors, or out-of-memory conditions, into structured alerts.

Normalized events and generated alerts are published to separate Kafka topics, netstream.events and netstream.alerts. A dedicated Consumer service reads these topics using manual offset commits and persists the resulting records to PostgreSQL. The database stores both normalized events and generated alerts and maintains indexes for common access patterns such as agent, timestamp, category, and severity.

The system is split into independent ingestion, processing, messaging, and persistence stages, allowing event collection and downstream storage to operate independently. The implementation combines asynchronous TCP networking, explicit application-level framing, message-broker-based decoupling, rule-based processing, and transactional persistence.

Технологии: C++17, Boost.Asio, nlohmann/json, Kafka(librdkafka), PostgreSQL(libpqxx).
