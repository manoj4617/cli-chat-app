#include "UserRepo.hpp"
#include "Error.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Savepoint.h"
#include "SQLiteCpp/Statement.h"
#include "sqlite3.h"
#include "types.hpp"
#include <chrono>
#include <ctime>

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

UserRepository::UserRepository(std::shared_ptr<SQLite::Database> db){
    db_ = db;
}

Result<std::monostate> UserRepository::create_user(const UserAccount &user){
    const char* query = 
    "INSERT INTO users"
    "(user_id, username, hashed_password, salt, created_at)"
    "VALUES (:user_id, :username, :password, :salt, :created_at)";
    try{

        SQLite::Statement exec_query(*db_.get(), query);
        exec_query.bind(":user_id", user.user_id.c_str());
        exec_query.bind(":username", user.username.c_str());
        exec_query.bind(":hashed_password", user.hashed_password.c_str());
        exec_query.bind(":salt", user.salt.c_str());
        
        exec_query.bind(":created_at", format_timestamp_iso8601(user.created_at).c_str());

        int rows_affected = exec_query.exec();
        if(1 != rows_affected){
            return Error{ErrorCode::DATABASE_ERROR, "User creation failed: insert affected " + std::to_string(rows_affected) + " rows."};
        }
        return Success{};
    } catch(SQLite::Exception &ex){
        if(ex.getErrorCode() == SQLITE_CONSTRAINT_UNIQUE){
            return Error{ErrorCode::DUPLICATE_ENTRY, "Username '" + user.username + "' already exists."};
        }
        return Error{ErrorCode::DATABASE_ERROR, std::string("Database error: ") + ex.what()};
    }
}

Result<UserAccount> UserRepository::get_user_by_username(const std::string &username){
    const char* query = 
    "SELECT user_id, username, hashed_password, salt, created_at"
    "FROM users "
    "WHERE username = :username";

    try{
        SQLite::Statement statement(*db_.get(), query);
        statement.bind(":username", username.c_str());
        if(statement.executeStep()){
            UserAccount user;
            user.user_id = statement.getColumn("user_id").getString();
            user.username = statement.getColumn("username").getString();
            user.hashed_password = statement.getColumn("hashed_password").getString();
            user.salt = statement.getColumn("salt").getString();

            std::string created_at_str = statement.getColumn("created_at").getString();
            user.created_at = parse_timestamp_iso8601(created_at_str);
            if (statement.executeStep()) {
                return Error{ErrorCode::DATABASE_ERROR, "Critical: Multiple users found for username '" + username + "'."};
            }
            return user;
        } else {
            return Error{ErrorCode::USER_NOT_FOUND, "User '" + username + "' not found."};
        }
    } catch(const SQLite::Exception &ex){
        return Error{ErrorCode::DATABASE_ERROR, std::string("Database query failed: ") + ex.what()};
    }
}

Result<UserAccount> UserRepository::get_user_by_id(const std::string &user_id){
    const char* query = 
    "SELECT user_id, username, hashed_password, salt, created_at"
    "FROM users "
    "WHERE user_id = :user_id";

    try{
        SQLite::Statement statement(*db_.get(), query);
        statement.bind(":user_id", user_id.c_str());
        if(statement.executeStep()){
            UserAccount user;
            user.user_id = statement.getColumn("user_id").getString();
            user.username = statement.getColumn("username").getString();
            user.hashed_password = statement.getColumn("hashed_password").getString();
            user.salt = statement.getColumn("salt").getString();

            std::string created_at_str = statement.getColumn("created_at").getString();
            user.created_at = parse_timestamp_iso8601(created_at_str);
            if (statement.executeStep()) {
                return Error{ErrorCode::DATABASE_ERROR, "Critical: Multiple users found for user_id'" + user_id + "'."};
            }
            return user;
        } else {
            return Error{ErrorCode::USER_NOT_FOUND, "User '" + user_id + "' not found."};
        }
    } catch(const SQLite::Exception &ex){
        return Error{ErrorCode::DATABASE_ERROR, std::string("Database query failed: ") + ex.what()};
    }
}

Result<std::monostate> UserRepository::update_user_password(const std::string user_id, const std::string hashed_password, const std::string &salt){
    const char* query =
    "UPDATE users SET hashed_password = :hashed_passoword, "
    "salt = :salt"
    "WHERE user_id = :user_id";

    try{
        SQLite::Statement statement(*db_.get(), query);
        statement.bind(":hashed_password", hashed_password.c_str());
        statement.bind(":salt", salt.c_str());
        statement.bind(":user_id", user_id.c_str());

        int rows_affected = statement.exec();
        if(1 != rows_affected){
            return Error{ErrorCode::DATABASE_ERROR, "User password update failed: update affected " + std::to_string(rows_affected) + " rows."};
        }
        return Success{};
    } catch(const SQLite::Exception &ex){
        return Error{ErrorCode::DATABASE_ERROR, std::string("Database query failed: ") + ex.what()};
    }
}