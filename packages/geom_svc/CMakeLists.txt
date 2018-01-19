# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(geom_svc CXX C)

# ROOT dict generation
file(GLOB dicts "")
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${DictC})
	list(APPEND dicts ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -c -I$ENV{OFFLINE_MAIN}/include ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

# source code
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.C  ${PROJECT_SOURCE_DIR}/*.cxx)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
add_library(geom_svc SHARED ${sources} ${dicts})
target_link_libraries(geom_svc -L$ENV{OFFLINE_MAIN}/lib -ljobopts_svc ${ROOT_LINK})

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(TARGETS geom_svc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)
