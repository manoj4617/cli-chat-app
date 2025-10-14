#ifndef APPSTATE_H
#define APPSTATE_H

#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <unordered_map>
#include <mutex>

#include "types.hpp"

struct Users{
    std::string username;
    std::chrono::steady_clock::time_point member_since;
};
struct Barracks{
    std::vector<Users> members;
    std::string barrack_id;
    std::string owner_id;
    std::string barrack_name;
    bool is_private;
    std::chrono::steady_clock::time_point created_on;
};

struct ClientChatMessage {
    std::string barrack_id;     
    std::string sender_user_id; 
    std::string content;
    std::chrono::system_clock::time_point sent_at;

    ClientChatMessage(std::string bid, std::string sid, std::string msg_content,
                std::chrono::system_clock::time_point sent)
        :barrack_id(std::move(bid)),
          sender_user_id(std::move(sid)), content(std::move(msg_content)), sent_at(sent) {}

    ClientChatMessage() = default;
    virtual ~ClientChatMessage() = default;
};

struct AppState{    
    using c_time = std::chrono::steady_clock;
    enum class ConnectionStatus {
        DISCONNECTED,
        CONNECTED,
        CONNECTING,
        FAILED
    };

    ConnectionStatus conn_status_ = ConnectionStatus::DISCONNECTED;

    std::mutex app_state_mutex_;
    c_time::time_point last_activity_ = c_time::now();
    bool is_logged_in_ = false;
    std::string user_id_;
    std::string username_;
    std::string current_barrack_name_;
    std::string current_barrack_id_;
    std::vector<std::string> barrack_members_;
    std::unordered_map<std::string, Barracks> total_barracks_;
    std::deque<ClientChatMessage> chat_history_;
    static const size_t MAX_HISTORY = 200;

    void set_connected(){
        conn_status_ = ConnectionStatus::DISCONNECTED;
    }
    void set_disconnected(const std::string& reason = ""){
        conn_status_ = ConnectionStatus::DISCONNECTED;
    }
    void set_login_success(const std::string& uid, const std::string& uname){
        user_id_ = uid;
        username_ = uname;
    }
    void set_logout(){
        user_id_.clear();
        username_.clear();
        current_barrack_id_.clear();
        current_barrack_name_.clear();
        barrack_members_.clear();
        clear_history();
    }
    void add_chat_message(ClientChatMessage& message){
        if(chat_history_.size() >= MAX_HISTORY){
            chat_history_.pop_front();
        }
        chat_history_.push_back(std::move(message));
    }

    std::string get_userid(){
        return user_id_;
    }

    bool barrack_exists(const std::string& barrack_name) const{
        return total_barracks_.find(barrack_name) != total_barracks_.end();
    }
    bool get_barrack_id(const std::string& barrack_name, std::string& barrack_id) const {
        auto barrack_itr = total_barracks_.find(barrack_name);
        if(barrack_itr != total_barracks_.end()){
            barrack_id.assign(barrack_itr->second.barrack_id);
            return true;
        }
        return false;
    }
    bool is_barrack_private(const std::string& barrack_name) const {
        auto barrack_itr = total_barracks_.find(barrack_name);
        if(barrack_itr != total_barracks_.end()){
            return barrack_itr->second.is_private;
        }
        return false;
    }

    std::string get_barrack_owner(const std::string& barrack_name){
        auto barrack_itr = total_barracks_.find(barrack_name);
        if(barrack_itr != total_barracks_.end()){
            return barrack_itr->second.owner_id;
        }
        return "";
    }
    void clear_history(){
        chat_history_.clear();
    }
};

#endif