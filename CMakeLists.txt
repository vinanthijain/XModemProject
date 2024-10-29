cmake_minimum_required(VERSION 3.10)
project(XModemProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release/lib)

option(ENABLE_TESTS "Enable building tests" ON)

add_subdirectory(uart_device)
add_subdirectory(xmodem_lib)
add_subdirectory(xmodem_utility)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
