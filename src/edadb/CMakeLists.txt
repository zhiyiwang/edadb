# add executable program
# all cpp files in this directory will be compiled to the executable
file(GLOB SOURCES "*.hpp" "*.cpp")

add_executable(edadb main.cpp ${SOURCES})

# Link SOCI and SQLite3 libraries to the executable
target_link_libraries(edadb PRIVATE sqlite3 fmt::fmt)

## # Specify the include directories for SOCI and SQLite3
## message(STATUS "SOCI_SOURCE_DIR[edadb]@ " ${soci_SOURCE_DIR})
## target_include_directories(edadb PRIVATE ${soci_SOURCE_DIR}/include)