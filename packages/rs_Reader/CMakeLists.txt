# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(rs_Reader CXX C)

# source code
file(GLOB          sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB     dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})

# ROOT dict generation
add_custom_command (
  OUTPUT rs_Reader_Dict.cc
  COMMAND rootcint
  ARGS -f rs_Reader_Dict.cc -noIncludePaths -inlineInputHeader -c -p
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${ROOT_CFLAGS}")

add_library(rs_Reader SHARED ${sources} rs_Reader_Dict.cc)
#include_directories("${PROJECT_SOURCE_DIR}/../../simulation")
#target_link_libraries(rs_Reader -linterface_main -lgeom_svc)

install(TARGETS rs_Reader DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# Install the pcm files in case of ROOT 6.
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm rs_Reader)
endif()
