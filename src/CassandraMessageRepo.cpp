#include "Error.hpp"
#include "types.hpp"
#include <CassandraMessageRepo.hpp>

// These will automatically call the correct `_free` function when they go out of scope.
using CassStatementPtr = std::unique_ptr<CassStatement, decltype(&cass_statement_free)>;
using CassFuturePtr    = std::unique_ptr<CassFuture, decltype(&cass_future_free)>;
using CassIteratorPtr  = std::unique_ptr<CassIterator, decltype(&cass_iterator_free)>;
using CassResultPtr    = std::unique_ptr<const CassResult, decltype(&cass_result_free)>;

CassandraMessageRepo::CassandraMessageRepo(std::shared_ptr<CassandraConnection> cass_conn){
    conn_ = cass_conn;
    const char* hosts = "127.0.0.1";
    CassError ec = cass_cluster_set_contact_points(conn_->cluster, hosts);
    if(ec != CASS_OK){
        std::cerr << "[ERROR] Failed to set contact points: "
                  << cass_error_desc(ec) << std::endl;
        throw std::runtime_error("Cassandra connection setup failed!!");
    }

    CassFuturePtr cass_future(cass_session_connect(conn_->session, conn_->cluster), cass_future_free);
    if(cass_future_error_code(cass_future.get()) == CASS_OK){
        std::cout << "Successfully connected to Cassandra!!\n";
    }
    else {
        const char* message;
        size_t message_size;
        cass_future_error_message(cass_future.get(), &message, &message_size);
        std::cerr << "[FATAL] Unable to connect to Cassandra: " << std::string(message, message_size) << std::endl;
    }
}

CassandraMessageRepo::~CassandraMessageRepo() {
    if (add_message_prepared_) cass_prepared_free(add_message_prepared_);
    if (get_message_prepared_) cass_prepared_free(get_message_prepared_);
}

Result<std::monostate> CassandraMessageRepo::init_database(){
    std::cout << "Initializing Cassandra Schema..." << std::endl;
    auto keyspace_res = execute_simple_query(CREATE_KEYSPACE_QUERY);
    if(std::holds_alternative<Error>(keyspace_res)){
        std::cerr << "Failed to create keyspace.\n";
        return keyspace_res;
    }

    std::cout << "Keyspace 'chat_app' is ready.\n";

    auto table_creation_res = execute_simple_query(CREATE_MESSAGES_TABLE_QUERY);
    if(std::holds_alternative<Error>(table_creation_res)){
        std::cerr << "Table creation failed.\n";
        return table_creation_res;
    }

    std::cout << "Table 'chat_app.messages' is ready.\n";

    if(!prepare_statements()){
        std::cerr << "[FATAL] Statment preperation failed";
        return Error{ErrorCode::DATABASE_ERROR, "[FATAL] Statment preperation failed"};
    }
    std::cout << "Cassandra schema initialization complete.\n";
    return Success{};
}

bool CassandraMessageRepo::prepare_statements(){
     // Prepare statements
    
    CassFuture* add_msg_future = cass_session_prepare(conn_->session, ADD_MESSAGE_TO_DATABASE);
    CassFuture* get_message_future = cass_session_prepare(conn_->session, GET_MESSAGES);
    CassError rc_2 = cass_future_error_code(get_message_future);
    CassError rc_1 = cass_future_error_code(add_msg_future);

    if(rc_1 != CASS_OK || rc_2 != CASS_OK){
        std::cerr << "[ERROR] Prepared statments creation failed: "
                  << cass_error_desc(rc_1) << std::endl ;
        return false;
    }
    add_message_prepared_ = cass_future_get_prepared(add_msg_future);
    get_message_prepared_ = cass_future_get_prepared(get_message_future);

    cass_future_free(add_msg_future);
    cass_future_free(get_message_future);
    return true;
}

Result<std::monostate> CassandraMessageRepo::add(const ChatMessage &message) {
    if(!add_message_prepared_){
        return Error{ErrorCode::DATABASE_ERROR, "Add messages statement is not prepared."};
    }

    CassStatementPtr statement(cass_prepared_bind(add_message_prepared_), cass_statement_free);

    if(cass_statement_bind_string(statement.get(), 0, message.barrack_id.c_str())){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind barrack_id."};
    }
    if(cass_statement_bind_string(statement.get(), 1, message.message_id.c_str())){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind message_id."};
    }
    if(cass_statement_bind_string(statement.get(), 2, message.sender_user_id.c_str())){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind sender_id."};
    }
    if(cass_statement_bind_string(statement.get(), 3, message.content.c_str())){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind content."};
    }
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(message.sent_at.time_since_epoch()).count();
    if(cass_statement_bind_int64(statement.get(), 4, ms)){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind sent_at."};
    }
    
    CassFuturePtr res_future(cass_session_execute(conn_->session, statement.get()), cass_future_free);

    if(cass_future_error_code(res_future.get()) != CASS_OK){
        const char* msg; size_t len;
        cass_future_error_message(res_future.get(), &msg, &len);
        return Error{ErrorCode::DATABASE_ERROR, "Failed to execute add_message query: " + std::string(msg, len)};
    }

    return Success{};
}

