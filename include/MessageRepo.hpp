#ifndef MESSAGEREPOSITORY_H
#define MESSAGEREPOSITORY_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "types.hpp"
#include "Error.hpp"
#include "DatabaseConn.hpp"

class MessageRepository {
public:
    explicit MessageRepository(std::shared_ptr<SQLite::Database> db);

    Result<std::monostate> add(const ChatMessage& message);
    Result<std::vector<ChatMessage>> get_for_barrack(const std::string& barrack_id, int limit = 100);

private:
    std::shared_ptr<SQLite::Database> db_;
};
#endif