#ifndef BARRACK_MANAGER_H
#define BARRACK_MANAGER_H

#include <vector>
#include <unordered_map>
#include <mutex>
#include <sodium.h>

#include "DatabaseManager.hpp"
#include "Error.hpp"
#include <variant>

class BarrackManager{
    public:
        using BarrackResult = Result<std::string>;
        using StatusResult = std::variant<Success, Error>;

        BarrackManager(std::shared_ptr<DatabaseManager> db) : db_(db) {};

        BarrackResult create_barrack(const std::string& barrack_name, const std::string& owner_uid, bool is_private, std::optional<std::string> password);
        BarrackResult destroy_barrack(const std::string& barrack_id, const std::string& owener_uid);

        StatusResult join_barrack(const std::string& barrack_id, const std::string& user_id, std::optional<std::string> password);
        StatusResult leave_barrack(const std::string& barrack_id, const std::string& user_id);
        std::optional<std::string> message_barrack(const std::string& barrack_id, const std::string& user_id, const std::string& message);

        std::optional<Barrack> get_barrack(const std::string& barrack_id);
        std::optional<BarrackMember> get_barrack_member(const std::string& barrack_id, const std::string& user_id);
        std::optional<std::vector<BarrackMember>> get_barrack_members(const std::string& barrack_id);
        std::optional<std::vector<ChatMessage>> get_barrack_messages(const std::string& barrack_id);

    private:
        std::string generate_barrack_id();
        std::string hash_password(const std::string& passowrd, const std::string& salt);
        bool verify_password(const std::string& hashed_password, const std::string& stored_hash);
        std::string generate_salt();
        std::unordered_map<std::string, Barrack> barracks_;                                 // barrack id -> barrack
        std::unordered_map<std::string, std::vector<BarrackMember>> barracks_members_;      // barrack id -> members
        std::unordered_map<std::string, std::vector<ChatMessage>> barracks_messages_;       // barrack id -> messages

        std::mutex mtx_;

        std::shared_ptr<DatabaseManager> db_;
};

#endif