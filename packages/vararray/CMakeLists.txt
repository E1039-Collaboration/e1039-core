# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(vararray CXX C)

# ROOT dict generation
add_custom_command (
  OUTPUT VariableArrayDict.cc
  COMMAND rootcint
  ARGS -f VariableArrayDict.cc -c
	-I$ENV{OFFLINE_MAIN}/include
  ${PROJECT_SOURCE_DIR}/VariableArray.h
  ${PROJECT_SOURCE_DIR}/VariableArrayContainer.h
  ${PROJECT_SOURCE_DIR}/VariableArrayLinkDef.h
	)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")


add_library(vararray SHARED
  ${PROJECT_SOURCE_DIR}/VariableArray.cc
  ${PROJECT_SOURCE_DIR}/VariableArrayContainer.cc
  VariableArrayDict.cc
  ${PROJECT_SOURCE_DIR}/VariableArrayUtils.cc
)
target_link_libraries(vararray -L./ -L$ENV{OFFLINE_MAIN}/lib -lhalf -lphool )

install(TARGETS vararray 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/VariableArray.h
  ${PROJECT_SOURCE_DIR}/VariableArrayContainer.h
  ${PROJECT_SOURCE_DIR}/VariableArrayIds.h
  ${PROJECT_SOURCE_DIR}/VariableArrayUtils.h
)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



