#include <json.hpp>
#include <string>
#include <unordered_map>
#include <functional>

#include "CommandFactory.hpp"

CommandFactory::CommandFactory(){
    register_command("LOGIN", [](const nlohmann::json& p) { return std::make_unique<LoginCommand>(p); });
    register_command("CREATEUSER", [](const nlohmann::json& p) { return std::make_unique<CreateUserCommand>(p); });
    register_command("GETUSER", [](const nlohmann::json& p){ return std::make_unique<GetUsernameCommand>(p); });
    register_command("JOINBARRACK", [](const nlohmann::json& p){ return std::make_unique<JoinBarrackCommand>(p); });
    register_command("CREATEBARRACK", [](const nlohmann::json& p){ return std::make_unique<CreateBarrackCommand>(p); });
    register_command("DESTROYBARRACK", [](const nlohmann::json& p){ return std::make_unique<DestroyBarrackCommand>(p); });
    register_command("LEAVEBARRACK", [](const nlohmann::json& p){ return std::make_unique<LeaveBarrackCommand>(p); });
    register_command("MESSAGEBARRACK", [](const nlohmann::json& p){ return std::make_unique<MessageBarrackCommand>(p); });
    register_command("GETBARRACKMEMBER", [](const nlohmann::json& p){ return std::make_unique<GetBarrackMemberCommand>(p); });
    register_command("GETBARRACKMEMBERS", [](const nlohmann::json& p){ return std::make_unique<GetBarrackMembersCommand>(p); });
    register_command("GETBARRACKMESSAGES", [](const nlohmann::json& p){ return std::make_unique<GetBarrackMessagesCommand>(p); });
    register_command("GETBARRACK", [](const nlohmann::json& p){ return std::make_unique<GetBarrackCommand>(p); });
    register_command("GETBARRACKS", [](const nlohmann::json& p){ return std::make_unique<GetBarracks>(p); });
}
std::unique_ptr<ICommand> CommandFactory::create_command(const std::string& type, const nlohmann::json& payload){
    auto it = command_map.find(type);
    if(it != command_map.end()){
        return it->second(payload);
    }
    return nullptr;
}
void CommandFactory::register_command(const std::string& type,
                                      std::function<std::unique_ptr<ICommand>(const nlohmann::json&)> factory){
    command_map[type] = factory;
}