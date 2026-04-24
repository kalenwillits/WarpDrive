#include "warp_controller.h"

#include "XPLMUtilities.h"

#include "constants.h"

#include <algorithm>
#include <string>

WarpController::WarpController()
    : m_ground_speed_ref(nullptr),
      m_warp_value(warpdrive::constants::WARP_DEFAULT),
      m_initialized(false)
{
}

WarpController::~WarpController() {
    cleanup();
}

bool WarpController::initialize() {
    if (m_initialized) {
        return true;
    }

    m_ground_speed_ref = XPLMFindDataRef(warpdrive::constants::GROUND_SPEED_DATAREF);
    if (!m_ground_speed_ref) {
        std::string error_msg = "WarpDrive: ERROR - Could not find dataref: ";
        error_msg += warpdrive::constants::GROUND_SPEED_DATAREF;
        error_msg += "\n";
        XPLMDebugString(error_msg.c_str());
        return false;
    }

    m_warp_value = warpdrive::constants::WARP_DEFAULT;
    m_initialized = true;

    XPLMDebugString("WarpDrive: Warp controller initialized successfully\n");
    return true;
}

void WarpController::cleanup() {
    m_ground_speed_ref = nullptr;
    m_initialized = false;
}

void WarpController::warp_up() {
    if (!m_initialized) {
        return;
    }

    if (m_warp_value < warpdrive::constants::WARP_MAX) {
        m_warp_value += 1;
        apply_warp_value();

        std::string log_msg = "WarpDrive: Warp up -> ";
        log_msg += std::to_string(m_warp_value);
        log_msg += "\n";
        XPLMDebugString(log_msg.c_str());
    }
}

void WarpController::warp_down() {
    if (!m_initialized) {
        return;
    }

    if (m_warp_value > warpdrive::constants::WARP_MIN) {
        m_warp_value -= 1;
        apply_warp_value();

        std::string log_msg = "WarpDrive: Warp down -> ";
        log_msg += std::to_string(m_warp_value);
        log_msg += "\n";
        XPLMDebugString(log_msg.c_str());
    }
}

void WarpController::warp_reset() {
    if (!m_initialized) {
        return;
    }

    m_warp_value = warpdrive::constants::WARP_DEFAULT;
    apply_warp_value();

    std::string log_msg = "WarpDrive: Warp reset -> ";
    log_msg += std::to_string(m_warp_value);
    log_msg += "\n";
    XPLMDebugString(log_msg.c_str());
}

int WarpController::get_warp_value() const {
    return m_warp_value;
}

void WarpController::apply_warp_value() {
    if (!m_ground_speed_ref) {
        return;
    }

    int clamped = std::max<int>(warpdrive::constants::WARP_MIN,
                                std::min<int>(warpdrive::constants::WARP_MAX, m_warp_value));
    m_warp_value = clamped;

    XPLMSetDatai(m_ground_speed_ref, m_warp_value);
}
