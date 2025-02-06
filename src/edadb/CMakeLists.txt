# add executable program
# all cpp files in this directory will be compiled to the executable
file(GLOB TEST_FILES "*.cpp")

# Create the executable using the list of source files
add_executable(soci_test ${TEST_FILES})

# Link SOCI and SQLite3 libraries to the executable
target_link_libraries(soci_test soci_core soci_sqlite3 sqlite3)

# Specify the include directories for SOCI and SQLite3
message(STATUS "SOCI_SOURCE_DIR[soci_test]@ " ${soci_SOURCE_DIR})
target_include_directories(soci_test PRIVATE ${soci_SOURCE_DIR}/include)