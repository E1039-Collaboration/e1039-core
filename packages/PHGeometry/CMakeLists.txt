# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phgeom CXX C)

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

## Geant4
#find_program(G4CONF "geant4-config")
#if(G4CONF)
#  message("-- Detecting Geant4:  found at ${G4CONF}")
#else()
#  message(FATAL_ERROR "-- Detecting Geant4:  not found")
#endif()
#find_package(Geant4 REQUIRED vis_all)
#execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)
#
## Warning suppression
#string(REGEX REPLACE "-W[a-z-]*" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})
#string(REPLACE "-pedantic" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -I$ENV{OFFLINE_MAIN}/include/ ${GEANT4_CFLAGS} ${ROOT_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")

add_library(phgeom_io SHARED
  ${PROJECT_SOURCE_DIR}/PHGeomIOTGeo.cc
  PHGeomIOTGeo_Dict.C
)
target_link_libraries(phgeom_io -L$ENV{OFFLINE_MAIN}/lib ${ROOT_LINK} -lphool -lGeom)

add_library(phgeom SHARED
  ${PROJECT_SOURCE_DIR}/PHGeomTGeo.cc
  PHGeomTGeo_Dict.C
  ${PROJECT_SOURCE_DIR}/PHGeomFileImport.cc
  PHGeomFileImport_Dict.C
  ${PROJECT_SOURCE_DIR}/PHGeomUtility.cc
  PHGeomUtility_Dict.C 
)
target_link_libraries(phgeom -L./ -L$ENV{OFFLINE_MAIN}/lib -lSubsysReco -lphgeom_io )

install(TARGETS phgeom_io 				DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS phgeom 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/PHGeomTGeo.h
  ${PROJECT_SOURCE_DIR}/PHGeomIOTGeo.h
  ${PROJECT_SOURCE_DIR}/PHGeomUtility.h
)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



