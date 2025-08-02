#include "MessageRepo.hpp"

MessageRepository::MessageRepository(std::shared_ptr<SQLite::Database> db) {
    db_ = db;
}

Result<std::monostate> MessageRepository::add(const ChatMessage& message){
    
}

Result<std::vector<ChatMessage>> MessageRepository::get_for_barrack(const std::string &barrack_id, int limit){

}