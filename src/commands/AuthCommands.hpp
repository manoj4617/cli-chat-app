#ifndef AUTHCOMMANDS_H
#define AUTHCOMMANDS_H

#include "ICommand.hpp"
#include <json.hpp>

class LoginCommand : public ICommand {
    public:
        explicit LoginCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& context) override;
    
    private:
        std::string username_;
        std::string password_;
};

class CreateUserCommand : public ICommand {
    public:
        explicit CreateUserCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& context) override;
    private:
        std::string username_;
        std::string password_;
};

class GetUsernameCommand : public ICommand {
    public:
        explicit GetUsernameCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& context) override;
    private:
        std::string user_id_;
};

class LogoutCommand : public ICommand {
    public:
        explicit LogoutCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& context) override;
    private:
        std::string user_id_;
};

#endif