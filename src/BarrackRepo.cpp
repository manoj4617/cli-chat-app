#include "BarrackRepo.hpp"

BarrackRepository::BarrackRepository(std::shared_ptr<SQLite::Database> db){
    db_ = db;
}

Result<std::monostate>  BarrackRepository::create(const Barrack& barrack){

}

Result<std::monostate> BarrackRepository::destroy(const std::string& barrack_id){

}

Result<Barrack> BarrackRepository::find_by_id(const std::string &barrack_id){

}

Result<std::monostate> BarrackRepository::add_member(const std::string &barrack_id, const std::string &user_id){

}

Result<std::monostate> BarrackRepository::remove_member(const std::string &barrack_id, const std::string &user_id){

}

Result<std::vector<BarrackMember>> BarrackRepository::get_members(const std::string &barrack_id){
    
}