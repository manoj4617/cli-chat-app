#include "ConcurrentQueue.hpp"
#include "boost/asio/io_context.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <NetworkManager.hpp>
#include <iostream>

int main() {
  std::stringstream ss;
  net::io_context ioc;

  ConcurrentQueue<std::string> inbound_queue;
  ConcurrentQueue<std::string> outbound_queue;

  auto net_manager = NetworkManager::Create("localhost", 8080, ioc, inbound_queue, outbound_queue);
  net_manager->run();

  std::thread network_thread([&ioc]{ioc.run();});

  while(true){
    std::string login_json = R"({"type":"LOGIN", "payload":{"username":"user", "password":"pass"}})";
    net_manager->send(login_json);
    if(auto msg = inbound_queue.try_pop()){
      if(msg.has_value()){
        std::cout << msg.value();
      }
    }
  }
  return 0;
}