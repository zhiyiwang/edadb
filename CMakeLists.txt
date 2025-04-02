# Minimum CMake version required
cmake_minimum_required(VERSION 3.12)

# Set the project name and language
project(edadb LANGUAGES CXX)

# Enable C++17 (or higher) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# make flags
set(CMAKE_BUILD_TYPE Debug) # debug mode 

# output directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Enable C++17 (or higher) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the thirdparty directory for external libraries
add_subdirectory(src)