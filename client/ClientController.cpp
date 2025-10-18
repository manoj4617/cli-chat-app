#include "ClientController.hpp"

ClientController::ClientController(std::shared_ptr<AppState> state,
                                    ConcurrentQueue<std::string>& outbound_queue,
                                    ConcurrentQueue<std::string>& inbound_message)
                                    :   app_state_(state),
                                        outbound_queue_(outbound_queue),
                                        inbound_queue_(inbound_message)
{    }


void ClientController::on_user_input(const std::string& input){
    ClientCommand command = parse_input(input);

    ValidatorError val_error;
    {
        std::lock_guard<std::mutex> lock(app_state_mutex_);
        val_error = validator_.validate(command, *app_state_);
    }
    if(val_error){
        std::lock_guard<std::mutex> lock(app_state_mutex_);
        app_state_->add_chat_message(ClientChatMessage::make_notification("Error: " + *val_error));
        return;
    }

    std::optional<std::string> json_to_send;
    {
        std::lock_guard<std::mutex> lock(app_state_mutex_);
        json_to_send = command_serializer(command, *app_state_);
    }
    if(json_to_send){
        outbound_queue_.push(std::move(*json_to_send));
    }
}

void ClientController::process_network_messages(){
    std::optional<std::string> message;
    while((message = inbound_queue_.try_pop())){
        process_inbound_message(*message);
    }
}

void ClientController::init_message_handlers(){

}

void ClientController::process_inbound_message(const std::string& message){
    try{

        json data = json::parse(message);
        const std::string &type = data.at("type").get<std::string>();
        
        auto it = message_handlers_.find(type);
        if(it != message_handlers_.end()){
            it->second(data.at("payload"));
        } else {
            std::lock_guard<std::mutex> lock(app_state_mutex_);
            app_state_->add_chat_message(ClientChatMessage::make_notification("Receoved Unknown from the server: " + type));
        }
    } catch(const json::exception& ex){
        std::cerr << "Failed to parse server message: " << ex.what() << " | Message: " << message << std::endl;
    }
}

