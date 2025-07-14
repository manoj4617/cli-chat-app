cmake_minimum_required(VERSION 3.16)
project(CLI-CHAT-SERVER)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -g")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)

include_directories(
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/boost
    ${CMAKE_SOURCE_DIR}/SQLiteCpp
    ${CMAKE_SOURCE_DIR}/SQLiteCpp/sqlite3
)
include_directories(${CMAKE_SOURCE_DIR}/src)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build with ASan and LeakSanitizer")
    set(SANITIZER_FLAGS -fsanitize=address -g -O1 -fno-omit-frame-pointer)
endif()
set(COMMON_WARNINGS -Wall -Wextra -Wpedantic)

add_library(net STATIC src/net.cpp)
target_compile_options(net PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(net PRIVATE ${SANITIZER_FLAGS})


add_library(client_session STATIC 
            src/ClientSession.cpp
            src/ClientSession.hpp
)
add_library(message_manager STATIC
            src/MessageManager.cpp
            src/MessageManager.hpp)

target_link_libraries(client_session PRIVATE net message_manager)
target_compile_options(client_session PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(client_session PRIVATE ${SANITIZER_FLAGS})

target_link_libraries(client_session PRIVATE net)

add_library(connection_manager STATIC
            src/ConnectionManager.cpp
            src/ConnectionManager.hpp
)

target_link_libraries(connection_manager PRIVATE client_session)
target_compile_options(connection_manager PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(connection_manager PRIVATE ${SANITIZER_FLAGS})

target_link_libraries(connection_manager PRIVATE client_session)

add_library(listener STATIC
            src/Listener.cpp
            src/Listener.hpp
)

target_link_libraries(listener PRIVATE connection_manager)
target_compile_options(listener PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(listener PRIVATE ${SANITIZER_FLAGS})

add_executable(cli-chat-server  src/main.cpp)
#temp client bin
add_executable(client src/test-client.cpp)
target_link_libraries(listener PRIVATE connection_manager)


target_link_libraries(cli-chat-server
          PRIVATE
          listener
)

target_compile_options(cli-chat-server PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(cli-chat-server PRIVATE ${SANITIZER_FLAGS})
