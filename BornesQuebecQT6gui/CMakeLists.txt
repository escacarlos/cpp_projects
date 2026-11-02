# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Project name
project(BornesQC)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra -Wconversion -Wshadow")

include(CTest)

# Add the sources directory
add_subdirectory(Bornes_sources)

# Add the usage directory
add_subdirectory(Bornes_utilisation)

# Add the gui directory
add_subdirectory(Bornes_gui)