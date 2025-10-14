#ifndef CLIENTCONTROLLER_HPP
#define CLIENTCONTROLLER_HPP

#include <functional>

#include "CommandValidator.hpp"
#include "ClientCommands.hpp"
#include "AppState.hpp"
#include "ConcurrentQueue.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class ClientController {

    public:
        ClientController(std::shared_ptr<AppState> state,
                        ConcurrentQueue<ClientCommand>& outbound_commands,
                        ConcurrentQueue<std::string>& inbound_messages);
        void on_user_input();
        void process_network_messages();
    private:
        void process_inbound_message(const std::string);

        using MessageHandler = std::function<void(const json&)>;
        std::unordered_map<std::string, MessageHandler> message_handlers_;
        void init_message_handlers();

        void process_user_input(const std::string& input);
        void handle_login_response(const json& payload);
        void handle_logout_response(const json& payload);
        void handle_create_user_response(const json& payload);
        void handle_join_barrack_response(const json& payload);
        void handle_create_barrack_response(const json& payload);
        void handle_destroy_barrack_response(const json& payload);
        void handle_get_barracks_response(const json& payload);
        void handle_message_barrack_response(const json& payload);

        void handle_get_barrack_members_response(const json& payload);
        void handle_get_barrack_member_response(const json& payload);

        std::shared_ptr<AppState> app_state_;
        ConcurrentQueue<std::string>& outbound_queue_;
        ConcurrentQueue<std::string>& inbound_queue_; 
        CommandValidator validator_;
};

#endif