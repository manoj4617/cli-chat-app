#include "net.hpp"
#include <iostream>

void fail(beast::error_code ec, char const* what){
    std::cerr << what << ": " << ec.message() << "\n";
}