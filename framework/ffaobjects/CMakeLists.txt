# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(ffaobjects CXX C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{OFFLINE_MAIN}/lib)

# ROOT dict generation
add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/libffaobjects_Dict.cc
  COMMAND rootcint
  ARGS -f ${PROJECT_SOURCE_DIR}/libffaobjects_Dict.cc -c
	-I$ENV{OFFLINE_MAIN}/include
  ${PROJECT_SOURCE_DIR}/FlagSave.h
  ${PROJECT_SOURCE_DIR}/FlagSavev1.h
  ${PROJECT_SOURCE_DIR}/RunHeader.h
  ${PROJECT_SOURCE_DIR}/SyncObject.h
  ${PROJECT_SOURCE_DIR}/SyncObjectv1.h
  ${PROJECT_SOURCE_DIR}/SyncObjectv2.h
  ${PROJECT_SOURCE_DIR}/EventHeader.h
  ${PROJECT_SOURCE_DIR}/EventHeaderv1.h
  ${PROJECT_SOURCE_DIR}/libffaobjects_LinkDef.h
	)

# source code
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM dist_headers "${PROJECT_SOURCE_DIR}/libffaobjects_Dict.cc")

# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --prefix OUTPUT_VARIABLE ROOT_PREFIX  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L$ENV{OFFLINE_MAIN}/lib -lphool")
add_library(ffaobjects SHARED ${sources} ${headers} ${PROJECT_SOURCE_DIR}/libffaobjects_Dict.cc)
target_link_libraries(ffaobjects -L$ENV{OFFLINE_MAIN}/lib -lphool)


install(FILES ${dist_headers} DESTINATION $ENV{OFFLINE_MAIN}/include/ffaobjects/)
