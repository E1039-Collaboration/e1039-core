# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(interface_main CXX C)

# ROOT dict generation
file(GLOB dicts "")
file(GLOB pcms "")
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${DictC})
	string(REPLACE ".C" "_rdict.pcm" DictPcm ${DictC})
	list(APPEND dicts ${DictC})
	list(APPEND pcms  ${DictPcm})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC}  -noIncludePaths -inlineInputHeader -c -p -I${PROJECT_SOURCE_DIR} -I${PROJECT_SOURCE_DIR}/../simulation ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

# source code
include_directories("$ENV{MY_INSTALL}/include/" "$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/../simulation")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cxx)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O2 -std=c++0x -I$ENV{MY_INSTALL}/include/ -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L$ENV{OFFLINE_MAIN}/lib -lphool")

add_library(interface_main SHARED ${sources} ${dicts})
target_link_libraries(interface_main -L$ENV{MY_INSTALL}/lib -L$ENV{OFFLINE_MAIN}/lib -lphool)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(TARGETS interface_main DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up ${pcms} ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm interface_main)
endif()
