#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <random>
#include <variant>
#include "Crypto.hpp"
#include "types.hpp"
#include "Error.hpp"
#include "UserRepo.hpp"

class AuthManager{
    public:
        using AuthCreds = Result<std::pair<std::string, std::string>>;
        using AuthResult = Result<std::string>;
        using StatusResult = std::variant<Success, Error>;

        AuthManager(std::shared_ptr<UserRepository> user_repo) : user_repo_(user_repo), gen_(rd_()) {};

        AuthCreds authenticate_user(const std::string& username, const std::string& password);
        AuthCreds create_user(const std::string& username, const std::string& password);

        std::string generate_auth_token(const std::string& user_id);
        AuthResult validate_token(const std::string& token);
        void invalidate_token(const std::string& token);

        AuthResult get_username(const std::string& user_id);
        bool user_exists(const std::string& user_id);

    private:
        std::string generate_user_id();
        std::string hash_password(const std::string& passowrd, const std::string& salt);
        bool verify_password(const std::string& hashed_password, const std::string& stored_hash);
        std::string generate_salt();

        std::unordered_map<std::string, UserAccount> users_by_name_;    // username -> user account
        std::unordered_map<std::string, std::string> tokens_;           // username -> token
        std::unordered_map<std::string, std::string> usernames_;        // user_id  -> username

        std::shared_ptr<UserRepository> user_repo_;

        std::mutex mtx_;                                                // thread safety
        std::random_device rd_;
        std::mt19937 gen_;
        
        static const size_t AUTH_TOKEN_BYTES = 32;
        static const size_t UUID_BYTES = 16;
};

#endif