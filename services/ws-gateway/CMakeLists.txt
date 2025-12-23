add_executable(ws-gateway
  src/main.cpp
)

find_package(Boost REQUIRED COMPONENTS system json)

target_include_directories(ws-gateway PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(ws-gateway PRIVATE Boost::system Boost::json)
