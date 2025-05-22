#include "Listener.hpp"
#include <thread>

int main(){
    auto const address = net::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("8080"));

    int thread_num = 4;
    net::io_context ioc{thread_num};
    auto conn_manager = std::make_shared<ConnectionManager>();

    std::make_shared<Listener>(ioc, tcp::endpoint{address, port}, conn_manager)->run();

    std::vector<std::thread> v;
    for(auto i = thread_num - 1; i > 0; --i){
        v.emplace_back(
            [&ioc](){
                ioc.run();
            }
        );
    }
    ioc.run();
    return EXIT_SUCCESS;
}