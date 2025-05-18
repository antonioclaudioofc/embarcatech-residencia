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

project(dma_adc_temperature C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(dma_adc_temperature main.c setup/setup.c setup/display/display.c setup/temperature_sensor/temperature_sensor.c utils/ssd1306_i2c.c)

pico_set_program_name(dma_adc_temperature "dma_adc_temperature")
pico_set_program_version(dma_adc_temperature "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(dma_adc_temperature 0)
pico_enable_stdio_usb(dma_adc_temperature 1)

# Add the standard library to the build
target_link_libraries(dma_adc_temperature
        pico_stdlib hardware_i2c hardware_adc hardware_dma)

# Add the standard include files to the build
target_include_directories(dma_adc_temperature PRIVATE
        ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/setup ${CMAKE_CURRENT_LIST_DIR}/setup/display ${CMAKE_CURRENT_LIST_DIR}/setup/temperature_sensor ${CMAKE_CURRENT_LIST_DIR}/utils
)

# Add any user requested libraries
target_link_libraries(dma_adc_temperature 
        
        )

pico_add_extra_outputs(dma_adc_temperature)

