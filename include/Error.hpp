#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <source_location>
#include <variant>

#include "Messages.hpp"

enum ErrorCode {
    SUCCESSFUL,
    INVALID_CREDENTIALS,
    USER_NOT_FOUND,
    USER_ALREADY_EXISTS,
    INVALID_PASSWORD,
    DATABASE_ERROR,
    INVALID_TOKEN,
    INVALID_DATA,
    INVALID_OWNER_ID,
    MEMBER_NOT_FOUND, 
    // add as needed
};

struct Error{
    ErrorCode code;
    std::string message;
    std::string function_name;
#ifdef DEBUG
    std::string file_name;
    uint32_t line_number;
#endif
    Error() = default;
    Error(  ErrorCode c, 
            std::string msg,
            const std::source_location loc = std::source_location::current()) 
        :   code(c), 
            message(msg),
            function_name(loc.function_name())
#ifdef DEBUG
            , file_name(loc.file_name())
            , line(loc.line())        
#endif
        {}

        std::string what_happened() const {
#ifdef DEBUG
            return "Error[" + std::to_string(static_cast<int>(code)) + "] in " +
                    function_name + " at " + file_name + ":" +
                    std::to_string(line) + ": " + message;
#else
            return "Error[" + std::to_string(static_cast<int>(code)) + "] in " +
                    function_name + ": " + message;
#endif
        }
};

using Success = std::monostate;
const Success SUCCESS{};

template<typename T>
using Result = std::variant<T,Error>;
using StatusResult = std::variant<Success, Error>;

#endif