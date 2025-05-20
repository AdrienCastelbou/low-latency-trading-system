cmake_minimum_required(VERSION 3.10)

project(trading_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra")
set(CMAKE_CPP_COMPILER g++)

# Common library
file(GLOB_RECURSE COMMON_SOURCES "src/common/*.cpp")
file(GLOB_RECURSE COMMON_HEADERS "include/common/*.hpp")
add_library(common_lib STATIC ${COMMON_SOURCES})
target_include_directories(common_lib PUBLIC include)

# Exchange library
file(GLOB_RECURSE EXCHANGE_SOURCES "src/exchange/*.cpp")
file(GLOB_RECURSE EXCHANGE_HEADERS "include/exchange/*.hpp")
add_library(exchange_lib STATIC ${EXCHANGE_SOURCES})
target_include_directories(exchange_lib PUBLIC include)
target_link_libraries(exchange_lib PRIVATE common_lib)

# Trading library
file(GLOB_RECURSE TRADING_SOURCES "src/trading/*.cpp")
file(GLOB_RECURSE TRADING_HEADERS "include/trading/*.hpp")
add_library(trading_lib STATIC ${TRADING_SOURCES})
target_include_directories(trading_lib PUBLIC include)
target_link_libraries(trading_lib PRIVATE common_lib)

# Exchange executable
add_executable(exchange_engine exchangeMain.cpp)
target_link_libraries(exchange_engine PRIVATE exchange_lib)

# Trading executable
add_executable(trading_engine src/trading/tradingMain.cpp)
target_link_libraries(trading_engine PRIVATE trading_lib)


