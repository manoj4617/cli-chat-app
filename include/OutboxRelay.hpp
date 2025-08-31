#ifndef OUTBOXRELAY_H
#define OUTBOXRELAY_H

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "EventRepository.hpp"
#include "CassandraMessageRepo.hpp"

class OutboxRelay {
    public:
        OutboxRelay(
            std::shared_ptr<CassandraMessageRepo> cass_repo,
            std::shared_ptr<EventRepository> event_repo
        );

        ~OutboxRelay();

        OutboxRelay(const OutboxRelay&) = delete;
        OutboxRelay& operator=(const OutboxRelay&) = delete;

        void start();
        void stop();

    private:
        void run();
        void process_event(const OutboxEvent& event);

        std::shared_ptr<EventRepository> event_repo_;
        std::shared_ptr<CassandraMessageRepo> cass_repo_;

        std::thread worker_thread_;
        std::mutex mtx_;
        std::condition_variable cv_;
        std::atomic<bool> stop_requested_{false};
};

#endif