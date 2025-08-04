#ifndef CONNMANAGER_H
#define CONNMANAGER_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "ClientSession.hpp"

class ConnectionManager : public std::enable_shared_from_this<ConnectionManager> {
    private:
        std::mutex mtx_;
        std::unordered_map<ClientSession::SessionID, std::shared_ptr<ClientSession>> sessions_;
        std::shared_ptr<MessageDispatcher> message_dispatcher_;
        ClientSession::SessionID next_session_id_ = 1;
    public:
        ConnectionManager(std::shared_ptr<MessageDispatcher> message_dispatcher) :
            message_dispatcher_(message_dispatcher) {}

        void start_new_session(tcp::socket&&);
        void unregister_session(ClientSession::SessionID);
        size_t get_active_session_count();

        struct SessionInfo{
            ClientSession::SessionID id;
            std::string ip_address;
            unsigned short port;
            ConnStatus status;
            std::string authenticated_user_id;
            long long connection_duration_seconds;
            long long last_activity_seconds_ago;
        };

        std::vector<SessionInfo> get_all_sessions_info();
        std::shared_ptr<ClientSession> get_sessions(ClientSession::SessionID);
};

#endif