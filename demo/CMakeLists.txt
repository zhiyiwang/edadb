# add executable program
# all cpp files in this directory will be compiled to the executable demo
file(GLOB DEMO_SOURCES "*.cpp")
##file(GLOB DEMO_SOURCES "vec.cpp")

message(STATUS "CMakeLists.txt: demo dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMakeLists.txt: demo sources: ${DEMO_SOURCES}")

foreach(DEMO_SRC ${DEMO_SOURCES})
    # Set the project name using the filename without extension
    get_filename_component(DEMO_NAME ${DEMO_SRC} NAME_WE)
    add_executable(${DEMO_NAME} ${DEMO_SRC})

    target_compile_definitions(${DEMO_NAME} PUBLIC
        "_EDADB_DEBUG_TRACE_SQL_STMT_=1"
    )   
    
    # include header files
    target_include_directories(${DEMO_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # link libraries
    target_link_libraries(${DEMO_NAME} PRIVATE edadb)

    message(STATUS "Building demo: ${DEMO_NAME}")
endforeach()
