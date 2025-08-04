#ifndef ICOMMAND_H
#define ICOMMAND_H

// #include <memory>
#include "AuthManager.hpp"
#include "BarrackManager.hpp"

class ClientSession;

struct CommandContext {
    std::shared_ptr<AuthManager> auth_manager;
    std::shared_ptr<BarrackManager> barrack_manager;
};

class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual void execute(std::shared_ptr<ClientSession> session, const CommandContext& context) = 0; 
};

#endif