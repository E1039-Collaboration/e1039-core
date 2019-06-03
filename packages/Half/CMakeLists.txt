# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(half CXX C)

add_library(half SHARED
  ${PROJECT_SOURCE_DIR}/half.cpp
)
target_link_libraries(half)

install(TARGETS half 				DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

file(GLOB dist_headers
  ${PROJECT_SOURCE_DIR}/half.h
)
install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)



