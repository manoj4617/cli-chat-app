#ifndef LISTENER_H
#define LISTENER_H

#include "ConnectionManager.hpp"

class Listener : public std::enable_shared_from_this<Listener>{
    private:
        net::io_context& ioc_;
        tcp::acceptor acceptor_;
        std::shared_ptr<ConnectionManager> conn_manager_;

        void do_accept();
        void on_accept(error_code, tcp::socket);
    
    public:
        Listener(net::io_context&, tcp::endpoint, std::shared_ptr<ConnectionManager>);
        void run();
};

#endif