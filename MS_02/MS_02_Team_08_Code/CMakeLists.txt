
cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(tictactoe_lcd C CXX ASM)

pico_sdk_init()

add_executable(tictactoe_lcd
    main.c
    lcd.c
    pot.c
    rgb_led.c
    button.c
    buzzer.c
)

target_link_libraries(tictactoe_lcd
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_pwm
)

pico_enable_stdio_usb(tictactoe_lcd 1)
pico_enable_stdio_uart(tictactoe_lcd 0)

pico_add_extra_outputs(tictactoe_lcd)
