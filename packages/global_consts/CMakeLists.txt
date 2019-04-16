# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
#project(global_consts CXX C)

file(GLOB dist_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
#install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

add_custom_target(global_consts
  COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/include
  COMMAND cp -a ${dist_headers} ${CMAKE_INSTALL_PREFIX}/include
)
