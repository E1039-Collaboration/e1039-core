# Setup the project
#cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
#project(jobopts_svc CXX C)

# ROOT dict generation
file(GLOB dicts "")
file(GLOB LinkDefhs ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" DictC ${DictC})
	list(APPEND dicts ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -c -I$ENV{OFFLINE_MAIN}/include ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

# source code
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.C  ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(jobopts_svc SHARED ${sources} ${dicts})

## NG.  For now the two lines below are necessary, where
## the 1st line is for rootcint via ListIncDirs() and
## the 2nd line is for normal compilation via add_library().
## How can I unify these?
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(jobopts_svc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS jobopts_svc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
