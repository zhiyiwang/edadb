
###  SOCI 
# set SOCI compile options, then unset after compile
set(WITH_SQLITE3 ON CACHE BOOL "Enable SQLite3 support in SOCI")

# disable static and shared library
set(SOCI_SHARD OFF CACHE BOOL "Disable shared library in SOCI")
set(SOCI_STATIC OFF CACHE BOOL "Disable static library in SOCI")
set(SOCI_TESTS OFF CACHE BOOL "Disable tests in SOCI")
set(WITH_BOOST OFF CACHE BOOL "Disable Boost support in SOCI")

set(SOCI_EMPTY OFF CACHE BOOL "Disable empty backend in SOCI")
set(WITH_DB2 OFF CACHE BOOL "Disable DB2 support in SOCI")
set(WITH_FIREBIRD OFF CACHE BOOL "Disable Firebird support in SOCI")
set(WITH_MYSQL OFF CACHE BOOL "Disable MySQL support in SOCI")
set(WITH_ODBC OFF CACHE BOOL "Disable ODBC support in SOCI")
set(WITH_ORACLE OFF CACHE BOOL "Disable Oracle support in SOCI")
set(WITH_POSTGRESQL OFF CACHE BOOL "Disable PostgreSQL support in SOCI")

# Bring the populated content into the build
set(soci_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/soci CACHE PATH "SOCI source directory")
set(soci_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/soci CACHE PATH "SOCI binary directory")
message(STATUS "soci_SOURCE_DIR[3ed-party] @ " ${soci_SOURCE_DIR})
message(STATUS "soci_BINARY_DIR[3ed-party] @ " ${soci_BINARY_DIR})

add_subdirectory(${soci_SOURCE_DIR} ${soci_BINARY_DIR})



# # Use FetchContent module to download and populate the third-party dependencies
# include(FetchContent)
#
# ## 1. SOCI repository, version v4.0.3
# FetchContent_Declare(
#     soci
#     GIT_REPOSITORY https://github.com/SOCI/soci.git # SOCI repository
#     GIT_TAG v4.0.3                                  # SOCI version v4.0.3
#     SOURCE_DIR ${THIRDPARTY_SOURCE_DIR}/soci        # SOCI source directory
#     BINARY_DIR ${THIRDPARTY_BINARY_DIR}/soci        # SOCI binary directory
#     OVERRIDE_FIND_PACKAGE
# )
# 
# # avoid download again
# FetchContent_GetProperties(soci)
# if(NOT soci_POPULATED)
#   FetchContent_Populate(soci)
# 
#   # set SOCI compile options, then unset after compile
#   set(WITH_SQLITE3 ON CACHE BOOL "Enable SQLite3 support in SOCI")
# 
#   set(WITH_BOOST OFF CACHE BOOL "Disable Boost support in SOCI")
#   set(WITH_TESTS OFF CACHE BOOL "Disable tests in SOCI")
#   set(WITH_TOOLS OFF CACHE BOOL "Disable tools in SOCI")
# 
#   set(SOCI_EMPTY OFF CACHE BOOL "Disable empty backend in SOCI")
#   set(WITH_DB2 OFF CACHE BOOL "Disable DB2 support in SOCI")
#   set(WITH_FIREBIRD OFF CACHE BOOL "Disable Firebird support in SOCI")
#   set(WITH_MYSQL OFF CACHE BOOL "Disable MySQL support in SOCI")
#   set(WITH_ODBC OFF CACHE BOOL "Disable ODBC support in SOCI")
#   set(WITH_ORACLE OFF CACHE BOOL "Disable Oracle support in SOCI")
#   set(WITH_POSTGRESQL OFF CACHE BOOL "Disable PostgreSQL support in SOCI")
#   
#   # Bring the populated content into the build
#   add_subdirectory(${soci_SOURCE_DIR} ${soci_BINARY_DIR})
# endif()
# 
# 
# # find_package get soci
# FetchContent_MakeAvailable(soci)
# message(STATUS "soci_SOURCE_DIR @ " ${soci_SOURCE_DIR})
# message(STATUS "soci_BINARY_DIR @ " ${soci_BINARY_DIR})