# tests/setup_gtest.cmake
find_package(GTest REQUIRED)

# Include GTest directories
include_directories(${GTEST_INCLUDE_DIRS})

find_library(GMOCK_LIB NAMES gmock PATHS /usr/lib)
find_library(GMOCK_MAIN_LIB NAMES gmock_main PATHS /usr/lib)
find_library(GTEST_MAIN_LIB NAMES gtest_main PATHS /usr/lib)
find_library(GTEST_LIB NAMES gtest PATHS /usr/lib)

# Link Google Test libraries
function(link_gtest TARGET)
    target_link_libraries(${TARGET} PRIVATE ${GTEST_LIB} ${GTEST_MAIN_LIB} ${GMOCK_LIB} ${GMOCK_MAIN_LIB})
endfunction()
