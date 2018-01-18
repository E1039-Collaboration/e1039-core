# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(interface_main CXX C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{OFFLINE_MAIN}/lib)

# ROOT dict generation
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
message("LinkDefhs: " ${LinkDefhs})
foreach(LinkDefh ${LinkDefhs})
	#string(REPLACE "${PROJECT_SOURCE_DIR}" "" LinkDefh ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	message("DictC: " ${DictC})
	message("Dicth: " ${Dicth})
	message("Linkh: " ${LinkDefh})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -c -I$ENV{OFFLINE_MAIN}/include ${Dicth} ${LinkDefh})
	add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/SQSpill_Dict.C
	COMMAND rootcint ARGS -f ${PROJECT_SOURCE_DIR}/SQSpill_Dict.C -c
	-I$ENV{OFFLINE_MAIN}/include
	${PROJECT_SOURCE_DIR}/SQSpill.h
	${PROJECT_SOURCE_DIR}/SQSpillLinkDef.h
	)
endforeach(LinkDefh)

# source code
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.C)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB dicts ${PROJECT_SOURCE_DIR}/*Dict.C)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L$ENV{OFFLINE_MAIN}/lib -lphool")
#message("sources: " ${sources})
message("dicts: " ${dicts})
add_library(interface_main SHARED ${sources} ${dicts})
target_link_libraries(interface_main -L$ENV{OFFLINE_MAIN}/lib -lphool)


file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
#list(REMOVE_ITEM dist_headers ${dicts})

install(FILES ${dist_headers} DESTINATION $ENV{OFFLINE_MAIN}/include/interface_main/)
