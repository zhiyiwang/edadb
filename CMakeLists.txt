# Minimum CMake version required
cmake_minimum_required(VERSION 3.12)

# Set the project name and language
project(edadb LANGUAGES CXX)

# output directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# # set cmake module path
# # 1. The package should be installed by package manager, such as apt-get, yum, brew, etc.
# # 2. Config file FindXXX.cmake created in CMAKE_MODULE_PATH directory
# # 3. Then use find_package(XXX) to find the package
# include(FindPkgConfig)
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
# message(STATUS "CMAKE_MODULE_PATH @ " ${CMAKE_MODULE_PATH})


# Enable C++17 (or higher) for SOCI support
set(CMAKE_CXX_STANDARD 17)

# Add the thirdparty directory for external libraries
add_subdirectory(src)
