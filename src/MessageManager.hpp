#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H
#include <memory>

#include "Messages.hpp"
#include "AuthManager.hpp"
#include "BarrackManager.hpp"

class ClientSession;

class MessageManager{
    private:
        std::shared_ptr<AuthManager> auth_manager_;
        std::shared_ptr<BarrackManager> barrack_manager_;
        void SendAuthMessage(std::shared_ptr<ClientSession>, const nlohmann::json&);
        void SendBarrackMessage(std::shared_ptr<ClientSession>, const nlohmann::json&);
    public:
        MessageManager(std::shared_ptr<AuthManager> auth_manager, std::shared_ptr<BarrackManager> barrack_manager)
            : auth_manager_(auth_manager), barrack_manager_(barrack_manager) {}
        void handle_json_message(std::shared_ptr<ClientSession>, MessageType, const nlohmann::json&);
};

#endif // MESSAGEMANAGER_H