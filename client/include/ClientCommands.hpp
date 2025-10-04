#ifndef CLIENTCOMMANDS_H
#define CLIENTCOMMANDS_H

#include <string>
#include <variant>
#include <optional>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <typeinfo>

                                                // usage....
struct LoginCommand  {                           // /login username password
    std::string username;
    std::string password;
};
struct LogoutCommand {};                         // /logout
struct CreateUserCommand  {                      // /newuser username password
    std::string username;
    std::string password;
};
struct JoinBarrackCommand {                      // /join barrack_name
    std::string barrack_name;
    std::optional<std::string> password;
};
struct LeaveBarrackCommand  {                    // /leave
};
struct SendMessageCommand  {                     // simply type message and send
    std::string message;
};
struct CreateBarrackCommand  {                   // /create barrack_name is_private[1/0] password
    std::string barrack_name;
    bool is_private;
    std::optional<std::string> password;
};

struct DestroyBarrackCommand  {                  // /destroy barrack_name
};                  
struct GetBarrackCommand  {};                    // /barracks
struct InvalidCommand  {
    std::string invalid_command;
    std::string message;
};


using ClientCommand = std::variant<
    LoginCommand,
    LogoutCommand,
    JoinBarrackCommand,
    LeaveBarrackCommand,
    SendMessageCommand,
    GetBarrackCommand,
    CreateBarrackCommand,
    DestroyBarrackCommand,
    CreateUserCommand,
    InvalidCommand
>;


ClientCommand parse_input(const std::string& input);

#endif