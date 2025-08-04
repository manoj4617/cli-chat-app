#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <memory>
#include <chrono>
#include <deque>
#include "net.hpp"
#include "MessageDispatcher.hpp"


using c_time = std::chrono::steady_clock;
class ConnectionManager;

enum ConnStatus{
    HANDSHAKING = 0,
    CONNECTING,
    AUTHENTICATING,
    ACTIVE,
    CLOSED_BY_CLIENT,
    CLOSING
};

class ClientSession : public std::enable_shared_from_this<ClientSession> {
    public:
        using SessionID = uint64_t;

    private:
        websocket::stream<beast::tcp_stream> ws_;
        beast::flat_buffer buffer_;
        std::weak_ptr<ConnectionManager> conn_manager_;
        std::weak_ptr<MessageDispatcher> message_dispatcher_;
        SessionID session_id_;
        std::deque<std::string> write_msg_;
        bool is_writing_ = false;
        
        std::string client_ip_;
        unsigned short client_port_;
        c_time::time_point conn_time_;
        c_time::time_point last_activity_;
        ConnStatus status_;
        std::string authenticated_user_id_;
        std::atomic<uint64_t> next_sequence_id_{0};

        void close_session(websocket::close_reason = {});
        void update_last_activity();
        void do_actual_write();
    public:
        explicit ClientSession(tcp::socket&&, ClientSession::SessionID, std::shared_ptr<ConnectionManager>, std::shared_ptr<MessageDispatcher>);
        void run();
        void on_run();
        void on_accept(error_code);
        void do_read();
        void on_read(error_code, std::size_t);
        void on_write(error_code, std::size_t);

        /* getters */
        SessionID get_id() const { return session_id_;}
        std::string get_client_ip_addr() const { return client_ip_; }
        unsigned short get_client_port() const { return client_port_; }

        ConnStatus get_status() const { return status_; }
        std::string get_authentiated_user_id() const { return authenticated_user_id_; }
        
        c_time::time_point get_connection_time() const { return conn_time_; }
        c_time::time_point get_last_activity_time() const { return last_activity_; }
        uint64_t get_next_sequence_id() { return next_sequence_id_++;}
        /* getters */

        /* setters */
        void set_status(const ConnStatus);
        void set_authenticated_user(const std::string&);
        /* setters */

        void send_message(const std::string&);
};

#endif