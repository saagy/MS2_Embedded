
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico CACHE STRING "Board type")   # plain Raspberry Pi Pico (no Wi-Fi)

# Works with PICO_SDK_PATH set as an env var or by the VS Code Pico extension
if(NOT PICO_SDK_PATH AND DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

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

# ---- Hardware test: RGB LED only (Phase 1) ----
add_executable(test_rgb
    test_rgb.c
    rgb_led.c
)
target_link_libraries(test_rgb
    pico_stdlib
    hardware_pwm
)
pico_enable_stdio_usb(test_rgb 1)
pico_enable_stdio_uart(test_rgb 0)
pico_add_extra_outputs(test_rgb)

# ---- Hardware test: potentiometers + status LED (Phase 3) ----
add_executable(test_pots
    test_pots.c
    pot.c
    rgb_led.c
)
target_link_libraries(test_pots
    pico_stdlib
    hardware_adc
    hardware_pwm
)
pico_enable_stdio_usb(test_pots 1)
pico_enable_stdio_uart(test_pots 0)
pico_add_extra_outputs(test_pots)

# ---- Diagnostic: button + buzzer wiring ----
add_executable(test_io
    test_io.c
    rgb_led.c
)
target_link_libraries(test_io
    pico_stdlib
    hardware_pwm
)
pico_enable_stdio_usb(test_io 1)
pico_enable_stdio_uart(test_io 0)
pico_add_extra_outputs(test_io)

# ---- Hardware test: buzzer tones + button ----
add_executable(test_buzzer
    test_buzzer.c
    buzzer.c
    button.c
)
target_link_libraries(test_buzzer
    pico_stdlib
    hardware_pwm
)
pico_enable_stdio_usb(test_buzzer 1)
pico_enable_stdio_uart(test_buzzer 0)
pico_add_extra_outputs(test_buzzer)

# ---- Diagnostic: which pins are wired to GND ----
add_executable(test_scan test_scan.c)
target_link_libraries(test_scan pico_stdlib)
pico_enable_stdio_usb(test_scan 1)
pico_enable_stdio_uart(test_scan 0)
pico_add_extra_outputs(test_scan)

# ---- Hardware test: 12 V DC motors via L298N (Phase 5/6) ----
add_executable(test_motor
    test_motor.c
    motor.c
    button.c
    rgb_led.c
)
target_link_libraries(test_motor
    pico_stdlib
    hardware_pwm
)
pico_enable_stdio_usb(test_motor 1)
pico_enable_stdio_uart(test_motor 0)
pico_add_extra_outputs(test_motor)

# ---- Hardware test: laptop <-> Pico serial link (Phase 8) ----
add_executable(test_link
    test_link.c
    rgb_led.c
    button.c
)
target_link_libraries(test_link
    pico_stdlib
    hardware_pwm
)
pico_enable_stdio_usb(test_link 1)
pico_enable_stdio_uart(test_link 0)
pico_add_extra_outputs(test_link)

# ---- USB bench test: send a nine-cell board and read the solver decision ----
add_executable(test_minimax_usb test_minimax_usb.c minimax.c)
target_link_libraries(test_minimax_usb pico_stdlib)
pico_enable_stdio_usb(test_minimax_usb 1)
pico_enable_stdio_uart(test_minimax_usb 0)
pico_add_extra_outputs(test_minimax_usb)

# ---- Hardware test: SG90 servo on GP28 (5 V from the L298N) ----
add_executable(test_servo
    test_servo.c
    servo.c
    button.c
)
target_link_libraries(test_servo
    pico_stdlib
    hardware_pwm
)
pico_enable_stdio_usb(test_servo 1)
pico_enable_stdio_uart(test_servo 0)
pico_add_extra_outputs(test_servo)

# ---- Hardware test: ESP-01S Wi-Fi module on UART0 (GP0/GP1) ----
add_executable(test_esp test_esp.c)
target_link_libraries(test_esp
    pico_stdlib
    hardware_uart
)
pico_enable_stdio_usb(test_esp 1)
pico_enable_stdio_uart(test_esp 0)
pico_add_extra_outputs(test_esp)

# ---- Wi-Fi test: laptop -> ESP-01S -> Pico Minimax -> back to laptop ----
add_executable(test_wifi_minimax test_wifi_minimax.c minimax.c)
target_link_libraries(test_wifi_minimax
    pico_stdlib
    hardware_uart
)
pico_enable_stdio_usb(test_wifi_minimax 1)
pico_enable_stdio_uart(test_wifi_minimax 0)
pico_add_extra_outputs(test_wifi_minimax)
