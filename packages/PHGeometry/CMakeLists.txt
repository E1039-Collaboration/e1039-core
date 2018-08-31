# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phgeom CXX C)

# source code
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

# ROOT dict generation
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
file(GLOB dicts "")
foreach(LinkDefh ${LinkDefhs})
	message("LinkDefh: " ${LinkDefh})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.cc" DictC ${DictC})
	list(APPEND dicts ${DictC})
  message("Dicth: " ${Dicth})
  message("DictC: " ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -c -I$ENV{OFFLINE_MAIN}/include ${Dicth} ${LinkDefh} DEPENDS ${LinkDefh})
endforeach(LinkDefh)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS} -std=c++0x ")

add_library(phgeom SHARED
  ${PROJECT_SOURCE_DIR}/PHGeomTGeo.cc
  ${PROJECT_SOURCE_DIR}/PHGeomFileImport.cc
  ${PROJECT_SOURCE_DIR}/PHGeomUtility.cc
  PHGeomTGeo_Dict.cc
  PHGeomFileImport_Dict.cc 
  PHGeomUtility_Dict.cc 
)

add_library(phgeom_io SHARED
  ${PROJECT_SOURCE_DIR}/PHGeomIOTGeo.cc
  PHGeomIOTGeo_Dict.cc 
)

target_link_libraries(phgeom_io -L./ -L$ENV{OFFLINE_MAIN}/lib -lphool -L${ROOT_PREFIX}/lib -lGeom)

target_link_libraries(phgeom -L./ -lphgeom_io -L$ENV{OFFLINE_MAIN}/lib -lSubsysReco)

install(TARGETS phgeom_io	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS phgeom	  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  PHGeomTGeo.h
  PHGeomIOTGeo.h
  PHGeomUtility.h
)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



