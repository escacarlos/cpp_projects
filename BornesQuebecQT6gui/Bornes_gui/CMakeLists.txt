cmake_minimum_required(VERSION 3.16)

project(BornesQC_GUI)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Automatically generate MOC files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add the executable
add_executable(bqc_gui
main.cpp
BorneFontaineAjouter.cpp
BorneFontaineAjouter.h
BornesGUI.cpp
BornesGUI.h
BorneStationnementAjouter.cpp
BorneStationnementAjouter.h
SupprimerBorne.cpp
SupprimerBorne.h
BorneFontaineAjouter.ui
BornesGUI.ui
BorneStationnementAjouter.ui
SupprimerBorne.ui
)

# Link against Qt6
target_link_libraries(bqc_gui PRIVATE Qt6::Widgets BornesLibrary)
