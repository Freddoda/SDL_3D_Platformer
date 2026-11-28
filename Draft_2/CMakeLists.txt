cmake_minimum_required(VERSION 3.10.0)

project(3D LANGUAGES CXX)

add_executable(3D 3d_draft_2.cpp)

find_package(SDL3 REQUIRED)
target_link_libraries(3D PRIVATE SDL3::SDL3)

include(CTest)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)