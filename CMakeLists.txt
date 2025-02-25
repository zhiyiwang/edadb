cmake_minimum_required(VERSION 3.6)

project(edadb)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable -Wno-pessimizing-move -Wno-reorder")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -std=c++20")

set(CMAKE_BUILD_TYPE Debug)

set(THIRD_PARTY_HEADER
    include/
    include/third_party
    include/third_party/hdr
    include/third_party/json
    include/third_party/msgpack
    include/third_party/rapidjson    
    include/third_party/spdlog
    include/third_party/sqlite
    include/third_party/unqlite
)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(SOURCE_FILES
    src/sqlite/sqlite3.c
    # src/edadbTest1.cpp
    
    src/edadbTest4.cpp
	
        # include/lib/edadb/edadb.hpp
        include/lib/edadb/edadbTest4.hpp
        include/lib/utils/MD5.hpp
        include/lib/utils/Singleton.hpp
)
list(APPEND CMAKE_MODULE_PATH "/home/ran/soci/cmake/modules")
# set(CMAKE_PREFIX_PATH "/home/ran/soci/cmake" ${CMAKE_PREFIX_PATH})

include(FindBoost)
find_package(Boost 1.65 COMPONENTS system filesystem program_options REQUIRED)

# include(FindSoci)
# find_package(Soci 3.2 COMPONENTS sqlite3 REQUIRED)

include_directories( ${Boost_INCLUDE_DIR} ${SOCI_INCLUDE_DIR} )

if(${SOCI_FOUND})
  link_directories(${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN})
else()
  message(WARNING "SOCI not found")
endif()

add_definitions(-DBUN_SQLITE -DFMT_HEADER_ONLY)

include_directories(include)
include_directories(${THIRD_PARTY_HEADER})
include_directories(${INCLUDE_DIR})
include_directories(${SOCI_INCLUDE})

add_library(libsoci_sqlite3 STATIC IMPORTED)


# find_library(SOCI_LIB NAME libsoci_sqlite3 HINTS ${SOCI_LIBRARY})
add_executable(edadb ${SOURCE_FILES})
target_link_libraries(edadb libsoci_sqlite3.a libsoci_core.a dl pthread)

