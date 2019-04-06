# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phool CXX C)

# ROOT dict generation
add_custom_command (
  OUTPUT PHBase_dict.cc
  COMMAND rootcint
  ARGS -f PHBase_dict.cc -c
	${PROJECT_SOURCE_DIR}/PHFlag.h
	${PROJECT_SOURCE_DIR}/phool.h
	${PROJECT_SOURCE_DIR}/PHObject.h
	${PROJECT_SOURCE_DIR}/PHRandomSeed.h
	${PROJECT_SOURCE_DIR}/PHTimeServer.h
	${PROJECT_SOURCE_DIR}/PHTimeStamp.h
	${PROJECT_SOURCE_DIR}/recoConsts.h
	${PROJECT_SOURCE_DIR}/PHBase_LinkDef.h
	)

file(GLOB pcms "")
list(APPEND pcms "PHBase_dict_rdict.pcm")

# source code
include_directories(${PROJECT_SOURCE_DIR}/)
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{OFFLINE_MAIN}/include ${ROOT_CFLAGS} -std=c++11 ")
add_library(phool SHARED ${sources} PHBase_dict.cc)
target_link_libraries(phool ${ROOT_LINK})

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(TARGETS phool DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up ${pcms} ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm phool)
endif()

