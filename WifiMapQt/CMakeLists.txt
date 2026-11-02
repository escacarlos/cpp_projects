cmake_minimum_required(VERSION 3.21)

project(WifiMapQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_executable(WifiMapQt
    src/main.cpp
    src/domain/mapmodel.cpp
    src/domain/mapmodel.h
    src/ui/mapcanvas.cpp
    src/ui/mapcanvas.h
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
)

target_link_libraries(WifiMapQt PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_include_directories(WifiMapQt PRIVATE src)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.2 QUIET COMPONENTS Test)
    if(TARGET Qt6::Test)
        qt_add_executable(WifiMapQtTests
            tests/test_mapmodel.cpp
            src/domain/mapmodel.cpp
            src/domain/mapmodel.h
        )
        target_link_libraries(WifiMapQtTests PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
        add_test(NAME WifiMapQtTests COMMAND WifiMapQtTests)
    else()
        message(STATUS "Qt Test not found; skipping WifiMapQtTests")
    endif()
endif()
