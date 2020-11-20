# $Id: CMakeLists.txt 
# David Baranyai | 2020.07.20 $

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(IMS2SQL)

set(INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include/)
set(SOURCES_DIRECTORY ${PROJECT_SOURCE_DIR}/source/)

#----------------------------------------------------------------------------
# Add the executable and Locate sources and headers for this project
file(GLOB_RECURSE sources ${SOURCES_DIRECTORY}/*.cpp)
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp ${sources})
add_executable(DecayToolTest DecayToolTest.cpp ${sources})
include_directories(${INCLUDE_DIRECTORY})
#----------------------------------------------------------------------------
# Set standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#----------------------------------------------------------------------------
# Compatibility between linux and MacOS
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    target_link_libraries(${PROJECT_NAME} -lc++)
else()
    #any other compiler
    target_link_libraries(${PROJECT_NAME} stdc++fs)
    target_link_libraries(DecayToolTest stdc++fs)
endif()
#----------------------------------------------------------------------------
# Link MySQL C Connector
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")
find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${MYSQL_LIBRARY})
#----------------------------------------------------------------------------
# Decay Tool Test
target_link_libraries(DecayToolTest ${MYSQL_LIBRARY})
#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
install(TARGETS ${PROJECT_NAME} DESTINATION bin)