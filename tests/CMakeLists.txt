
include(${CMAKE_CURRENT_SOURCE_DIR}/setup_gtest.cmake)

add_executable(unit_tests
    gtest/XModemTests.cpp
)

target_link_libraries(unit_tests PRIVATE xmodem_lib pthread)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/xmodem_lib/include ${CMAKE_SOURCE_DIR}/tests/gmock)

link_gtest(unit_tests)

add_test(NAME XModemTests COMMAND unit_tests)