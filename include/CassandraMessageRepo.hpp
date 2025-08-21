#ifndef CASSANDRAREPO_H
#define CASSANDRAREPO_H

#include "MessageRepo.hpp"
#include <cassandra.h>
#include <memory>

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

        Result<std::monostate> add(const ChatMessage& message) override;
        Result<std::vector<ChatMessage>> get_for_barrack(const std::string& barrack_id, int limit) override;

    private:
        std::shared_ptr<CassandraConnection> conn_;
        const CassPrepared* add_message_prepared_ = nullptr;
        const CassPrepared* get_message_prepared_ = nullptr;
};

#endif