#include "OutboxRelay.hpp"
#include "Error.hpp"
#include <variant>

OutboxRelay::OutboxRelay(
    std::shared_ptr<CassandraMessageRepo> cass_repo,
    std::shared_ptr<EventRepository> event_repo)
    : event_repo_(event_repo), cass_repo_(cass_repo)  {}

OutboxRelay::~OutboxRelay(){
    stop();
}

void OutboxRelay::start() {
    if(!worker_thread_.joinable()){
        worker_thread_ = std::thread(&OutboxRelay::run, this);
    }
}

void OutboxRelay::stop(){
    if(worker_thread_.joinable()){
        stop_requested_.store(true);
        cv_.notify_one();
        worker_thread_.join();
    }
}

void OutboxRelay::run(){
    std::cout << "Outbox relay worker thread started..." << std::endl;
    while(!stop_requested_.load()){
        try{
            auto events_result = event_repo_->get_unprocessed_events();
            if(auto* events =  std::get_if<std::vector<OutboxEvent>>(&events_result)){
                if(events->empty()){
                    std::unique_lock<std::mutex> lock(mtx_);
                    cv_.wait_for(lock, std::chrono::seconds(5),[this]{ return stop_requested_.load();});
                    
                } else {
                    for(const auto& event : *events){
                        process_event(event);
                    }
                }
            }
            else {
                std::cerr << "OutboxRelay: Failed to fetch events from outbox." << std::endl;
            }
        } catch(const std::exception &ex){
             std::cerr << "OutboxRelay: Unhandled exception in worker loop: " << ex.what() << std::endl;
            // Sleep to prevent fast-spinning crash loops
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    std::cout << "OutboxRelay worker thread stopped." << std::endl;
}

void OutboxRelay::process_event(const OutboxEvent& event){
    if(event.event_type == "BarrackDestroyed"){
        try{
            nlohmann::json payload = nlohmann::json::parse(event.payload);
            std::string barrack_id = payload.at("barrack_id").get<std::string>();

            std::cout << "Processing BarrackDestroyed event for barrack_id: " << barrack_id << std::endl;

            auto delete_result = cass_repo_->delete_barrack_messages(barrack_id);

            if(std::holds_alternative<Error>(delete_result)){
                std::cerr << "Failed to process event " << event.event_id 
                          << ". Error: " << std::get<Error>(delete_result).message << ". Will retry." << std::endl;
            }
            else{
                event_repo_->delete_event(event.event_id);
            }
        } catch(const nlohmann::json::exception &ex){
            std::cerr << "Failed to parse payload for event " << event.event_id << ": " << ex.what() << std::endl;
        }
    }
}