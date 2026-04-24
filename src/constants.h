#pragma once

#include <cstddef>

namespace warpdrive {
namespace constants {

constexpr int XPLANE_STRING_BUFFER_SIZE = 256;
constexpr int XPLANE_PATH_BUFFER_SIZE = 512;

// Warp value bounds
constexpr int WARP_MIN = 1;
constexpr int WARP_MAX = 16;
constexpr int WARP_DEFAULT = WARP_MIN;

// X-Plane dataref used as the warp control value
constexpr const char* GROUND_SPEED_DATAREF = "sim/time/ground_speed";

// Menu text buffer size for safe string formatting
constexpr size_t MENU_TEXT_BUFFER_SIZE = 64;

// Menu item IDs
enum MenuItems {
    MENU_WARP_HEADER = 1000,
    MENU_WARP_UP     = 1001,
    MENU_WARP_DOWN   = 1002,
    MENU_WARP_RESET  = 1003
};

} // namespace constants
} // namespace warpdrive
