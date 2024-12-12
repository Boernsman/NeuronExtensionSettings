cmake_minimum_required(VERSION 3.19)

project(neuron_extension_settings VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(include ${LIBMODBUS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_libraries(${LIBMODBUS_LIBRARIES} ${Boost_LIBRARIES})

add_compile_definitions(${LIBMODBUS_CFLAGS_OTHER})
link_directories(${LIBMODBUS_LIBRARY_DIRS} )

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/modbus_client.cpp
    src/neuron_bus.cpp)