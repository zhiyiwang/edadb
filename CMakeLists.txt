# Minimum CMake version required
cmake_minimum_required(VERSION 3.12)

# Set the project name and language
project(edadb LANGUAGES CXX)

# make flags
set(CMAKE_BUILD_TYPE Debug) # debug mode 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG_SQLITE3_API")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG_SQLITE3_INSERT")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG_SQLITE3_SCAN")


# output directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Enable C++17 (or higher) 
set(CMAKE_CXX_STANDARD 17)

# Add the thirdparty directory for external libraries
add_subdirectory(src)