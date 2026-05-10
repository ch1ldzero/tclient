cmake_minimum_required(VERSION 3.14)
project(Torrent-Client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

set(cpr_SOURCE_DIR ${EXTERNAL_DIR}/cpr)
if(EXISTS ${cpr_SOURCE_DIR}/CMakeLists.txt)
    message(STATUS "Using local CPR from external/cpr")
    add_subdirectory(${cpr_SOURCE_DIR})
else()
    message(FATAL_ERROR "Cannot proceed: CPR not found in external/cpr.")
endif()

set(ftxui_SOURCE_DIR ${EXTERNAL_DIR}/ftxui)
if(EXISTS ${ftxui_SOURCE_DIR}/CMakeLists.txt)
    message(STATUS "Using local FTXUI from external/ftxui")
    add_subdirectory(${ftxui_SOURCE_DIR})
else()
    message(FATAL_ERROR "Cannot proceed: FTXUI not found in external/ftxui.")
endif()

add_subdirectory(src)
