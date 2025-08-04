#include "AuthCommands.hpp"
#include "Messages.hpp"
#include "ClientSession.hpp"

CreateUserCommand::CreateUserCommand  (const nlohmann::json& payload){
    username_ = payload.value("username", "");
    password_ = payload.value("password", "");
}

void CreateUserCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context) {
    auto result = context.auth_manager->create_user(username_, password_);

    if(std::holds_alternative<Error>(result)){
        AuthFailure auth_failure;
        auth_failure.error_code_str =  message_type_to_string(MessageType::AUTH_FAILURE);
        auth_failure.message = std::get<Error>(result).message;
        auth_failure.sequence_id_ = session->get_next_sequence_id();
        auth_failure.type_ = MessageType::AUTH_FAILURE;
        nlohmann::json response = {
            {"type", message_type_to_string(auth_failure.type_)},
            {"sequence_id", auth_failure.sequence_id_},
            {"payload", {
                {"error_code", auth_failure.error_code_str},
                {"message", auth_failure.message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        AuthSuccess auth_success;
        auth_success.sequence_id_ = session->get_next_sequence_id();
        auth_success.token = std::get<std::pair<std::string, std::string>>(result).second;
        auth_success.user_id = std::get<std::pair<std::string, std::string>>(result).first;
        auth_success.user_created = true;
        auth_success.type_  = MessageType::AUTH_SUCCESS;
        
        nlohmann::json response {
            {"type", message_type_to_string(auth_success.type_)},
            {"sequence_id", auth_success.sequence_id_},
            {"payload", {
                {"token", auth_success.token},
                {"user_id", auth_success.user_id},
                {"user_created", auth_success.user_created}
            }}
        };

        session->send_message(response.dump());
        session->set_authenticated_user(auth_success.user_id);
    }
}

LoginCommand::LoginCommand(const nlohmann::json& payload){
    username_ = payload.value("username","");
    password_ = payload.value("password","");
}

void LoginCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext& context){
    auto result = context.auth_manager->authenticate_user(username_, password_);

    if(std::holds_alternative<Error>(result)){
        AuthFailure auth_failure;
        auth_failure.error_code_str =  message_type_to_string(MessageType::AUTH_FAILURE);
        auth_failure.message = std::get<Error>(result).message;
        auth_failure.sequence_id_ = session->get_next_sequence_id();
        auth_failure.type_ = MessageType::AUTH_FAILURE;
        nlohmann::json response = {
            {"type", message_type_to_string(auth_failure.type_)},
            {"sequence_id", auth_failure.sequence_id_},
            {"payload", {
                {"error_code", auth_failure.error_code_str},
                {"message", auth_failure.message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        AuthSuccess auth_success;
        auth_success.sequence_id_ = session->get_next_sequence_id();
        auth_success.token = std::get<std::pair<std::string, std::string>>(result).second;
        auth_success.user_id = std::get<std::pair<std::string, std::string>>(result).first;
        auth_success.user_created = false;
        auth_success.type_  = MessageType::AUTH_SUCCESS;
        
        nlohmann::json response {
            {"type", message_type_to_string(auth_success.type_)},
            {"sequence_id", auth_success.sequence_id_},
            {"payload", {
                {"token", auth_success.token},
                {"user_id", auth_success.user_id},
                {"user_created", auth_success.user_created}
            }}
        };

        session->send_message(response.dump());
        session->set_authenticated_user(auth_success.user_id);
    }
}

GetUsernameCommand::GetUsernameCommand(const nlohmann::json& payload){
    user_id_ = payload.value("user_id", "");
}

void GetUsernameCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.auth_manager->get_username(user_id_);

    if(std::holds_alternative<Error>(result)){
        nlohmann::json response {
            {"sequence_id", session->get_next_sequence_id()},
            {"type", message_type_to_string(MessageType::GET_USER_NAME)},
            {"message" , std::get<Error>(result).message}
        };
        session->send_message(response.dump());
    } else{
        nlohmann::json response {
            {"sequence_id", session->get_next_sequence_id()},
            {"type", message_type_to_string(MessageType::GET_USER_NAME)},
            {"username" , std::get<std::string>(result)}
        };
        session->send_message(response.dump());
    }
}

