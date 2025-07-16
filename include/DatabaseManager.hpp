#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <vector>
#include <string>
#include <variant>

#include "types.hpp"
#include "Error.hpp"

const std::variant<std::monostate, Error> SUCCESS_RESULT = std::monostate{};

class DatabaseManager {
    public:
        explicit DatabaseManager(const std::string& db_path);
        
        Error initialize_database();
        bool is_valid() const;
        //User operations
        std::variant<std::monostate, Error> create_user(const UserAccount& user);
        std::variant<UserAccount, Error> get_user_by_username(const std::string& username);
        std::variant<UserAccount, Error> get_user_by_id(const std::string& user_id);
        std::variant<std::monostate, Error> update_user_password(std::string user_id, std::string new_password);

        //Barrack operations
        std::variant<std::monostate, Error> create_barrack(const Barrack& barrack);
        std::variant<std::monostate, Error> destroy_barrack(const std::string& barrack_id);
        std::variant<std::monostate, Error> join_barrack(const std::string& barrack_id, const std::string& user_id);
        std::variant<std::monostate, Error> leave_barrack(const std::string& barrack_id, const std::string& user_id);

        std::variant<Barrack, Error> get_barrack_by_id(const std::string& barrack_id);
        std::variant<std::monostate, Error> add_barrack_member(const BarrackMember& member);
        std::variant<std::monostate, Error> remove_barrack_member(const BarrackMember& member);

        //Message Operations
        std::variant<std::monostate, Error> add_message(const ChatMessage& message);
        std::vector<ChatMessage> get_barrack_messages(const std::string& barrack_id);
    
    private:
        std::unique_ptr<SQLite::Database> db_;
        bool is_initialized_;
};

#endif