# WifiMap Qt

WifiMap Qt is the C++/Qt port of the university Java Swing application. It retains
the Wi-Fi heatmap, access points, material-dependent barriers, editing and
dragging, zoom/pan, grid snapping, undo/redo, map resizing, JSON persistence,
and PNG export.

Dragging an access point or barrier recalculates the heatmap live. Use
**Edit → Copy element** and **Edit → Paste element** (Ctrl+C/Ctrl+V while the
map is focused) to duplicate a selected element; the paste is offset by five
cells in each direction, reversing an axis when needed to fit the map.
The map canvas displays zero-based X and Y coordinate axes, with tick spacing
adjusted to the current zoom level.

## Requirements

- C++17 compiler
- CMake 3.21 or newer
- Qt 6.2 or newer: Core, Gui, and Widgets
- Qt Test (only needed to build the included tests)
- Ninja is optional; the default CMake generator also works

Qt Core's JSON classes are used for `.json` map files; no third-party JSON
dependency is required.

On Debian/Ubuntu, install the development packages with:

```sh
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools
```

## Build and run

From this directory:

```sh
cmake -S . -B build -G Ninja
cmake --build build
./build/WifiMapQt
```

Run the model tests with:

```sh
ctest --test-dir build --output-on-failure
```

## Map JSON format

Files use format version 1. Parcels and the heatmap are recalculated when a map
is loaded, so only dimensions and editable elements are stored:

```json
{
  "formatVersion": 1,
  "length": 500,
  "width": 500,
  "accessPoints": [
    { "id": 1, "x": 20, "y": 30, "frequencyGHz": 2.4, "powerDbm": 18 }
  ],
  "barriers": [
    {
      "id": 2, "x1": 40, "y1": 10, "x2": 40, "y2": 80,
      "thicknessCm": 30, "material": "Wood"
    }
  ]
}
```

Coordinates use the same convention as the Java application: `(0, 0)` is the
bottom-left cell. Access points use 2.4 or 5.0 GHz and 11–20 dBm. Barriers are
horizontal or vertical and use Plaster, Wood, Brick, Cement, Reinforced
concrete, or Metal.
