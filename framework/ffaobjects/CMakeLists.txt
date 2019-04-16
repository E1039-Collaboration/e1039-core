# Setup the project
#cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
#project(ffaobjects CXX C)

# source code
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# ROOT dict generation
ListIncDirs(inc_opts phool)
message(STATUS "inc_opts = '${inc_opts}'")
add_custom_command (
  OUTPUT libffaobjects_Dict.cc
  COMMAND rootcint
  ARGS -f libffaobjects_Dict.cc -c
  ${inc_opts}
  ${CMAKE_CURRENT_SOURCE_DIR}/FlagSave.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FlagSavev1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/RunHeader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SyncObject.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SyncObjectv1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SyncObjectv2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/EventHeader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/EventHeaderv1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/libffaobjects_LinkDef.h
)

add_library(ffaobjects SHARED ${sources} libffaobjects_Dict.cc)
target_link_libraries     (ffaobjects phool)
target_include_directories(ffaobjects INTERFACE phool)

install(TARGETS ffaobjects DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB dist_headers     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB non_dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)
list(REMOVE_ITEM dist_headers ${non_dist_headers})
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
