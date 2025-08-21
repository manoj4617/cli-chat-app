#include <CassandraMessageRepo.hpp>

CassandraMessageRepo::CassandraMessageRepo(std::shared_ptr<CassandraConnection> cass_conn){
    conn_ = cass_conn;
    const char* hosts = "127.0.0.1";
    CassError ec = cass_cluster_set_contact_points(conn_->cluster, hosts);
    if(ec != CASS_OK){
        std::cerr << "[ERROR] Failed to set contact points: "
                  << cass_error_desc(ec) << std::endl;
        throw std::runtime_error("Cassandra connection setup failed!!");
    }
}

Result<std::monostate> CassandraMessageRepo::add(const ChatMessage &message) {

}

Result<std::vector<ChatMessage>> CassandraMessageRepo::get_for_barrack(const std::string &barrack_id, int limit) {

}