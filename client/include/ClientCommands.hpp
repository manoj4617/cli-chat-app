#ifndef CLIENTCOMMANDS_H
#define CLIENTCOMMANDS_H

#include <string>
#include <variant>
#include <optional>
                                                // usage....
struct LoginCommand {                           // /login username password
    std::string username;
    std::string password;
};
struct LogoutCommand{};                         // /logout
struct JoinBarrackCommand{                      // /join barrack_name
    std::string barrack_id;
    std::string user_id;
    std::optional<std::string> password;
};
struct LeaveBarrackCommand {                    // /leave
    std::string barrack_id;
    std::string user_id;
};
struct SendMessageCommand {                     // simply type message and send
    std::string barrack_id;
    std::string user_id;
    std::string message;
};
struct GetBarrackCommand {};                    // /barracks
struct InvalidCommand {};

using ClientCommand = std::variant<
    LoginCommand,
    LogoutCommand,
    JoinBarrackCommand,
    LeaveBarrackCommand,
    SendMessageCommand,
    GetBarrackCommand,
    InvalidCommand
>;

ClientCommand parse_input(const std::string& input);

#endif