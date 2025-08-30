#ifndef CASSANDRAREPO_H
#define CASSANDRAREPO_H

#include "MessageRepo.hpp"
#include <cassandra.h>
#include <memory>

static constexpr const char* CREATE_KEYSPACE_QUERY =
        "CREATE KEYSPACE IF NOT EXISTS chat_app "
        "WITH replication = {'class': 'SimpleStrategy', 'replication_factor': '1'};";

static constexpr const char* CREATE_MESSAGES_TABLE_QUERY =
        "CREATE TABLE IF NOT EXISTS chat_app.messages ("
        "    barrack_id text,"
        "    message_id timeuuid,"
        "    sender_id text,"
        "    content text,"
        "    timestamp timestamp,"
        "    PRIMARY KEY (barrack_id, message_id)"
        ") WITH CLUSTERING ORDER BY (message_id DESC);";

static constexpr const char* ADD_MESSAGE_TO_DATABASE = 
        "INSERT INTO chat_app.messages "
        "(barrack_id, message_id, sender_id, content, timestamp)"
        "VALUES (?, ?, ?, ?, ?)";

static constexpr const char* GET_MESSAGES = 
        "SELECT barrack_id, message_id, sender_id, content, timestamp "
        "FROM chat_app.messages WHERE barrack_id = ? LIMIT ?";

struct CassandraConnection {
    CassCluster *cluster = nullptr;
    CassSession *session = nullptr;

    CassandraConnection() {
        cluster = cass_cluster_new();
        session = cass_session_new();
    }

    ~CassandraConnection() {
        if(cluster) cass_cluster_free(cluster);
        if(session) cass_session_free(session);
    }
};

class CassandraMessageRepo : public MessageRepository {
    public:
        explicit CassandraMessageRepo(std::shared_ptr<CassandraConnection> cass_conn);
        ~CassandraMessageRepo();
        Result<std::monostate> init_database();
        Result<std::monostate> add(const ChatMessage& message) override;
        Result<std::vector<ChatMessage>> get_for_barrack(const std::string& barrack_id, int limit) override;

    private:
        std::shared_ptr<CassandraConnection> conn_;
        Result<std::monostate> execute_simple_query(const char* query);
        bool prepare_statements();
        const CassPrepared* add_message_prepared_ = nullptr;
        const CassPrepared* get_message_prepared_ = nullptr;
};

#endif