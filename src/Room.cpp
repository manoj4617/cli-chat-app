#include "Room.hpp"
#include "ClientSession.hpp"

void Room::join(std::shared_ptr<ClientSession> session){
    std::scoped_lock<std::mutex> lock(mtx_);
    auto itr = members_.find(session);
    if(itr == members_.end()){
        members_.insert(session);
        std::cout << "Session ID: " << session->get_id() << " Joined barrack: " << barrack_id_ << " at: " << std::chrono::system_clock::now().time_since_epoch();
    }
    std::cout << "Session ID: " << session->get_id() << " Already member of barrack: " << barrack_id_;
}

void Room::leave(std::shared_ptr<ClientSession> session){
    std::scoped_lock<std::mutex> lock(mtx_);
    auto itr = members_.find(session);
    if(itr != members_.end()){
        std::cout << "Session ID: " << session->get_id() << " Left barrack: " << barrack_id_ << " at: " << std::chrono::system_clock::now().time_since_epoch();
        members_.erase(session);
    }
    std::cout << "Session ID: " << session->get_id() << " Not a member of: " << barrack_id_;
    if(members_.empty()){
        members_.clear();
    }
}

void Room::broadcast(const std::string& message){
    std::scoped_lock<std::mutex> lock(mtx_);
    for(auto& session : members_){
        if(auto sess = session.lock()){
            sess->send_message(message);
        }
    } 
}