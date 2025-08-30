#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators (random, name-based, etc.)
#include <boost/uuid/uuid_io.hpp>
#include <variant>

#include "BarrackManager.hpp"
#include "Crypto.hpp"
#include "Error.hpp"
#include "types.hpp"

using Clock = std::chrono::system_clock;

void BarrackManager::dispatch_cass_message(){
    while (auto msg_optional = message_queue_.wait_and_pop()) {
        ChatMessage& msg_to_db = *msg_optional;

        auto res = msg_repo_->add(msg_to_db);
        if (std::holds_alternative<Error>(res)) {
            std::cerr << "Chat messages insertion to database failed for message ID: "
                      << msg_to_db.message_id << std::endl;
        } else {
            std::cout << "Message ID: " << msg_to_db.message_id << " saved to database." << std::endl;
        }
    }
    std::cout << "Message dispatcher thread finished." << std::endl;
}

BarrackManager::BarrackResult BarrackManager::create_barrack(const std::string& barrack_name, 
                                                             const std::string& owner_id,
                                                             bool is_private,
                                                             std::optional<std::string> password)
{

    if(barrack_name.size() <= MIN_BARRAK_NAME_LEN || owner_id.empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }
    std::string hashed_password = "";
    std::string salt = "";
    if(is_private){
        if(password.value().empty()){
            return Error{ErrorCode::INVALID_DATA, "Invalid data"};
        }
        salt = generate_salt();
        hashed_password = hash_password(password.value(), salt);
    }
    Clock::time_point created_at = Clock::now(); 
    
    std::string b_id = generate_barrack_id();
    Barrack new_barrack(b_id, barrack_name, owner_id, is_private, hashed_password, salt, created_at);
    std::lock_guard<std::mutex> lock(mtx_);
    barracks_[b_id] = new_barrack;
    auto result = barrack_repo_->create(new_barrack);
    if(std::holds_alternative<Error>(result)){
        barracks_.erase(b_id);
        return std::get<Error>(result);
    }
    return b_id;
}

BarrackManager::~BarrackManager(){
    message_queue_.shutdown();
    if(message_dispatcher_.joinable()){
        message_dispatcher_.join();
    }
}

BarrackManager::StatusResult BarrackManager::destroy_barrack(const std::string& barrack_id, const std::string& owner_id){
    if(barrack_id.empty() || owner_id.empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }

    auto barrack = barrack_repo_->find_by_id(barrack_id);
    if(std::holds_alternative<Error>(barrack)){
        return std::get<Error>(barrack);
    }
    if(std::get<Barrack>(barrack).admin_id != owner_id){
        return Error{ErrorCode::INVALID_OWNER_ID, "Invalid owner ID"};
    }

    auto result = barrack_repo_->destroy(barrack_id);
    if(std::holds_alternative<Error>(result)){
        return std::get<Error>(result);
    }

    std::lock_guard<std::mutex> lock(mtx_);
    barracks_.erase(barrack_id);
    barracks_members_.erase(barrack_id);
    barracks_messages_.erase(barrack_id);

    return SUCCESS;
}

BarrackManager::StatusResult BarrackManager::join_barrack(const std::string &barrack_id, const std::string &user_id, std::optional<std::string> password){
    bool pass_result;
    if(barrack_id.empty() || user_id.empty() || password->empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }

    auto result = barrack_repo_->find_by_id(barrack_id);

    if(std::holds_alternative<Error>(result)){
        return std::get<Error>(result);
    }

    std::string stored_pass = std::get<Barrack>(result).hashed_password.value();    // std::optional<std::string>::value
    if(password.has_value()){
        pass_result = verify_password(password.value(),stored_pass);   
        if(!pass_result){
            return Error{ErrorCode::INVALID_PASSWORD, "Invalid barrack password"};
        }
    }

    auto join_result = barrack_repo_->add_member(barrack_id, user_id);
    if(std::holds_alternative<Error>(join_result)){
        return std::get<Error>(join_result);
    }

    std::lock_guard<std::mutex> lock(mtx_);
    barracks_members_[barrack_id].emplace_back(barrack_id, user_id, Clock::now());
    return SUCCESS;
}


