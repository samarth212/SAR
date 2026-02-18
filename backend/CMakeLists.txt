cmake_minimum_required(VERSION 3.16)
project(main LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(anomalies)

add_executable(main
    main.cpp
    data_parser.cpp
    api.cpp
    socket.cpp
)
target_link_libraries(main PRIVATE anomalies Boost::boost OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
