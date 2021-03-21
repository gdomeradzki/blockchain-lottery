cmake_minimum_required(VERSION 3.5)

project(Lottery LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lottery main.cpp Node.cpp)
target_link_directories(Lottery PRIVATE .)
