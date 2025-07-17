#include <exception>
#include <iostream>
#include <mutex>
#include <string>

#include "AuthManager.hpp"
#include "DatabaseManager.hpp"
#include "Messages.hpp"
#include "types.hpp"


AuthManager::AuthResult AuthManager::authenticate_user(const std::string& username, const std::string& password){
    if(username.size() <= 0 || password.size() <= 0){
        return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
    }

    try{
        
        auto result = db_->get_user_by_username(username);
        if(UserAccount* user_ptr = std::get_if<UserAccount>(&result)){
            UserAccount &user = *user_ptr;

            std::string hash_p = hash_password(password);
            if(!verify_password(hash_p, user.hashed_password)){
                return Error{ErrorCode::INVALID_PASSWORD, "Password entered is invalid"};
            }
            std::string token = generate_auth_token(user.user_id);
            std::lock_guard<std::mutex> lock(mtx_);
            tokens_[user.user_id] = token;
            usernames_[user.user_id] = user.username;
            std::cout << "[INFO] User " << user.username << " authenticated successfully." << std::endl;
            return token;
        }

    } catch(const std::exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, ex.what()};
    }
}

AuthManager::AuthResult AuthManager::create_user(const std::string& username, const std::string& password){
    if(username.size() <= 0 || password.size() <= 0){
        return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
    }

    try{
        auto result = db_->get_user_by_username(username);
        if(UserAccount* user_ptr = std::get_if<UserAccount>(&result)){
            return Error{ErrorCode::USER_ALREADY_EXISTS, "User already exists"};
        }
        std::string salt = generate_salt();
        std::string hashed_password = hash_password(password + salt);
        std::string user_id = generate_user_id();

        UserAccount user(user_id,
                         username,
                         hashed_password,
                         generate_salt(),
                         std::chrono::system_clock::now());
        auto create_result = db_->create_user(user);
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
        return tokens_[user.user_id];
    } catch(const std::exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, ex.what()};
    }
}

std::string AuthManager::get_username(const std::string& user_id){
    if(user_id.size() <= 0){
        return "";
    }
    {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = usernames_.find(user_id);
        if(it != usernames_.end()){
            return it->second;
        }
    }

    try{
        auto result = db_->get_user_by_id(user_id);
        if(UserAccount* user_ptr = std::get_if<UserAccount>(&result)){
            UserAccount &user = *user_ptr;
            std::lock_guard<std::mutex> lock(mtx_);
            usernames_[user_id] = user.username;
            return user.username;
        }
        return "";
    } catch(const std::exception& ex){
        std::cerr << "Database error: " << ex.what() << std::endl;
        return "";
    }
}

bool AuthManager::user_exists(const std::string& username){
    return (get_username(username) != "");
}

std::string AuthManager::generate_auth_token(const std::string& user_id){
    std::lock_guard<std::mutex> lock(mtx_);
    std::string token =
        std::to_string(std::hash<std::string>{}(user_id + std::to_string(std::chrono::system_clock::now().time_since_epoch().count())));
    tokens_[user_id] = token;
    return token;
}

AuthManager::AuthResult validate_token(const std::string& token){
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