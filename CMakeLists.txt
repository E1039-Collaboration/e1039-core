# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(e1039-core CXX C)

# source code
#include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")

# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --prefix  OUTPUT_VARIABLE ROOT_PREFIX  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --cflags  OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs    OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER     OUTPUT_STRIP_TRAILING_WHITESPACE)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_MAJOR_VER)

set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS}           -I$ENV{OFFLINE_MAIN}/include ${ROOT_CFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L$ENV{OFFLINE_MAIN}/lib     ${ROOT_LINK}"  )

# add sub directories
add_subdirectory(packages/global_consts)
add_subdirectory(packages/jobopts_svc)
add_subdirectory(packages/geom_svc)
add_dependencies(jobopts_svc global_consts)
add_dependencies(geom_svc    jobopts_svc)

add_subdirectory(framework/phool)
#add_subdirectory(framework/ffaobjects)
#add_subdirectory(framework/fun4all)
#add_subdirectory(interface_main)
#add_subdirectory(online/onlmonserver
#add_subdirectory(online/decoder_maindaq
#add_subdirectory(packages/Half
#add_subdirectory(packages/vararray
#add_subdirectory(database/pdbcal/base
#add_subdirectory(database/PHParameter
#add_subdirectory(packages/PHGeometry
#add_subdirectory(packages/PHField
#add_subdirectory(generators/phhepmc
#add_subdirectory(generators/PHPythia8
#add_subdirectory(simulation/g4decayer
#add_subdirectory(simulation/g4gdml
#add_subdirectory(simulation/g4main
#add_subdirectory(simulation/g4detectors
#add_subdirectory(simulation/g4dst
#add_subdirectory(simulation/g4eval
#add_subdirectory(packages/db2g4
#add_subdirectory(packages/PHGenFitPkg/GenFitExp
#add_subdirectory(packages/PHGenFitPkg/PHGenFit
#add_subdirectory(packages/ktracker
#add_subdirectory(module_example

# Install the pcm files in case of ROOT 6.
#execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_MAJOR_VER)
#string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
#if (ROOT_VER GREATER 5)
#   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
#   add_dependencies(install_pcm jobopts_svc geom_svc)
#endif()
