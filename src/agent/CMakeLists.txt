cmake_minimum_required(VERSION 3.10)

project(netstream-agent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(netstream-agent
    main.cpp
    Agent.cpp
    TcpSender.cpp
)

target_include_directories(netstream-agent PRIVATE
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(netstream-agent PRIVATE
    Boost::system
    Threads::Threads
)
