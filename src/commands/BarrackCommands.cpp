#include "BarrackCommands.hpp"
#include "Messages.hpp"
#include "ClientSession.hpp"

CreateBarrackCommand::CreateBarrackCommand(const nlohmann::json& payload){
    barrack_name_ = payload.value("barrack_name", "");
    owner_uid_ = payload.value("owner_id", "");
    std::string private_ = payload.value("is_private", "");
    is_private_ = private_ == "true" ? true : false;
    password_ = payload.value("password", "");
}

void CreateBarrackCommand::execute(std::shared_ptr<ClientSession> session , const CommandContext& context){
    auto result = context.barrack_manager->create_barrack(barrack_name_, owner_uid_, is_private_, password_);

    if(std::holds_alternative<Error>(result)){
        CreateBarrackFailure barrack_fail;
        barrack_fail.error_code_str = message_type_to_string(MessageType::CREATE_BARRACK_FAILURE);
        barrack_fail.message = std::get<Error>(result).message;
        barrack_fail.type_ = MessageType::CREATE_BARRACK_FAILURE;
        barrack_fail.sequence_id_ = session->get_next_sequence_id();

        nlohmann::json response = {
            {"type", message_type_to_string(barrack_fail.type_)},
            {"sequence_id", barrack_fail.sequence_id_},
            {"payload", {
                {"error_code", barrack_fail.error_code_str},
                {"message", barrack_fail.message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        CreateBarrackSuccess barrack_success;
        barrack_success.error_code_str = message_type_to_string(MessageType::CREATE_BARRACK_SUCCESS);
        barrack_success.message = std::string("Barrack Created Successfully");
        barrack_success.type_ = MessageType::CREATE_BARRACK_SUCCESS;
        barrack_success.sequence_id_ = session->get_next_sequence_id();
        barrack_success.barrack_id = std::get<std::string>(result);
        nlohmann::json response = {
            {"type", message_type_to_string(barrack_success.type_)},
            {"sequence_id", barrack_success.sequence_id_},
            {"payload", {
                {"error_code", barrack_success.error_code_str},
                {"message", barrack_success.message},
                {"barrack_id", barrack_success.barrack_id},
                {"owner_id", owner_uid_}
            }}
        };
        session->send_message(response.dump());
    }
}

DestroyBarrackCommand::DestroyBarrackCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
    owner_uid_ = payload.value("owner_id", "");
}

void DestroyBarrackCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->destroy_barrack(barrack_id_, owner_uid_);

    if(std::holds_alternative<Error>(result)){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::DESTROY_BARRACK_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::DESTROY_BARRACK_SUCCESS)},
                {"message", std::get<Error>(result).message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::DESTROY_BARRACK_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack destroyed successfully"},
            }}
        };
        session->send_message(response.dump());
    }
}

