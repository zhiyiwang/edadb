cmake_minimum_required(VERSION 3.6)

project(demo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable -Wno-pessimizing-move -Wno-reorder")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

set(CMAKE_BUILD_TYPE Debug)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(SOURCE_FILES
    src/sqlite/sqlite3.c
    src/demo.cpp
)

# find_package(SOCI REQUIRED)

message(STATUS "SOCI_INCLUDE_DIRS: ${SOCI_INCLUDE_DIRS}")
message(STATUS "SOCI_LIBRARIES: ${SOCI_LIBRARIES}")


add_executable(demo ${SOURCE_FILES})
target_link_libraries(demo
    # soci_core
    libsoci_sqlite3.a
    libsoci_core.a
    dl
    pthread
)

# add_executable(demo ${SOURCE_FILES})
# target_link_libraries(demo dl pthread)
 

# add_library(libsoci_sqlite3 STATIC IMPORTED)
# add_library(libsoci_core STATIC IMPORTED include/blib/bun/QueryHelper.hpp)

# add_executable(demo ${SOURCE_FILES})
# target_link_libraries(demo libsoci_sqlite3.a /libsoci_core.a dl pthread)

