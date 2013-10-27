cmake_minimum_required(VERSION 2.6)
project(MEDIAKHAN)

include(CheckIncludeFiles)

include_directories(/usr/local/include/)
include_directories(/usr/local/include/fuse)
include_directories(/usr/local/include/osxfuse/fuse)
include_directories(/opt/local/include/fuse)

link_directories(/usr/local/lib)
link_directories(/opt/local/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -stdlib=libstdc++")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pg")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

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

find_library(BDB_FOUND db)
if(BDB_FOUND)
	message("found bdb")
endif()
check_include_files(db_cxx.h BDB_FOUND)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
set(SOURCES khan.cpp; localizations.cpp; database.cpp; utils.cpp; log.cpp)

if(APPLE)
  set(LIBS osxfuse)
else()
  set(LIBS fuse)
endif()

if(VOLDEMORT_FOUND)
  list(APPEND SOURCES voldemort.cpp)
  list(APPEND LIBS voldemort)
endif()

if(REDIS_FOUND)
  list(APPEND SOURCES redis.cpp)
  list(APPEND LIBS hiredis)
endif()

if(BDB_FOUND)
  list(APPEND SOURCES bdb.cpp)
  list(APPEND LIBS db)
endif()

if(VOLDEMORT_FOUND OR REDIS_FOUND OR BDB_FOUND)
  message("you have a db, congratz")
else()
  message("you must have hiredis (the redis c client)  or voldemort (the c client) installed to compile this filesystem")
endif()
add_executable(khan ${SOURCES})
target_link_libraries(khan ${LIBS})
target_link_libraries(khan ${PYTHON_LIBRARIES})
