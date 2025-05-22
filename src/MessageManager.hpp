#include "Messages.hpp"

class AuthManager;
class BarrackManager;
class ClientSession;

class MessageManager{
    private:
        std::shared_ptr<AuthManager> auth_manager_;
        std::shared_ptr<BarrackManager> barrack_manager_;
    public:
        MessageManager(std::shared_ptr<AuthManager> auth_manager, std::shared_ptr<BarrackManager> barrack_manager)
            : auth_manager_(auth_manager), barrack_manager_(barrack_manager) {}
        void handle_json_message(std::shared_ptr<ClientSession>, MessageType, const nlohmann::json&);
};