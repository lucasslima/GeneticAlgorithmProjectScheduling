cmake_minimum_required(VERSION 3.3)
project(GeneticAlgorithm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Ofast")
set(JSON_FLAGS "-ljsoncpp")
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${JSON_FLAGS}" )
set(SOURCE_FILES
        GeneticAlgorithm.cpp
        instance.cpp
    instance.h)

add_executable(GeneticAlgorithm ${SOURCE_FILES})
