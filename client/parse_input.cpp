#include "ClientCommands.hpp"
#include "AppState.hpp"

bool check_arg_count(const std::vector<std::string>& args, size_t expected_count) {
    // Command itself is args[0], so we check for expected_count + 1
    return args.size() == expected_count + 1;
}


ClientCommand parse_input(const std::string& input){
    if(input.empty()){
        return InvalidCommand{"", "Input cannot be empty"};
    }

    if (input[0] != '/') {
        return SendMessageCommand{input};
    }

    std::stringstream input_stream(input);
    std::vector<std::string> input_fields;
    std::string fields;
    while(input_stream >> fields){
        input_fields.push_back(fields);
    }

    if (input_fields.empty()) {
        return InvalidCommand{"", "Invalid command."};
    }

    const std::string& command_name = input_fields[0];
    
    if (command_name == "/login") {
        if (!check_arg_count(input_fields, 2)) 
            return InvalidCommand{"/login", "Usage: /login <username> <password>"};
        return LoginCommand{input_fields[1], input_fields[2]};
    }
    
    if (command_name == "/logout") {
        if (!check_arg_count(input_fields, 0)) 
            return InvalidCommand{"/logout", "Usage: /logout"};
        return LogoutCommand{};
    }

    if (command_name == "/join") {
        if (input_fields.size() < 2 || input_fields.size() > 3) 
            return InvalidCommand{"/join", "Usage: /join <barrack_name> [password]"};
        std::string password = (input_fields.size() == 3) ? input_fields[2] : "";
        return JoinBarrackCommand{input_fields[1], password};
    }
    
    if (command_name == "/leave") {
        if (!check_arg_count(input_fields, 0)) 
            return InvalidCommand{"/leave", "Usage: /leave"};
        return LeaveBarrackCommand{};
    }

    if(command_name == "/newuser"){
        if(!check_arg_count(input_fields, 2)) 
            return InvalidCommand{"/newuser", "Usage: /newuser <username> <password>"};

        return CreateUserCommand{input_fields[1], input_fields[2]};
    }

    if(command_name == "/create"){
        if(input_fields.size() < 2 || input_fields.size() > 4){
            return InvalidCommand{"/create", "Usage: /create barrack_name, is_private[0/1] [password]"};
        }
        std::string barrack_name = input_fields[1];
        if(barrack_name.empty()){
            return InvalidCommand{"/create", "Please provide barrack name"};
        }
        bool is_private = (input_fields.size() == 4) ? std::atoi(input_fields[2].c_str()) : 0;
        std::string password = (input_fields.size() == 4) ? input_fields[3] : "";
        return CreateBarrackCommand{barrack_name, is_private, password};
    }

    if(command_name == "/destroy"){
        if(!check_arg_count(input_fields, 0)){
            return InvalidCommand{"/destory", "Usage: /destroy"};
        }
        return DestroyBarrackCommand{};    
    }

    if(command_name == "/barracks"){
        if(!check_arg_count(input_fields, 0)){
            return InvalidCommand{"/barracks", "Usage: /barracks"};
        }
        return GetBarrackCommand{};    
    }
}