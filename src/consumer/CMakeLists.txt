cmake_minimum_required(VERSION 3.10)

project(netstream-consumer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(netstream-consumer
    main.cpp
    Consumer.cpp
    KafkaConsumer.cpp
    DatabaseWriter.cpp
)

target_include_directories(netstream-consumer PRIVATE
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(netstream-consumer PRIVATE
    Threads::Threads
    rdkafka++
    rdkafka
    pqxx
    pq
)
