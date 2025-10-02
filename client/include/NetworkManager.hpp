#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "ConcurrentQueue.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/core/error.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/asio/dispatch.hpp"
#include "boost/asio/strand.hpp"

#include <cstdint>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <ConcurrentQueue.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;          
namespace http = beast::http;  
using tcp = boost::asio::ip::tcp; 

class NetworkManager : public std::enable_shared_from_this<NetworkManager> {

    public:
    static std::shared_ptr<NetworkManager> Create(
        const std::string& host,
        uint16_t port,
        net::io_context& ioc,
        ConcurrentQueue<std::string>& inbound_queue,
        ConcurrentQueue<std::string>& outbound_queue
    ){
        return std::shared_ptr<NetworkManager>(new NetworkManager(host, port, ioc, inbound_queue, outbound_queue));
    }
    
    void run();
    void send(std::string& message);
    void close();
        
    private:
    NetworkManager(
        const std::string& host, uint16_t port, net::io_context& ioc,
        ConcurrentQueue<std::string>& inbound_queue,
        ConcurrentQueue<std::string>& outbound_queue
    );


        void on_resolve(beast::error_code ec, tcp::resolver::results_type type);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
        void on_handshake(beast::error_code ec);
        
        void do_read();
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void do_write();
        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        void on_close(beast::error_code ec);
        std::string host_;
        uint16_t port_;

        tcp::resolver resolver_;
        websocket::stream<beast::tcp_stream> ws_;
        beast::flat_buffer buffer_;

        ConcurrentQueue<std::string>& inbound_queue_;  // Writes to this
        ConcurrentQueue<std::string>& outbound_queue_; // Reads from this

        // State for the write loop
        std::deque<std::string> write_queue_;
        std::atomic<bool> is_writing_{false};
};

#endif