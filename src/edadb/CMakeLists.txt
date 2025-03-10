# add executable program
# all cpp files in this directory will be compiled to the executable
file(GLOB TEST_FILES "*.cpp")

# Create the executable using the list of source files
add_executable(interface_test ${TEST_FILES})

# Link SOCI, SQLite3, duckdb libraries to the executable
target_link_libraries(interface_test soci_core soci_sqlite3 sqlite3 duckdb duckdb_static pthread dl)

# Specify the include directories for SOCI and SQLite3
message(STATUS "SOCI_SOURCE_DIR[interface_test]@ " ${soci_SOURCE_DIR})
target_include_directories(interface_test PRIVATE ${soci_SOURCE_DIR}/include)