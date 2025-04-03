# add executable program
# all cpp files in this directory will be compiled to the executable
file(GLOB SOURCES "*.cpp")

add_executable(edadb main.cpp ${SOURCES})

# include header files
target_include_directories(edadb PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/edadb)

# link libraries
target_link_libraries(edadb PRIVATE
    sqlite3 fmt::fmt)


add_subdirectory(backend)