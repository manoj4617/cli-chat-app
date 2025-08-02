#ifndef DATABASECONN_H
#define DATABASECONN_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <mutex>
#include "Error.hpp"


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