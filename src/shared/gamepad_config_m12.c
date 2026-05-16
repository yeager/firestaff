/*
 * gamepad_config_m12.c — Controller binding UI for the Firestaff launcher.
 *
 * Stores an M12_GamepadMap with per-action button bindings and per-axis
 * configuration.  Persists to gamepad.toml in the Firestaff config
 * directory.  Named profiles are stored as gamepad-<name>.toml.
 */

#include "gamepad_config_m12.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Default button bindings ────────────────────────────────────────── */

static const struct {
    M12_InputAction   action;
    SDL_GamepadButton primary;
    SDL_GamepadButton secondary;
} s_button_defaults[] = {
    { M12_ACTION_MOVE_FORWARD,     SDL_GAMEPAD_BUTTON_DPAD_UP,       SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_MOVE_BACKWARD,    SDL_GAMEPAD_BUTTON_DPAD_DOWN,     SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_TURN_LEFT,        SDL_GAMEPAD_BUTTON_DPAD_LEFT,     SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_TURN_RIGHT,       SDL_GAMEPAD_BUTTON_DPAD_RIGHT,    SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_STRAFE_LEFT,      SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_STRAFE_RIGHT,     SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_ACCEPT,           SDL_GAMEPAD_BUTTON_SOUTH,         SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_BACK,             SDL_GAMEPAD_BUTTON_EAST,          SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_ACTION,           SDL_GAMEPAD_BUTTON_WEST,          SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_CYCLE_CHAMPION,   SDL_GAMEPAD_BUTTON_NORTH,         SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_REST_TOGGLE,      SDL_GAMEPAD_BUTTON_BACK,          SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_INVENTORY_TOGGLE, SDL_GAMEPAD_BUTTON_START,         SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_SPELL_CAST,       SDL_GAMEPAD_BUTTON_RIGHT_STICK,   SDL_GAMEPAD_BUTTON_INVALID },
    { M12_ACTION_MAP_TOGGLE,       SDL_GAMEPAD_BUTTON_LEFT_STICK,    SDL_GAMEPAD_BUTTON_INVALID },
};

#define BUTTON_DEFAULT_COUNT (sizeof(s_button_defaults) / sizeof(s_button_defaults[0]))

/* ── Default axis configuration ─────────────────────────────────────── */

static const struct {
    SDL_GamepadAxis     axis;
    M12_AxisRole        role;
} s_axis_defaults[] = {
    { SDL_GAMEPAD_AXIS_LEFTX,          M12_AXIS_ROLE_MOVE },
    { SDL_GAMEPAD_AXIS_LEFTY,          M12_AXIS_ROLE_MOVE },
    { SDL_GAMEPAD_AXIS_RIGHTX,         M12_AXIS_ROLE_TURN },
    { SDL_GAMEPAD_AXIS_RIGHTY,         M12_AXIS_ROLE_LOOK },
    { SDL_GAMEPAD_AXIS_LEFT_TRIGGER,   M12_AXIS_ROLE_NONE },
    { SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,  M12_AXIS_ROLE_NONE },
};

#define AXIS_DEFAULT_COUNT (sizeof(s_axis_defaults) / sizeof(s_axis_defaults[0]))

/* ── Action name table (reuse from input_remap_m12 via extern, but we
 *    keep our own copy for the gamepad config-file keys) ────────────── */

static const char* s_action_names[] = {
    "move_forward",
    "move_backward",
    "turn_left",
    "turn_right",
    "strafe_left",
    "strafe_right",
    "accept",
    "back",
    "action",
    "cycle_champion",
    "rest_toggle",
    "use_stairs",
    "pickup_item",
    "drop_item",
    "spell_rune_1",
    "spell_rune_2",
    "spell_rune_3",
    "spell_rune_4",
    "spell_rune_5",
    "spell_rune_6",
    "spell_cast",
    "spell_clear",
    "use_item",
    "map_toggle",
    "inventory_toggle",
    "quick_save",
    "quick_load",
};

/* ── Button name table ──────────────────────────────────────────────── */

typedef struct {
    SDL_GamepadButton button;
    const char*       name;
    const char*       display;
} M12_GpadButtonName;

