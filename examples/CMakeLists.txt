set (CMAKE_MINIMUM_REQUIRED_VERSION 3.19)

project (ccmd_test C)
set (CMAKE_C_STANDARD 11)

add_executable (ccmd_test example.c)
if (!WIN32)
    target_link_libraries (ccmd_test pthread)
endif ()
