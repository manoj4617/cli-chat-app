#ifndef EVENTREPOSITORY_HPP
#define EVENTREPOSITORY_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <chrono>
#include <memory>
#include <variant>
#include <vector>
#include "SQLiteCpp/Database.h"
#include "Error.hpp"

struct OutboxEvent {
    int64_t event_id;
    std::string event_type;
    std::string payload;
    std::chrono::system_clock::time_point created_at;
};


class EventRepository {
public:
    explicit EventRepository(std::shared_ptr<SQLite::Database> db) : db_(db) {}

    Result<std::vector<OutboxEvent>> get_unprocessed_events(int limit = 10);
    Result<std::monostate> delete_event(int64_t event_id);

private:
    std::shared_ptr<SQLite::Database> db_;
};

#endif
