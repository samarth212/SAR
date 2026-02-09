cmake_minimum_required(VERSION 3.16)
project(main LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(main
    main.cpp
    data_parser.cpp
    anomaly_detector.cpp
    priceAnomaly.cpp
    volumeAnomaly.cpp
)
target_link_libraries(main PRIVATE Boost::boost OpenSSL::SSL OpenSSL::Crypto)
