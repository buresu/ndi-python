cmake_minimum_required(VERSION 3.1)

project(NDIlib VERSION 3.8.0)

set(PYBIND11_CPP_STANDARD -std=c++17)

add_subdirectory("lib/pybind11")

pybind11_add_module(NDIlib SHARED ${CMAKE_SOURCE_DIR}/src/main.cpp)

target_link_libraries(NDIlib PRIVATE pybind11::module ndi)
