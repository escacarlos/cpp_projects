# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.15)

# Project name
project(creditcard)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors")

# Include directories
include_directories()

# Source files
file(GLOB SRC_FILES 
    ./*.cpp    
)

# Executable name
set(OBJ_NAME creditcard)

# Add the executable target
add_executable(${OBJ_NAME} ${SRC_FILES})

# Link against SDL2 and other libraries
target_link_libraries(${OBJ_NAME})
