# 3rd party download and build directory
set(THIRDPARTY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(THIRDPARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
message(STATUS "THIRDPARTY_SOURCE_DIR @ " ${THIRDPARTY_SOURCE_DIR})
message(STATUS "THIRDPARTY_BINARY_DIR @ " ${THIRDPARTY_BINARY_DIR})


add_subdirectory(thirdparty)
add_subdirectory(core)