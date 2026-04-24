# WarpDrive Plugin for X-Plane 12

WarpDrive is an X-Plane 12 plugin that exposes three bindable control commands for warping the aircraft's ground speed through a fixed integer range (1-16). It writes the current warp value to the `sim/time/ground_speed` dataref and tracks it in a plugin menu.

## Features

- **Custom Warp Commands**: 3 new commands for ground-speed warp control
  - `warpdrive/warp_up` - Increase the warp value by 1 (capped at 16)
  - `warpdrive/warp_down` - Decrease the warp value by 1 (floored at 1)
  - `warpdrive/warp_reset` - Reset the warp value to 1

- **Integer Warp Range**: Warp value is always an integer in the range `[1, 16]`

- **Plugin Menu**: `Plugins → WarpDrive` displays the live warp value and provides menu items for warp up / down / reset

- **Dataref Integration**: Writes the current warp value to `sim/time/ground_speed`

## Code Structure

### Core Components

- **`src/warpdrive.cpp`** - Main plugin entry point
  - Implements X-Plane plugin callbacks (`XPluginStart`, `XPluginStop`, etc.)
  - Registers custom commands (`warpdrive/warp_up`, `warp_down`, `warp_reset`)
  - Creates and maintains the plugin menu, updating the header as the value changes
  - Routes menu and command events into `WarpController`

- **`src/warp_controller.h/cpp`** - Warp value logic
  - Owns the integer warp value (1-16) and applies min/max clamping
  - Resolves and writes to the `sim/time/ground_speed` dataref
  - Exposes `warp_up()`, `warp_down()`, `warp_reset()`, `get_warp_value()`

- **`src/constants.h`** - Shared constants and enums
  - Defines `WARP_MIN`, `WARP_MAX`, `WARP_DEFAULT`
  - Holds the dataref name and menu item IDs
  - Buffer size constants for X-Plane API calls

### Architecture

The plugin follows the same modular design as its sister plugins (TrimGear, GlideStop, MultiBind):

1. **Plugin Layer** (`warpdrive.cpp`) - Interfaces with X-Plane SDK
2. **Controller Layer** (`warp_controller`) - Business logic for warp adjustments
3. **Constants Layer** (`constants`) - Shared definitions

## Building

### Prerequisites

- CMake 3.21 or higher
- C++17 compatible compiler
- X-Plane 12 SDK

### Quick Start (Recommended)

For first-time setup and building:

```bash
# Linux/Mac
./build-all.sh

# Windows
build-all.bat
```

These scripts will automatically download the X-Plane SDK, build the plugin, and provide installation instructions.

### Manual Setup & Build

#### Setup SDK

Option 1: Use setup scripts:
```bash
# Linux/Mac
./setup-sdk.sh

# Windows
setup-sdk.bat
```

Option 2: Manual setup:
1. Download the X-Plane SDK from [developer.x-plane.com](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
2. Extract to `SDK/` folder in project directory
3. Alternatively, set `XPLANE_SDK_PATH` environment variable

#### Build Commands

```bash
# Quick build (debug)
./build.sh

# Release build with package
./build.sh release

# Windows
build.bat
build.bat release

# Manual CMake (advanced)
mkdir build && cd build
cmake ..
cmake --build .
cmake --build . --target plugin
```

### Platform-Specific Outputs

- **Windows**: `build/WarpDrive/win.xpl`
- **macOS**: `build/WarpDrive/mac.xpl` (Universal Binary)
- **Linux**: `build/WarpDrive/lin.xpl`

## Installation

1. Build the plugin following the instructions above
2. Copy the entire `WarpDrive` folder to your X-Plane `Resources/plugins/` directory
3. Restart X-Plane

### Directory Structure
```
X-Plane 12/
└── Resources/
    └── plugins/
        └── WarpDrive/
            └── win.xpl (or mac.xpl/lin.xpl)
```

## Usage

### Basic Operation

1. **Assign Commands**: In X-Plane's joystick / keyboard settings, bind the WarpDrive commands to your preferred controls:
   - `warpdrive/warp_up`
   - `warpdrive/warp_down`
   - `warpdrive/warp_reset`

2. **Menu Usage**: Open `Plugins → WarpDrive` to see the current warp value and click **Warp Up**, **Warp Down**, or **Warp Reset** directly.

3. **Dataref**: The plugin writes the integer warp value to `sim/time/ground_speed` whenever it changes.

### Menu System

The plugin menu provides:
- **Warp: N / 16** - Live header showing the current warp value
- **Warp Up** - Increase warp value by 1 (disabled at max)
- **Warp Down** - Decrease warp value by 1 (disabled at min)
- **Warp Reset** - Reset warp value to 1 (disabled at default)

## Troubleshooting

### Common Issues

1. **Plugin Not Loading**
   - Check `Log.txt` for error messages
   - Verify SDK installation and platform-specific libraries
   - Ensure plugin is in the correct directory structure

2. **Commands Not Working**
   - Verify commands are properly assigned in X-Plane settings
   - Check that the `sim/time/ground_speed` dataref is available in your X-Plane build
   - Review `Log.txt` for dataref errors

### Debug Information

The plugin writes detailed logging to X-Plane's `Log.txt` file with prefix "WarpDrive:". Common log messages include:

- Plugin initialization status
- Dataref discovery results
- Warp value changes (up/down/reset)

## Requirements

- X-Plane 12
- 64-bit operating system (Windows 10+, macOS 10.14+, or Linux with X11)
