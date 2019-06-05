# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(g4dst CXX C)

add_library(g4dst SHARED
  g4dst.C
)

add_custom_command(
  OUTPUT g4dst.C
  COMMAND echo
  ARGS "//*** this is a generated empty file. Do not commit, do not edit" > g4dst.C
)

target_link_libraries(g4dst -lphg4hit -lg4detectors_io -lphgeom_io -lphhepmc_io -lphfield_io)

install(TARGETS g4dst	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

message(${CMAKE_PROJECT_NAME} " will be installed to " ${CMAKE_INSTALL_PREFIX})

install(FILES ${dist_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/)
