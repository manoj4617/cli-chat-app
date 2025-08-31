#include "EventRepository.hpp"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include <variant>

static std::chrono::system_clock::time_point parse_timestamp_iso8601(const std::string& str) {
    std::tm tm = {};
    std::stringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    // Extract milliseconds
    char dot;
    int milliseconds = 0;
    if (ss >> dot && dot == '.') {
        ss >> milliseconds;
    }
    #if defined(_WIN32) || defined(_WIN64)
        auto time_c = _mkgmtime(&tm);
    #else
        auto time_c = timegm(&tm);
    #endif
        auto time_point = std::chrono::system_clock::from_time_t(time_c);
        time_point += std::chrono::milliseconds(milliseconds);
        
        return time_point;
}

Result<std::vector<OutboxEvent>> EventRepository::get_unprocessed_events(int limit){
    const char* query =
    "SELECT event_id, event_type, payload, created_at "
    "FROM event_outbox "
    "LIMIT :limit";

    std::vector<OutboxEvent> events;
    try{
        OutboxEvent event;
        SQLite::Statement statement(*db_, query);
        statement.bind(":limit", limit);

        while(statement.executeStep()){
            event.event_id = statement.getColumn("event_id").getInt64();
            event.event_type = statement.getColumn("event_type").getString();
            event.payload = statement.getColumn("payload").getString();
            event.created_at = parse_timestamp_iso8601(statement.getColumn("created_at").getString());
            events.push_back(std::move(event));
        }
        return events;

    }catch(const SQLite::Exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, "Database error: " + std::string(ex.what())};
    }
}

Result<std::monostate> EventRepository::delete_event(int64_t event_id){
    const char* query =
    "DELETE FROM event_outbox "
    "WHERE event_id = :event_id ";

    try{
        SQLite::Statement statement(*db_, query);
        statement.bind(":event_id", event_id);

        int rows_affected = statement.exec();
        if(rows_affected > 1){
            return Error{ErrorCode::DATABASE_ERROR, "CRITICAL: Deleted " + std::to_string(rows_affected) + " rows for a single event_id."};
        }
        return Success{};
    } catch(const SQLite::Exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, "Database error: " + std::string(ex.what())};
    }
}