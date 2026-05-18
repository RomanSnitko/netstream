#pragma once
#include <string>
#include <memory>
#include "../common/Event.hpp"

namespace pqxx {
    class connection;
}

class DatabaseWriter {
public:
    DatabaseWriter(std::string connection_string);
    ~DatabaseWriter();

    void write_event(const Event& event);
    void write_alert(const Event& alert);

private:
    std::string connection_string;
    std::unique_ptr<pqxx::connection> conn;
};
