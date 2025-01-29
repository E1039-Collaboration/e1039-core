# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(fun4all_rus_file_manager CXX)

# Include the Fun4AllUniversalOutputManager source and header files
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})

# ROOT dictionary generation
add_custom_command (
  OUTPUT fun4all_rus_file_manager_Dict.cc
  COMMAND rootcint
  ARGS -f fun4all_rus_file_manager_Dict.cc -noIncludePaths -inlineInputHeader
    -I$ENV{OFFLINE_MAIN}/include/ -I${ROOT_PREFIX}/include/
    ${dist_headers}
    ${PROJECT_SOURCE_DIR}/LinkDef.h
  DEPENDS ${dist_headers} ${PROJECT_SOURCE_DIR}/LinkDef.h
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")

# Build the shared library
add_library(fun4all_rus_file_manager SHARED ${sources} fun4all_rus_file_manager_Dict.cc)
target_link_libraries(fun4all_rus_file_manager fun4all interface_main geom_svc ktracker phg4hit PHPythia8 g4detectors UtilAna)
install(TARGETS fun4all_rus_file_manager DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# Install the pcm files in case of ROOT 6.
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm fun4all_rus_file_manager)
endif()

