#include <sodium.h>
#include "../include/Crypto.hpp"

static const int OPSLIMIT = crypto_pwhash_OPSLIMIT_INTERACTIVE;
static const size_t MEMLIMIT = crypto_pwhash_MEMLIMIT_INTERACTIVE;
static const size_t HASH_BYTES = crypto_pwhash_STRBYTES;

void Crypto::initialize() {
  if (sodium_init() == -1) {
    throw std::runtime_error("libsodium initialization failed!");
  }
}

std::string Crypto::hash_password(const std::string &password,
                                 const std::string &salt) {
  if (salt.length() != crypto_pwhash_SALTBYTES * 2) {
    throw std::invalid_argument("Invalid salt length. Salt must be " +
                                std::to_string(crypto_pwhash_SALTBYTES * 2) +
                                " hexadecimal characters.");
  }

  unsigned char salt_bin[crypto_pwhash_SALTBYTES];
  if (sodium_hex2bin(salt_bin, sizeof(salt_bin), salt.c_str(), salt.length(),
                     nullptr, nullptr, nullptr) != 0) {
    throw std::runtime_error("Failed to convert hex salt to binary");
  }

  unsigned char hashed_password[HASH_BYTES];

  if (crypto_pwhash(hashed_password, sizeof(hashed_password), password.c_str(),
                    password.length(), salt_bin, OPSLIMIT, MEMLIMIT,
                    crypto_pwhash_ALG_ARGON2ID13) != 0) {
    throw std::runtime_error("Password hashing failed.");
  }

  std::string hashed_password_str(
      reinterpret_cast<const char *>(hashed_password));
  sodium_memzero(hashed_password, sizeof(hashed_password));
  sodium_memzero(salt_bin, sizeof(salt_bin));
  return hashed_password_str;
}

std::string Crypto::generate_salt() {
  unsigned char salt[crypto_pwhash_SALTBYTES];
  randombytes_buf(salt, sizeof salt);
  std::string salt_str(crypto_pwhash_SALTBYTES * 2, '\0');
  sodium_bin2hex(&salt_str[0], salt_str.length() + 1, salt, sizeof(salt));
  return salt_str;
}

bool Crypto::verify_password(const std::string &password,
                            const std::string &stored_hash) {
  if (password.empty() || stored_hash.empty()) {
    throw std::runtime_error("Invalid parameters for password verification");
  }

  return (crypto_pwhash_str_verify(stored_hash.c_str(), password.c_str(),
                                   password.length()) == 0);
}

std::string Crypto::generate_auth_token(size_t bytes) {
  unsigned char random_bytes[bytes];
  randombytes_buf(random_bytes, sizeof(random_bytes));

  std::string hex_str(bytes * 2, '\0');
  if (sodium_bin2hex(&hex_str[0], hex_str.length() + 1, random_bytes,
                     sizeof(random_bytes)) == nullptr) {
    throw std::runtime_error("Failed to convert random bytes to hex.");
  }

  sodium_memzero(random_bytes, sizeof(random_bytes));
  return hex_str;
}