# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phhepmc CXX C)

# source code
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

# ROOT dict generation
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
file(GLOB dicts "")
foreach(LinkDefh ${LinkDefhs})
	#message("LinkDefh: " ${LinkDefh})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${DictC})
	list(APPEND dicts ${DictC})
  #message("Dicth: " ${Dicth})
  #message("DictC: " ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -noIncludePaths -inlineInputHeader -c -p -I${PROJECT_SOURCE_DIR} ${Dicth} ${LinkDefh} DEPENDS ${LinkDefh})
endforeach(LinkDefh)

add_custom_command (
  OUTPUT PHHepMC_io_Dict.C
  COMMAND rootcint
  ARGS -f PHHepMC_io_Dict.C -noIncludePaths -inlineInputHeader -c -p
  -I${PROJECT_SOURCE_DIR}/
  ${PROJECT_SOURCE_DIR}/PHGenEvent.h
  ${PROJECT_SOURCE_DIR}/PHGenEventv1.h
  ${PROJECT_SOURCE_DIR}/PHGenEventList.h
  ${PROJECT_SOURCE_DIR}/PHGenEventListv1.h
  ${PROJECT_SOURCE_DIR}/PHHepMCGenEvent.h
  ${PROJECT_SOURCE_DIR}/PHHepMCGenEventMap.h
	$ENV{E1039_SHARE}/include/HepMC/*.h
  ${PROJECT_SOURCE_DIR}/PHHepMCLinkDef.h
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
execute_process(COMMAND root-config --evelibs   OUTPUT_VARIABLE ROOT_LINK    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CFLAGS} -std=c++0x ")

add_library(phhepmc SHARED
  ${PROJECT_SOURCE_DIR}/Fun4AllHepMCInputManager.cc
  ${PROJECT_SOURCE_DIR}/Fun4AllHepMCOutputManager.cc
  #${PROJECT_SOURCE_DIR}/Fun4AllOscarInputManager.cc
  ${PROJECT_SOURCE_DIR}/PHHepMCGenHelper.cc
  ${PROJECT_SOURCE_DIR}/PHHepMCParticleSelectorDecayProductChain.cc
  Fun4AllHepMCInputManager_Dict.C
  Fun4AllHepMCOutputManager_Dict.C
  #Fun4AllOscarInputManager_Dict.C
  PHHepMCGenHelper_Dict.C
  PHHepMCParticleSelectorDecayProductChain_Dict.C
)

add_library(phhepmc_io SHARED
  PHGenEventv1.cc
  PHGenEventListv1.cc
  PHHepMCGenEvent.cc
  PHHepMCGenEventMap.cc
  PHGenIntegral.cc
  PHGenIntegralv1.cc
  PHHepMC_io_Dict.C
  PHGenIntegral_Dict.C
  PHGenIntegralv1_Dict.C
)

target_link_libraries(phhepmc_io -lphool -lHepMC)

add_dependencies(phhepmc phhepmc_io)
target_link_libraries(phhepmc phhepmc_io -lphool -lSubsysReco -lboost_iostreams -lfun4all -lgsl -lgslcblas -lSQPrimaryVtxGen)

install(TARGETS phhepmc_io  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS phhepmc     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  Fun4AllHepMCInputManager.h
  Fun4AllHepMCOutputManager.h
  #Fun4AllOscarInputManager.h
  PHGenEvent.h
  PHGenEventv1.h
  PHGenEventList.h
  PHGenEventListv1.h
  PHGenIntegral.h
  PHGenIntegralv1.h
  PHHepMCGenEvent.h
  PHHepMCGenEventMap.h
  PHHepMCGenHelper.h
)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

# The same method as "database/pdbcal/base".
execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up *_rdict.pcm ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm phhepmc phhepmc_io)
endif()
