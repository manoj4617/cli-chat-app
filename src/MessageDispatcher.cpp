
#include <json.hpp>
#include <thread>
#include <vector>

#include "MessageDispatcher.hpp"

MessageDispatcher::MessageDispatcher(size_t num_threads, CommandContext context) : commandContext(context) {
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
    auto json_msg = nlohmann::json::parse(raw_payload);
    std::string type = json_msg.value("type", "");

    auto command = commandFactory.create_command(type, json_msg["payload"]);

    if (command) {
      command_queue.push({std::move(command), session});
    } else {
    }
  } catch (...) {
    throw std::runtime_error("Json parsing failed!!");
  }
}

void MessageDispatcher::stop(){
  done_ = true;
  command_queue.shutdown(); 
}

void MessageDispatcher::worker_loop() {
  while (!done_) {
    auto opt_task = command_queue.wait_and_pop();
    if (!opt_task.has_value()) {
      break;
    }
    auto task = std::move(opt_task.value());
    task.command->execute(task.session, commandContext);
  }
}
