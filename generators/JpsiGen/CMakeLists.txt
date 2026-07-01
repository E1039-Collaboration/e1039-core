cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(JpsiGen CXX C)

# source code
file(GLOB          sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB     dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})

# ROOT dict generation
add_custom_command (
  OUTPUT JpsiGen_Dict.cc
  COMMAND rootcint
  ARGS -f JpsiGen_Dict.cc -noIncludePaths -inlineInputHeader -c -p
    -I${PROJECT_SOURCE_DIR}/ -I${ROOT_PREFIX}/include/
    ${dist_headers} ${non_dist_headers}
)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS} ")

# Geant4
#find_program(G4CONF "geant4-config")
#if(G4CONF)
#  message("-- Detecting Geant4:  found at ${G4CONF}")
#else()
#  message(FATAL_ERROR "-- Detecting Geant4:  not found")
#endif()
##find_package(Geant4 REQUIRED gdml vis_all)
#execute_process(COMMAND geant4-config --prefix OUTPUT_VARIABLE GEANT4_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
#execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GEANT4_PREFIX}/include/ ")

add_library(JpsiGen SHARED  ${sources} JpsiGen_Dict.cc)
target_link_libraries(JpsiGen fun4all interface_main UtilAna SQPrimaryVtxGen)
#target_link_libraries(SQPrimaryGen -L./
#  -lgsl
#  -lgslcblas
#  -lphool
#  -lSubsysReco
#  -lfun4all
#  -lUtilAna
#  -lboost_iostreams
#  -lphhepmc
#  -lHepMC
#  -lphg4hit
#  -lg4testbench
#  -lpythia8
#  -lSQPrimaryVtxGen
#)

install(TARGETS JpsiGen  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
add_dependencies(install_pcm JpsiGen)
