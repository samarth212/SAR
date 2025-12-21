add_executable(hello-service
  src/main.cpp
)

find_package(Boost REQUIRED COMPONENTS system json)

target_include_directories(hello-service PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(hello-service PRIVATE Boost::system Boost::json)
