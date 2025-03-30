# backend/sqlite/CMakeLists.txt

# If header-only, nothing needed here
# You can also group them as an interface lib if desired:
add_library(sqlite_headers INTERFACE)
target_include_directories(sqlite_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

