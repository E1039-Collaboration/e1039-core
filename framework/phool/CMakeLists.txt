# Setup the project
#cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
#project(phool CXX C)

# ROOT dict generation
add_custom_command (
  OUTPUT PHBase_dict.cc
  COMMAND rootcint
  ARGS -f PHBase_dict.cc -c
	${CMAKE_CURRENT_SOURCE_DIR}/PHFlag.h
	${CMAKE_CURRENT_SOURCE_DIR}/phool.h
	${CMAKE_CURRENT_SOURCE_DIR}/PHObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/PHRandomSeed.h
	${CMAKE_CURRENT_SOURCE_DIR}/PHTimeServer.h
	${CMAKE_CURRENT_SOURCE_DIR}/PHTimeStamp.h
	${CMAKE_CURRENT_SOURCE_DIR}/recoConsts.h
	${CMAKE_CURRENT_SOURCE_DIR}/PHBase_LinkDef.h
	)

# source code
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(phool SHARED ${sources} PHBase_dict.cc)

## NG.  Same as jobopts_svc.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(phool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS phool DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
