cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(CMAKE_CXX_STANDARD 98)
set(PROJECT_NAME InterruptDrivenButton)

set(${PROJECT_NAME}_BOARD uno)
# set(ARDUINO_CPU)
project(InterruptDrivenButton)

# Define additional source and header files or default arduino sketch files
set(${PROJECT_NAME}_SRCS src/InterruptDrivenButton.cpp)
set(${PROJECT_NAME}_HDRS InterruptDrivenButton.h src/InterruptDrivenButton.h)

generate_arduino_firmware(${PROJECT_NAME})


print_board_list()
print_programmer_list()
