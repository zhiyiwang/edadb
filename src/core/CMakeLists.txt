# all cpp files in this directory will be compiled to library
file(GLOB SOURCES "*.cpp")

# complile all cpp files to object files
add_library(edadb ${SOURCES})
#add_executable(edadb main.cpp ${SOURCES})

# include header files
target_include_directories(edadb PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/edadb
    ${CMAKE_SOURCE_DIR}/include/edadb/backend
    ${CMAKE_SOURCE_DIR}/include/edadb/backend/sqlite
    ${CMAKE_SOURCE_DIR}/include/edadb/backend/duckdb
)

# link libraries
target_link_libraries(edadb PUBLIC
    sqlite3 fmt::fmt)


add_subdirectory(backend)