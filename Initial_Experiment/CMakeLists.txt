cmake_minimum_required(VERSION 3.10.0)

project(2.5D LANGUAGES CXX)

add_executable(2.5D initial.cpp)

find_package(SDL3 REQUIRED)
target_link_libraries(2.5D PRIVATE SDL3::SDL3)

include(CTest)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)