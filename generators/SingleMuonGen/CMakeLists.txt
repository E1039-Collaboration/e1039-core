# Setup the project
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(SingleMuonGen CXX C)

# source code
include_directories("${PROJECT_SOURCE_DIR}/")
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

# Geant4
find_program(G4CONF "geant4-config")
if(G4CONF)
  message("-- Detecting Geant4:  found at ${G4CONF}")
else()
  message(FATAL_ERROR "-- Detecting Geant4:  not found")
endif()
#find_package(Geant4 REQUIRED vis_all)
execute_process(COMMAND geant4-config --prefix OUTPUT_VARIABLE GEANT4_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)

# Warning suppression
string(REGEX REPLACE "-W[a-z-]*" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})
string(REPLACE "-pedantic" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})

# ROOT dict generation
file(GLOB dicts "")
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${DictC})
	list(APPEND dicts ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -noIncludePaths -inlineInputHeader -c -p -I${PROJECT_SOURCE_DIR} -I${GEANT4_PREFIX}/include/Geant4 ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

# source code
include_directories("${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cxx)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GEANT4_CFLAGS} ${ROOT_CFLAGS} -std=c++17")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lphool")
add_library(singlemuongen SHARED ${sources} ${dicts})
target_link_libraries(singlemuongen -lfun4all -lphool -lSubsysReco -lg4testbench -lphg4hit ${ROOT_LINK})

message(STATUS ${PROJECT_SOURCE_DIR} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(TARGETS singlemuongen DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# Install the pcm files in case of ROOT 6.
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm singlemuongen)
endif()
