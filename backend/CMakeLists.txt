cmake_minimum_required(VERSION 3.16)
project(alpaca_ws_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(alpaca_ws_demo alpaca_ws_demo.cpp)
target_link_libraries(alpaca_ws_demo PRIVATE Boost::boost OpenSSL::SSL OpenSSL::Crypto)
