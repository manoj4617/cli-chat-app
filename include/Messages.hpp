#ifndef MESSAGES_H
#define MESSAGES_H

#include <json.hpp>
#include <iostream>
#include <chrono>
#include <vector>

enum MessageType{
    UNKNOWN = -1,
    //client to server
    AUTH_REQUEST,
    SEND_MESSAGE_REQUEST,
    CREATE_BARRACK_REQUEST,
    DESTROY_BARRACK_REQUEST,
    JOIN_BARRACK_REQUEST,
    LEAVE_BARRACK_REQUEST,
    LIST_BARRACK_REQUEST,
    PONG, // C->S KEEP ALIVE

    //SERVER TO CLIENT
    AUTH_SUCCESS,
    AUTH_FAILURE,
    RECEIVE_MESSAGE_BROADCAST,
    CREATE_BARRACK_SUCCESS,
    CREATE_BARRACK_FAILURE,
    DESTROY_BARRACK_SUCCESS,
    DESTROY_BARRACK_FAILURE,
    JOIN_BARRACK_SUCCESS,
    JOIN_BARRACK_FAILURE,
    LEAVE_BARRACK_SUCCESS,
    LEAVE_BARRACK_FAILURE,
    MESSAGE_BARRACK_SUCCESS,
    MESSAGE_BARRACK_FAILURE,
    GET_BARRACK_MEMBER_SUCCESS,
    GET_BARRACK_MEMBER_FAILURE,
    LIST_BARRACK_RESPONSE,
    USER_JOINED_BARRACK_NOTIFY,
    GET_USER_NAME,
    USER_LEFT_BARRACK_NOTIFY,
    ERROR_MESSAGE,
    PING // S->C KEEP ALIVE REQUEST
};

inline std::string message_type_to_string(MessageType type){
    switch(type){
        case MessageType::AUTH_REQUEST: return std::string("AUTH_REQUEST"); 
        case MessageType::SEND_MESSAGE_REQUEST: return std::string("SEND_MESSAGE_REQUEST"); 
        case MessageType::CREATE_BARRACK_REQUEST: return std::string("CREATE_BARRACK_REQUEST");
        case MessageType::JOIN_BARRACK_REQUEST: return std::string("JOIN_BARRACK_REQUEST");
        case MessageType::LEAVE_BARRACK_REQUEST: return std::string("LEAVE_BARRACK_REQUEST");
        case MessageType::LIST_BARRACK_REQUEST: return std::string("LIST_BARRACK_REQUEST");
        case MessageType::PONG: return std::string("PONG");  
        case MessageType::AUTH_SUCCESS: return std::string("AUTH_SUCCESS"); 
        case MessageType::AUTH_FAILURE: return std::string("AUTH_FAILURE");
        case MessageType::RECEIVE_MESSAGE_BROADCAST: return std::string("RECEIVE_MESSAGE_BROADCAST");
        case MessageType::CREATE_BARRACK_SUCCESS: return std::string("CREATE_BARRACK_SUCCESS");
        case MessageType::CREATE_BARRACK_FAILURE: return std::string("CREATE_BARRACK_FAILURE"); 
        case MessageType::DESTROY_BARRACK_SUCCESS: return std::string("DESTROY_BARRACK_SUCCESS");
        case MessageType::DESTROY_BARRACK_FAILURE: return std::string("DESTROY_BARRACK_FAILURE");
        case MessageType::JOIN_BARRACK_SUCCESS: return std::string("JOIN_BARRACK_SUCCESS");
        case MessageType::JOIN_BARRACK_FAILURE: return std::string("JOIN_BARRACK_FAILURE");
        case MessageType::LEAVE_BARRACK_SUCCESS: return std::string("LEAVE_BARRACK_SUCCESS"); 
        case MessageType::LEAVE_BARRACK_FAILURE: return std::string("LEAVE_BARRACK_FAILURE");
        case MessageType::LIST_BARRACK_RESPONSE: return std::string("LIST_BARRACK_RESPONSE");
        case MessageType::USER_JOINED_BARRACK_NOTIFY: return std::string("USER_JOINED_BARRACK_NOTIFY");
        case MessageType::USER_LEFT_BARRACK_NOTIFY: return std::string("USER_LEFT_BARRACK_NOTIFY");
        case MessageType::ERROR_MESSAGE: return std::string("ERROR_MESSAGE"); 
        case MessageType::PING : return std::string("PING");  
        default: return std::string("UNKNOWN");
    }
}

