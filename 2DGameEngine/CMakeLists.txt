cmake_minimum_required(VERSION 3.16)
project(2DGameEngine LANGUAGES CXX)

find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_image REQUIRED CONFIG)
find_package(SDL2_ttf REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)

add_executable(gameengine
    src/Main.cpp
    src/Game/Game.cpp
    src/Game/LevelLoader.cpp
    src/Logger/Logger.cpp
    src/ECS/ECS.cpp
    src/AssetStore/AssetStore.cpp
)

target_compile_features(gameengine PRIVATE cxx_std_17)
target_compile_options(gameengine PRIVATE -Wall -Wextra -Wfatal-errors)
target_include_directories(gameengine PRIVATE libs)
target_compile_definitions(gameengine PRIVATE GAMEENGINE_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(gameengine PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    nlohmann_json::nlohmann_json
)
