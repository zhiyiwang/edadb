# add executable program
add_executable(edadb main.cpp)

# Link SOCI and SQLite3 libraries to the executable
target_link_libraries(edadb soci_core soci_sqlite3 sqlite3)

# Specify the include directories for SOCI and SQLite3
message(STATUS "SOCI_SOURCE_DIR[edadb]@ " ${soci_SOURCE_DIR})
target_include_directories(edadb PRIVATE ${soci_SOURCE_DIR}/include)