inline MessageType string_to_message_type(const std::string& type_str) {
    if (type_str == "AUTH_REQUEST") return MessageType::AUTH_REQUEST;
    else if (type_str == "SEND_MESSAGE_REQUEST") return MessageType::SEND_MESSAGE_REQUEST;
    else if (type_str == "CREATE_BARRACK_REQUEST") return MessageType::CREATE_BARRACK_REQUEST;
    else if (type_str == "JOIN_BARRACK_REQUEST") return MessageType::JOIN_BARRACK_REQUEST;
    else if (type_str == "LEAVE_BARRACK_REQUEST") return MessageType::LEAVE_BARRACK_REQUEST;
    else if (type_str == "LIST_BARRACK_REQUEST") return MessageType::LIST_BARRACK_REQUEST;
    else if (type_str == "PONG") return MessageType::PONG;
    else if (type_str == "AUTH_SUCCESS") return MessageType::AUTH_SUCCESS;
    else if (type_str == "AUTH_FAILURE") return MessageType::AUTH_FAILURE;
    else if (type_str == "RECEIVE_MESSAGE_BROADCAST") return MessageType::RECEIVE_MESSAGE_BROADCAST;
    else if (type_str == "CREATE_BARRACK_SUCCESS") return MessageType::CREATE_BARRACK_SUCCESS;
    else if (type_str == "CREATE_BARRACK_FAILURE") return MessageType::CREATE_BARRACK_FAILURE;
    else if (type_str == "DESTROY_BARRACK_SUCCESS") return MessageType::DESTROY_BARRACK_SUCCESS;
    else if (type_str == "DESTROY_BARRACK_FAILURE") return MessageType::DESTROY_BARRACK_FAILURE;
    else if (type_str == "JOIN_BARRACK_SUCCESS") return MessageType::JOIN_BARRACK_SUCCESS;
    else if (type_str == "JOIN_BARRACK_FAILURE") return MessageType::JOIN_BARRACK_FAILURE;
    else if (type_str == "LEAVE_BARRACK_SUCCESS") return MessageType::LEAVE_BARRACK_SUCCESS;
    else if (type_str == "LEAVE_BARRACK_FAILURE") return MessageType::LEAVE_BARRACK_FAILURE;
    else if (type_str == "LIST_BARRACK_RESPONSE") return MessageType::LIST_BARRACK_RESPONSE;
    else if (type_str == "USER_JOINED_BARRACK_NOTIFY") return MessageType::USER_JOINED_BARRACK_NOTIFY;
    else if (type_str == "USER_LEFT_BARRACK_NOTIFY") return MessageType::USER_LEFT_BARRACK_NOTIFY;
    else if (type_str == "ERROR_MESSAGE") return MessageType::ERROR_MESSAGE;
    else if (type_str == "PING") return MessageType::PING;
    else return MessageType::UNKNOWN;
}

class BaseMessage {
    public:
        MessageType type_;
        std::optional<uint64_t> sequence_id_;
        std::chrono::steady_clock::time_point timestamp_ = 
            std::chrono::steady_clock::now();
        virtual ~BaseMessage() = default;
    
    protected:
        BaseMessage(MessageType type) : type_(type){}
        BaseMessage() = default;
};

//---Specific message classes ---

/* client -> server*/
struct AuthRequest : public BaseMessage {
    std::string username;
    std::string password;
    bool create_user;
    AuthRequest() : BaseMessage(MessageType::AUTH_REQUEST){}
};

struct  SendMessageRequest : public BaseMessage {
    std::string barrack_id;
    std::string message_;
    SendMessageRequest() : BaseMessage(MessageType::SEND_MESSAGE_REQUEST){}
};

