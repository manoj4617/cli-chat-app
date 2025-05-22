#include "Listener.hpp"

Listener::Listener(net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<ConnectionManager> cm)
    : ioc_(ioc), acceptor_(ioc), conn_manager_(cm)
{
    error_code ec;
    
    // Open acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec){
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true));
    if(ec){
        fail(ec, "set_options");
        return;
    }

    //Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec){
        fail(ec, "bind");
        return;
    }

    //Start listening for the connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if(ec){
        fail(ec, "listen");
        return;
    }
}

void Listener::run(){
    do_accept();
}

void Listener::do_accept(){
    // each new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(
            &Listener::on_accept,
            shared_from_this()
        )
    );
}

void Listener::on_accept(error_code ec, tcp::socket socket){
    if(ec){
        fail(ec, "accept");
    }
    else{
        conn_manager_->start_new_session(std::move(socket));
    }
    do_accept();
}