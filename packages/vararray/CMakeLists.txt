# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(vararray CXX C)

# ROOT dict generation
add_custom_command (
  OUTPUT VariableArrayDict.cc
  COMMAND rootcint
  ARGS -f VariableArrayDict.cc -noIncludePaths -inlineInputHeader -c -p
  -I${PROJECT_SOURCE_DIR}/
  ${PROJECT_SOURCE_DIR}/VariableArray.h
  ${PROJECT_SOURCE_DIR}/VariableArrayContainer.h
  ${PROJECT_SOURCE_DIR}/VariableArrayLinkDef.h
  )

file(GLOB pcms "")
list(APPEND pcms "VariableArrayDict_rdict.pcm")

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
execute_process(COMMAND root-config --libs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS}")

add_library(vararray SHARED
  ${PROJECT_SOURCE_DIR}/VariableArray.cc
  ${PROJECT_SOURCE_DIR}/VariableArrayContainer.cc
  VariableArrayDict.cc
  ${PROJECT_SOURCE_DIR}/VariableArrayUtils.cc
)
target_link_libraries(vararray -lhalf -lphool )

install(TARGETS vararray 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/VariableArray.h
  ${PROJECT_SOURCE_DIR}/VariableArrayContainer.h
  ${PROJECT_SOURCE_DIR}/VariableArrayIds.h
  ${PROJECT_SOURCE_DIR}/VariableArrayUtils.h
)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up ${pcms} ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm vararray)
endif()


