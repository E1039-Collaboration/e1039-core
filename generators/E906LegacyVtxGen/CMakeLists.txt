# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(E906LegacyVtxGen CXX C)

# source codes and headers
include_directories("${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.C)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS} ")

#Generate shared library
add_library(SQPrimaryVtxGen SHARED
  SQPrimaryVertexGen.C
  SQBeamlineObject.C
)

target_link_libraries(SQPrimaryVtxGen -L./
  -lgsl
  -lgslcblas
  -lphool
  -lphgeom 
  -lUtilAna
  -lboost_iostreams
)

install(TARGETS SQPrimaryVtxGen DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  SQPrimaryVertexGen.h
  SQBeamlineObject.h
)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# enable/disable test code build
option(BUILDTEST "Build the test code for the package" OFF)
if(BUILDTEST)
  file(GLOB tests ${PROJECT_SOURCE_DIR}/test/test*.C)
  foreach(test ${tests})
    string(REPLACE "${PROJECT_SOURCE_DIR}/test/" "" test ${test})
    string(REPLACE ".C" "" test ${test})
    message(STATUS "Adding tests to the compile list: ${test}")

    add_executable(${test} ${PROJECT_SOURCE_DIR}/test/${test}.C)
    target_link_libraries(${test} ${PROJECT_BINARY_DIR}/libSQPrimaryVtxGen.so  -lgsl -lgslcblas -lphool
    -lboost_iostreams -lphgeom  -lGeom "${ROOT_LINK}")
    add_dependencies(${test} SQPrimaryVtxGen)
    install(TARGETS ${test} DESTINATION ${PROJECT_SOURCE_DIR}/test)
  endforeach(test)
endif()
