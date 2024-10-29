# uart_device/CMakeLists.txt
add_library(uart_device SHARED src/UARTSerial.cpp
)

target_include_directories(uart_device PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
