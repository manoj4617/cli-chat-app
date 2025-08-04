#include "ICommand.hpp"
#include "AuthCommands.hpp"
#include "BarrackCommands.hpp"


class CommandFactory {
    public:
        CommandFactory();

        std::unique_ptr<ICommand> create_command(const std::string& type, const nlohmann::json& payload);

    private:
        void register_command(const std::string& type,
                                std::function<std::unique_ptr<ICommand>(const nlohmann::json&)> factory);

        std::unordered_map<std::string, std::function<std::unique_ptr<ICommand>(const nlohmann::json&)>> command_map;
};