#include "NetworkManager.hpp"
#include "boost/beast/core/bind_handler.hpp"
#include "boost/beast/core/error.hpp"
#include "boost/beast/core/role.hpp"
#include "boost/beast/core/stream_traits.hpp"
#include "boost/beast/version.hpp"
#include "boost/beast/websocket/rfc6455.hpp"
#include "boost/beast/websocket/stream_base.hpp"
#include <chrono>
#include <string>
#include <iostream>

void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

NetworkManager::NetworkManager(
    const std::string& host, uint16_t port, net::io_context& ioc,
    ConcurrentQueue<std::string>& inbound_queue,
    ConcurrentQueue<std::string>& outbound_queue
) : host_(host),
    port_(port),
    resolver_(net::make_strand(ioc)),
    ws_(net::make_strand(ioc)),
    inbound_queue_(inbound_queue),
    outbound_queue_(outbound_queue) {}

void NetworkManager::run(){
    resolver_.async_resolve(
        host_.c_str(),
        std::to_string(port_).c_str(),
        beast::bind_front_handler(&NetworkManager::on_resolve, shared_from_this())
    );
}

void NetworkManager::send(std::string& message){
    net::post(
        ws_.get_executor(), [self = shared_from_this(), msg = std::move(message)]() mutable{
            self->write_queue_.push_back(std::move(msg));
            if(!self->is_writing_.exchange(true)){
                self->do_write();
            }
        }
    );
}

void NetworkManager::close() {
    net::post(ws_.get_executor(), [self = shared_from_this()]() {
        if (self->ws_.is_open()) {
            self->ws_.async_close(websocket::close_code::normal,
                beast::bind_front_handler(&NetworkManager::on_close, self));
        }
    });
}

void NetworkManager::on_resolve(beast::error_code ec, tcp::resolver::results_type results){
    if(ec){
        fail(ec, "resolve");
    }

    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(&NetworkManager::on_connect, shared_from_this())
    );
}

void NetworkManager::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep){
    if(ec){
        fail(ec, "on_connect");
    }

    beast::get_lowest_layer(ws_).expires_never();
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::client)
        );
    
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type &req){
            req.set(
                http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                " websocket-client-async");

        }));

    host_ += ':' + std::to_string(ep.port());
    ws_.async_handshake(
        host_,
        "/",
        beast::bind_front_handler(
            &NetworkManager::on_handshake,
            shared_from_this()
        )
    );
}

void NetworkManager::on_handshake(beast::error_code ec){
     if (ec) {
        // Here you would push a failure message to the inbound_queue_
        inbound_queue_.push(R"({"type":"ERROR", "payload":{"message":"Handshake failed"}})");
        return fail(ec, "handshake");
    }

    // Connection is successful!
    // Push a success message to the UI.
    inbound_queue_.push(R"({"type":"CONNECTED"})");

    // *** THE KEY CHANGE ***
    // Instead of writing, we now kick off the INDEPENDENT read and write loops.
    
    // 1. Start listening for messages from the server permanently.
    do_read();

    // 2. Start processing any messages the UI might want to send.
    // (We start the write pump, which will check the outbound queue)
    // This is a more advanced approach, for now, the send() method is enough.
}

void NetworkManager::do_write() {
    // This function is always called on the strand.
    if (write_queue_.empty()) {
        is_writing_ = false;
        return;
    }

    ws_.async_write(
        net::buffer(write_queue_.front()),
        beast::bind_front_handler(&NetworkManager::on_write, shared_from_this())
    );
}

void NetworkManager::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        inbound_queue_.push(R"({"type":"DISCONNECTED"})");
        is_writing_ = false;
        return fail(ec, "write");
    }

    // The write was successful. Remove the message from the queue.
    write_queue_.pop_front();

    // If there are more messages, continue the write loop. Otherwise, stop.
    if (!write_queue_.empty()) {
        do_write();
    } else {
        is_writing_ = false;
    }
}

void NetworkManager::do_read() {
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(&NetworkManager::on_read, shared_from_this())
    );
}


void NetworkManager::on_read(beast::error_code ec, std::size_t bytes_transferred){
   boost::ignore_unused(bytes_transferred);

    if (ec) {
        inbound_queue_.push(R"({"type":"DISCONNECTED"})");
        return fail(ec, "read");
    }

    inbound_queue_.push(beast::buffers_to_string(buffer_.data()));
    
    buffer_.consume(buffer_.size());
    do_read();
}

void NetworkManager::on_close(beast::error_code ec){
    if (ec)
            return fail(ec, "close");

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer_.data()) << std::endl;
}