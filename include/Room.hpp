#ifndef ROOM_H
#define ROOM_H

#include <set>
#include <iostream>
#include <memory>
#include <mutex>

// Forward declaration to avoid recursive include
class ClientSession;

class Room {
    public:
        explicit Room(const std::string& barrack_id) : barrack_id_(barrack_id){}
        void join(std::shared_ptr<ClientSession> session);
        void leave(std::shared_ptr<ClientSession> session);
        void broadcast(const std::string& message);
    private:
        std::string barrack_id_;
        std::mutex mtx_;
        std::set<std::weak_ptr<ClientSession>, std::owner_less<std::weak_ptr<ClientSession>>> members_;
};

#endif