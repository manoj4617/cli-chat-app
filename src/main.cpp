#include <cstdlib>
#include <thread>
#include <iostream>

#include <Listener.hpp>
#include <MessageDispatcher.hpp>
#include <AuthManager.hpp>
#include <BarrackManager.hpp>
#include <DatabaseConn.hpp>
#include <UserRepo.hpp>
#include <BarrackRepo.hpp>
#include <CassandraMessageRepo.hpp>
#include <Error.hpp>
#include <variant>

int main(){
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("8080"));

    int thread_num = 4;
    std::cout << "[INFO] Starting char server on " << address << ":" << port << " with " << thread_num << " threads." << std::endl;
    net::io_context ioc{thread_num};

    auto cass_db = std::make_shared<CassandraMessageRepo>(std::make_shared<CassandraConnection>());
    auto res = cass_db->init_database();
    if(std::holds_alternative<Error>(res)){
        std::cerr << "[FATAL] Could not initialize Cassandra Database. Shutting down." << std::endl;
        return EXIT_FAILURE;
    }
    auto database = std::make_shared<DatabaseConnection>("chat-server.db3");
    if(!database->is_valid()){
        std::cerr << "[FATAL] Could not initialize Database Manager. Shutting down." << std::endl;
        return EXIT_FAILURE;
    }

    // Error schema_error = database->initialize_database();
    // if(schema_error.code != ErrorCode::SUCCESS){
    //     std::cerr << "[FATAL] " << schema_error.message << ". Shutting down." << std::endl;
    //     return EXIT_FAILURE; // Exit if tables can't be created
    // }
    auto user_repo = std::make_shared<UserRepository>(database->get_connection());
    auto barrack_repo = std::make_shared<BarrackRepository>(database->get_connection());

    auto auth_manager = std::make_shared<AuthManager>(user_repo);
    auto barrack_manager = std::make_shared<BarrackManager>(barrack_repo, cass_db);
    CommandContext command_context {
        .auth_manager = auth_manager,
        .barrack_manager = barrack_manager
    };
    auto message_dispatcher = std::make_shared<MessageDispatcher>(thread_num, command_context);
    auto conn_manager = std::make_shared<ConnectionManager>(message_dispatcher);

    std::cout <<"[INFO] Initializing Listener" << std::endl;
    std::make_shared<Listener>(ioc, tcp::endpoint{address, port}, conn_manager)->run();

    std::vector<std::thread> v;
    for(auto i = thread_num - 1; i > 0; --i){
        v.emplace_back(
            [&ioc](){
                std::cout << "[INFO] Worker thread started" << std::endl;
                ioc.run();
                std::cout << "[INFO] Worker thread stopped" << std::endl;
            }
        );
    }
    std::cout << "[INFO] Main thread running IO context\n";
    ioc.run();
    std::cout << "[INFO] Server shutting down\n";
    return EXIT_SUCCESS;
}