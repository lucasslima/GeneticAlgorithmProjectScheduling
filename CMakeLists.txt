cmake_minimum_required(VERSION 3.3)
project(GeneticAlgorithm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        GeneticAlgorithm.cpp
        instance.cpp
    instance.h)

add_executable(GeneticAlgorithm ${SOURCE_FILES})