cmake_minimum_required(VERSION 3.16)
project(CLI-CHAT-CLIENT VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)


set(COMMON_WARNINGS -Wall -Wextra -Wpedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build with ASan and LeakSanitizer")
    add_definitions(-DDEBUG)
    set(SANITIZER_FLAGS -fsanitize=address -g -O1 -fno-omit-frame-pointer)
endif()

set(BOOST_INCLUDE_DIR ../)

set(NLOHMANN_JSON_INCLUDE_DIR /nlohmann)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM REQUIRED libsodium)

include_directories(${LIBSODIUM_INCLUDE_DIRS})
link_directories(${LIBSODIUM_LIB_DIRS})

find_package(Threads REQUIRED)

add_library(project_common_properties INTERFACE)
target_compile_options(project_common_properties INTERFACE ${COMMON_WARNINGS} ${SANITIZER_FLAGS})
target_link_options(project_common_properties INTERFACE ${SANITIZER_FLAGS})
target_include_directories(project_common_properties INTERFACE
    ../include
    ${BOOST_INCLUDE_DIR}
    ${NLOHMANN_JSON_INCLUDE_DIR}
)


include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v6.1.9
)

FetchContent_MakeAvailable(ftxui)

add_executable(client
  main.cpp
  NetworkManager.cpp   # make sure this is added here
)
target_include_directories(client
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_link_libraries(client PRIVATE
  project_common_properties
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)