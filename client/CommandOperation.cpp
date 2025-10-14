#include <CommandValidator.hpp>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using opt_str = std::optional<std::string>;

ValidatorError CommandValidator::validate(const ClientCommand& command, AppState& state){
    return std::visit([this, &state](auto&& cmd) -> ValidatorError {
        return this->validate(cmd, state);
    }, command);
}

ValidatorError CommandValidator::validate(const LoginCommand &command, AppState &state){
    if(state.is_logged_in_){
        return "You are alrady logged in. Use /logout first.";
    }
    return std::nullopt;
}

ValidatorError CommandValidator::validate(const LogoutCommand& command, AppState& state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }
    state.set_logout();
    return std::nullopt;
}

ValidatorError CommandValidator::validate(const JoinBarrackCommand &command, AppState &state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }
    if(command.barrack_name == state.current_barrack_name_){
        return "You are already in a barrack";
    }
    if(!state.barrack_exists(command.barrack_name)){
        return "Barrack does not exists";
    }

    if(state.is_barrack_private(command.barrack_name) && command.password->empty()){
        return "This barrack is private, please provide barrack password";
    }

    return std::nullopt;
}

ValidatorError CommandValidator::validate(const LeaveBarrackCommand &command, AppState &state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }
    if(state.current_barrack_name_.empty()){
        return "You are not in a barrack";
    }
    return std::nullopt;
}

ValidatorError CommandValidator::validate(const SendMessageCommand &command, AppState &state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }
    if(state.current_barrack_name_.empty()){
        return "You must /join a barrack to send messages.";
    }

    ClientChatMessage chat_msg(
        state.current_barrack_id_,
        state.user_id_,
        command.message,
        std::chrono::system_clock::now()
    );

    state.add_chat_message(chat_msg);

    return std::nullopt;
}

ValidatorError CommandValidator::validate(const GetBarrackCommand &command, AppState &state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }
    return std::nullopt;
}

ValidatorError CommandValidator::validate(const CreateBarrackCommand &command, AppState &state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }
    if(!state.current_barrack_name_.empty()){
        return "You must /leave a barrack to create a new one.";
    }
    return std::nullopt;
}

ValidatorError CommandValidator::validate(const DestroyBarrackCommand &command, AppState &state){
    if(!state.is_logged_in_){
        return "You are not logged in.";
    }

    std::string owner_id = state.get_barrack_owner(command.barrack_name);
    if(owner_id != state.user_id_){
        return "You cannot delete this barrack.";
    }
    return std::nullopt;
}

ValidatorError CommandValidator::validate(const CreateUserCommand &command, AppState &state){
    if(state.is_logged_in_){
        return "You are logged in please /logout to create a new user.";
    }
    return std::nullopt;
}

struct Serializer {
    const AppState& state;

    opt_str operator()(const LoginCommand& cmd){
        json payload = {
            {"type", "LOGIN"},
            {"payload", {
                    {"username", cmd.username},
                    {"password", cmd.password}
            }}
        };

        return payload.dump();
    }

    opt_str operator()(const LogoutCommand& cmd){
        json payload = {
            {"type", "LOGOUT"},
            {"payload", {
                {"user_id", state.user_id_}
            }}
        };
        return payload.dump();
    }

    opt_str operator()(const CreateUserCommand& cmd){
        json payload = {
            {"type", "CREATEUSER"},
            {"payload", {
                    {"username", cmd.username},
                    {"password", cmd.password}
            }}
        };
        return payload.dump();
    }

    opt_str operator()(const JoinBarrackCommand& cmd){
        std::string barrack_id("");
        state.get_barrack_id(cmd.barrack_name, barrack_id);
        json payload = {
            {"type", "JOINBARRACK"},
            {"payload", {
                {"barrack_id", barrack_id},
                {"password", cmd.password.value_or("")},
                {"user_id", state.user_id_}
            }}
        };
        return payload.dump();
    }

    opt_str operator()(const CreateBarrackCommand& cmd){
        json payload = {
            {"type", "CREATEBARRACK"},
            {"payload",{
                {"barrack_name", cmd.barrack_name},
                {"owner_id", state.user_id_},
                {"is_private", cmd.is_private},
                {"password", cmd.password.value_or("")}
            }}
        };

        return payload.dump();
    }

    opt_str operator()(const DestroyBarrackCommand& cmd){
        std::string barrack_id("");
        state.get_barrack_id(cmd.barrack_name, barrack_id);
        json payload = {
            {"type", "DESTROYBARRACK"},
            {"payload", {
                {"barrack_id", barrack_id},
                {"owner_id", state.user_id_}
            }}
        };

        return payload.dump();
    }
    
    opt_str operator()(const LeaveBarrackCommand& cmd){
        std::string barrack_id("");
        state.get_barrack_id(state.current_barrack_name_, barrack_id);
        json payload = {
            {"type", "LEAVEBARRACK"},
            {"payload",{
                {"barrack_id", barrack_id},
                {"user_id", state.user_id_}
            }}
        };
    
        return payload.dump();
    }
    
    opt_str operator()(const SendMessageCommand& cmd){
        std::string barrack_id("");
        state.get_barrack_id(state.current_barrack_name_, barrack_id);
        json payload = {
            {"type", "MESSAGEBARRACK"},
            {"payload", {
                {"barrack_id", barrack_id},
                {"user_id", state.user_id_},
                {"message", cmd.message}
            }}
        };
        return payload.dump();
    }

    opt_str operator()(const GetBarrackCommand& cmd){
        json payload = {
            {"type", "GETBARRACKS"},
            {"payload", {}}
        };

        return payload.dump();
    }

    opt_str operator()(const auto&){
        return std::nullopt;
    }
};

opt_str command_serializer(const ClientCommand &command, AppState &state){
    return std::visit(Serializer{state}, command);
}