static const M12_GpadButtonName s_button_names[] = {
    { SDL_GAMEPAD_BUTTON_SOUTH,          "south",          "A / Cross"       },
    { SDL_GAMEPAD_BUTTON_EAST,           "east",           "B / Circle"      },
    { SDL_GAMEPAD_BUTTON_WEST,           "west",           "X / Square"      },
    { SDL_GAMEPAD_BUTTON_NORTH,          "north",          "Y / Triangle"    },
    { SDL_GAMEPAD_BUTTON_BACK,           "back",           "Back / Select"   },
    { SDL_GAMEPAD_BUTTON_GUIDE,          "guide",          "Guide / Home"    },
    { SDL_GAMEPAD_BUTTON_START,          "start",          "Start / Options" },
    { SDL_GAMEPAD_BUTTON_LEFT_STICK,     "left_stick",     "Left Stick"      },
    { SDL_GAMEPAD_BUTTON_RIGHT_STICK,    "right_stick",    "Right Stick"     },
    { SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  "left_shoulder",  "LB / L1"         },
    { SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, "right_shoulder", "RB / R1"         },
    { SDL_GAMEPAD_BUTTON_DPAD_UP,        "dpad_up",        "D-Pad Up"        },
    { SDL_GAMEPAD_BUTTON_DPAD_DOWN,      "dpad_down",      "D-Pad Down"      },
    { SDL_GAMEPAD_BUTTON_DPAD_LEFT,      "dpad_left",      "D-Pad Left"      },
    { SDL_GAMEPAD_BUTTON_DPAD_RIGHT,     "dpad_right",     "D-Pad Right"     },
    { SDL_GAMEPAD_BUTTON_MISC1,          "misc1",          "Misc 1"          },
    { SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,  "right_paddle1",  "Right Paddle 1"  },
    { SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,   "left_paddle1",   "Left Paddle 1"   },
    { SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,  "right_paddle2",  "Right Paddle 2"  },
    { SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,   "left_paddle2",   "Left Paddle 2"   },
    { SDL_GAMEPAD_BUTTON_TOUCHPAD,       "touchpad",       "Touchpad"        },
};

#define BUTTON_NAME_COUNT (sizeof(s_button_names) / sizeof(s_button_names[0]))

/* ── Axis name table ────────────────────────────────────────────────── */

typedef struct {
    SDL_GamepadAxis axis;
    const char*     name;
    const char*     display;
} M12_GpadAxisName;

static const M12_GpadAxisName s_axis_names[] = {
    { SDL_GAMEPAD_AXIS_LEFTX,          "left_x",          "Left Stick X"    },
    { SDL_GAMEPAD_AXIS_LEFTY,          "left_y",          "Left Stick Y"    },
    { SDL_GAMEPAD_AXIS_RIGHTX,         "right_x",         "Right Stick X"   },
    { SDL_GAMEPAD_AXIS_RIGHTY,         "right_y",         "Right Stick Y"   },
    { SDL_GAMEPAD_AXIS_LEFT_TRIGGER,   "left_trigger",    "Left Trigger"    },
    { SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,  "right_trigger",   "Right Trigger"   },
};

#define AXIS_NAME_COUNT (sizeof(s_axis_names) / sizeof(s_axis_names[0]))

/* ── Axis role name table ───────────────────────────────────────────── */

static const char* s_role_names[] = {
    "none",
    "move",
    "turn",
    "look",
};

/* ── Name ↔ enum helpers ────────────────────────────────────────────── */

static const char* m12_button_to_name(SDL_GamepadButton button) {
    size_t i;
    if (button == SDL_GAMEPAD_BUTTON_INVALID) return "none";
    for (i = 0; i < BUTTON_NAME_COUNT; ++i) {
        if (s_button_names[i].button == button) return s_button_names[i].name;
    }
    return "none";
}

static SDL_GamepadButton m12_name_to_button(const char* name) {
    size_t i;
    if (!name || name[0] == '\0' || strcmp(name, "none") == 0)
        return SDL_GAMEPAD_BUTTON_INVALID;
    for (i = 0; i < BUTTON_NAME_COUNT; ++i) {
        if (strcmp(s_button_names[i].name, name) == 0)
            return s_button_names[i].button;
    }
    return SDL_GAMEPAD_BUTTON_INVALID;
}

static const char* m12_axis_to_name(SDL_GamepadAxis axis) {
    size_t i;
    if (axis == SDL_GAMEPAD_AXIS_INVALID) return "none";
    for (i = 0; i < AXIS_NAME_COUNT; ++i) {
        if (s_axis_names[i].axis == axis) return s_axis_names[i].name;
    }
    return "none";
}

