# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(geom_svc CXX C)

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
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -noIncludePaths -inlineInputHeader -c -p -I${PROJECT_SOURCE_DIR}/ ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

# source code
include_directories("${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.C  ${PROJECT_SOURCE_DIR}/*.cxx  ${PROJECT_SOURCE_DIR}/*.cc)
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
target_link_libraries(geom_svc -ldb_svc -lphool ${ROOT_LINK})

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(TARGETS geom_svc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up ${pcms} ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm geom_svc)
endif()
