#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMDataAccess.h"

#include "warp_controller.h"
#include "constants.h"

#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <cstdio>

// Global plugin state
static XPLMMenuID g_menu_id = nullptr;
static XPLMMenuID g_submenu_id = nullptr;
static std::vector<XPLMCommandRef> g_warp_commands;
static std::unique_ptr<WarpController> g_warp_controller;

// Menu item indices for dynamic updates
static int g_menu_header_item = -1;
static int g_menu_warp_up_item = -1;
static int g_menu_warp_down_item = -1;
static int g_menu_warp_reset_item = -1;

// Forward declarations
static void menu_handler(void* menu_ref, void* item_ref);
static int warp_command_handler(XPLMCommandRef command, XPLMCommandPhase phase, void* refcon);
static void create_warp_commands();
static void create_menu_system();
static void update_menu_display();

PLUGIN_API int XPluginStart(char* out_name, char* out_sig, char* out_desc)
{
    using namespace warpdrive::constants;

    std::string name = "WarpDrive";
    std::string sig = "warpdrive.plugin";
    std::string desc = "Ground speed warp control plugin for X-Plane";

    // Safely copy strings with proper null termination
    std::strncpy(out_name, name.c_str(), XPLANE_STRING_BUFFER_SIZE - 1);
    std::strncpy(out_sig, sig.c_str(), XPLANE_STRING_BUFFER_SIZE - 1);
    std::strncpy(out_desc, desc.c_str(), XPLANE_STRING_BUFFER_SIZE - 1);
    out_name[XPLANE_STRING_BUFFER_SIZE - 1] = '\0';
    out_sig[XPLANE_STRING_BUFFER_SIZE - 1] = '\0';
    out_desc[XPLANE_STRING_BUFFER_SIZE - 1] = '\0';

    // Initialize controller
    g_warp_controller = std::make_unique<WarpController>();
    if (!g_warp_controller->initialize()) {
        XPLMDebugString("WarpDrive: ERROR - Failed to initialize warp controller\n");
        return 0;
    }

    // Create plugin menu
    g_menu_id = XPLMFindPluginsMenu();
    if (g_menu_id) {
        int menu_item = XPLMAppendMenuItem(g_menu_id, "WarpDrive", nullptr, 1);
        g_submenu_id = XPLMCreateMenu("WarpDrive", g_menu_id, menu_item, menu_handler, nullptr);
        create_menu_system();
    }

    // Create custom commands
    create_warp_commands();

    update_menu_display();

    XPLMDebugString("WarpDrive: Plugin started successfully\n");
    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    // Cleanup commands
    for (auto command : g_warp_commands) {
        if (command) {
            XPLMUnregisterCommandHandler(command, warp_command_handler, 0, nullptr);
        }
    }
    g_warp_commands.clear();

    // Cleanup controller
    if (g_warp_controller) {
        g_warp_controller->cleanup();
        g_warp_controller.reset();
    }

    XPLMDebugString("WarpDrive: Plugin stopped\n");
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, int, void*)
{
}

static void create_warp_commands()
{
    struct CommandDef {
        const char* name;
        const char* description;
        warpdrive::constants::MenuItems action;
    };

    CommandDef commands[] = {
        {"warpdrive/warp_up",    "Increase warp value by 1",   warpdrive::constants::MENU_WARP_UP},
        {"warpdrive/warp_down",  "Decrease warp value by 1",   warpdrive::constants::MENU_WARP_DOWN},
        {"warpdrive/warp_reset", "Reset warp value to minimum", warpdrive::constants::MENU_WARP_RESET}
    };

    g_warp_commands.reserve(sizeof(commands) / sizeof(commands[0]));

    for (const auto& cmd : commands) {
        XPLMCommandRef command = XPLMCreateCommand(cmd.name, cmd.description);
        if (command) {
            intptr_t refcon = static_cast<intptr_t>(cmd.action);
            XPLMRegisterCommandHandler(command, warp_command_handler, 1, reinterpret_cast<void*>(refcon));
            g_warp_commands.push_back(command);

            std::string log_msg = "WarpDrive: Created command: ";
            log_msg += cmd.name;
            log_msg += "\n";
            XPLMDebugString(log_msg.c_str());
        } else {
            std::string error_msg = "WarpDrive: ERROR - Failed to create command: ";
            error_msg += cmd.name;
            error_msg += "\n";
            XPLMDebugString(error_msg.c_str());
        }
    }
}