static SDL_GamepadAxis m12_name_to_axis(const char* name) {
    size_t i;
    if (!name || name[0] == '\0' || strcmp(name, "none") == 0)
        return SDL_GAMEPAD_AXIS_INVALID;
    for (i = 0; i < AXIS_NAME_COUNT; ++i) {
        if (strcmp(s_axis_names[i].name, name) == 0)
            return s_axis_names[i].axis;
    }
    return SDL_GAMEPAD_AXIS_INVALID;
}

static const char* m12_role_to_name(M12_AxisRole role) {
    if (role >= 0 && role < M12_AXIS_ROLE_COUNT) return s_role_names[role];
    return "none";
}

static M12_AxisRole m12_name_to_role(const char* name) {
    int i;
    if (!name || name[0] == '\0') return M12_AXIS_ROLE_NONE;
    for (i = 0; i < M12_AXIS_ROLE_COUNT; ++i) {
        if (strcmp(s_role_names[i], name) == 0) return (M12_AxisRole)i;
    }
    return M12_AXIS_ROLE_NONE;
}

/* ── Trim helper ────────────────────────────────────────────────────── */

static void m12_gpad_trim(char* text) {
    char* start;
    char* end;
    size_t len;
    if (!text) return;
    start = text;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
        ++start;
    if (start != text) memmove(text, start, strlen(start) + 1U);
    len = strlen(text);
    while (len > 0U) {
        end = &text[len - 1U];
        if (*end != ' ' && *end != '\t' && *end != '\r' && *end != '\n') break;
        *end = '\0';
        --len;
    }
}

static int m12_gpad_parse_int(const char* value, int fallback) {
    char* end = NULL;
    long parsed;
    if (!value || value[0] == '\0') return fallback;
    parsed = strtol(value, &end, 10);
    if (end == value) return fallback;
    return (int)parsed;
}

/* ── Config file path helpers ───────────────────────────────────────── */

static int m12_gamepad_config_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, "gamepad.toml")) {
            return 1;
        }
    }
    snprintf(out, outSize, "gamepad.toml");
    return 1;
}

static int m12_gamepad_profile_path(char* out, size_t outSize,
                                    const char* profileName) {
    char configDir[FSP_PATH_MAX];
    char filename[128];
    if (!profileName || profileName[0] == '\0') return 0;
    snprintf(filename, sizeof(filename), "gamepad-%s.toml", profileName);
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, filename)) {
            return 1;
        }
    }
    snprintf(out, outSize, "%s", filename);
    return 1;
}

/* ── Set defaults ───────────────────────────────────────────────────── */

void M12_GamepadMap_SetDefaults(M12_GamepadMap* map) {
    size_t i;
    if (!map) return;
    memset(map, 0, sizeof(*map));
    map->enabled = 1;

    /* Initialize all button bindings to unbound */
    for (i = 0; i < (size_t)M12_ACTION_COUNT; ++i) {
        map->buttons[i].action    = (M12_InputAction)i;
        map->buttons[i].primary   = SDL_GAMEPAD_BUTTON_INVALID;
        map->buttons[i].secondary = SDL_GAMEPAD_BUTTON_INVALID;
    }

    /* Apply default button mappings */
    for (i = 0; i < BUTTON_DEFAULT_COUNT; ++i) {
        int idx = (int)s_button_defaults[i].action;
        if (idx >= 0 && idx < M12_ACTION_COUNT) {
            map->buttons[idx].primary   = s_button_defaults[i].primary;
            map->buttons[idx].secondary = s_button_defaults[i].secondary;
        }
    }

    /* Initialize all axes to defaults */
    for (i = 0; i < (size_t)SDL_GAMEPAD_AXIS_COUNT; ++i) {
        map->axes[i].role        = M12_AXIS_ROLE_NONE;
        map->axes[i].deadzone    = M12_GAMEPAD_AXIS_DEADZONE_DEFAULT;
        map->axes[i].sensitivity = M12_GAMEPAD_SENSITIVITY_DEFAULT;
        map->axes[i].inverted    = 0;
    }

    /* Apply default axis roles */
    for (i = 0; i < AXIS_DEFAULT_COUNT; ++i) {
        int idx = (int)s_axis_defaults[i].axis;
        if (idx >= 0 && idx < SDL_GAMEPAD_AXIS_COUNT) {
            map->axes[idx].role = s_axis_defaults[i].role;
        }
    }
}

