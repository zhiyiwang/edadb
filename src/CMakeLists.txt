## # store thirdparty in a separate directory, fetch_base_dir
## include (FetchContent)
## set(FETCHCONTENT_QUIET off)
## 
## # get real path of fetch_base_dir
## get_filename_component(fetch_base_dir "./thirdparty/" 
##                        REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
## set(FETCHCONTENT_BASE_DIR ${fetch_base_dir})
## message(STATUS "edadb download thirdparty @ " ${fetch_base_dir})


# download 3rd party source code 
set(THIRDPARTY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
message(STATUS "THIRDPARTY_SOURCE_DIR @ " ${THIRDPARTY_SOURCE_DIR})

# compile 3rd party binaries
set(THIRDPARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
message(STATUS "THIRDPARTY_BINARY_DIR @ " ${THIRDPARTY_BINARY_DIR})


add_subdirectory(thirdparty)

add_subdirectory(edadb)