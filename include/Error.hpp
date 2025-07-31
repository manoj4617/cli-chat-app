#ifndef ERROR_H
#define ERROR_H

#include "Messages.hpp"
#include <string>

enum ErrorCode {
    SUCCESS,
    INVALID_CREDENTIALS,
    USER_NOT_FOUND,
    USER_ALREADY_EXISTS,
    INVALID_PASSWORD,
    DATABASE_ERROR,
    INVALID_TOKEN,
    INVALID_DATA,
    INVALID_OWNER_ID, 
    // add as needed
};

struct Error{
    ErrorCode code;
    std::string message;
    Error() = default;
    Error(ErrorCode c, std::string msg) : code(c), message(msg){}
};

#endif