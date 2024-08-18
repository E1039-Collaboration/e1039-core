# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(ktracker CXX C)

# source code
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
execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)

# Warning suppression
string(REGEX REPLACE "-W[a-z-]*" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})
string(REPLACE "-pedantic" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})

include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/../interface")
include_directories("${PROJECT_SOURCE_DIR}/../ktracker")
include_directories("${PROJECT_SOURCE_DIR}/../kfitter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GEANT4_CFLAGS} ${ROOT_CFLAGS} -std=c++17 -Wno-shadow")

add_library(sqgenfit SHARED ${sources})
target_link_libraries(sqgenfit -lgenfit2 -lktracker_interface -lphfield)

install(TARGETS sqgenfit 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(STATUS ${PROJECT_SOURCE_DIR}/ " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/GFFitter.h
  ${PROJECT_SOURCE_DIR}/GFTrack.h
  ${PROJECT_SOURCE_DIR}/GFField.h
  ${PROJECT_SOURCE_DIR}/GFMeasurement.h
)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# enable/disable test code build
option(BUILDTEST "Build the test code for the package" OFF)
if(BUILDTEST)
  file(GLOB tests ${PROJECT_SOURCE_DIR}/test/test*.cxx)
  foreach(test ${tests})
    string(REPLACE "${PROJECT_SOURCE_DIR}/test/" "" test ${test})
    string(REPLACE ".cxx" "" test ${test})
    message(STATUS "Adding tests to the compile list: ${test}")

    add_executable(${test} ${PROJECT_SOURCE_DIR}/test/${test}.cxx)
    target_link_libraries(${test} ${PROJECT_BINARY_DIR}/libsqgenfit.so -lgenfit2 -lktracker -lktracker_interface -lphfield -lphfield_io -lGeom -lgeom_svc -ldb_svc -lphool "${ROOT_LINK}")
    add_dependencies(${test} sqgenfit)
    install(TARGETS ${test} DESTINATION ${PROJECT_SOURCE_DIR}/test)
  endforeach(test)
endif()