static void create_menu_system()
{
    using namespace warpdrive::constants;

    if (!g_submenu_id) return;

    // Header showing current warp value (updated dynamically)
    g_menu_header_item = XPLMAppendMenuItem(g_submenu_id, "Warp: 1", nullptr, 0);

    XPLMAppendMenuSeparator(g_submenu_id);

    // Warp action items
    g_menu_warp_up_item = XPLMAppendMenuItem(g_submenu_id, "Warp Up",
                                             reinterpret_cast<void*>(MENU_WARP_UP), 1);
    g_menu_warp_down_item = XPLMAppendMenuItem(g_submenu_id, "Warp Down",
                                               reinterpret_cast<void*>(MENU_WARP_DOWN), 1);
    g_menu_warp_reset_item = XPLMAppendMenuItem(g_submenu_id, "Warp Reset",
                                                reinterpret_cast<void*>(MENU_WARP_RESET), 1);
}

static void menu_handler(void*, void* item_ref)
{
    using namespace warpdrive::constants;

    intptr_t item = reinterpret_cast<intptr_t>(item_ref);

    if (!g_warp_controller) {
        return;
    }

    switch (item) {
        case MENU_WARP_UP:
            g_warp_controller->warp_up();
            break;
        case MENU_WARP_DOWN:
            g_warp_controller->warp_down();
            break;
        case MENU_WARP_RESET:
            g_warp_controller->warp_reset();
            break;
        default:
            return;
    }

    update_menu_display();
}

static int warp_command_handler(XPLMCommandRef, XPLMCommandPhase phase, void* refcon)
{
    using namespace warpdrive::constants;

    if (phase != xplm_CommandBegin || !g_warp_controller) {
        return 0;
    }

    intptr_t action = reinterpret_cast<intptr_t>(refcon);
    switch (action) {
        case MENU_WARP_UP:
            g_warp_controller->warp_up();
            break;
        case MENU_WARP_DOWN:
            g_warp_controller->warp_down();
            break;
        case MENU_WARP_RESET:
            g_warp_controller->warp_reset();
            break;
        default:
            return 0;
    }

    update_menu_display();
    return 0;
}

static void update_menu_display()
{
    using namespace warpdrive::constants;

    if (!g_submenu_id || !g_warp_controller) return;

    int warp_value = g_warp_controller->get_warp_value();

    if (g_menu_header_item >= 0) {
        char header_text[MENU_TEXT_BUFFER_SIZE];
        int result = std::snprintf(header_text, sizeof(header_text),
                                   "Warp: %d / %d", warp_value, WARP_MAX);
        if (result >= 0 && result < static_cast<int>(sizeof(header_text))) {
            XPLMSetMenuItemName(g_submenu_id, g_menu_header_item, header_text, 0);
        }
    }

    if (g_menu_warp_up_item >= 0) {
        XPLMEnableMenuItem(g_submenu_id, g_menu_warp_up_item,
                           warp_value < WARP_MAX ? 1 : 0);
    }
    if (g_menu_warp_down_item >= 0) {
        XPLMEnableMenuItem(g_submenu_id, g_menu_warp_down_item,
                           warp_value > WARP_MIN ? 1 : 0);
    }
    if (g_menu_warp_reset_item >= 0) {
        XPLMEnableMenuItem(g_submenu_id, g_menu_warp_reset_item,
                           warp_value != WARP_DEFAULT ? 1 : 0);
    }
}
