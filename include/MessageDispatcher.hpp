#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include "ConcurrentQueue.hpp"
#include "../src/commands/ICommand.hpp"
#include "../src/commands/CommandFactory.hpp"


class MessageDispatcher{
    public:
        MessageDispatcher(size_t num_threads, CommandContext context);

        ~MessageDispatcher();

        void dispatch(std::shared_ptr<ClientSession> session, const std::string& raw_payload);

        void stop();
    private:

        void worker_loop();
        struct CommandTask {
            std::unique_ptr<ICommand> command;
            std::shared_ptr<ClientSession> session;
        };
        CommandFactory commandFactory;
        CommandContext commandContext;
        std::unique_ptr<ConcurrentQueue<CommandTask>> command_queue;
        std::vector<std::thread> workers_;
        bool done_ = false;
};

#endif // MESSAGEDISPATCHER_H