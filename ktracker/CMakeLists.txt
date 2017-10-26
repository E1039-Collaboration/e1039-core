# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(kTracker CXX C Fortran)

# Put executables into a bin
set(CMAKE_Fortran_COMPILER gfortran)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_MACOSX_RPATH 1)  # only needed to suppress cmake warning

# command line options to turn on DEBUG level
option(DEBUG1 "Enable debug level 1" OFF)
option(DEBUG2 "Enable debug level 2" OFF)
if(DEBUG1)
  message("-- IMPORTANT! Enabled debuging info level-1")
  add_definitions(-D_DEBUG_ON)
endif()
if(DEBUG2)
  message("-- IMPORTANT! Enabled debuging info level-2")
  add_definitions(-D_DEBUG_ON_LEVEL_2)
endif()

# command line option to turn off KMAG
option(KMAG "Turn KMAG ON/OFF" ON)
if(KMAG)
  add_definitions(-DKMAG_ON=1)
else()
  message("-- IMPORTANT! Turning KMAG off")
  add_definitions(-DKMAG_ON=0)
endif()

# command line option to turn off KF
option(KFENABLE "Enable Kalman filter fitting in track fitting" ON)
if(KFENABLE)
  add_definitions(-D_ENABLE_KF)
else()
  message("-- IMPORTANT! Kalman filter fitting is disabled")
endif()

# command line option to enable alignment mode
option(ALIGNMENTMODE "Enable alignment mode" OFF)
if(ALIGNMENTMODE)
  message("-- IMPORTANT! Alignment mode is enabled")
  add_definitions(-DALIGNMENT_MODE)
endif()

# Add version number from git if exists
if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
  execute_process(COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git rev-parse --short HEAD      OUTPUT_VARIABLE GIT_HASH   OUTPUT_STRIP_TRAILING_WHITESPACE)
  add_definitions("-DGIT_VERSION=\"${GIT_BRANCH}-${GIT_HASH}\"")
else()
  add_definitions("-DGIT_VERSION=\"1.7.0\"")
endif()

# ROOT dict generation
add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/src/FastTrackletDict.cxx ${PROJECT_SOURCE_DIR}/src/FastTrackletDict.h
  COMMAND rootcint
  ARGS -f ${PROJECT_SOURCE_DIR}/src/FastTrackletDict.cxx -c ${PROJECT_SOURCE_DIR}/inc/FastTracklet.h ${PROJECT_SOURCE_DIR}/inc/FastTrackletLinkDef.h
  )

add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx ${PROJECT_SOURCE_DIR}/src/SRawEventDict.h
  COMMAND rootcint
  ARGS -f ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx -c ${PROJECT_SOURCE_DIR}/inc/SRawEvent.h ${PROJECT_SOURCE_DIR}/inc/SRawEventLinkDef.h
  )

add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/src/SRecEventDict.cxx ${PROJECT_SOURCE_DIR}/src/SRecEventDict.h
  COMMAND rootcint
  ARGS -f ${PROJECT_SOURCE_DIR}/src/SRecEventDict.cxx -c ${PROJECT_SOURCE_DIR}/inc/SRecEvent.h ${PROJECT_SOURCE_DIR}/inc/SRecEventLinkDef.h
  )

add_custom_command (
  OUTPUT ${PROJECT_SOURCE_DIR}/src/TriggerRoadDict.cxx ${PROJECT_SOURCE_DIR}/src/TriggerRoadDict.h
  COMMAND rootcint
  ARGS -f ${PROJECT_SOURCE_DIR}/src/TriggerRoadDict.cxx -c ${PROJECT_SOURCE_DIR}/inc/TriggerRoad.h ${PROJECT_SOURCE_DIR}/inc/TriggerRoadLinkDef.h
  )

if(ALIGNMENTMODE)
  add_custom_command (
    OUTPUT ${PROJECT_SOURCE_DIR}/src/SMillepedeUtilDict.cxx ${PROJECT_SOURCE_DIR}/src/SMillepedeUtilDict.h
    COMMAND rootcint
    ARGS -f ${PROJECT_SOURCE_DIR}/src/SMillepedeUtilDict.cxx -c ${PROJECT_SOURCE_DIR}/inc/SMillepedeUtil.h ${PROJECT_SOURCE_DIR}/inc/SMillepedeUtilLinkDef.h
    )
