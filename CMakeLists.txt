cmake_minimum_required(VERSION 3.19)

project(neuron_extension_settings VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED IMPORTED_TARGET libmodbus)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/modbus_client.cpp
    src/neuron_bus.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    PkgConfig::LIBMODBUS
    Boost::program_options
)
