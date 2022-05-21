cmake_minimum_required(VERSION 3.17)

project(NDIlib VERSION 5.1.1)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

add_subdirectory("lib/pybind11")

find_package(NDI REQUIRED)

file(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

source_group("" FILES ${SOURCES} ${INCLUDES})

pybind11_add_module(NDIlib SHARED ${INCLUDES} ${SOURCES})

target_include_directories(NDIlib PRIVATE ${NDI_INCLUDE_DIR})
target_link_directories(NDIlib PRIVATE ${NDI_LIBRARY_DIR})
target_link_libraries(NDIlib PRIVATE pybind11::module ${NDI_LIBS})

set_target_properties(NDIlib PROPERTIES SKIP_RPATH TRUE)
set_target_properties(NDIlib PROPERTIES BUILD_RPATH_USE_ORIGIN TRUE)
set_target_properties(NDIlib PROPERTIES SKIP_BUILD_RPATH FALSE)
set_target_properties(NDIlib PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(NDIlib PROPERTIES INSTALL_RPATH_USE_LINK_PATH FALSE)
if(APPLE)
  set_target_properties(NDIlib PROPERTIES INSTALL_RPATH "@loader_path")
elseif(UNIX)
  set_target_properties(NDIlib PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

# install
install(
  TARGETS NDIlib
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
)
if(WIN32)
  install(
    FILES
    "${NDI_DIR}/Bin/${NDI_ARCH}/Processing.NDI.Lib.${NDI_ARCH}.dll"
    "${NDI_LICENSE_DIR}/Processing.NDI.Lib.Licenses.txt"
    DESTINATION ${CMAKE_INSTALL_PREFIX}
  )
elseif(APPLE)
  install(
    FILES
    "${NDI_LIBRARY_DIR}/libndi.dylib"
    "${NDI_LICENSE_DIR}/libndi_licenses.txt"
    DESTINATION ${CMAKE_INSTALL_PREFIX}
  )
elseif(UNIX)
  file(GLOB DLL "${NDI_LIBRARY_DIR}/libndi.so*")
  install(
    FILES
    ${DLL}
    "${NDI_LICENSE_DIR}/libndi_licenses.txt"
    DESTINATION ${CMAKE_INSTALL_PREFIX}
  )
endif()
