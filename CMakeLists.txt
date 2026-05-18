cmake_minimum_required(VERSION 3.10)

project(netstream VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/agent)
add_subdirectory(src/gateway)
add_subdirectory(src/consumer)