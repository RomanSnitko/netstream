#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "../common/Event.hpp"

class TCPSender;

class Agent {
public:
    Agent(std::string agent_id, std::string gateway_host, int gateway_port);
    ~Agent();

    void run();
    void shutdown();

private:
    void read_syslog();
    void add_event(Event event);
    void reset_batch();
    void reset_batch_locked();
    void batch_timer_loop();
    std::string serialize_to_json(const std::vector<Event>& batch);

    std::string agent_id;
    std::atomic<bool> running{false};
    std::vector<Event> batch_buffer;
    std::mutex batch_mutex;
    size_t max_batch_size = 100;
    std::chrono::milliseconds max_wait_time{1000};
    std::vector<std::thread> source_threads;
    std::thread batch_thread;
    std::unique_ptr<TCPSender> tcp_sender;
};
