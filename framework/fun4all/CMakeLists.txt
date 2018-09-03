# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(fun4all CXX C)

# ROOT dict generation
add_custom_command (
  OUTPUT Fun4All_Dict.cc
  COMMAND rootcint
  ARGS -f Fun4All_Dict.cc -c
  -I$ENV{MY_INSTALL}/include
	-I$ENV{OFFLINE_MAIN}/include
	${PROJECT_SOURCE_DIR}/Fun4AllDstInputManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllDstOutputManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllDummyInputManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllHistoManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllInputManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllSyncManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllNoSyncDstInputManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllOutputManager.h
  ${PROJECT_SOURCE_DIR}/Fun4AllServer.h
  ${PROJECT_SOURCE_DIR}/Fun4AllLinkDef.h
	)

add_custom_command (
  OUTPUT SubsysReco_Dict.cc
  COMMAND rootcint
  ARGS -f SubsysReco_Dict.cc -c
  -I$ENV{MY_INSTALL}/include
	-I$ENV{OFFLINE_MAIN}/include
  ${PROJECT_SOURCE_DIR}/Fun4AllBase.h
  ${PROJECT_SOURCE_DIR}/SubsysReco.h
  ${PROJECT_SOURCE_DIR}/SubsysRecoLinkDef.h	
	)

# source code
include_directories("$ENV{MY_INSTALL}/include/" "$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{MY_INSTALL}/include/ -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")

add_library(SubsysReco SHARED
	${PROJECT_SOURCE_DIR}/Fun4AllBase.cc
	${PROJECT_SOURCE_DIR}/SubsysReco.cc
	SubsysReco_Dict.cc
)
target_link_libraries(SubsysReco ${ROOT_LINK})

add_library(TDirectoryHelper SHARED
	${PROJECT_SOURCE_DIR}/TDirectoryHelper.cc
)
target_link_libraries(TDirectoryHelper ${ROOT_LINK})

add_library(fun4all SHARED
  ${PROJECT_SOURCE_DIR}/Fun4AllDstInputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllDstOutputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllDummyInputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllHistoManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllInputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllSyncManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllNoSyncDstInputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllOutputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllServer.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllUtils.cc
  ${PROJECT_SOURCE_DIR}/PHTFileServer.cxx
  Fun4All_Dict.cc
)
target_link_libraries(fun4all -L./ -L$ENV{MY_INSTALL}/lib -L$ENV{OFFLINE_MAIN}/lib -lSubsysReco -lTDirectoryHelper -lboost_filesystem -lffaobjects -lphool)

install(TARGETS SubsysReco 				DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS TDirectoryHelper	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS fun4all 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${PROJECT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



