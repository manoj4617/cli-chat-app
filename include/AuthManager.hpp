#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <random>
#include <variant>
#include <sodium.h>
#include "DatabaseManager.hpp"
#include "Error.hpp"

class AuthManager{
    public:
    using AuthResult = std::variant<std::string, Error>;
        AuthManager(std::shared_ptr<DatabaseManager> db) : db_(db), gen_(rd_()) {};

        AuthResult authenticate_user(const std::string& username, const std::string& password);
        AuthResult create_user(const std::string& username, const std::string& password);

        std::string generate_auth_token(const std::string& user_id);
        AuthResult validate_token(const std::string& token);
        void invalidate_token(const std::string& token);

        std::string get_username(const std::string& user_id);
        bool user_exists(const std::string& username);

        static void initializeSodium();
    private:
        std::string hash_password(const std::string& passowrd, const std::string& salt);
        bool verify_password(const std::string& hashed_password, const std::string& stored_hash);
        std::string generate_user_id();
        std::string generate_salt();

        std::unordered_map<std::string, UserAccount> users_by_name_;    // username -> user account
        std::unordered_map<std::string, std::string> tokens_;           // username -> token
        std::unordered_map<std::string, std::string> usernames_;        // user_id  -> username

        std::shared_ptr<DatabaseManager> db_;

        std::mutex mtx_;                                                // thread safety
        std::random_device rd_;
        std::mt19937 gen_;

        static const int OPSLIMIT = crypto_pwhash_OPSLIMIT_INTERACTIVE;
        static const size_t MEMLIMIT = crypto_pwhash_MEMLIMIT_INTERACTIVE;
        static const size_t HASH_BYTES = crypto_pwhash_STRBYTES;
        static const size_t AUTH_TOKEN_BYTES = 32;
        static const size_t UUID_BYTES = 16;
};

#endif