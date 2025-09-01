cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(rp2040_netsupply C CXX ASM)
pico_sdk_init()

add_executable(rp2040_netsupply
    src/main.c
    src/bridge.c
    src/webserver.c
    src/telnet.c
    src/ws2812.c
    src/config.c
    src/ethernet.c
)
target_include_directories(rp2040_netsupply PRIVATE include)
target_link_libraries(rp2040_netsupply pico_stdlib hardware_spi hardware_uart lwip)

pico_add_extra_outputs(rp2040_netsupply)
