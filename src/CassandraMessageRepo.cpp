#include "Error.hpp"
#include <CassandraMessageRepo.hpp>
#include <cassandra.h>
#include <variant>

CassandraMessageRepo::CassandraMessageRepo(std::shared_ptr<CassandraConnection> cass_conn){
    conn_ = cass_conn;
    const char* hosts = "127.0.0.1";
    CassError ec = cass_cluster_set_contact_points(conn_->cluster, hosts);
    if(ec != CASS_OK){
        std::cerr << "[ERROR] Failed to set contact points: "
                  << cass_error_desc(ec) << std::endl;
        throw std::runtime_error("Cassandra connection setup failed!!");
    }

    CassFuture* cass_future = cass_session_connect(conn_->session, conn_->cluster);
    if(cass_future_error_code(cass_future) == CASS_OK){
        std::cout << "Successfully connected to Cassandra!!\n";
        cass_future_free(cass_future);
    }
    else {
        const char* message;
        size_t message_size;
        cass_future_error_message(cass_future, &message, &message_size);
        std::cerr << "Unable to connect to Cassandra: " << std::string(message, message_size) << std::endl;
        cass_future_free(cass_future);
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

    std::cout << "Cassandra schema initialization complete.\n";
    return Success{};
}

Result<std::monostate> CassandraMessageRepo::add(const ChatMessage &message) {

}

Result<std::vector<ChatMessage>> CassandraMessageRepo::get_for_barrack(const std::string &barrack_id, int limit) {

}

Result<std::monostate> CassandraMessageRepo::execute_simple_query(const char* query){
    CassStatement* cass_statement = cass_statement_new(query, 0);
    cass_statement_set_consistency(cass_statement, CASS_CONSISTENCY_QUORUM);

    CassFuture* cass_future = cass_session_execute(conn_->session, cass_statement);
    cass_future_wait(cass_future);

    CassError rc = cass_future_error_code(cass_future);

    Result<std::monostate> result;
    if(rc != CASS_OK){
         const char* message;
        size_t message_length;
        cass_future_error_message(cass_future, &message, &message_length);
        std::string error_msg = "Cassandra query failed: " + std::string(message, message_length);
        result = Error{ErrorCode::DATABASE_ERROR, error_msg};
    } else {
        result = Success{};
    }

    cass_future_free(cass_future);
    cass_statement_free(cass_statement);

    return result;
}