#include <chrono>
#include <new>
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
    barracks_[b_id] = new_barrack;
    auto result = db_->create_barrack(new_barrack);
    if(std::holds_alternative<Error>(result)){
        barracks_.erase(b_id);
        return std::get<Error>(result);
    }
    return b_id;
}

BarrackManager::BarrackResult BarrackManager::destroy_barrack(const std::string& barrack_id, const std::string& owner_id){
    if(barrack_id.empty() || owner_id.empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }

    auto barrack = db_->get_barrack_by_id(barrack_id);
    if(std::holds_alternative<Error>(barrack)){
        return std::get<Error>(barrack);
    }
    if(std::get<Barrack>(barrack).admin_id != owner_id){
        return Error{ErrorCode::INVALID_OWNER_ID, "Invalid owner ID"};
    }

    auto result = db_->destroy_barrack(barrack_id);
    if(std::holds_alternative<Error>(result)){
        return std::get<Error>(result);
    }

    barracks_.erase(barrack_id);
    barracks_members_.erase(barrack_id);
    barracks_messages_.erase(barrack_id);

    return std::string("Barrack destroyed");
}

BarrackManager::MemberResult BarrackManager::join_barrack(const std::string &barrack_id, const std::string &user_id, std::optional<std::string> password){
    bool pass_result;
    if(barrack_id.empty() || user_id.empty() || password->empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }

    auto result = db_->get_barrack_by_id(barrack_id);

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

    auto join_result = db_->join_barrack(barrack_id, user_id);
    if(std::holds_alternative<Error>(join_result)){
        return std::get<Error>(join_result);
    }

    return std::string("Joined barrack");
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