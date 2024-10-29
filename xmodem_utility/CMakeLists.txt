add_executable(xmodem_utility
    src/main.cpp
)

target_link_libraries(xmodem_utility PRIVATE
    xmodem_lib
)