struct CreateBarrackRequest : public BaseMessage {
    std::string barrack_name;
    bool is_private;
    std::optional<std::string> password; //password if private

    CreateBarrackRequest() : BaseMessage(MessageType::CREATE_BARRACK_REQUEST){}
};

struct LeaveBarrackRequest : public BaseMessage {
    std::string barrack_id;

    LeaveBarrackRequest() : BaseMessage(MessageType::LEAVE_BARRACK_REQUEST){}
};


struct JoinBarrackRequest : public BaseMessage {
    std::string barrack_id;
    std::optional<std::string> password;

    JoinBarrackRequest() : BaseMessage(MessageType::JOIN_BARRACK_REQUEST){}
};

struct PongMessage : public BaseMessage {
    PongMessage() : BaseMessage(MessageType::PONG){}
};
/* client -> server*/

/* server -> client*/

struct AuthSuccess : public BaseMessage {
    std::string token;
    std::string user_id;
    bool user_created;
    AuthSuccess() : BaseMessage(MessageType::AUTH_SUCCESS){}
};

struct AuthFailure : public BaseMessage {
    std::string error_code_str;
    std::string message;
    
    AuthFailure() : BaseMessage(MessageType::AUTH_FAILURE){}
};

struct BarrackMemberInfo {
    std::string user_id;
    std::string username;
};

struct ReceiveMessageBroadcast : public BaseMessage {
    std::string barrack_id;
    std::string sender_id;
    std::string sender_username;
    std::string content;
    std::string message_id;

    ReceiveMessageBroadcast() : BaseMessage(MessageType::RECEIVE_MESSAGE_BROADCAST){}
};

struct CreateBarrackSuccess : public BaseMessage {
    std::string error_code_str;
    std::string message;
    std::string barrack_id;
    CreateBarrackSuccess() : BaseMessage(MessageType::CREATE_BARRACK_SUCCESS){}
};

struct CreateBarrackFailure : public BaseMessage {
    std::string error_code_str;
    std::string message;

    CreateBarrackFailure() : BaseMessage(MessageType::CREATE_BARRACK_FAILURE){}
};

struct JoinBarrackSuccess : public BaseMessage {
    std::string barrack_id;
    std::string barrack_name;
    std::vector<BarrackMemberInfo> members;

    JoinBarrackSuccess() : BaseMessage(MessageType::JOIN_BARRACK_SUCCESS){}
};

struct JoinBarrackFailure : public BaseMessage {
    std::string barrack_id;

    JoinBarrackFailure() : BaseMessage(MessageType::JOIN_BARRACK_FAILURE){}
};


struct LeaveBarrackSuccess : public BaseMessage {
    std::string barrack_id;

    LeaveBarrackSuccess() : BaseMessage(MessageType::LEAVE_BARRACK_SUCCESS){}
};

struct BarrackInfo {
    std::string barrack_id;
    std::string barrack_name;
    int member_count;
};

struct ListBarrackResponse :public  BaseMessage {
    std::vector<BarrackInfo> barracks;
    ListBarrackResponse() : BaseMessage(MessageType::LIST_BARRACK_RESPONSE){}
};

struct UserJoinedBarrackNotify : public BaseMessage{
    std::string barrack_id;
    std::string user_id;
    std::string username;

    UserJoinedBarrackNotify() : BaseMessage(MessageType::USER_JOINED_BARRACK_NOTIFY){}
};

struct UserLeftBarrackNotify : public BaseMessage{
    std::string barrack_id;
    std::string user_id;
    std::string username;

    UserLeftBarrackNotify() : BaseMessage(MessageType::USER_LEFT_BARRACK_NOTIFY){}
};

struct ErrorMessage : public BaseMessage {
    std::string error_code_str;
    std::string message;
    ErrorMessage() : BaseMessage(MessageType::ERROR_MESSAGE){}
};

struct PingMessage : BaseMessage {
    PingMessage() : BaseMessage(MessageType::PING){}
};

#endif