Result<std::vector<ChatMessage>> CassandraMessageRepo::get_for_barrack(const std::string &barrack_id, int limit) {
    if(!get_message_prepared_){
        return Error{ErrorCode::DATABASE_ERROR, "Get messages statement is not prepared."};
    }

    CassStatementPtr statement(cass_prepared_bind(get_message_prepared_), cass_statement_free);
    
    if(cass_statement_bind_string(statement.get(), 0, barrack_id.c_str())){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind barrack_id."};
    }
    if(cass_statement_bind_int32(statement.get(), 1, limit)){
        return Error{ErrorCode::DATABASE_ERROR, "Failed to bind limit."};
    }

    // executing query
    CassFuturePtr future(cass_session_execute(conn_->session, statement.get()), cass_future_free);
    cass_future_wait(future.get()); // wait for results

    if(cass_future_error_code(future.get()) != CASS_OK){
        const char* msg; size_t len;
        cass_future_error_message(future.get(), &msg, &len);
        return Error{ErrorCode::DATABASE_ERROR, "Failed to execute get_messages query: " + std::string(msg, len)};
    }

    CassResultPtr result(cass_future_get_result(future.get()), cass_result_free);
    CassIteratorPtr iterator(cass_iterator_from_result(result.get()), cass_iterator_free);

    std::vector<ChatMessage> messages;
    messages.reserve(cass_result_row_count(result.get()));

    while(cass_iterator_next(iterator.get())){
        const CassRow* row = cass_iterator_get_row(iterator.get());
        ChatMessage msg;
        const char* str_val;
        size_t str_len;

        if(cass_value_get_string(cass_row_get_column_by_name(row, "barrack_id"), &str_val, &str_len)){
            return Error{ErrorCode::DATABASE_ERROR, "Failed to get barrack_id from row."};
        }
        msg.barrack_id.assign(str_val, str_len);

        CassUuid uuid;
        if(cass_value_get_uuid(cass_row_get_column_by_name(row, "message_id"), &uuid)){
            return Error{ErrorCode::DATABASE_ERROR, "Failed to get message_id from row."};
        }
        char uuid_str[CASS_UUID_STRING_LENGTH];
        cass_uuid_string(uuid, uuid_str);
        msg.message_id = uuid_str;

        if(cass_value_get_string(cass_row_get_column_by_name(row, "sender_id"), &str_val, &str_len)){
            return Error{ErrorCode::DATABASE_ERROR, "Failed to get sender_id from row."};
        }
        msg.sender_user_id.assign(str_val, str_len);

        if(cass_value_get_string(cass_row_get_column_by_name(row, "content"), &str_val, &str_len)){
            return Error{ErrorCode::DATABASE_ERROR, "Failed to get content from row."};
        }
        msg.content.assign(str_val, str_len);

        cass_int64_t timestamp_ms;
        if(cass_value_get_int64(cass_row_get_column_by_name(row, "timestamp"), &timestamp_ms)){
            return Error{ErrorCode::DATABASE_ERROR, "Failed to get timestamp from row."};
        }
        msg.sent_at = std::chrono::system_clock::time_point(std::chrono::milliseconds(timestamp_ms));
        
        messages.push_back(std::move(msg));
    }

    return messages;
}

Result<std::monostate> CassandraMessageRepo::execute_simple_query(const char* query){
    CassStatementPtr cass_statement(cass_statement_new(query, 0), cass_statement_free);
    cass_statement_set_consistency(cass_statement.get(), CASS_CONSISTENCY_QUORUM);

    CassFuturePtr cass_future(cass_session_execute(conn_->session, cass_statement.get()), cass_future_free);
    cass_future_wait(cass_future.get());

    CassError rc = cass_future_error_code(cass_future.get());

    Result<std::monostate> result;
    if(rc != CASS_OK){
        const char* message;
        size_t message_length;
        cass_future_error_message(cass_future.get(), &message, &message_length);
        std::string error_msg = "Cassandra query failed: " + std::string(message, message_length);
        result = Error{ErrorCode::DATABASE_ERROR, error_msg};
    } else {
        result = Success{};
    }

    return result;
}