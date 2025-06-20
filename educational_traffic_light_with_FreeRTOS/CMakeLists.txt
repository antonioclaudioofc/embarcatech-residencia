# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(educational_traffic_light_with_FreeRTOS C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(educational_traffic_light_with_FreeRTOS src/main.c setup/setup.c setup/led/led.c)

pico_set_program_name(educational_traffic_light_with_FreeRTOS "educational_traffic_light_with_FreeRTOS")
pico_set_program_version(educational_traffic_light_with_FreeRTOS "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(educational_traffic_light_with_FreeRTOS 0)
pico_enable_stdio_usb(educational_traffic_light_with_FreeRTOS 1)

# Caminho para o FreeRTOS
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel)

# Importa o FreeRTOS
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

target_sources(educational_traffic_light_with_FreeRTOS
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

# Add the standard library to the build
target_link_libraries(educational_traffic_light_with_FreeRTOS
        pico_stdlib
        FreeRTOS-Kernel
        )

# Add the standard include files to the build
target_include_directories(educational_traffic_light_with_FreeRTOS PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Add any user requested libraries
target_link_libraries(educational_traffic_light_with_FreeRTOS 
        
        )

pico_add_extra_outputs(educational_traffic_light_with_FreeRTOS)