/* ── Persistence: write a map to a file ─────────────────────────────── */

static int m12_gamepad_save_to_file(const M12_GamepadMap* map,
                                    const char* path) {
    FILE* fp;
    int i;
    if (!map || !path) return 0;

    fp = fopen(path, "w");
    if (!fp) return 0;

    fprintf(fp, "# Firestaff gamepad configuration\n");
    fprintf(fp, "# Delete this file to reset to defaults.\n\n");

    fprintf(fp, "[general]\n");
    fprintf(fp, "enabled = %d\n\n", map->enabled);

    fprintf(fp, "[buttons]\n");
    fprintf(fp, "# Format: action_name = primary_button [, secondary_button]\n");
    for (i = 0; i < M12_ACTION_COUNT; ++i) {
        const M12_GamepadButtonBinding* b = &map->buttons[i];
        const char* pName = m12_button_to_name(b->primary);
        const char* sName = m12_button_to_name(b->secondary);

        if (b->secondary != SDL_GAMEPAD_BUTTON_INVALID) {
            fprintf(fp, "%s = %s, %s\n", s_action_names[i], pName, sName);
        } else {
            fprintf(fp, "%s = %s\n", s_action_names[i], pName);
        }
    }

    fprintf(fp, "\n[axes]\n");
    fprintf(fp, "# Format: axis_name.property = value\n");
    for (i = 0; i < SDL_GAMEPAD_AXIS_COUNT; ++i) {
        const M12_GamepadAxisConfig* a = &map->axes[i];
        const char* axName = m12_axis_to_name((SDL_GamepadAxis)i);
        if (strcmp(axName, "none") == 0) continue;

        fprintf(fp, "%s.role = %s\n", axName, m12_role_to_name(a->role));
        fprintf(fp, "%s.deadzone = %d\n", axName, a->deadzone);
        fprintf(fp, "%s.sensitivity = %d\n", axName, a->sensitivity);
        fprintf(fp, "%s.inverted = %d\n", axName, a->inverted);
    }

    fclose(fp);
    return 1;
}

/* ── Persistence: read a map from a file ────────────────────────────── */

