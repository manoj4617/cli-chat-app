#include <exception>
#include <iostream>
#include <mutex>
#include <string>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators (random, name-based, etc.)
#include <boost/uuid/uuid_io.hpp> 

#include "AuthManager.hpp"
#include "UserRepo.hpp"
#include "Messages.hpp"
#include "types.hpp"


AuthManager::AuthCreds AuthManager::authenticate_user(const std::string& username, const std::string& password){
    if(username.empty() || password.empty()){
        return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
    }

    try{
        
        auto result = user_repo_->get_user_by_username(username);
        if(UserAccount* user_ptr = std::get_if<UserAccount>(&result)){
            UserAccount &user = *user_ptr;

            if(!verify_password(password, user.hashed_password)){
                return Error{ErrorCode::INVALID_PASSWORD, "Password entered is invalid"};
            }
            std::string token = generate_auth_token(user.user_id);
            std::lock_guard<std::mutex> lock(mtx_);
            tokens_[user.user_id] = token;
            usernames_[user.user_id] = user.username;
            std::cout << "[INFO] User " << user.username << " authenticated successfully." << std::endl;
            return std::make_pair(user.user_id, token);
        } else if(std::get_if<Error>(&result)){
            return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
        } else {
            return Error{ErrorCode::INVALID_CREDENTIALS, "Unexpected Database response"};
        }

    } catch(const std::exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, ex.what()};
    }
}

AuthManager::AuthCreds AuthManager::create_user(const std::string& username, const std::string& password){
    if(username.empty() || password.empty()){
        return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
    }

    try{
        auto result = user_repo_->get_user_by_username(username);
        if(std::get_if<UserAccount>(&result)){
            return Error{ErrorCode::USER_ALREADY_EXISTS, "User already exists"};
        }
        std::string salt = generate_salt();
        std::string hashed_password = hash_password(password, salt);
        std::string user_id = generate_user_id();

        UserAccount user(user_id,
                         username,
                         hashed_password,
                         salt,
                         std::chrono::system_clock::now());
        auto create_result = user_repo_->create_user(user);
        if(std::holds_alternative<Error>(create_result)){
            return std::get<Error>(create_result);
        }
        std::cout << "[INFO] User " << username << " created successfully." << std::endl;
        std::string token = generate_auth_token(user.user_id);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tokens_[user.user_id] = token;
            usernames_[user.user_id] = username;
        }
        return std::make_pair(user.user_id,tokens_[user.user_id]);
    } catch(const std::exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, ex.what()};
    }
}

AuthManager::AuthResult AuthManager::get_username(const std::string& user_id){
    if(user_id.empty()){
        return Error{ErrorCode::INVALID_DATA, "Invalid data"};
    }
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = usernames_.find(user_id);
        if(it != usernames_.end()){
            return it->second;
        }
    }

    try{
        auto result = user_repo_->get_user_by_id(user_id);
        if(UserAccount* user_ptr = std::get_if<UserAccount>(&result)){
            UserAccount &user = *user_ptr;
            std::lock_guard<std::mutex> lock(mtx_);
            usernames_[user_id] = user.username;
            return user.username;
        }
        return Error{ErrorCode::USER_NOT_FOUND, "User not found"};
    } catch(const std::exception& ex){
        std::cerr << "Database error: " << ex.what() << std::endl;
        std::string msg = std::string("Database error: ") + ex.what();
        return Error{ErrorCode::DATABASE_ERROR, msg};
    }
}

bool AuthManager::user_exists(const std::string& user_id){
    auto res = get_username(user_id);
    return std::holds_alternative<Error>(res); 
}

std::string AuthManager::generate_auth_token(const std::string& user_id){
    std::string token_hex = Crypto::generate_auth_token(AUTH_TOKEN_BYTES);
    
    std::lock_guard<std::mutex> lock(mtx_);
    tokens_[user_id] = token_hex; 
    return token_hex;
}

AuthManager::AuthResult AuthManager::validate_token(const std::string& token){
    std::lock_guard<std::mutex> lock(mtx_);
    for(const auto& pair : tokens_){
        if(pair.second == token){
            return pair.first; // Return user_id
        }
    }
    return Error{ErrorCode::INVALID_TOKEN, "Token is invalid or expired"};
}

void AuthManager::invalidate_token(const std::string& token){
    std::lock_guard<std::mutex> lock(mtx_);
    for(auto it = tokens_.begin(); it != tokens_.end();){
        if(it->second == token){
            it = tokens_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AuthManager::hash_password(const std::string& password, const std::string& salt){
   return Crypto::hash_password(password, salt);
}

std::string AuthManager::generate_salt(){
    return Crypto::generate_salt();
}


std::string AuthManager::generate_user_id(){
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();

    std::stringstream ss;
    ss << uuid;
    return ss.str();
}

bool AuthManager::verify_password(const std::string& password, const std::string& stored_hash){
    return Crypto::verify_password(password, stored_hash);
}
   