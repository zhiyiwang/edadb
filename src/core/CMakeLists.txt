# edadb include directory
set(EDADB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../include)

# all cpp files in this directory will be compiled to library
file(GLOB SOURCES "*.cpp")

# complile all cpp files to object files
add_library(edadb STATIC ${SOURCES})
#add_executable(edadb main.cpp ${SOURCES})

# include header files
target_include_directories(edadb PUBLIC
    ${EDADB_INCLUDE_DIR}
    ${EDADB_INCLUDE_DIR}/edadb
    ${EDADB_INCLUDE_DIR}/edadb/backend
    ${EDADB_INCLUDE_DIR}/edadb/backend/sqlite
    ${EDADB_INCLUDE_DIR}/edadb/backend/duckdb
)

# link libraries
target_link_libraries(edadb PUBLIC sqlite3)


add_subdirectory(backend)