static int m12_gamepad_load_from_file(M12_GamepadMap* map,
                                      const char* path) {
    char line[512];
    FILE* fp;
    int section = 0;  /* 0=none, 1=general, 2=buttons, 3=axes */

    if (!map || !path) return 0;

    M12_GamepadMap_SetDefaults(map);

    fp = fopen(path, "r");
    if (!fp) return 0;

    while (fgets(line, (int)sizeof(line), fp)) {
        char* equals;
        char* key;
        char* value;

        m12_gpad_trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        /* Section headers */
        if (line[0] == '[') {
            if (strstr(line, "[general]"))     section = 1;
            else if (strstr(line, "[buttons]")) section = 2;
            else if (strstr(line, "[axes]"))    section = 3;
            else section = 0;
            continue;
        }

        equals = strchr(line, '=');
        if (!equals) continue;

        *equals = '\0';
        key = line;
        value = equals + 1;
        m12_gpad_trim(key);
        m12_gpad_trim(value);

        if (section == 1) {
            /* [general] */
            if (strcmp(key, "enabled") == 0) {
                map->enabled = m12_gpad_parse_int(value, 1);
            }
        } else if (section == 2) {
            /* [buttons] — action = button [, button] */
            int actionIdx = -1;
            int ai;
            for (ai = 0; ai < M12_ACTION_COUNT; ++ai) {
                if (strcmp(key, s_action_names[ai]) == 0) {
                    actionIdx = ai;
                    break;
                }
            }
            if (actionIdx < 0) continue;

            {
                char* comma = strchr(value, ',');
                if (comma) {
                    char primaryStr[64];
                    char secondaryStr[64];
                    size_t plen = (size_t)(comma - value);
                    if (plen >= sizeof(primaryStr)) plen = sizeof(primaryStr) - 1;
                    memcpy(primaryStr, value, plen);
                    primaryStr[plen] = '\0';
                    snprintf(secondaryStr, sizeof(secondaryStr), "%s", comma + 1);
                    m12_gpad_trim(primaryStr);
                    m12_gpad_trim(secondaryStr);
                    map->buttons[actionIdx].primary   = m12_name_to_button(primaryStr);
                    map->buttons[actionIdx].secondary  = m12_name_to_button(secondaryStr);
                } else {
                    map->buttons[actionIdx].primary   = m12_name_to_button(value);
                    map->buttons[actionIdx].secondary  = SDL_GAMEPAD_BUTTON_INVALID;
                }
            }
        } else if (section == 3) {
            /* [axes] — axis_name.property = value */
            char* dot = strchr(key, '.');
            if (!dot) continue;
            *dot = '\0';
            {
                const char* axisName = key;
                const char* prop     = dot + 1;
                SDL_GamepadAxis axis = m12_name_to_axis(axisName);
                int axIdx;

                if (axis == SDL_GAMEPAD_AXIS_INVALID) continue;
                axIdx = (int)axis;
                if (axIdx < 0 || axIdx >= SDL_GAMEPAD_AXIS_COUNT) continue;

                if (strcmp(prop, "role") == 0) {
                    map->axes[axIdx].role = m12_name_to_role(value);
                } else if (strcmp(prop, "deadzone") == 0) {
                    int dz = m12_gpad_parse_int(value, M12_GAMEPAD_AXIS_DEADZONE_DEFAULT);
                    if (dz < M12_GAMEPAD_AXIS_DEADZONE_MIN) dz = M12_GAMEPAD_AXIS_DEADZONE_MIN;
                    if (dz > M12_GAMEPAD_AXIS_DEADZONE_MAX) dz = M12_GAMEPAD_AXIS_DEADZONE_MAX;
                    map->axes[axIdx].deadzone = dz;
                } else if (strcmp(prop, "sensitivity") == 0) {
                    int s = m12_gpad_parse_int(value, M12_GAMEPAD_SENSITIVITY_DEFAULT);
                    if (s < M12_GAMEPAD_SENSITIVITY_MIN) s = M12_GAMEPAD_SENSITIVITY_MIN;
                    if (s > M12_GAMEPAD_SENSITIVITY_MAX) s = M12_GAMEPAD_SENSITIVITY_MAX;
                    map->axes[axIdx].sensitivity = s;
                } else if (strcmp(prop, "inverted") == 0) {
                    map->axes[axIdx].inverted = (m12_gpad_parse_int(value, 0) != 0) ? 1 : 0;
                }
            }
        }
    }

    fclose(fp);
    return 1;
}

/* ── Public persistence API ─────────────────────────────────────────── */

int M12_GamepadMap_Load(M12_GamepadMap* map) {
    char path[FSP_PATH_MAX];
    if (!map) return 0;
    if (!m12_gamepad_config_path(path, sizeof(path))) return 0;
    return m12_gamepad_load_from_file(map, path);
}

int M12_GamepadMap_Save(const M12_GamepadMap* map) {
    char path[FSP_PATH_MAX];
    char configDir[FSP_PATH_MAX];
    if (!map) return 0;
    if (!m12_gamepad_config_path(path, sizeof(path))) return 0;

    /* Ensure config directory exists */
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        FSP_CreateDirectoryRecursive(configDir);
    }

    return m12_gamepad_save_to_file(map, path);
}

/* ── Button queries ─────────────────────────────────────────────────── */

SDL_GamepadButton M12_GamepadMap_GetButton(const M12_GamepadMap* map,
                                           M12_InputAction action) {
    if (!map || action < 0 || action >= M12_ACTION_COUNT)
        return SDL_GAMEPAD_BUTTON_INVALID;
    return map->buttons[action].primary;
}

SDL_GamepadButton M12_GamepadMap_GetSecondaryButton(const M12_GamepadMap* map,
                                                    M12_InputAction action) {
    if (!map || action < 0 || action >= M12_ACTION_COUNT)
        return SDL_GAMEPAD_BUTTON_INVALID;
    return map->buttons[action].secondary;
}

M12_InputAction M12_GamepadMap_ActionForButton(const M12_GamepadMap* map,
                                               SDL_GamepadButton button) {
    int i;
    if (!map || button == SDL_GAMEPAD_BUTTON_INVALID) return M12_ACTION_COUNT;
    for (i = 0; i < M12_ACTION_COUNT; ++i) {
        if (map->buttons[i].primary == button ||
            map->buttons[i].secondary == button) {
            return (M12_InputAction)i;
        }
    }
    return M12_ACTION_COUNT;
}

