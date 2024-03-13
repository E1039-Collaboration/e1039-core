# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phpythia8 CXX C)

# source code
include_directories("${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

add_custom_command (
  OUTPUT PHPythia8_Dict.C
  COMMAND rootcint
  ARGS -f PHPythia8_Dict.C -noIncludePaths -inlineInputHeader -c -p
  -I${PROJECT_SOURCE_DIR}/
  ${PROJECT_SOURCE_DIR}/PHPythia8.h
  ${PROJECT_SOURCE_DIR}/PHPy8GenTrigger.h
  ${PROJECT_SOURCE_DIR}/PHPy8ParticleTrigger.h
  ${PROJECT_SOURCE_DIR}/PHPythia8LinkDef.h
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
execute_process(COMMAND root-config --evelibs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS} ")

add_library(PHPythia8 SHARED
  PHPythia8.C
  PHPy8GenTrigger.C
  PHPy8ParticleTrigger.C
  PHPythia8_Dict.C
)

target_link_libraries(PHPythia8 -L./
  -lgsl
  -lgslcblas
  -lphool
  -lSubsysReco
  -lfun4all
  -lboost_iostreams
  -lpythia8
  -lphhepmc
  -lHepMC
)

install(TARGETS PHPythia8     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  PHPythia8.h
  PHPy8GenTrigger.h
  PHPy8ParticleTrigger.h
)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# The same method as "database/pdbcal/base".
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm PHPythia8)
endif()
