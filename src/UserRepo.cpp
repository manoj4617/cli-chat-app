#include "UserRepo.hpp"

UserRepository::UserRepository(std::shared_ptr<SQLite::Database> db){
    db_ = db;
}

Result<std::monostate> UserRepository::create_user(const UserAccount &user){

}

Result<UserAccount> UserRepository::get_user_by_username(const std::string &username){

}

Result<UserAccount> UserRepository::get_user_by_id(const std::string &user_id){

}

Result<std::monostate> UserRepository::update_user_password(const std::string user_id, const std::string new_password, const std::string &salt){
    
}