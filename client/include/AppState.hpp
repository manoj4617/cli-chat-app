#ifndef APPSTATE_H
#define APPSTATE_H

#include <string>
#include <vector>
#include <deque>
#include <chrono>

#include "types.hpp"

struct AppState{    
    using c_time = std::chrono::steady_clock;
    enum class ConnectionStatus {
        DISCONNECTED,
        CONNECTED,
        CONNECTING,
        FAILED
    };

    ConnectionStatus conn_status_ = ConnectionStatus::DISCONNECTED;

    c_time::time_point last_activity_ = c_time::now();
    bool is_logged_in_ = false;
    std::string user_id_;
    std::string username_;
    std::string current_barrack_name_;
    std::string current_barrack_id_;
    std::vector<std::string> barrack_members_;
    std::deque<ChatMessage> chat_history_;
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
    void add_chat_message(ChatMessage message){
        if(chat_history_.size() >= MAX_HISTORY){
            chat_history_.pop_front();
        }
        chat_history_.push_back(std::move(message));
    }

    void clear_history(){
        chat_history_.clear();
    }
};

#endif