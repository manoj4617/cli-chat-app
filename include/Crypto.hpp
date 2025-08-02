#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <stdexcept>


class Crypto {
public:
    static void initialize();
    static std::string hash_password(const std::string& password, const std::string& salt);
    static std::string generate_salt();
    static bool verify_password(const std::string& password, const std::string& stored_hash);
    static std::string generate_auth_token(size_t bytes);

private:
    Crypto() = delete;
};
#endif