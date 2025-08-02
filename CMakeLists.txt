cmake_minimum_required(VERSION 3.16)
project(CLI-CHAT-SERVER VERSION 1.0.0 LANGUAGES CXX)

# --- Global Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Let CMake handle output directories unless you have a specific reason not to.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)


# --- Compilation Flags ---
set(COMMON_WARNINGS -Wall -Wextra -Wpedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build with ASan and LeakSanitizer")
    add_definitions(-DDEBUG)
    set(SANITIZER_FLAGS -fsanitize=address -g -O1 -fno-omit-frame-pointer)
endif()


# ==============================================================================
# === Third-Party Dependencies
# ==============================================================================

# --- Boost (Header-Only) ---
set(BOOST_INCLUDE_DIR ${CMAKE_SOURCE_DIR})

# --- nlohmann/json (Header-Only) ---
set(NLOHMANN_JSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/nlohmann)

set(SQLITECPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/SQLiteCpp/include)


# --- SQLiteCpp ---

set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "Disable SQLiteCpp examples")
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "Disable SQLiteCpp tests")
add_subdirectory(${CMAKE_SOURCE_DIR}/SQLiteCpp)

# --- libsodium ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM REQUIRED libsodium)

include_directories(${LIBSODIUM_INCLUDE_DIRS})
link_directories(${LIBSODIUM_LIB_DIRS})

find_package(Threads REQUIRED)


# ==============================================================================
# === Project Libraries
# ==============================================================================


add_library(project_common_properties INTERFACE)
target_compile_options(project_common_properties INTERFACE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(project_common_properties INTERFACE ${SANITIZER_FLAGS})
target_include_directories(project_common_properties INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${BOOST_INCLUDE_DIR}
    ${NLOHMANN_JSON_INCLUDE_DIR}
    ${SQLITECPP_INCLUDE_DIR}
)

function(add_project_library NAME)
    add_library(${NAME} STATIC ${ARGN})
    target_link_libraries(${NAME} PRIVATE project_common_properties)
endfunction()

# --- Core Libraries ---
add_project_library(net
    src/net.cpp
)

add_project_library(client_session
    src/ClientSession.cpp
)
add_project_library(auth_manager
    src/AuthManager.cpp
)
add_project_library(barrack_manager
    src/BarrackManager.cpp
)
add_project_library(message_manager
    src/MessageManager.cpp
)
add_project_library(connection_manager
    src/ConnectionManager.cpp
)
add_project_library(listener
    src/Listener.cpp
)

add_library(crypto_utils
    src/Crypto.cpp
)

add_project_library(data_layer
    src/DatabaseConn.cpp
    src/UserRepo.cpp
    src/BarrackRepo.cpp
    src/MessageRepo.cpp
)


# ==============================================================================
# === Library Dependencies (The Link Graph)
# ==============================================================================

# `database` needs to link to SQLiteCpp's library and requires pthread.
# The `SQLiteCpp` target from `add_subdirectory` handles its own dependencies correctly.
target_link_libraries(data_layer PRIVATE
    SQLiteCpp
    Threads::Threads 
    dl
)

target_link_libraries(crypto_utils PRIVATE 
    ${LIBSODIUM_LIBRARIES}
)

# `auth_manager` and `barrack_manager` depend on `database`
target_link_libraries(auth_manager PRIVATE data_layer crypto_utils)
target_link_libraries(barrack_manager PRIVATE data_layer crypto_utils)


# `message_manager` depends on auth and barrack managers
target_link_libraries(message_manager PUBLIC
    auth_manager
    barrack_manager
)

# `client_session` depends on net and message_manager
target_link_libraries(client_session PRIVATE
    net
    message_manager
)

target_link_libraries(connection_manager PRIVATE client_session)
target_link_libraries(listener PRIVATE connection_manager)


# ==============================================================================
# === Executables
# ==============================================================================

add_executable(cli-chat-server src/main.cpp)
target_link_libraries(cli-chat-server PRIVATE
    listener
    project_common_properties
)

add_executable(client src/test-client.cpp)
target_link_libraries(client PRIVATE
    project_common_properties
)