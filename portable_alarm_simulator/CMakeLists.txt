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

project(portable_alarm_simulator C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# pico_set_program_name(portable_alarm_simulator "portable_alarm_simulator")
# pico_set_program_version(portable_alarm_simulator "0.1")

# Modify the below lines to enable/disable output over UART/USB
# pico_enable_stdio_uart(portable_alarm_simulator 0)
# pico_enable_stdio_usb(portable_alarm_simulator 1)

# Add executable. Default name is the project name, version 0.1

add_executable(portable_alarm_simulator_background
        main.c 
        setup/setup.c 
        setup/buzzer/buzzer.c
        setup/led/led.c
        setup/display/display.c
        utils/ssd1306_i2c.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        setup/access_point/access_point.c
        )

target_include_directories(portable_alarm_simulator_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        ${CMAKE_CURRENT_LIST_DIR}/setup ${CMAKE_CURRENT_LIST_DIR}/utils 
        )

target_link_libraries(portable_alarm_simulator_background
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib hardware_pwm hardware_i2c hardware_clocks hardware_gpio
        )
# You can change the address below to change the address of the access point
pico_configure_ip4_address(portable_alarm_simulator_background PRIVATE
        CYW43_DEFAULT_IP_AP_ADDRESS 192.168.4.1
        )
pico_add_extra_outputs(portable_alarm_simulator_background)




add_executable(portable_alarm_simulator_poll
        main.c 
        setup/setup.c 
        setup/buzzer/buzzer.c
        setup/led/led.c
        setup/display/display.c
        utils/ssd1306_i2c.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        setup/access_point/access_point.c
        )
target_include_directories(portable_alarm_simulator_poll PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        ${CMAKE_CURRENT_LIST_DIR}/setup ${CMAKE_CURRENT_LIST_DIR}/utils 
        )
target_link_libraries(portable_alarm_simulator_poll
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        hardware_pwm hardware_i2c hardware_clocks hardware_gpio
        )
# You can change the address below to change the address of the access point
pico_configure_ip4_address(portable_alarm_simulator_poll PRIVATE
        CYW43_DEFAULT_IP_AP_ADDRESS 192.168.4.1
        )
pico_add_extra_outputs(portable_alarm_simulator_poll)