/* ── Axis queries & processing ──────────────────────────────────────── */

const M12_GamepadAxisConfig* M12_GamepadMap_GetAxisConfig(
    const M12_GamepadMap* map, SDL_GamepadAxis axis) {
    if (!map || axis < 0 || axis >= SDL_GAMEPAD_AXIS_COUNT) return NULL;
    return &map->axes[axis];
}

int M12_GamepadAxis_Process(const M12_GamepadAxisConfig* cfg, int rawValue) {
    int sign;
    int magnitude;
    int range;
    int scaled;

    if (!cfg) return rawValue;

    /* Apply inversion */
    if (cfg->inverted) rawValue = -rawValue;

    /* Apply dead-zone */
    sign = (rawValue >= 0) ? 1 : -1;
    magnitude = (rawValue >= 0) ? rawValue : -rawValue;

    if (magnitude < cfg->deadzone) return 0;

    /* Scale remaining range to full output range with sensitivity */
    range = 32767 - cfg->deadzone;
    if (range <= 0) return 0;

    magnitude -= cfg->deadzone;

    /* Sensitivity multiplier: 1 = 0.2x, 5 = 1.0x, 10 = 2.0x */
    scaled = (int)((long)magnitude * (long)cfg->sensitivity * 2L /
                   (long)(M12_GAMEPAD_SENSITIVITY_DEFAULT * 2) *
                   32767L / (long)range);

    if (scaled > 32767) scaled = 32767;

    return sign * scaled;
}

/* ── Connection monitoring ──────────────────────────────────────────── */

void M12_GamepadStatus_Update(M12_GamepadStatus* status) {
    int count = 0;
    SDL_JoystickID* gamepads;

    if (!status) return;

    /* If we have a handle, check if it's still connected */
    if (status->handle) {
        if (SDL_GamepadConnected(status->handle)) {
            status->connected = 1;
            return;
        }
        /* Lost connection */
        SDL_CloseGamepad(status->handle);
        status->handle     = NULL;
        status->connected  = 0;
        status->instanceId = 0;
        status->name[0]    = '\0';
    }

    /* Try to find and open the first available gamepad */
    gamepads = SDL_GetGamepads(&count);
    if (gamepads && count > 0) {
        SDL_Gamepad* gp = SDL_OpenGamepad(gamepads[0]);
        if (gp) {
            const char* gpName = SDL_GetGamepadName(gp);
            status->connected  = 1;
            status->instanceId = gamepads[0];
            status->handle     = gp;
            if (gpName) {
                snprintf(status->name, sizeof(status->name), "%s", gpName);
            } else {
                snprintf(status->name, sizeof(status->name), "Unknown Controller");
            }
        }
        SDL_free(gamepads);
    } else {
        status->connected  = 0;
        status->instanceId = 0;
        status->name[0]    = '\0';
        if (gamepads) SDL_free(gamepads);
    }
}

void M12_GamepadStatus_Close(M12_GamepadStatus* status) {
    if (!status) return;
    if (status->handle) {
        SDL_CloseGamepad(status->handle);
        status->handle = NULL;
    }
    status->connected  = 0;
    status->instanceId = 0;
    status->name[0]    = '\0';
}

/* ── Profile management ─────────────────────────────────────────────── */

int M12_GamepadProfile_Save(const char* profileName,
                            const M12_GamepadMap* map) {
    char path[FSP_PATH_MAX];
    char configDir[FSP_PATH_MAX];
    if (!profileName || !map) return 0;
    if (profileName[0] == '\0') return 0;
    if (!m12_gamepad_profile_path(path, sizeof(path), profileName)) return 0;

    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        FSP_CreateDirectoryRecursive(configDir);
    }

    return m12_gamepad_save_to_file(map, path);
}

int M12_GamepadProfile_Load(const char* profileName,
                            M12_GamepadMap* map) {
    char path[FSP_PATH_MAX];
    if (!profileName || !map) return 0;
    if (profileName[0] == '\0') return 0;
    if (!m12_gamepad_profile_path(path, sizeof(path), profileName)) return 0;
    return m12_gamepad_load_from_file(map, path);
}

int M12_GamepadProfile_Delete(const char* profileName) {
    char path[FSP_PATH_MAX];
    if (!profileName || profileName[0] == '\0') return 0;
    if (!m12_gamepad_profile_path(path, sizeof(path), profileName)) return 0;
    return (remove(path) == 0) ? 1 : 0;
}

