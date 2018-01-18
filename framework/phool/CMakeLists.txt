# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phool CXX C)

# Put executables into a bin
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{OFFLINE_MAIN}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{OFFLINE_MAIN}/lib)

# ROOT dict generation
add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/PHBase_dict.cc
  COMMAND rootcint
  ARGS -f ${PROJECT_SOURCE_DIR}/PHBase_dict.cc -c
	${PROJECT_SOURCE_DIR}/PHFlag.h
	${PROJECT_SOURCE_DIR}/phool.h
	${PROJECT_SOURCE_DIR}/PHObject.h
	${PROJECT_SOURCE_DIR}/PHTimeServer.h
	${PROJECT_SOURCE_DIR}/PHTimeStamp.h
	${PROJECT_SOURCE_DIR}/recoConsts.h
	${PROJECT_SOURCE_DIR}/PHBase_LinkDef.h
	)

# source code
include_directories(${PROJECT_SOURCE_DIR}/)
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM dist_headers "${PROJECT_SOURCE_DIR}/PHBase_LinkDef.h")

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

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ROOT_LINK}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{OFFLINE_MAIN}/include ${ROOT_CFLAGS}")
add_library(phool SHARED ${sources} ${headers} ${PROJECT_SOURCE_DIR}/PHBase_dict.cc)
target_link_libraries(phool ${ROOT_LINK})


#install(TARGETS phool DESTINATION $ENV{OFFLINE_MAIN}/lib)
install(FILES ${dist_headers} DESTINATION $ENV{OFFLINE_MAIN}/include/phool/)