endif()

# DPSim source code
include_directories(${PROJECT_SOURCE_DIR}/inc)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cxx   ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hh)
if(ALIGNMENTMODE)
  list(APPEND sources ${PROJECT_SOURCE_DIR}/src/millepede.f)
else()
  list(REMOVE_ITEM sources ${PROJECT_SOURCE_DIR}/src/SMillepede.cxx ${PROJECT_SOURCE_DIR}/src/SMillepedeUtil.cxx)
  list(REMOVE_ITEM headers ${PROJECT_SOURCE_DIR}/inc/SMillepede.h   ${PROJECT_SOURCE_DIR}/src/SMillepedeUtil.h)
endif()

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

# the rest packages are not needed for libonly mode
# Geant4
find_program(G4CONF "geant4-config")
if(G4CONF)
  message("-- Detecting Geant4:  found at ${G4CONF}")
else()
  message(FATAL_ERROR "-- Detecting Geant4:  not found")
endif()
find_package(Geant4 REQUIRED vis_all)
execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)

# MySQL
find_program(MYSQLCONF "mysql_config")
if(MYSQLCONF)
  message("-- Detecting MySQL:   found at ${MYSQLCONF}")
else()
  message(FATAL_ERROR "-- Detecting MySQL:   not found")
endif()
execute_process(COMMAND mysql_config --cflags OUTPUT_VARIABLE MYSQL_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs   OUTPUT_VARIABLE MYSQL_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Warning suppression
string(REGEX REPLACE "-W[a-z-]*" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})
string(REPLACE "-pedantic" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})

#find BOOST
find_package(Boost)
if(Boost_FOUND)
  message("-- Detecting BOOST:   found at ${Boost_INCLUDE_DIRS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${Boost_INCLUDE_DIRS}")
elseif(EXISTS $ENV{BOOST_ROOT})
  message("-- Detecting BOOST:   using Boost installation at $ENV{BOOST_ROOT}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{BOOST_ROOT}/include")
else()
  message(FATAL_ERROR "-- Detecting BOOST:   not found")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GEANT4_CFLAGS} ${ROOT_CFLAGS} ${MYSQL_CFLAGS}")
if(ALIGNMENTMODE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GEANT4_LINK} ${ROOT_LINK} ${MYSQL_LINK} -lgfortran")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GEANT4_LINK} ${ROOT_LINK} ${MYSQL_LINK} -lgfortran")
  add_library(kTracker SHARED ${sources} ${headers} ${PROJECT_SOURCE_DIR}/src/FastTrackletDict.cxx ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx ${PROJECT_SOURCE_DIR}/src/SRecEventDict.cxx ${PROJECT_SOURCE_DIR}/src/SMillepedeUtilDict.cxx ${PROJECT_SOURCE_DIR}/src/TriggerRoadDict.cxx)
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GEANT4_LINK} ${ROOT_LINK} ${MYSQL_LINK}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GEANT4_LINK} ${ROOT_LINK} ${MYSQL_LINK}")
  add_library(kTracker SHARED ${sources} ${headers} ${PROJECT_SOURCE_DIR}/src/FastTrackletDict.cxx ${PROJECT_SOURCE_DIR}/src/SRawEventDict.cxx ${PROJECT_SOURCE_DIR}/src/SRecEventDict.cxx ${PROJECT_SOURCE_DIR}/src/TriggerRoadDict.cxx)
endif()

file(GLOB execs ${PROJECT_SOURCE_DIR}/exe/*.cxx)
foreach(exec ${execs})
  string(REPLACE  "${PROJECT_SOURCE_DIR}/exe/" "" exec ${exec})
  string(REPLACE  ".cxx" "" exec ${exec})
  message(STATUS "Adding targets to the compile list: ${exec}")

  add_executable(${exec} ${PROJECT_SOURCE_DIR}/exe/${exec}.cxx)
  target_link_libraries(${exec} ${PROJECT_SOURCE_DIR}/lib/libkTracker.so)
  add_dependencies(${exec} kTracker)
endforeach(exec)
