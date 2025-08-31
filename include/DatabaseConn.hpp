#ifndef DATABASECONN_H
#define DATABASECONN_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <mutex>
#include "Error.hpp"


#define CREATE_USERS_TABLE \
    "CREATE TABLE IF NOT EXISTS users (" \
    "user_id TEXT PRIMARY KEY, " \
    "username TEXT UNIQUE NOT NULL, " \
    "hashed_password TEXT NOT NULL, " \
    "salt TEXT NOT NULL, " \
    "created_at TEXT " \
    ");"

#define CREATE_BARRACKS_TABLE \
    "CREATE TABLE IF NOT EXISTS barracks (" \
    "barrack_id TEXT PRIMARY KEY, " \
    "name TEXT UNIQUE NOT NULL, " \
    "admin_id TEXT NOT NULL, " \
    "is_private BOOLEAN DEFAULT FALSE, " \
    "hashed_password TEXT, " \
    "salt TEXT, " \
    "created_at TEXT, " \
    "FOREIGN KEY (admin_id) REFERENCES users(user_id)" \
    ");"

#define CREATE_BARRACK_MEMBERS_TABLE \
    "CREATE TABLE IF NOT EXISTS barrack_members (" \
    "barrack_id TEXT NOT NULL, " \
    "user_id TEXT NOT NULL, " \
    "joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, " \
    "PRIMARY KEY (barrack_id, user_id), " \
    "FOREIGN KEY (user_id) REFERENCES users(user_id), " \
    "FOREIGN KEY (barrack_id) REFERENCES barracks(barrack_id) ON DELETE CASCADE " \
    ");"


#define CREATE_EVENT_OUTBOX_TABLE \
    "CREATE TABLE IF NOT EXISTS event_outbox (" \
    "event_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "event_type TEXT NOT NULL, "\
    "payload TEXT NOT NULL, "\
    "created_at TEXT NOT NULL "\
    ");"

class DatabaseConnection {
    public:
        explicit DatabaseConnection(const std::string& db_path);
        Error initialize_database();
        bool is_valid() const;
        std::shared_ptr<SQLite::Database> get_connection();
    
    private:
        std::shared_ptr<SQLite::Database> db_;
        std::mutex mtx_;
        bool is_initialized_;
};

#endif