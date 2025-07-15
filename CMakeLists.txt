cmake_minimum_required(VERSION 3.16)
project(CLI-CHAT-SERVER VERSION 1.0.0 LANGUAGES CXX)

# Global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)

# Compilation flags
set(COMMON_WARNINGS -Wall -Wextra -Wpedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build with ASan and LeakSanitizer")
    set(SANITIZER_FLAGS -fsanitize=address -g -O1 -fno-omit-frame-pointer)
endif()

# Include directories
set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/boost
    ${CMAKE_SOURCE_DIR}/SQLiteCpp
    ${CMAKE_SOURCE_DIR}/SQLiteCpp/sqlite3
    ${CMAKE_SOURCE_DIR}/src
)
include_directories(${INCLUDE_DIRS})

# Helper function to add libraries with common settings
function(add_project_library NAME)
    add_library(${NAME} STATIC ${ARGN})
    target_compile_options(${NAME} PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
    target_link_options(${NAME} PRIVATE ${SANITIZER_FLAGS})
endfunction()

# Core libraries
add_project_library(net 
    src/net.cpp
)

add_project_library(client_session 
    src/ClientSession.cpp
    include/ClientSession.hpp
)

add_project_library(auth_manager
    src/AuthManager.cpp
    include/AuthManager.hpp
)

add_project_library(barrack_manager
    src/BarrackManager.cpp
    include/BarrackManager.hpp
)

add_project_library(message_manager
    src/MessageManager.cpp
    include/MessageManager.hpp
)

add_project_library(connection_manager
    src/ConnectionManager.cpp
    include/ConnectionManager.hpp
)

add_project_library(listener
    src/Listener.cpp
    include/Listener.hpp
)

# Dependencies
target_link_libraries(message_manager PRIVATE auth_manager barrack_manager)
target_link_libraries(client_session PRIVATE 
    net 
    message_manager
)
target_link_libraries(connection_manager PRIVATE client_session)
target_link_libraries(listener PRIVATE connection_manager)

# Executables
add_executable(cli-chat-server src/main.cpp)
add_executable(client src/test-client.cpp)

# Executable dependencies
target_link_libraries(cli-chat-server PRIVATE listener)
target_compile_options(cli-chat-server PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(cli-chat-server PRIVATE ${SANITIZER_FLAGS})

# SQLiteCpp dependency (add this when ready to integrate database)
# find_package(SQLiteCpp CONFIG REQUIRED)
# target_link_libraries(auth_manager PRIVATE 
#     SQLiteCpp
#     sqlite3
#     pthread
#     dl
# )