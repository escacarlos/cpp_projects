# 2D Game Engine (C++)

The C++ engine is built with CMake and loads levels from the JSON files in
`assets/levels`. Run commands from this project directory so asset paths in the
level files resolve correctly. The Python engine and its files are independent
and are not part of the C++ build.

## Dependencies

- CMake 3.16 or newer
- A C++17 compiler (for example, GCC)
- SDL2 development libraries
- SDL2_image development library
- SDL2_ttf development library
- nlohmann/json development package

On Debian or Ubuntu, install them with:

```sh
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev \
  libsdl2-ttf-dev nlohmann-json3-dev
```

GLM is included under `libs/glm`; SDL2_mixer is not currently used by the C++
engine.

## Build

```sh
cmake -S . -B build
cmake --build build
```

The existing Makefile is also retained:

```sh
make build
```

## Run

The default level is `assets/levels/level1.json`:

```sh
./build/gameengine
```

Pass a different JSON level path as the first argument:

```sh
./build/gameengine ./assets/levels/level2.json
```

Use the arrow keys to move and Escape to quit. F1 toggles collider debugging.
