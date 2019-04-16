# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
#project(geom_svc CXX C)

# source code
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.C  ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# ROOT dict generation
ListIncDirs(inc_opts jobopts_svc)
#message(STATUS "inc_opts = '${inc_opts}'")
file(GLOB dicts "")
file(GLOB LinkDefhs ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)
foreach(LinkDefh ${LinkDefhs})
	string(REPLACE "LinkDef.h" ".h" Dicth ${LinkDefh})
	string(REPLACE "LinkDef.h" "_Dict.C" DictC ${LinkDefh})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" DictC ${DictC})
	list(APPEND dicts ${DictC})
	add_custom_command(OUTPUT ${DictC} COMMAND rootcint ARGS -f ${DictC} -c -I$ENV{OFFLINE_MAIN}/include ${inc_opts} ${Dicth} ${LinkDefh})
endforeach(LinkDefh)

add_library(geom_svc SHARED ${sources} ${dicts})
target_link_libraries     (geom_svc jobopts_svc)
target_include_directories(geom_svc INTERFACE jobopts_svc)

install(TARGETS geom_svc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
