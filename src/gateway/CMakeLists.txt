cmake_minimum_required(VERSION 3.10)

project(netstream-gateway VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(netstream-gateway
    main.cpp
    TcpServer.cpp
    Gateway.cpp
    RuleEngine.cpp
    KafkaProducer.cpp
)

target_include_directories(netstream-gateway PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(netstream-gateway PRIVATE
    Boost::system
    Threads::Threads
    rdkafka++
    rdkafka
)
