#include <CommandValidator.hpp>

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
