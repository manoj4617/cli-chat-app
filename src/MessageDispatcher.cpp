
#include <json.hpp>
#include <thread>
#include <vector>
#include "ClientSession.hpp"
#include "MessageDispatcher.hpp"

MessageDispatcher::MessageDispatcher(size_t num_threads, CommandContext context) : 
    commandContext(context), command_queue(std::make_unique<ConcurrentQueue<CommandTask>>()) {
    for(size_t i = 0; i < num_threads; i++){
        workers_.emplace_back(&MessageDispatcher::worker_loop, this);
    }
} 
MessageDispatcher::~MessageDispatcher(){
    if (!done_) {
        stop();
    }
    for(auto& worker : workers_){
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void MessageDispatcher::dispatch(std::shared_ptr<ClientSession> session,
                                 const std::string &raw_payload) {
   try {
        // Parse JSON and validate structure
        auto json_msg = nlohmann::json::parse(raw_payload);
        
        // Check if type field exists
        if (!json_msg.contains("type")) {
            nlohmann::json error_response = {
                {"type", "ERROR"},
                {"payload", {
                    {"error_code", "MISSING_TYPE"},
                    {"message", "Message must contain a 'type' field"}
                }}
            };
            session->send_message(error_response.dump());
            
            return;
        }

        std::string type = json_msg["type"].get<std::string>();
        
        // Check if payload field exists
        if (!json_msg.contains("payload")) {
            nlohmann::json error_response = {
                {"type", "ERROR"},
                {"payload", {
                    {"error_code", "MISSING_PAYLOAD"},
                    {"message", "Message must contain a 'payload' field"}
                }}
            };
            session->send_message(error_response.dump());
            return;
        }

        // Try to create command
        auto command = commandFactory.create_command(type, json_msg["payload"]);

        if (command) {
            command_queue->push({std::move(command), session});
        } else {
            nlohmann::json error_response = {
                {"type", "ERROR"},
                {"payload", {
                    {"error_code", "INVALID_COMMAND_TYPE"},
                    {"message", "Unknown command type: " + type}
                }}
            };
            session->send_message(error_response.dump());
        }

    } catch (const nlohmann::json::parse_error& e) {
        // JSON parsing error
        nlohmann::json error_response = {
            {"type", "ERROR"},
            {"payload", {
                {"error_code", "INVALID_JSON"},
                {"message", "Failed to parse JSON: " + std::string(e.what())}
            }}
        };
        session->send_message(error_response.dump());
    } catch (const nlohmann::json::type_error& e) {
        // Type conversion error
        nlohmann::json error_response = {
            {"type", "ERROR"},
            {"payload", {
                {"error_code", "TYPE_ERROR"},
                {"message", "Invalid data type in JSON: " + std::string(e.what())}
            }}
        };
        session->send_message(error_response.dump());
    } catch (const std::exception& e) {
        // Generic error handler
        nlohmann::json error_response = {
            {"type", "ERROR"},
            {"payload", {
                {"error_code", "INTERNAL_ERROR"},
                {"message", "Internal server error: " + std::string(e.what())}
            }}
        };
        session->send_message(error_response.dump());
    }
}

void MessageDispatcher::stop(){
  done_ = true;
  command_queue->shutdown(); 
}

void MessageDispatcher::worker_loop() {
  while (!done_) {
    auto opt_task = command_queue->wait_and_pop();
    if (!opt_task.has_value()) {
      std::cerr << "No task available, breaking\n";
      break;
    }
    auto task = std::move(opt_task.value());
    if (!task.command) {
      std::cerr << "Null command received\n";
      continue;
    }
    if (!task.session) {
      std::cerr << "Null session received\n";
      continue;
    }
    task.command->execute(task.session, commandContext);
  }
}
