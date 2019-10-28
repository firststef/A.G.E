cmake_minimum_required(VERSION 3.15)
project(AGEProj)

set(CMAKE_CXX_STANDARD 14)

include_directories(AGEProj)

add_executable(AGEProj
        AGEProj/AGE.h
        AGEProj/AlgorithmsAGE0.h
        AGEProj/FunctionsAGE0.h
        AGEProj/json.hpp
        AGEProj/Source.cpp AGEProj/FunctionsAGE1.h AGEProj/AlgorithmsAGE1.h)
