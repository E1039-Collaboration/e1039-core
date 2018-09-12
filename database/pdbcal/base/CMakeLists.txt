# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(pdbcalbase CXX C)

# source code
include_directories("$ENV{OFFLINE_MAIN}/include/" "${PROJECT_SOURCE_DIR}/")
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

# ROOT dict generation
file(GLOB LinkDefhs "")
foreach(header ${headers})
	#message("header: " ${header})
	string(REPLACE ".h" "_LinkDef.h" LinkDefh ${header})
	string(REPLACE "${PROJECT_SOURCE_DIR}/" "" LinkDefh ${LinkDefh})
	#message("LinkDefh: " ${LinkDefh})
	#message("cmd: " "${PROJECT_SOURCE_DIR}/PdbRootify.pl")
	list(APPEND LinkDefhs ${LinkDefh})
	add_custom_command(OUTPUT ${LinkDefh} COMMAND perl ARGS ${PROJECT_SOURCE_DIR}/PdbRootify.pl ${LinkDefh})
endforeach(header)

list(REMOVE_ITEM LinkDefhs "PdbBankManagerFactory_LinkDef.h")
list(REMOVE_ITEM LinkDefhs "RunToTimeFactory_LinkDef.h")

file(GLOB dicts "")
foreach(LinkDefh ${LinkDefhs})
	#message("LinkDefh: " ${LinkDefh})
	string(REPLACE "_LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "_LinkDef.h" "_dict.C" DictC ${LinkDefh})
	list(APPEND dicts ${DictC})
  #message("Dicth: " ${PROJECT_SOURCE_DIR}/${Dicth})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -c -I$ENV{OFFLINE_MAIN}/include ${PROJECT_SOURCE_DIR}/${Dicth} ${LinkDefh} DEPENDS ${LinkDefh})
endforeach(LinkDefh)

add_custom_command (
  OUTPUT PdbBankManagerFactory_dict.C
  COMMAND rootcint
  ARGS -f PdbBankManagerFactory_dict.C -c
	-I$ENV{OFFLINE_MAIN}/include
  ${PROJECT_SOURCE_DIR}/PHGenericFactoryT.h
  ${PROJECT_SOURCE_DIR}/PdbBankManagerFactory.h
  PdbBankManagerFactory_LinkDef.h
  DEPENDS PdbBankManagerFactory_LinkDef.h
	)

add_custom_command (
  OUTPUT RunToTimeFactory_dict.C
  COMMAND rootcint
  ARGS -f RunToTimeFactory_dict.C -c
	-I$ENV{OFFLINE_MAIN}/include
  ${PROJECT_SOURCE_DIR}/PHGenericFactoryT.h
  ${PROJECT_SOURCE_DIR}/PdbBankManagerFactory.h
  PdbBankManagerFactory_LinkDef.h
  DEPENDS PdbBankManagerFactory_LinkDef.h
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{OFFLINE_MAIN}/include/ ${ROOT_CFLAGS}")


add_library(pdbcalBase SHARED
  ${PROJECT_SOURCE_DIR}/PdbApplication.cc 
  PdbApplication_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbApplicationFactory.cc 
  PdbApplicationFactory_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbBankID.cc 
  PdbBankID_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbBankListIterator.cc 
  PdbBankListIterator_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbBankList.cc 
  PdbBankList_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbBankManager.cc 
  PdbBankManager_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbBankManagerFactory.cc 
  PdbBankManagerFactory_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbCalBank.cc 
  PdbCalBank_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbCalBankIterator.cc 
  PdbCalBankIterator_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbCalChan.cc 
  PdbCalChan_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbParameter.cc 
  PdbParameter_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbParameterError.cc 
  PdbParameterError_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbParameterMap.cc 
  PdbParameterMap_dict.C 
  ${PROJECT_SOURCE_DIR}/PdbParameterMapContainer.cc 
  PdbParameterMapContainer_dict.C 
  ${PROJECT_SOURCE_DIR}/PHGenericFactoryT.C 
  ${PROJECT_SOURCE_DIR}/RunToTime.cc 
  RunToTime_dict.C
)
target_link_libraries(pdbcalBase -L./ -L$ENV{OFFLINE_MAIN}/lib -lphool -L${ROOT_PREFIX}/lib -lXMLIO)

install(TARGETS pdbcalBase 					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/Pdb.h 
  ${PROJECT_SOURCE_DIR}/PdbApplication.h 
  ${PROJECT_SOURCE_DIR}/PdbApplicationFactory.h 
  ${PROJECT_SOURCE_DIR}/PdbBankID.h 
  ${PROJECT_SOURCE_DIR}/PdbBankList.h 
  ${PROJECT_SOURCE_DIR}/PdbBankListIterator.h 
  ${PROJECT_SOURCE_DIR}/PdbBankManager.h 
  ${PROJECT_SOURCE_DIR}/PdbBankManagerFactory.h 
  ${PROJECT_SOURCE_DIR}/PdbCalBank.h 
  ${PROJECT_SOURCE_DIR}/PdbCalBankIterator.h 
  ${PROJECT_SOURCE_DIR}/PdbCalChan.h 
  ${PROJECT_SOURCE_DIR}/PdbClassMap.h 
  ${PROJECT_SOURCE_DIR}/PdbParameter.h 
  ${PROJECT_SOURCE_DIR}/PdbParameterError.h 
  ${PROJECT_SOURCE_DIR}/PdbParameterMap.h 
  ${PROJECT_SOURCE_DIR}/PdbParameterMapContainer.h 
  ${PROJECT_SOURCE_DIR}/PHGenericFactoryT.h 
  ${PROJECT_SOURCE_DIR}/RunToTime.h
)

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



