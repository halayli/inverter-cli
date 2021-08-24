cmake_minimum_required(VERSION 3.16)
project(inverter-cli)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_link_options(--ld-path=ld.lld-12)
endif()

add_compile_options(-stdlib=libc++)
link_libraries(c++ c++abi)

find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)

add_executable(inverter-cli
    src/main.cc
    src/holding_register.cc
    src/modbus_rtu.cc
    src/tcp_client.cc
    src/modbus_rtu_streamer.cc
)

target_link_libraries(inverter-cli
    spdlog::spdlog
    fmt::fmt
    nlohmann_json::nlohmann_json
    CLI11::CLI11
)

install(TARGETS inverter-cli RUNTIME DESTINATION bin)
