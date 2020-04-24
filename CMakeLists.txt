cmake_minimum_required(VERSION 3.1)

project(NDIlib VERSION 4.5.1)

set(PYBIND11_CPP_STANDARD -std=c++17)

add_subdirectory("lib/pybind11")

file(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

pybind11_add_module(NDIlib SHARED ${INCLUDES} ${SOURCES})

target_link_libraries(NDIlib PRIVATE pybind11::module ndi)

source_group("" FILES ${SOURCES} ${INCLUDES})
