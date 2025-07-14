#include "Listener.hpp"
#include "MessageManager.hpp"
#include "AuthManager.hpp"
#include "BarrackManager.hpp"
#include <thread>

int main(){
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("8080"));

    int thread_num = 4;
    std::cout << "[INFO] Starting char server on " << address << ":" << port << " with " << thread_num << " threads." << std::endl;
    net::io_context ioc{thread_num};
    auto auth_manager = std::make_shared<AuthManager>();
    auto barrack_manager = std::make_shared<BarrackManager>();
    auto message_manager = std::make_shared<MessageManager>(auth_manager, barrack_manager);
    auto conn_manager = std::make_shared<ConnectionManager>(message_manager);

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