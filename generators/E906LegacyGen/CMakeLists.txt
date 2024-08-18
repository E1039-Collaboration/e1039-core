# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(E906LegacyGen CXX C)

# source code
include_directories("${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.C)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

add_custom_command (
  OUTPUT SQPrimaryGen_Dict.C
  COMMAND rootcint
  ARGS -f SQPrimaryGen_Dict.C -noIncludePaths -inlineInputHeader -c -p 
  -I${PROJECT_SOURCE_DIR}/
  ${PROJECT_SOURCE_DIR}/SQPrimaryParticleGen.h
  ${PROJECT_SOURCE_DIR}/SQPileupGen.h
  ${PROJECT_SOURCE_DIR}/SQPrimaryGenLinkDef.h
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GEANT4_PREFIX}/include/ ")

 # LHAPDF
find_program(LHAPDFCONF "lhapdf-config")
if(LHAPDFCONF)
  message("-- Detecting LHAPDF:  found at ${LHAPDFCONF}")
elseif(NOT(LIBONLY))
  message(FATAL_ERROR "-- Detecting LHAPDF:  not found")
endif()
execute_process(COMMAND lhapdf-config --prefix OUTPUT_VARIABLE LHAPDF_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND lhapdf-config --cflags OUTPUT_VARIABLE LHAPDF_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND lhapdf-config --libs   OUTPUT_VARIABLE LHAPDF_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${LHAPDF_PREFIX}/include/ ")

add_library(SQPrimaryGen SHARED
  SQPrimaryParticleGen.C
  SQPileupGen.C
  SQPrimaryGen_Dict.C	
)

target_link_libraries(SQPrimaryGen -L./
  -lgsl
  -lgslcblas
  -lphool
  -lSubsysReco
  -lfun4all
  -lUtilAna
  -lboost_iostreams
  -lphhepmc
  -lHepMC
  -lphg4hit
  -lg4testbench
  -lpythia8
  -lSQPrimaryVtxGen
  ${LHAPDF_LINK}
)

install(TARGETS SQPrimaryGen DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  SQPrimaryParticleGen.h
  SQPileupGen.h
)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# The same method as "database/pdbcal/base".
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm SQPrimaryGen)
endif()
