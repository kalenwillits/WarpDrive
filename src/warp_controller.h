#pragma once

#include "XPLMDataAccess.h"

class WarpController {
public:
    WarpController();
    ~WarpController();

    bool initialize();
    void cleanup();

    void warp_up();
    void warp_down();
    void warp_reset();

    int get_warp_value() const;

private:
    XPLMDataRef m_ground_speed_ref;
    int m_warp_value;
    bool m_initialized;

    void apply_warp_value();
};
