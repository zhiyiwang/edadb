file(GLOB SQLITE_BACKEND_SOURCES *.cpp)

if(SQLITE_BACKEND_SOURCES)
    add_library(sqlite_backend ${SQLITE_BACKEND_SOURCES})
    target_include_directories(sqlite_backend PUBLIC ${CMAKE_SOURCE_DIR}/include)
endif()