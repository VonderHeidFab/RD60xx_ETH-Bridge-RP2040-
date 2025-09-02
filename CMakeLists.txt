# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

### 📄 CMakeLists.txt

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
    src/sys_arch.c
    lib/fatfs/ff.c         # <--- FatFS Hauptdatei
    lib/fatfs/diskio.c     # <--- ggf. weitere FatFS-Dateien
)

target_include_directories(rp2040_netsupply PRIVATE include lib/fatfs)

target_link_libraries(rp2040_netsupply
    pico_stdlib
    hardware_spi
    hardware_uart
    hardware_pio
    hardware_flash
    pico_lwip
)

pico_enable_stdio_uart(rp2040_netsupply 1)
pico_enable_stdio_usb(rp2040_netsupply 0)
pico_generate_pio_header(rp2040_netsupply ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)
pico_add_extra_outputs(rp2040_netsupply)
