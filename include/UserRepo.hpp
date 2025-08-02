#ifndef USERREPO_H
#define USERREPO_H

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <variant>
#include "types.hpp"
#include "Error.hpp"


class UserRepository {
    public:
        explicit UserRepository(std::shared_ptr<SQLite::Database> db);

        Result<std::monostate> create_user(const UserAccount& user);
        Result<UserAccount> get_user_by_username(const std::string& username);
        Result<UserAccount> get_user_by_id(const std::string& user_id);
        Result<std::monostate> update_user_password(const std::string user_id, const std::string new_password, const std::string& salt);

    private:
        std::shared_ptr<SQLite::Database> db_;
};

#endif