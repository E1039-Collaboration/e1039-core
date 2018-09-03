# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(g4eval CXX C)

# source code
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
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
execute_process(COMMAND root-config --evelibs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Geant4
  find_program(G4CONF "geant4-config")
  if(G4CONF)
    message("-- Detecting Geant4:  found at ${G4CONF}")
  else()
    message(FATAL_ERROR "-- Detecting Geant4:  not found")
  endif()
  find_package(Geant4 REQUIRED gdml vis_all)
  execute_process(COMMAND geant4-config --prefix OUTPUT_VARIABLE GEANT4_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)

# ROOT dict generation
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
file(GLOB dicts "")
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${DictC})
	list(APPEND dicts ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC}
  -c
  -I$ENV{OFFLINE_MAIN}/include 
  -I${GEANT4_PREFIX}/include 
  ${Dicth} ${LinkDefh} DEPENDS ${LinkDefh})
endforeach(LinkDefh)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -msse2 -std=c++11 -I$ENV{OFFLINE_MAIN}/include/ -I${GEANT4_PREFIX}/include/ -I${ROOT_PREFIX}/include/")

add_library(g4eval SHARED
  ${PROJECT_SOURCE_DIR}/PHG4DSTReader.cc
  PHG4DSTReader_Dict.C
)

target_link_libraries(g4eval -L./ -L$ENV{OFFLINE_MAIN}/lib -lfun4all -lg4dst)

install(TARGETS g4eval DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



