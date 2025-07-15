#ifndef BARRACK_MANAGER_H
#define BARRACK_MANAGER_H

#include <vector>
#include <unordered_map>
#include <mutex>

#include "DatabaseManager.hpp"
#include "Error.hpp"
#include <variant>

class BarrackManager{
    public:
        using BarrackResult = std::variant<std::string, Error>;
        using MemberResult = std::variant<std::string, Error>;

        BarrackManager(std::shared_ptr<DatabaseManager> db) : db_(db) {};

        BarrackResult create_barrack(const std::string& barrack_name, const std::string& owner_uid, bool is_private, std::optional<std::string> password);
        BarrackResult destroy_barrack(const std::string& barrack_id, const std::string& owener_uid);

        MemberResult join_barrack(const std::string& barrack_id, const std::string& user_id, std::optional<std::string> password);
        MemberResult leave_barrack(const std::string& barrack_id, const std::string& user_id);
        std::optional<std::string> message_barrack(const std::string& barrack_id, const std::string& user_id, const std::string& message);

        std::optional<Barrack> get_barrack(const std::string& barrack_id);
        std::optional<BarrackMember> get_barrack_member(const std::string& barrack_id, const std::string& user_id);
        std::optional<std::vector<BarrackMember>> get_barrack_members(const std::string& barrack_id);
        std::optional<std::vector<ChatMessage>> get_barrack_messages(const std::string& barrack_id);

    private:
        std::unordered_map<std::string, Barrack> barracks_;                                 // barrack id -> barrack
        std::unordered_map<std::string, std::vector<BarrackMember>> barracks_members_;      // barrack id -> members
        std::unordered_map<std::string, std::vector<ChatMessage>> barracks_messages_;       // barrack id -> messages

        std::mutex mtx_;

        std::shared_ptr<DatabaseManager> db_;
};

#endif