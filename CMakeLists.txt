set (CMAKE_MINIMUM_REQUIRED_VERSION 3.19)
project (ccmd C)

set (CMAKE_C_STANDARD 11)

set (CMAKE_BUILD_TYPE Debug)

add_executable(ccmd_test "source/ccmd.c")

# if on linux, depend on pthreads
if (UNIX AND NOT APPLE)
    target_link_libraries(ccmd_test pthread)
    # set build directory to ./build_deb
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_deb)
else()
    # set build directory to ./build
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_win)
endif()