BarrackManager::StatusResult BarrackManager::leave_barrack(const std::string &barrack_id, const std::string &user_id){
    if(barrack_id.empty() || user_id.empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid Data to"};
    }

    auto result = barrack_repo_->find_by_id(barrack_id);
    if(std::holds_alternative<Error>(result)){
        return std::get<Error>(result);
    }

    auto barrack = std::get<Barrack>(result);
    std::unique_lock<std::mutex> lock(mtx_);
    auto members = barracks_members_[barrack_id];
    auto member_it = std::find_if(members.begin(), members.end(), 
                [&user_id](const BarrackMember& member){
                    return member.user_id == user_id;
                });
    lock.unlock();
    if(member_it != members.end()){
        auto leave_result = barrack_repo_->remove_member(barrack_id, user_id);
        if(std::holds_alternative<Error>(leave_result)){
            return std::get<Error>(leave_result);
        }
        members.erase(member_it);
        return SUCCESS;
    }
    return Error{ErrorCode::MEMBER_NOT_FOUND, "User is not member of this barrack"};
}

BarrackManager::StatusResult BarrackManager::message_barrack(const std::string &barrack_id, const std::string &user_id, const std::string &message){
    if(barrack_id.empty() || user_id.empty() || message.empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }

    std::unique_lock<std::mutex> lock(mtx_);
    auto member_present = std::find_if(barracks_members_[barrack_id].begin(), barracks_members_[barrack_id].end(),
                                                [&user_id](const BarrackMember& member){
                                                        return member.user_id == user_id;
                                                });

    if(member_present == barracks_members_[barrack_id].end()){
        lock.unlock();
        return Error{ErrorCode::USER_NOT_FOUND, "User not member of the group"};
    }
    lock.unlock();
    ChatMessage msg(generate_message_id(),
                    barrack_id,
                    user_id,
                    message,
                    Clock::now());
               
    barracks_messages_[barrack_id].push_back(msg);
    message_queue_.push(std::move(msg));

    return SUCCESS;
}

std::optional<Barrack> BarrackManager::get_barrack(const std::string &barrack_id){

    std::unique_lock<std::mutex> lock(mtx_);
    if(barracks_.find(barrack_id) != barracks_.end()){
        lock.unlock();
        return barracks_[barrack_id];
    }
    lock.unlock();
    auto result = barrack_repo_->find_by_id(barrack_id);
    if(std::holds_alternative<Error>(result)){
        return std::nullopt;
    }

    return std::get<Barrack>(result);
}

std::optional<BarrackMember> BarrackManager::get_barrack_member(const std::string &barrack_id, const std::string &user_id){
    if(barrack_id.empty() || user_id.empty()){
        return std::nullopt;
    }

    std::unique_lock<std::mutex> lock(mtx_);
    auto members_it = barracks_members_.find(barrack_id);
    if(members_it == barracks_members_.end()){
        lock.unlock();    
        return std::nullopt;
    }
    auto& members = members_it->second;
    auto member_itr = std::find_if(members.begin(), members.end(),
    [&user_id](const BarrackMember& member){
        return member.user_id == user_id;
    });
    
    if(member_itr != members.end()){
        lock.unlock();
        return *member_itr;
    }
    lock.unlock();
    return std::nullopt;
}

std::optional<std::vector<ChatMessage>> BarrackManager::get_barrack_messages(const std::string &barrack_id){
    if(barrack_id.empty()){
        return std::nullopt;
    }
    std::unique_lock<std::mutex> lock(mtx_);
    if(barracks_messages_.find(barrack_id) != barracks_messages_.end()){
        lock.unlock();
        return barracks_messages_[barrack_id];
    }
    lock.unlock();
    auto result = msg_repo_->get_for_barrack(barrack_id);
    if(std::holds_alternative<Error>(result)){
        return std::nullopt;
    }
    return std::get<std::vector<ChatMessage>>(result);
}

std::optional<std::vector<BarrackMember>> BarrackManager::get_barrack_members(const std::string &barrack_id){
    if(barrack_id.empty()){
        return std::nullopt;
    }
    std::unique_lock<std::mutex> lock(mtx_);

    if(barracks_members_.find(barrack_id) != barracks_members_.end()){
        lock.unlock();
        return barracks_members_[barrack_id];
    }
    lock.unlock();

    auto result = barrack_repo_->get_members(barrack_id);
    if(std::holds_alternative<Error>(result)){
        return std::nullopt;
    }
    return std::get<std::vector<BarrackMember>>(result);
}

std::string BarrackManager::hash_password(const std::string& password, const std::string& salt){
   return Crypto::hash_password(password, salt); 
}

std::string BarrackManager::generate_salt(){
    return Crypto::generate_salt();
}

bool BarrackManager::verify_password(const std::string& password, const std::string& stored_hash){
    return Crypto::verify_password(password, stored_hash);
}

std::string BarrackManager::generate_barrack_id(){
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();

    std::stringstream ss;
    ss << "barrack_" << uuid;
    return ss.str();
}

std::string BarrackManager::generate_message_id(){
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();

    std::stringstream ss;
    ss << "msg_" << uuid;
    return ss.str();
}