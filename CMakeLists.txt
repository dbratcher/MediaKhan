cmake_minimum_required(VERSION 2.6)
project(MEDIAKHAN)

include(CheckIncludeFiles)

include_directories(/usr/local/include/fuse)
link_directories(/usr/local/lib)

add_definitions(-D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26)

find_library(VOLDEMORT_FOUND voldemort)
if(VOLDEMORT_FOUND)
	message("found voldemort")
endif()
check_include_files(voldemort/voldemort.h VOLDEMORT_FOUND)

find_library(REDIS_FOUND hiredis)
if(REDIS_FOUND)
	message("found redis")
endif()
check_include_files(hiredis/hiredis.h REDIS_FOUND)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if(VOLDEMORT_FOUND AND REDIS_FOUND)
	message("compiling for both redis and voldemort...")
	add_executable(khan khan.cpp redis.cpp voldemort.cpp log.cpp)
	target_link_libraries(khan fuse voldemort rt hiredis)
elseif(VOLDEMORT_FOUND)
	message("compiling for voldmort only...")
	add_executable(khan khan.cpp voldemort.cpp log.cpp)
	target_link_libraries(khan fuse voldemort)
elseif(REDIS_FOUND)
	message("compiling for redis only...")
	add_executable(khan khan.cpp redis.cpp log.cpp)
	target_link_libraries(khan fuse rt hiredis)
else()
	message("you must have hiredis (the redis c client)  or voldemort (the c client) installed to compile this filesystem")
endif() 
