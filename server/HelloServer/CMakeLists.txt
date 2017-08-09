cmake_minimum_required(VERSION 2.8)
project(HelloServer)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -std=c++11 -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -std=c++11 -O0")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

aux_source_directory(./  SRC_LIST)
aux_source_directory(./  BASE_SRC_LIST)


list(APPEND SRC_LIST ${BASE_SRC_LIST}  )


include_directories(${PROJECT_SOURCE_DIR}/../../include)



link_directories(${PROJECT_SOURCE_DIR}/../../lib)


add_executable(HelloServer ${SRC_LIST})
target_link_libraries(HelloServer libflyfish.a  -lpthread)
