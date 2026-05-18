#include "Agent.hpp"
#include <csignal>
#include <thread>

std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::string agent_id = "agent1";
    std::string gateway_host = "localhost";
    int gateway_port = 8080;

    if (argc > 1) {
        agent_id = argv[1];
    }
    if (argc > 2) {
        gateway_host = argv[2];
    }
    if (argc > 3) {
        gateway_port = std::stoi(argv[3]);
    }

    Agent agent(agent_id, gateway_host, gateway_port);
    agent.run();

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    agent.shutdown();
    return 0;
}
