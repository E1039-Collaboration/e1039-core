# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(chan_map CXX)

execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
message("ROOT_VER: ${ROOT_VER}")

# ROOT dict generation
if (ROOT_VER GREATER 5)
  add_custom_command (
    OUTPUT ChanMap_Dict.cc
    COMMAND rootcint
    ARGS -f ChanMap_Dict.cc -c
      -I$ENV{OFFLINE_MAIN}/include/ -I${ROOT_PREFIX}/include/
      ${PROJECT_SOURCE_DIR}/ChanMapper*.h
      ${PROJECT_SOURCE_DIR}/CalibParam*.h
      ${PROJECT_SOURCE_DIR}/LinkDef.h
  )
endif()

link_directories("./" "$ENV{OFFLINE_MAIN}/lib/")
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")

file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

# MySQL
find_program(MYSQL_CONF "mysql_config")
if(MYSQL_CONF)
  message("-- Detecting MySQL:    found at ${MYSQL_CONF}")
else()
  message(FATAL_ERROR "-- Detecting MySQL:    not found")
endif()
execute_process(COMMAND mysql_config --cflags OUTPUT_VARIABLE MYSQL_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs   OUTPUT_VARIABLE MYSQL_LIBS    OUTPUT_STRIP_TRAILING_WHITESPACE)


# ROOT
find_program(ROOTCONF "root-config")
if(ROOTCONF)
  message("-- Detecting ROOT:    found at ${ROOTCONF}")
else()
  message(FATAL_ERROR "-- Detecting ROOT:    not found")
endif()
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS} ${MYSQL_CFLAGS}")

if (ROOT_VER GREATER 5)
  add_library(chan_map SHARED ${sources} ChanMap_Dict.cc)
else ()
  add_library(chan_map SHARED ${sources})
endif()
target_link_libraries(chan_map ${MYSQL_LIBS} ${ROOT_LINK} geom_svc db_svc)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(TARGETS chan_map DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# Install the pcm files in case of ROOT 6.
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm chan_map)
endif()
