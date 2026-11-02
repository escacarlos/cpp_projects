# Bornes Quebec

Carlos Alfredo Escalante Castillo

## Linux dependencies

The project is intended to build on Debian-based Linux distributions, including
Debian and Kali Linux. Install the compiler, CMake, Git, and Qt 6 Widgets
development files:

```sh
sudo apt update
sudo apt install build-essential cmake git qt6-base-dev
```

`build-essential` provides the C++ compiler and standard build tools. `qt6-base-dev`
provides Qt 6 Widgets and the CMake integration used by the GUI. When tests are
enabled, CMake downloads the pinned GoogleTest release during configuration; an
internet connection is required the first time.

## Build

Run these commands from this directory:

```sh
cmake -S . -B build
cmake --build build
```

To build without the unit tests (and without downloading GoogleTest), configure
with:

```sh
cmake -S . -B build -DBUILD_TESTING=OFF
cmake --build build
```

## Run

Run the graphical application from a Linux desktop session:

```sh
./build/Bornes_gui/bqc_gui
```

The command-line application is available at:

```sh
./build/Bornes_utilisation/bqc
```

Run the unit tests with:

```sh
ctest --test-dir build --output-on-failure
```
