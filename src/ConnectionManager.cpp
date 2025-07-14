#include "ConnectionManager.hpp"

void ConnectionManager::start_new_session(tcp::socket&& socket){
    ClientSession::SessionID current_id;
    std::shared_ptr<ClientSession> new_session;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        current_id = next_session_id_++;
        new_session = std::make_shared<ClientSession>(std::move(socket), current_id,
                                                        shared_from_this(), message_manager_);
        sessions_[current_id] = new_session;
    }

    std::cout << "ConnectionManager: Registered new session ID " << current_id
              << " from " << new_session->get_client_ip_addr() << ":" << new_session->get_client_port() << "\n";
    new_session->run();
}

void ConnectionManager::unregister_session(ClientSession::SessionID id){
    bool erased = false;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if(sessions_.erase(id) > 0){
            erased = true;
        }
    }
    if(erased){
        std::cout << "ConnectionManager: Unregister session ID " << id << "\n";  
    }
}

size_t ConnectionManager::get_active_session_count(){
    std::lock_guard<std::mutex> lock(mtx_);
    return sessions_.size();
}

std::vector<ConnectionManager::SessionInfo> ConnectionManager::get_all_sessions_info(){
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<SessionInfo> infos;
    auto now = c_time::now();
    for(const auto& pair : sessions_){
        const auto& session = pair.second;
        infos.push_back(
            {
                session->get_id(),
                session->get_client_ip_addr(),
                session->get_client_port(),
                session->get_status(),
                session->get_authentiated_user_id(),
                std::chrono::duration_cast<std::chrono::seconds>(now - session->get_connection_time()).count(),
                std::chrono::duration_cast<std::chrono::seconds>(now - session->get_last_activity_time()).count()
            });
    }
    return infos;
}

std::shared_ptr<ClientSession> ConnectionManager::get_sessions(ClientSession::SessionID id){
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = sessions_.find(id);
    if(it != sessions_.end()){
        return it->second;
    }
    return nullptr;
}