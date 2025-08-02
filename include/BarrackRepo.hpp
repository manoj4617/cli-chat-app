#ifndef BARRACKREPOSITORY_H
#define BARRACKREPOSITORY_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "types.hpp"
#include "Error.hpp"

class BarrackRepository {
public:
    explicit BarrackRepository(std::shared_ptr<SQLite::Database> db);

    Result<std::monostate> create(const Barrack& barrack);
    Result<std::monostate> destroy(const std::string& barrack_id);
    Result<Barrack> find_by_id(const std::string& barrack_id);
    
    // Member operations
    Result<std::monostate> add_member(const std::string& barrack_id, const std::string& user_id);
    Result<std::monostate> remove_member(const std::string& barrack_id, const std::string& user_id);
    Result<std::vector<BarrackMember>> get_members(const std::string& barrack_id);

private:
    std::shared_ptr<SQLite::Database> db_;
};
#endif