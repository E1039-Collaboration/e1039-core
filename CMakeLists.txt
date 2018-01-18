# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(seaquest-offline CXX C)

add_subdirectory(${PROJECT_SOURCE_DIR}/framework/phool)
add_subdirectory(${PROJECT_SOURCE_DIR}/framework/ffaobjects)
add_subdirectory(${PROJECT_SOURCE_DIR}/framework/fun4all)
add_subdirectory(${PROJECT_SOURCE_DIR}/interface_main)
