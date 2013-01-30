cmake_minimum_required(VERSION 2.6)
project(MEDIAKHAN)

include_directories(/usr/local/include/fuse)
link_directories(/usr/local/lib)

add_definitions(-D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26)

add_executable(khan khan.cpp redis.cpp voldemort.cpp log.cpp)

target_link_libraries(khan fuse voldemort rt hiredis)
