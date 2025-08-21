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
find_library(CASS_LIB cassandra REQUIRED)

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

# --- Data Layer ---
add_project_library(data_layer
    src/DatabaseConn.cpp
    src/UserRepo.cpp
    src/BarrackRepo.cpp
    src/CassandraMessageRepo.cpp
)

target_link_libraries(data_layer PUBLIC
    SQLiteCpp
    ${CASS_LIB}
)
target_link_libraries(data_layer PRIVATE
    Threads::Threads
    dl
)

# --- Crypto --- #
add_project_library(crypto_utils
    src/Crypto.cpp
)
target_link_libraries(crypto_utils PRIVATE 
    ${LIBSODIUM_LIBRARIES}
)

# --- Business Logic Managers ---
add_project_library(auth_manager
    src/AuthManager.cpp
)
target_link_libraries(auth_manager PRIVATE data_layer crypto_utils)

add_project_library(barrack_manager
    src/BarrackManager.cpp
)
target_link_libraries(barrack_manager PRIVATE data_layer crypto_utils)

# --- Core Application & Networking Logic ---

add_project_library(app_core
    src/net.cpp
    src/ClientSession.cpp
    src/ConnectionManager.cpp
    src/Listener.cpp
    src/MessageDispatcher.cpp
    src/commands/AuthCommands.cpp
    src/commands/BarrackCommands.cpp
    src/commands/CommandFactory.cpp
)

target_link_libraries(app_core PUBLIC
    auth_manager
    barrack_manager
    project_common_properties
)


# ==============================================================================
# === Executables
# ==============================================================================
add_executable(cli-chat-server src/main.cpp)
target_link_libraries(cli-chat-server PRIVATE
    app_core
)

target_compile_options(data_layer PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_compile_options(crypto_utils PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_compile_options(auth_manager PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_compile_options(barrack_manager PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_compile_options(app_core PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_compile_options(cli-chat-server PRIVATE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})

target_include_directories(cli-chat-server PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(data_layer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(crypto_utils PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(auth_manager PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(barrack_manager PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(app_core PUBLIC ${CMAKE_SOURCE_DIR}/include)


add_executable(client src/test-client.cpp)
target_link_libraries(client PRIVATE project_common_properties)

