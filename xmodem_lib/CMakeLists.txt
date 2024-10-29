add_library(xmodem_lib SHARED
    src/XModem.cpp
)

target_include_directories(xmodem_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(xmodem_lib PUBLIC uart_device)
