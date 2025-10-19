#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <chrono>
#include <optional>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators (random, name-based, etc.)
#include <boost/uuid/uuid_io.hpp>

#define MIN_BARRAK_NAME_LEN 5

struct UserAccount {
    std::string user_id;
    std::string username;
    std::string password;
    std::string hashed_password;
    std::string salt;

    std::chrono::system_clock::time_point created_at;
    UserAccount() = default;
    UserAccount(std::string uid, std::string uname, std::string h_pass, std::string s,
                std::chrono::system_clock::time_point created)
        : user_id(std::move(uid)), username(std::move(uname)),
          hashed_password(std::move(h_pass)), salt(std::move(s)),
          created_at(created) {}
    virtual ~UserAccount() = default;
};

struct Barrack{
    std::string barrack_id;
    std::string barrack_name;
    std::string admin_id;
    bool is_private;
    std::optional<std::string> hashed_password;
    std::optional<std::string> salt;   
    std::chrono::system_clock::time_point created_at;
     Barrack(std::string bid, std::string bname, std::string owner_uid, bool is_priv,
            std::optional<std::string> h_pass, std::optional<std::string> salt,
            std::chrono::system_clock::time_point created)
        : barrack_id(std::move(bid)), barrack_name(std::move(bname)),
          admin_id(std::move(owner_uid)), is_private(is_priv),
          hashed_password(std::move(h_pass)), salt(std::move(salt)),
          created_at(created) {}

    Barrack() = default;

    virtual ~Barrack() = default;
};


struct BarrackMember {
    std::string barrack_id;
    std::string user_id;
    std::chrono::system_clock::time_point joined_at;

     BarrackMember(std::string bid, std::string uid, std::chrono::system_clock::time_point joined)
        : barrack_id(std::move(bid)), user_id(std::move(uid)), joined_at(joined) {}

    BarrackMember() = default;
    virtual ~BarrackMember() = default;
};

struct ChatMessage {
    boost::uuids::uuid message_id;     // Primary Key (e.g., UUID)
    std::string barrack_id;     // Foreign Key to Barracks.barrack_id (where message was sent)
    std::string sender_user_id; // Foreign Key to UserAccounts.user_id
    std::string content;        // The message text
    uint64_t sequence_id;
    std::chrono::system_clock::time_point sent_at;

    ChatMessage(boost::uuids::uuid mid, std::string bid, std::string sid, std::string msg_content,
                uint64_t sequence_id_, std::chrono::system_clock::time_point sent)
        : message_id(std::move(mid)), barrack_id(std::move(bid)),
          sender_user_id(std::move(sid)), 
          content(std::move(msg_content)), 
          sequence_id(sequence_id_),
          sent_at(sent) {}

    ChatMessage() = default;
    virtual ~ChatMessage() = default;
};
#endif