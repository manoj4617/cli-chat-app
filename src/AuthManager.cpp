#include <exception>
#include <iostream>
#include <mutex>
#include <string>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators (random, name-based, etc.)
#include <boost/uuid/uuid_io.hpp> 

#include "AuthManager.hpp"
#include "DatabaseManager.hpp"
#include "Messages.hpp"
#include "types.hpp"


AuthManager::AuthResult AuthManager::authenticate_user(const std::string& username, const std::string& password){
    if(username.empty() || password.empty()){
        return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
    }

    try{
        
        auto result = db_->get_user_by_username(username);
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
            return token;
        } else if(std::get_if<Error>(&result)){
            return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
        } else {
            return Error{ErrorCode::INVALID_CREDENTIALS, "Unexpected Database response"};
        }

    } catch(const std::exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, ex.what()};
    }
}

AuthManager::AuthResult AuthManager::create_user(const std::string& username, const std::string& password){
    if(username.empty() || password.empty()){
        return Error{ErrorCode::INVALID_CREDENTIALS, "Invalid Credentials"};
    }

    try{
        auto result = db_->get_user_by_username(username);
        if(UserAccount* user_ptr = std::get_if<UserAccount>(&result)){
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
    if(user_id.empty()){
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
    unsigned char token_bytes[AUTH_TOKEN_BYTES];
    randombytes_buf(token_bytes, sizeof(token_bytes)); 
    
    std::string token_hex(AUTH_TOKEN_BYTES * 2, '\0');
    if (sodium_bin2hex(&token_hex[0], token_hex.length() + 1,
                       token_bytes, sizeof(token_bytes)) == nullptr) {
        throw std::runtime_error("Failed to convert random bytes to hex for token.");
    }
    
    std::lock_guard<std::mutex> lock(mtx_);
    tokens_[user_id] = token_hex; 

    sodium_memzero(token_bytes, sizeof(token_bytes)); 

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

void AuthManager::initializeSodium(){
    if(sodium_init() == -1){
        throw std::runtime_error("libsodium initialization failed!");
    }
}

std::string AuthManager::hash_password(const std::string& password, const std::string& salt){
   if(salt.length() != crypto_pwhash_SALTBYTES * 2) {
        throw std::invalid_argument("Invalid salt length. Salt must be " +
                                    std::to_string(crypto_pwhash_SALTBYTES * 2) + " hexadecimal characters.");
    }

    unsigned char salt_bin[crypto_pwhash_SALTBYTES];
    if(sodium_hex2bin(salt_bin, sizeof(salt_bin), 
                    salt.c_str(), salt.length(), 
                    nullptr, nullptr, nullptr) == 0){
                        throw std::runtime_error("Failed to convert hex salt to binary");
    }
    
    unsigned char hashed_password_cstr[HASH_BYTES];

   if (crypto_pwhash(hashed_password_cstr, sizeof(hashed_password_cstr),
                      password.c_str(), password.length(), // Password as C-string and its length
                      salt_bin, // Binary salt
                      OPSLIMIT, // Operations limit (CPU cost)
                      MEMLIMIT, // Memory limit (RAM cost)
                      crypto_pwhash_ALG_ARGON2ID13) != 0) { // Algorithm: Argon2id version 13
        throw std::runtime_error("Password hashing failed.");
    }

    std::string hashed_password_str(reinterpret_cast<const char*>(hashed_password_cstr));
    sodium_memzero(hashed_password_cstr, sizeof(hashed_password_cstr));
    sodium_memzero(salt_bin, sizeof(salt_bin));
    return hashed_password_str;
}

std::string AuthManager::generate_salt(){
    unsigned char salt[crypto_pwhash_SALTBYTES];
    randombytes_buf(salt, sizeof salt);
    std::string salt_str(crypto_pwhash_SALTBYTES * 2, '\0');
    sodium_bin2hex(&salt_str[0], salt_str.length() + 1, salt, sizeof(salt));
    return salt_str;
}

std::string AuthManager::generate_user_id(){
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();

    std::stringstream ss;
    ss << uuid;
    return ss.str();
}

bool AuthManager::verify_password(const std::string& password, const std::string& stored_hash){
    if(password.empty()  || stored_hash.empty() ){
        throw std::runtime_error("Invalid parameters for password verification");
    }

    int res = crypto_pwhash_str_verify(stored_hash.c_str(), password.c_str(), password.length());

    return (res == 0);
}
   