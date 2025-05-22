cmake_minimum_required(VERSION 3.16)
project(CLI-CHAT-SERVER)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -g")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)

include_directories(${CMAKE_SOURCE_DIR}/boost)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(net STATIC src/net.cpp)

add_library(client_session STATIC 
            src/ClientSession.cpp
            src/ClientSession.hpp
)

target_link_libraries(client_session PRIVATE net)

add_library(connection_manager STATIC
            src/ConnectionManager.cpp
            src/ConnectionManager.hpp
)

target_link_libraries(connection_manager PRIVATE client_session)

add_library(listener STATIC
            src/Listener.cpp
            src/Listener.hpp
)

target_link_libraries(listener PRIVATE connection_manager)

add_executable(cli-chat-server  src/main.cpp)

target_link_libraries(cli-chat-server
          PRIVATE
          listener
)

