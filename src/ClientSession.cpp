#include "ConnectionManager.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ClientSession::ClientSession(tcp::socket&& socket,
                            SessionID session_id,
                            std::shared_ptr<ConnectionManager> conn_manager)
                : ws_(std::move(socket)),
                  conn_manager_(conn_manager),
                  session_id_(session_id),
                  conn_time_(c_time::now()),
                  last_activity_(conn_time_),
                  status_(ConnStatus::CONNECTING)
{
    try{

        if(ws_.next_layer().socket().is_open()){
            client_ip_  = ws_.next_layer()
                                .socket()
                                .remote_endpoint()
                                .address()
                                .to_string();
            client_port_ = ws_.next_layer()
                                .socket()
                                .remote_endpoint()
                                .port();
        }
    }
    catch(const boost::system::system_error& e){
        std::cerr << "Session " << session_id_ << ": Error gettig remote endpoint: "
                  << e.what() << "\n";
        client_ip_ = "UNKNOWN";
        client_port_ = 0;
    }
}

void ClientSession::run(){
    net::dispatch(
        ws_.get_executor(),
        beast::bind_front_handler(
            &ClientSession::on_run,
            shared_from_this()
        )
    );
}

void ClientSession::on_run(){
    set_status(ConnStatus::HANDSHAKING);
    update_last_activity();
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws_.set_option(websocket::stream_base::decorator(
                    [](websocket::response_type& res){
                        res.set(http::field::server, "cli-chat-server/1.0");
                    }));

    ws_.async_accept(
        [self = shared_from_this()](error_code ec){
            self->on_accept(ec);
        }
    );
}

void ClientSession::on_accept(error_code ec){
    if(ec){
        fail(ec, "accept");
        close_session();
        return;
    }
    set_status(ConnStatus::ACTIVE);
    update_last_activity();
    do_read();
}

void ClientSession::do_read(){
    ws_.async_read(buffer_,
        [self = shared_from_this()](error_code ec, std::size_t bytes_transfered){
            self->on_read(ec, bytes_transfered);
        }
    );
}

void ClientSession::on_read(error_code ec, std::size_t bytes_transfered){
    update_last_activity();
    boost::ignore_unused(bytes_transfered);

    if(ec == websocket::error::closed){
        set_status(ConnStatus::CLOSED_BY_CLIENT);
        close_session();
        return;
    }
    if(ec){
        fail(ec, "read");
        return;
    }

    //1. Convert Buffer to string
    std::string payload = beast::buffers_to_string(buffer_.data());

    //2. Consume the buffer basically this clears the buffer_
    buffer_.consume(buffer_.size());

    // 3. TODO: Parse received_payload_str to JSON
    try{
        json json_msg = json::parse(payload);
    }
    catch(const json::parse_error& ex){
        send_message("{\"type\":\"ERROR\", \"payload\":{\"code\":\"INVALID_JSON\", \"message\":\"" + std::string(ex.what()) + "\"}}");
        do_read(); // Continue reading for next message
        return;
    }

    //4. Pass json payload to the message manager
    std::cout << "Session id: " << session_id_ << " Received: " << payload << "\n";  

    if(payload == "quit"){
        close_session();
        return;
    }
    send_message(payload);
    do_read();
}

void ClientSession::on_write(error_code ec, std::size_t bytes_transfered){
    boost::ignore_unused(bytes_transfered);
    update_last_activity();
    if(ec){
        fail(ec, "write");
        is_writing_ = false;
        write_msg_.clear();
        close_session();
        return;
    }

    if(!write_msg_.empty()){
        write_msg_.pop_front();
    }
    
    if(!write_msg_.empty()){
        do_actual_write();
    } else {
        is_writing_ = false;
    }
}

void ClientSession::send_message(const std::string& message){
    if(!ws_.is_open()){
        std::cerr << "Session " << session_id_ << ": Attempted to write on a closed socket\n";
        return;
    }
    auto self = shared_from_this();
    net::post(ws_.get_executor(), [self, message](){
        self->write_msg_.push_back(message);
        if(!self->is_writing_){
            self->do_actual_write();
        }
    });
}

void ClientSession::do_actual_write(){
    if(write_msg_.empty()){
        is_writing_ = false;
        return;
    }

    is_writing_ = true;

    ws_.text(true);
    ws_.async_write(net::buffer(write_msg_.front()),
                    beast::bind_front_handler(
                        &ClientSession::on_write,
                        shared_from_this()
                    )    
                );
}

void ClientSession::close_session(websocket::close_reason reason){
    set_status(ConnStatus::CLOSING);

    if(ws_.is_open()){
        // Send a websocket close frame
        // the desctuctor will handle TCP socket closure
        ws_.async_close(reason, [self = shared_from_this()](error_code ec){
            if(ec){
                fail(ec, "websocket close");
            }
        });
    }

    if(auto cm = conn_manager_.lock()){
        cm->unregister_session(session_id_);
    }
}

void ClientSession::set_authenticated_user(const std::string& user_id){
    authenticated_user_id_ = user_id;
    set_status(ConnStatus::AUTHENTICATING);
}

void ClientSession::update_last_activity(){
    last_activity_ = c_time::now();
}

void ClientSession::set_status(ConnStatus status){
    status_ = status;
}