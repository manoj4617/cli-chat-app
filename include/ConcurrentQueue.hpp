#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ConcurrentQueue {
    public:
        ConcurrentQueue() = default;
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue operator=(const ConcurrentQueue&) = delete;

        void push(T&& value){
            std::unique_lock<std::mutex> lock(mtx_);
            con_queue.push(std::move(value));
            cv_.notify_one();            
        }

        std::optional<T> wait_and_pop(){
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{ return !con_queue.empty() || done_ ;});
            if(done_ || con_queue.empty()){
                return std::nullopt;
            }
            T value = std::move(con_queue.front());
            con_queue.pop();
            return value;
        }

        std::optional<T> try_pop(){
            std::lock_guard<std::mutex> lock(mtx_);
            if(con_queue.empty()){
                return std::nullopt;
            }

            T val = std::move(con_queue.front());
            con_queue.pop();
            return val;
        }

        void shutdown(){
            std::unique_lock<std::mutex> lock(mtx_);
            done_ = true;
            cv_.notify_all();
        }

        bool empty() {
            std::unique_lock<std::mutex> lock(mtx_);
            return con_queue.empty();
        }
    
    private:
        mutable std::mutex mtx_;
        std::condition_variable cv_;
        std::queue<T> con_queue;
        bool done_ = false;
};
#endif