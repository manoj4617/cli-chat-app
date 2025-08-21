#ifndef MESSAGEREPOSITORY_H
#define MESSAGEREPOSITORY_H

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "types.hpp"
#include "Error.hpp"

class MessageRepository {
    public:
        virtual ~MessageRepository() = default;
        virtual Result<std::monostate> add(const ChatMessage& message) = 0;
        virtual Result<std::vector<ChatMessage>> get_for_barrack(const std::string& barrack_id, int limit = 50) = 0;  
};
#endif