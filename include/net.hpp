#ifndef NET_H
#define NET_H


#include "boost/beast/core.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/asio/dispatch.hpp"
#include "boost/asio/strand.hpp"

#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;

using tcp = boost::asio::ip::tcp;
using error_code = beast::error_code;

void fail(beast::error_code ec, char const* what);

#endif