int M12_GamepadProfile_List(char namesOut[][M12_GAMEPAD_PROFILE_NAME_MAX],
                            int maxProfiles) {
    char configDir[FSP_PATH_MAX];
    char pattern[FSP_PATH_MAX];
    int found = 0;

    if (!namesOut || maxProfiles <= 0) return 0;
    if (!FSP_GetUserConfigDir(configDir, sizeof(configDir))) return 0;

    /*
     * Scan the config directory for gamepad-*.toml files.
     * We use a simple fopen probe for well-known profile names since
     * the portable compat layer doesn't expose directory iteration.
     * For a full implementation, you'd use opendir/readdir or
     * FindFirstFile, but this keeps us within the FSP abstraction.
     *
     * Instead, we try common profile names and names from the
     * current config.  This is a pragmatic simplification.
     */
    {
        static const char* s_probe_names[] = {
            "default", "dm1", "dm2", "csb", "custom",
            "profile1", "profile2", "profile3", "profile4",
            NULL
        };
        int pi;
        for (pi = 0; s_probe_names[pi] && found < maxProfiles; ++pi) {
            if (m12_gamepad_profile_path(pattern, sizeof(pattern),
                                         s_probe_names[pi])) {
                if (FSP_FileExists(pattern)) {
                    snprintf(namesOut[found], M12_GAMEPAD_PROFILE_NAME_MAX,
                             "%s", s_probe_names[pi]);
                    ++found;
                }
            }
        }
    }

    return found;
}

/* ── Human-readable names ───────────────────────────────────────────── */

const char* M12_GamepadButton_Name(SDL_GamepadButton button) {
    size_t i;
    if (button == SDL_GAMEPAD_BUTTON_INVALID) return "None";
    for (i = 0; i < BUTTON_NAME_COUNT; ++i) {
        if (s_button_names[i].button == button) return s_button_names[i].display;
    }
    return "Unknown";
}

const char* M12_GamepadAxis_Name(SDL_GamepadAxis axis) {
    size_t i;
    if (axis == SDL_GAMEPAD_AXIS_INVALID) return "None";
    for (i = 0; i < AXIS_NAME_COUNT; ++i) {
        if (s_axis_names[i].axis == axis) return s_axis_names[i].display;
    }
    return "Unknown";
}

const char* M12_AxisRole_Name(M12_AxisRole role) {
    switch (role) {
        case M12_AXIS_ROLE_NONE: return "Disabled";
        case M12_AXIS_ROLE_MOVE: return "Movement";
        case M12_AXIS_ROLE_TURN: return "Turning";
        case M12_AXIS_ROLE_LOOK: return "Look";
        default:                 return "Unknown";
    }
}

/* ── Remap helpers ──────────────────────────────────────────────────── */

void M12_GamepadRemap_Begin(M12_GamepadRemapState* state,
                            M12_InputAction action, int slot) {
    if (!state) return;
    state->active       = 1;
    state->targetAction = action;
    state->targetSlot   = slot;
}

int M12_GamepadRemap_HandleButton(M12_GamepadRemapState* state,
                                  M12_GamepadMap* map,
                                  SDL_GamepadButton button) {
    if (!state || !state->active || !map) return 0;

    /* Back or Start cancels the remap */
    if (button == SDL_GAMEPAD_BUTTON_BACK ||
        button == SDL_GAMEPAD_BUTTON_START) {
        M12_GamepadRemap_Cancel(state);
        return 0;
    }

    /* Unbind this button from any other action to avoid duplicates */
    {
        int i;
        for (i = 0; i < M12_ACTION_COUNT; ++i) {
            if (map->buttons[i].primary == button)
                map->buttons[i].primary = SDL_GAMEPAD_BUTTON_INVALID;
            if (map->buttons[i].secondary == button)
                map->buttons[i].secondary = SDL_GAMEPAD_BUTTON_INVALID;
        }
    }

    /* Assign */
    if (state->targetSlot == 0) {
        map->buttons[state->targetAction].primary = button;
    } else {
        map->buttons[state->targetAction].secondary = button;
    }

    state->active = 0;
    return 1;
}

void M12_GamepadRemap_Cancel(M12_GamepadRemapState* state) {
    if (!state) return;
    state->active = 0;
}
