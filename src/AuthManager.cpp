#include <exception>
#include <iostream>

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

            // password stuff
            
            return user.user_id;
        }

    } catch(const std::exception& ex){
        return Error{ErrorCode::DATABASE_ERROR, ex.what()};
    }
}