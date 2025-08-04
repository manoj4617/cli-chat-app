#ifndef BARRACKCOMMANDS_H
#define BARRACKCOMMANDS_H

#include "ICommand.hpp"
#include <json.hpp>

class CreateBarrackCommand : public ICommand {
    public:
        explicit CreateBarrackCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_name_;
        std::string owner_uid_;
        bool is_private_;
        std::optional<std::string> password_;
};

class DestroyBarracKCommand : public ICommand {
    public:
        explicit DestroyBarracKCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
        std::string owner_uid_;
};

class JoinBarrackCommand : public ICommand {
    public:
        explicit JoinBarrackCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
        std::string owner_uid_;
        std::optional<std::string> password_;
};

class LeaveBarrackCommand : public ICommand {
    public:
        explicit LeaveBarrackCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
        std::string user_uid_;
};

class MessageBarrackCommand : public ICommand {
    public:
        explicit MessageBarrackCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
        std::string user_uid_;
        std::string message_;
};

class GetBarrackMemberCommand : public ICommand {
    public:
        explicit GetBarrackMemberCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
        std::string user_uid_;
};

class GetBarrackMembersCommand : public ICommand {
    public:
        explicit GetBarrackMembersCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
};


class GetBarrackMessagesCommand : public ICommand {
    public:
        explicit GetBarrackMessagesCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
};

class GetBarrackCommand : public ICommand {
    public:
        explicit GetBarrackCommand(const nlohmann::json& payload);
        void execute(std::shared_ptr<ClientSession> session, const CommandContext& contex) override;

    private:
        std::string barrack_id_;
};

#endif
