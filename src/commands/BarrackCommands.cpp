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

JoinBarrackCommand::JoinBarrackCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
    owner_uid_ = payload.value("user_id", "");
    password_ = payload.value("password", "");
}

void JoinBarrackCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->join_barrack(barrack_id_, owner_uid_, password_);

    if(std::holds_alternative<Error>(result)){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::JOIN_BARRACK_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::JOIN_BARRACK_FAILURE)},
                {"message", std::get<Error>(result).message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::JOIN_BARRACK_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack joined successfully"},
            }}
        };
        session->send_message(response.dump());
    }
}

LeaveBarrackCommand::LeaveBarrackCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
    user_uid_ = payload.value("user_id", "");
}

void LeaveBarrackCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->leave_barrack(barrack_id_, user_uid_);

    if(std::holds_alternative<Error>(result)){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::LEAVE_BARRACK_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::LEAVE_BARRACK_FAILURE)},
                {"message", std::get<Error>(result).message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::LEAVE_BARRACK_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack left successfully"},
            }}
        };
        session->send_message(response.dump());
    }
}

MessageBarrackCommand::MessageBarrackCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
    user_uid_ = payload.value("user_id", "");
    message_ = payload.value("message", "");
}

void MessageBarrackCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->message_barrack(barrack_id_, user_uid_, message_);

    if(std::holds_alternative<Error>(result)){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::MESSAGE_BARRACK_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::MESSAGE_BARRACK_FAILURE)},
                {"message", std::get<Error>(result).message}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::MESSAGE_BARRACK_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Message sent successfully"},
            }}
        };
        session->send_message(response.dump());
    }
}

GetBarrackMemberCommand::GetBarrackMemberCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
    user_uid_ = payload.value("user_id", "");
}

void GetBarrackMemberCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->get_barrack_member(barrack_id_, user_uid_);

    if(result == std::nullopt){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_MEMBER_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::GET_BARRACK_MEMBER_FAILURE)},
                {"message", "Member not found"}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_MEMBER_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack member fetched successfully"},
            }}
        };
        session->send_message(response.dump());
    }
}

GetBarrackMembersCommand::GetBarrackMembersCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
}

void GetBarrackMembersCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->get_barrack_members(barrack_id_);

    if(result == std::nullopt){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_MEMBER_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::GET_BARRACK_MEMBER_FAILURE)},
                {"message", "Members not found"}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        
        std::vector<nlohmann::json> members_json;
        for(const auto& member : *result){
            members_json.push_back(
              {
                {"barrack_id", member.barrack_id},
                {"user_id", member.user_id},
                {"joined_at", std::chrono::system_clock::to_time_t(member.joined_at)} 
              }  
            );
        }
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_MEMBER_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack members fetched successfully"},
                {"members", members_json}
            }}
        };
        session->send_message(response.dump());
    }
}


GetBarrackMessagesCommand::GetBarrackMessagesCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
}

void GetBarrackMessagesCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->get_barrack_messages(barrack_id_);

    if(result == std::nullopt){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_MESSAGES_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::GET_BARRACK_MESSAGES_FAILURE)},
                {"message", "Messages not found"}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        std::vector<nlohmann::json> messages_json;
        for(const auto& message : *result){
            messages_json.push_back(
              {
                {"barrack_id", message.barrack_id},
                {"user_id", message.sender_user_id},
                {"message", message.content},
                {"created_at", std::chrono::system_clock::to_time_t(message.sent_at)} 
              }  
            );
        }
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_MESSAGES_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack messages fetched successfully"},
                {"messages", messages_json}
            }}
        };
        session->send_message(response.dump());
    }
}

GetBarrackCommand::GetBarrackCommand(const nlohmann::json& payload){
    barrack_id_ = payload.value("barrack_id", "");
}

void GetBarrackCommand::execute(std::shared_ptr<ClientSession> session, const CommandContext &context){
    auto result = context.barrack_manager->get_barrack(barrack_id_);

    if(result == std::nullopt){
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_FAILURE)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"error_code", message_type_to_string(MessageType::GET_BARRACK_FAILURE)},
                {"message", "Barrack not found"}
            }}
        };
        session->send_message(response.dump());
    }
    else {
        nlohmann::json response = {
            {"type", message_type_to_string(MessageType::GET_BARRACK_SUCCESS)},
            {"sequence_id", session->get_next_sequence_id()},
            {"payload", {
                {"message", "Barrack fetched successfully"},
                {"barrack_id", result->barrack_id},
                {"name", result->barrack_name},
                {"admin_id", result->admin_id},
                {"created_at", std::chrono::system_clock::to_time_t(result->created_at)},
                {"is_private", result->is_private},
            }}
        };        
        session->send_message(response.dump());
    }
}
