# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(g4gdml CXX C)

# source code
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
  #find_package(Geant4 REQUIRED gdml vis_all)
  execute_process(COMMAND geant4-config --prefix OUTPUT_VARIABLE GEANT4_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4INTY_USE_XT -DG4VIS_USE_OPENGL -DG4UI_USE_TCSH -DG4UI_USE_XM -DG4VIS_USE_OPENGLXM -DG4VIS_USE_RAYTRACERX -DG4VIS_USE_OPENGLX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -I$ENV{XERCESCROOT}/include -I${ROOT_PREFIX}/include/ -I${GEANT4_PREFIX}/include/ -I${GEANT4_PREFIX}/include/Geant4/"
)

add_library(phg4gdml SHARED
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWrite.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWriteDefine.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWriteMaterials.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWriteSolids.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWriteSetup.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWriteParamvol.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLWriteStructure.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLUtility.cc
  ${PROJECT_SOURCE_DIR}/PHG4GDMLConfig.cc
)

target_link_libraries(phg4gdml -lphool -lSubsysReco -lfun4all -L$ENV{XERCESCROOT}/lib -lxerces-c ${GEANT4_LINK})

install(TARGETS phg4gdml 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  PHG4GDMLUtility.hh
  PHG4GDMLConfig.hh
)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



