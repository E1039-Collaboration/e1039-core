# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(phfield CXX C)

# ROOT dict generation
file(GLOB dicts "")
file(GLOB pcms "")
file(GLOB LinkDefhs ${PROJECT_SOURCE_DIR}/*LinkDef.h)
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" DictC ${DictC})
	string(REPLACE ".C" "_rdict.pcm" DictPcm ${DictC})
	list(APPEND dicts ${DictC})
	list(APPEND pcms  ${DictPcm})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -noIncludePaths -inlineInputHeader -c -p -I${PROJECT_SOURCE_DIR} ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

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

# Geant4
find_program(G4CONF "geant4-config")
if(G4CONF)
  message("-- Detecting Geant4:  found at ${G4CONF}")
else()
  message(FATAL_ERROR "-- Detecting Geant4:  not found")
endif()
#find_package(Geant4 REQUIRED vis_all)
execute_process(COMMAND geant4-config --cflags OUTPUT_VARIABLE GEANT4_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND geant4-config --libs   OUTPUT_VARIABLE GEANT4_LINK   OUTPUT_STRIP_TRAILING_WHITESPACE)

# Warning suppression
string(REGEX REPLACE "-W[a-z-]*" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})
string(REPLACE "-pedantic" "" GEANT4_CFLAGS ${GEANT4_CFLAGS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GEANT4_CFLAGS} ${ROOT_CFLAGS} ")

add_library(phfield_io SHARED
  PHFieldConfig_Dict.C
  ${PROJECT_SOURCE_DIR}/PHFieldConfig.cc
  PHFieldConfig_v1_Dict.C
  ${PROJECT_SOURCE_DIR}/PHFieldConfig_v1.cc
  PHFieldConfig_v2_Dict.C
  ${PROJECT_SOURCE_DIR}/PHFieldConfig_v2.cc
  PHFieldConfig_v3_Dict.C
  ${PROJECT_SOURCE_DIR}/PHFieldConfig_v3.cc 
)

add_library(phfield SHARED
  ${PROJECT_SOURCE_DIR}/PHField.cc
  ${PROJECT_SOURCE_DIR}/PHFieldUniform.cc
  ${PROJECT_SOURCE_DIR}/PHField2D.cc
  ${PROJECT_SOURCE_DIR}/PHField3DCylindrical.cc
  ${PROJECT_SOURCE_DIR}/PHField3DCartesian.cc
  ${PROJECT_SOURCE_DIR}/SQField3DCartesian.cc
  ${PROJECT_SOURCE_DIR}/PHFieldRegionalConst.cc
  ${PROJECT_SOURCE_DIR}/PHFieldSeaQuest.cc
  PHFieldUtility_Dict.C
  ${PROJECT_SOURCE_DIR}/PHFieldUtility.cc 
)

add_dependencies(phfield phfield_io)

target_link_libraries(phfield_io -lphool)
target_link_libraries(phfield    phfield_io -lfun4all)

install(TARGETS phfield_io 				DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS phfield 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/PHFieldConfig.h
  ${PROJECT_SOURCE_DIR}/PHFieldConfig_v1.h
  ${PROJECT_SOURCE_DIR}/PHFieldConfig_v2.h
  ${PROJECT_SOURCE_DIR}/PHFieldConfig_v3.h
  ${PROJECT_SOURCE_DIR}/PHFieldUtility.h
  ${PROJECT_SOURCE_DIR}/PHField.h
)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)

execute_process(COMMAND root-config --version OUTPUT_VARIABLE ROOT_VER)
string(SUBSTRING ${ROOT_VER} 0 1 ROOT_VER)
if (ROOT_VER GREATER 5)
   add_custom_target(install_pcm ALL COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/lib COMMAND cp -up ${pcms} ${CMAKE_INSTALL_PREFIX}/lib)
   add_dependencies(install_pcm phfield phfield_io)
endif()


