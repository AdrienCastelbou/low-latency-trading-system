cmake_minimum_required(VERSION 3.10)

project(trading_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set (CMAKE_CXX_FLAGS "-Werror -Wall -Wextra")
set(CMAKE_CPP_COMPILER g++)

file(GLOB_RECURSE SOURCES "src/*/*.cpp" "src/*/*/*.cpp" "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*/*.hpp" "include/*/*/*.hpp" "include/*.hpp")

add_library(trading_engine_lib STATIC ${SOURCES})
target_include_directories(trading_engine_lib PRIVATE include)
add_executable(trading_engine exchangeMain.cpp)
target_link_libraries(trading_engine PRIVATE trading_engine_lib)


