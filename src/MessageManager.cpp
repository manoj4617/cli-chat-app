#include "MessageManager.hpp"
#include "Messages.hpp"
#include "ClientSession.hpp"

void MessageManager::handle_json_message(std::shared_ptr<ClientSession> session, MessageType type, const nlohmann::json& full_json_doc){
    if(!full_json_doc.contains("payload") || !full_json_doc["payload"].is_object()){
        if(type != MessageType::LIST_BARRACK_REQUEST  && type != MessageType::PONG){
            session->send_message("{\"type\":\"ERROR_MESSAGE\", \"payload\":{\"error_code\":\"INVALID_PAYLOAD\", \"message\":\"'payload' is missing or not an object.\"}}");
        }
    }

    switch(type){

    }
}