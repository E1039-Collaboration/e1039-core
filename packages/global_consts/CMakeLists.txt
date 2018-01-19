# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(global_consts CXX C)

file(GLOB dist_headers ${PROJECT_SOURCE_DIR}/*.h)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
