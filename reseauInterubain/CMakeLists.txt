cmake_minimum_required(VERSION 4.3)
project(reseauInterubain)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(reseauInterubain
        Graphe.cpp
        Graphe.h
        Principal.cpp
        ReseauInterurbain.cpp
        ReseauInterurbain.h)
