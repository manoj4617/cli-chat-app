#include "BarrackRepo.hpp"
#include "Error.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include "SQLiteCpp/Transaction.h"
#include "sqlite3.h"
#include "types.hpp"
#include <chrono>
#include <string>

static std::string format_timestamp_iso8601(const std::chrono::system_clock::time_point& tp){
    auto in_time_t = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%S");
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return ss.str();
}

static std::chrono::system_clock::time_point parse_timestamp_iso8601(const std::string& str) {
    std::tm tm = {};
    std::stringstream ss(str);
    // Handles format like "2023-03-15T12:00:00.123Z"
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

BarrackRepository::BarrackRepository(std::shared_ptr<SQLite::Database> db){
    db_ = db;
}

Result<std::monostate>  BarrackRepository::create(const Barrack& barrack){
    const char* query =
    "INSERT INTO barracks"
    "(barrack_id, name, admin_id, is_private, hashed_password, salt, created_at)"
    "VALUES (:barrack_id, :name, :admin_id, :is_private, :hashed_password, :salt, :created_at)";
    
    try{
        SQLite::Statement statement(*db_.get(), query);
        statement.bind(":barrack_id", barrack.barrack_id.c_str());
        statement.bind(":name", barrack.barrack_name.c_str());
        statement.bind(":admin_id", barrack.admin_id.c_str());
        statement.bind(":is_private", barrack.is_private);
        statement.bind(":hashed_password", barrack.hashed_password->c_str());
        statement.bind(":salt", barrack.salt->c_str());

        statement.bind(":created_at", format_timestamp_iso8601(barrack.created_at));

        int rows_affected = statement.exec();
        if(1 != rows_affected){
            return Error{ErrorCode::DATABASE_ERROR, "Barrack creation: insert affected " + std::to_string(rows_affected) + " rows."};
        }
        return Success{};
    } catch(const SQLite::Exception &ex){
        if(ex.getErrorCode() == SQLITE_CONSTRAINT_UNIQUE){
            return Error{ErrorCode::DUPLICATE_ENTRY, "Barrack with name: " + barrack.barrack_name + " alreary exists."};
        }
        return Error{ErrorCode::DATABASE_ERROR, "Database error: " + std::string(ex.what())};
    }
}

Result<std::monostate> BarrackRepository::destroy(const std::string& barrack_id){
    const char* query =
    "DELETE FROM barracks "
    "WHERE barrack_id = :barrack_id";

    const char* insert_event_query = 
    "INSERT INTO event_outbox (event_type, payload, created_at) VALUES (:type, :payload, :created_at)";

    try{
        SQLite::Transaction transaction(*db_.get(), SQLite::TransactionBehavior::IMMEDIATE);
        
        SQLite::Statement statement(*db_.get(), query);
        statement.bind(":barrack_id", barrack_id.c_str());
        int rows_affected = statement.exec();
        if(rows_affected > 1){
            return Error{ErrorCode::DATABASE_ERROR, "CRITICAL: Deleted " + std::to_string(rows_affected) + " rows for a single barrack_id."};
        }

        nlohmann::json payload;
        payload["barrack_id"] = barrack_id;

        SQLite::Statement insert_event_smt(*db_.get(), insert_event_query);
        insert_event_smt.bind(":type" , "BarrackDestroy");
        insert_event_smt.bind(":payload", payload.dump());
        insert_event_smt.bind(":created_at", format_timestamp_iso8601(std::chrono::system_clock::now()));

        insert_event_smt.exec();

        transaction.commit();
        return Success{};
    } catch(const SQLite::Exception &ex){
        return Error{ErrorCode::DATABASE_ERROR, "Database error: " + std::string(ex.what())};
    }
}

Result<Barrack> BarrackRepository::find_by_id(const std::string &barrack_id){
    const char* query =
        "SELECT barrack_id, barrack_name, owner_uid, is_private, hashed_password, salt, created_at "
        "FROM barracks "
        "WHERE barrack_id = :barrack_id";

    try {
        SQLite::Statement statement(*db_, query);
        statement.bind(":barrack_id", barrack_id);

        if (statement.executeStep()) {
            Barrack barrack;
            barrack.barrack_id   = statement.getColumn("barrack_id").getString();
            barrack.barrack_name = statement.getColumn("barrack_name").getString();
            barrack.admin_id    = statement.getColumn("admin_id").getString();
            barrack.is_private   = statement.getColumn("is_private").getInt();
            
            if (!statement.getColumn("hashed_password").isNull()) {
                barrack.hashed_password = statement.getColumn("hashed_password").getString();
            }
            if (!statement.getColumn("salt").isNull()) {
                barrack.salt = statement.getColumn("salt").getString();
            }

            barrack.created_at = parse_timestamp_iso8601(statement.getColumn("created_at").getString());

            return barrack;
        } else {
            return Error{ErrorCode::BARRACK_NOT_FOUND, "Barrack with ID '" + barrack_id + "' not found."};
        }
    } catch (const SQLite::Exception& ex) {
        return Error{ErrorCode::DATABASE_ERROR, "Database query failed: " + std::string(ex.what())};
    }
}

Result<std::monostate> BarrackRepository::add_member(const std::string &barrack_id, const std::string &user_id){
    const char* query =
        "INSERT INTO barrack_members (barrack_id, user_id, joined_at) "
        "VALUES (:barrack_id, :user_id, :joined_at)";

    try {
        SQLite::Statement statement(*db_.get(), query);
        statement.bind(":barrack_id", barrack_id);
        statement.bind(":user_id", user_id);
        statement.bind(":joined_at", format_timestamp_iso8601(std::chrono::system_clock::now()));

        if (statement.exec() != 1) {
             return Error{ErrorCode::DATABASE_ERROR, "Failed to add member, insert did not affect 1 row."};
        }

        return Success{};
    } catch (const SQLite::Exception& ex) {
        if (ex.getErrorCode() == SQLITE_CONSTRAINT_PRIMARYKEY || ex.getErrorCode() == SQLITE_CONSTRAINT_UNIQUE) {
            return Error{ErrorCode::DUPLICATE_ENTRY, "User '" + user_id + "' is already a member of barrack '" + barrack_id + "'."};
        }
        return Error{ErrorCode::DATABASE_ERROR, "Database error adding member: " + std::string(ex.what())};
    }
}

Result<std::monostate> BarrackRepository::remove_member(const std::string &barrack_id, const std::string &user_id){
    const char* query =
        "DELETE FROM barrack_members "
        "WHERE barrack_id = :barrack_id AND user_id = :user_id";

    try {
        SQLite::Statement statement(*db_, query);
        statement.bind(":barrack_id", barrack_id);
        statement.bind(":user_id", user_id);
        
        statement.exec();

        return Success{};
    } catch (const SQLite::Exception& ex) {
        return Error{ErrorCode::DATABASE_ERROR, "Database error removing member: " + std::string(ex.what())};
    }
}

Result<std::vector<BarrackMember>> BarrackRepository::get_members(const std::string &barrack_id){
    const char* query =
        "SELECT barrack_id, user_id, joined_at "
        "FROM barrack_members "
        "WHERE barrack_id = :barrack_id";

    try {
        std::vector<BarrackMember> members;
        SQLite::Statement statement(*db_, query);
        statement.bind(":barrack_id", barrack_id);

        while (statement.executeStep()) {
            BarrackMember member;
            member.barrack_id = statement.getColumn("barrack_id").getString();
            member.user_id    = statement.getColumn("user_id").getString();
            member.joined_at  = parse_timestamp_iso8601(statement.getColumn("joined_at").getString());
            members.push_back(std::move(member));
        }

        return members;
    } catch (const SQLite::Exception& ex) {
        return Error{ErrorCode::DATABASE_ERROR, "Database query failed for get_members: " + std::string(ex.what())};
    }
}