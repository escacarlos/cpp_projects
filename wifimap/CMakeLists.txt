cmake_minimum_required(VERSION 3.10)
project(Wifimap)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable GLVND preference to fix OpenGL warnings
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add glad.c as a compilation unit
set(GLAD_SOURCE ${CMAKE_SOURCE_DIR}/opengl_src/glad.c)

set(STBI_SOURCE ${CMAKE_SOURCE_DIR}/image_loader/stb_image.cpp)

# Add Dear ImGui source files
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC imgui imgui/backends)

# Detect platform
if (WIN32)
    # Windows-specific configuration

    # Find OpenGL
    find_package(OpenGL REQUIRED)

    # Add the executable
    add_executable(Wifimap main.cpp ./domain_src/Map.cpp ./domain_src/AccessPoint.cpp ./domain_src/Spot.cpp ${GLAD_SOURCE} ${STBI_SOURCE} ${DOMAIN_SOURCE})

    # Link GLFW and OpenGL
    set(GLFW_LIB_PATH ${CMAKE_SOURCE_DIR}/lib/libglfw3.a)

    target_link_libraries(Wifimap imgui ${GLFW_LIB_PATH} ${OPENGL_LIBRARIES})
else()
    # Linux-specific configuration
    find_package(OpenGL REQUIRED)
    #find_package(GLM REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    #find_package(GLFW3 REQUIRED)
    
    # Add the executable
    add_executable(Wifimap main.cpp ${GLAD_SOURCE} ${STBI_SOURCE} ${DOMAIN_SOURCE})
    
    # Link system-installed GLFW and OpenGL
    target_link_libraries(Wifimap imgui ${GLFW3_LIBRARIES} OpenGL::GL)
endif()