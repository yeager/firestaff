/*
 * input_remap_m12.c — Configurable key bindings for the Firestaff launcher.
 *
 * Stores an M12_InputMap with per-action primary + secondary SDL keycodes.
 * Persists to keybindings.toml in the Firestaff config directory.
 */

#include "input_remap_m12.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Default bindings ───────────────────────────────────────────────── */

static const struct {
    M12_InputAction action;
    SDL_Keycode     primary;
    SDL_Keycode     secondary;
} s_defaults[] = {
    { M12_ACTION_MOVE_FORWARD,     SDLK_UP,        SDLK_W       },
    { M12_ACTION_MOVE_BACKWARD,    SDLK_DOWN,      SDLK_S       },
    { M12_ACTION_TURN_LEFT,        SDLK_LEFT,      SDLK_Q       },
    { M12_ACTION_TURN_RIGHT,       SDLK_RIGHT,     SDLK_E       },
    { M12_ACTION_STRAFE_LEFT,      SDLK_A,         0            },
    { M12_ACTION_STRAFE_RIGHT,     SDLK_D,         0            },
    { M12_ACTION_ACCEPT,           SDLK_RETURN,    SDLK_KP_ENTER },
    { M12_ACTION_BACK,             SDLK_ESCAPE,    0            },
    { M12_ACTION_ACTION,           SDLK_SPACE,     0            },
    { M12_ACTION_CYCLE_CHAMPION,   SDLK_TAB,       0            },
    { M12_ACTION_REST_TOGGLE,      SDLK_R,         0            },
    { M12_ACTION_USE_STAIRS,       SDLK_X,         0            },
    { M12_ACTION_PICKUP_ITEM,      SDLK_G,         0            },
    { M12_ACTION_DROP_ITEM,        SDLK_P,         0            },
    { M12_ACTION_SPELL_RUNE_1,     SDLK_1,         0            },
    { M12_ACTION_SPELL_RUNE_2,     SDLK_2,         0            },
    { M12_ACTION_SPELL_RUNE_3,     SDLK_3,         0            },
    { M12_ACTION_SPELL_RUNE_4,     SDLK_4,         0            },
    { M12_ACTION_SPELL_RUNE_5,     SDLK_5,         0            },
    { M12_ACTION_SPELL_RUNE_6,     SDLK_6,         0            },
    { M12_ACTION_SPELL_CAST,       SDLK_C,         0            },
    { M12_ACTION_SPELL_CLEAR,      SDLK_U,         0            },
    { M12_ACTION_USE_ITEM,         SDLK_I,         0            },
    { M12_ACTION_MAP_TOGGLE,       SDLK_M,         0            },
    { M12_ACTION_INVENTORY_TOGGLE, SDLK_V,         0            },
    { M12_ACTION_QUICK_SAVE,       SDLK_F5,        0            },
    { M12_ACTION_QUICK_LOAD,       SDLK_F9,        0            },
};

#define DEFAULT_COUNT (sizeof(s_defaults) / sizeof(s_defaults[0]))

/* ── Action name table (config-file identifiers) ────────────────────── */

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

/* ── Human-readable display names ───────────────────────────────────── */

static const char* s_action_display_names[] = {
    "MOVE FORWARD",
    "MOVE BACKWARD",
    "TURN LEFT",
    "TURN RIGHT",
    "STRAFE LEFT",
    "STRAFE RIGHT",
    "ACCEPT / SELECT",
    "BACK / CANCEL",
    "ACTION",
    "CYCLE CHAMPION",
    "REST TOGGLE",
    "USE STAIRS",
    "PICKUP ITEM",
    "DROP ITEM",
    "SPELL RUNE 1",
    "SPELL RUNE 2",
    "SPELL RUNE 3",
    "SPELL RUNE 4",
    "SPELL RUNE 5",
    "SPELL RUNE 6",
    "SPELL CAST",
    "SPELL CLEAR",
    "USE ITEM",
    "MAP TOGGLE",
    "INVENTORY TOGGLE",
    "QUICK SAVE",
    "QUICK LOAD",
};

/* ── Keycode ↔ name mapping ─────────────────────────────────────────── */

typedef struct {
    SDL_Keycode key;
    const char* name;
} M12_KeyName;

static const M12_KeyName s_key_names[] = {
    { SDLK_UP,         "up"        },
    { SDLK_DOWN,       "down"      },
    { SDLK_LEFT,       "left"      },
    { SDLK_RIGHT,      "right"     },
    { SDLK_RETURN,     "return"    },
    { SDLK_KP_ENTER,   "kp_enter"  },
    { SDLK_SPACE,      "space"     },
    { SDLK_ESCAPE,     "escape"    },
    { SDLK_TAB,        "tab"       },
    { SDLK_BACKSPACE,  "backspace" },
    { SDLK_DELETE,     "delete"    },
    { SDLK_INSERT,     "insert"    },
    { SDLK_HOME,       "home"      },
    { SDLK_END,        "end"       },
    { SDLK_PAGEUP,     "pageup"    },
    { SDLK_PAGEDOWN,   "pagedown"  },
    { SDLK_F1,         "f1"        },
    { SDLK_F2,         "f2"        },
    { SDLK_F3,         "f3"        },
    { SDLK_F4,         "f4"        },
    { SDLK_F5,         "f5"        },
    { SDLK_F6,         "f6"        },
    { SDLK_F7,         "f7"        },
    { SDLK_F8,         "f8"        },
    { SDLK_F9,         "f9"        },
    { SDLK_F10,        "f10"       },
    { SDLK_F11,        "f11"       },
    { SDLK_F12,        "f12"       },
    { SDLK_A,          "a"         },
    { SDLK_B,          "b"         },
    { SDLK_C,          "c"         },
    { SDLK_D,          "d"         },
    { SDLK_E,          "e"         },
    { SDLK_F,          "f"         },
    { SDLK_G,          "g"         },
    { SDLK_H,          "h"         },
    { SDLK_I,          "i"         },
    { SDLK_J,          "j"         },
    { SDLK_K,          "k"         },
    { SDLK_L,          "l"         },
    { SDLK_M,          "m"         },
    { SDLK_N,          "n"         },
    { SDLK_O,          "o"         },
    { SDLK_P,          "p"         },
    { SDLK_Q,          "q"         },
    { SDLK_R,          "r"         },
    { SDLK_S,          "s"         },
    { SDLK_T,          "t"         },
    { SDLK_U,          "u"         },
    { SDLK_V,          "v"         },
    { SDLK_W,          "w"         },
    { SDLK_X,          "x"         },
    { SDLK_Y,          "y"         },
    { SDLK_Z,          "z"         },
    { SDLK_0,          "0"         },
    { SDLK_1,          "1"         },
    { SDLK_2,          "2"         },
    { SDLK_3,          "3"         },
    { SDLK_4,          "4"         },
    { SDLK_5,          "5"         },
    { SDLK_6,          "6"         },
    { SDLK_7,          "7"         },
    { SDLK_8,          "8"         },
    { SDLK_9,          "9"         },
    { SDLK_LSHIFT,     "lshift"    },
    { SDLK_RSHIFT,     "rshift"    },
    { SDLK_LCTRL,      "lctrl"     },
    { SDLK_RCTRL,      "rctrl"     },
    { SDLK_LALT,       "lalt"      },
    { SDLK_RALT,       "ralt"      },
};

#define KEY_NAME_COUNT (sizeof(s_key_names) / sizeof(s_key_names[0]))

static const char* m12_keycode_to_name(SDL_Keycode key) {
    size_t i;
    if (key == 0) return "none";
    for (i = 0; i < KEY_NAME_COUNT; ++i) {
        if (s_key_names[i].key == key) return s_key_names[i].name;
    }
    return "unknown";
}

static SDL_Keycode m12_name_to_keycode(const char* name) {
    size_t i;
    if (!name || name[0] == '\0' || strcmp(name, "none") == 0) return 0;
    for (i = 0; i < KEY_NAME_COUNT; ++i) {
        if (strcmp(s_key_names[i].name, name) == 0) return s_key_names[i].key;
    }
    return 0;
}

/* ── Config file path ───────────────────────────────────────────────── */

static int m12_keybindings_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, "keybindings.toml")) {
            return 1;
        }
    }
    snprintf(out, outSize, "keybindings.toml");
    return 1;
}

/* ── Trim helper ────────────────────────────────────────────────────── */

static void m12_remap_trim(char* text) {
    char* start;
    char* end;
    size_t len;
    if (!text) return;
    start = text;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') ++start;
    if (start != text) memmove(text, start, strlen(start) + 1U);
    len = strlen(text);
    while (len > 0U) {
        end = &text[len - 1U];
        if (*end != ' ' && *end != '\t' && *end != '\r' && *end != '\n') break;
        *end = '\0';
        --len;
    }
}

/* ── API implementation ─────────────────────────────────────────────── */

void M12_InputMap_SetDefaults(M12_InputMap* map) {
    size_t i;
    if (!map) return;
    memset(map, 0, sizeof(*map));
    for (i = 0; i < (size_t)M12_ACTION_COUNT; ++i) {
        map->bindings[i].action = (M12_InputAction)i;
    }
    for (i = 0; i < DEFAULT_COUNT; ++i) {
        int idx = (int)s_defaults[i].action;
        if (idx >= 0 && idx < M12_ACTION_COUNT) {
            map->bindings[idx].primary   = s_defaults[i].primary;
            map->bindings[idx].secondary = s_defaults[i].secondary;
        }
    }
}

int M12_InputMap_Load(M12_InputMap* map) {
    char path[FSP_PATH_MAX];
    char line[512];
    FILE* fp;
    if (!map) return 0;

    M12_InputMap_SetDefaults(map);

    if (!m12_keybindings_path(path, sizeof(path))) return 0;

    fp = fopen(path, "r");
    if (!fp) return 0;

    while (fgets(line, (int)sizeof(line), fp)) {
        char* equals;
        char* key;
        char* value;
        char* comma;
        int actionIdx;

        m12_remap_trim(line);
        if (line[0] == '\0' || line[0] == '#' || line[0] == '[') continue;

        equals = strchr(line, '=');
        if (!equals) continue;

        *equals = '\0';
        key = line;
        value = equals + 1;
        m12_remap_trim(key);
        m12_remap_trim(value);

        /* Find the action by name */
        actionIdx = -1;
        {
            int ai;
            for (ai = 0; ai < M12_ACTION_COUNT; ++ai) {
                if (strcmp(key, s_action_names[ai]) == 0) {
                    actionIdx = ai;
                    break;
                }
            }
        }
        if (actionIdx < 0) continue;

        /* Value format: "primary" or "primary, secondary" */
        comma = strchr(value, ',');
        if (comma) {
            char primaryStr[64];
            char secondaryStr[64];
            size_t plen = (size_t)(comma - value);
            if (plen >= sizeof(primaryStr)) plen = sizeof(primaryStr) - 1;
            memcpy(primaryStr, value, plen);
            primaryStr[plen] = '\0';
            snprintf(secondaryStr, sizeof(secondaryStr), "%s", comma + 1);
            m12_remap_trim(primaryStr);
            m12_remap_trim(secondaryStr);
            map->bindings[actionIdx].primary   = m12_name_to_keycode(primaryStr);
            map->bindings[actionIdx].secondary = m12_name_to_keycode(secondaryStr);
        } else {
            map->bindings[actionIdx].primary   = m12_name_to_keycode(value);
            map->bindings[actionIdx].secondary = 0;
        }
    }

    fclose(fp);
    return 1;
}

int M12_InputMap_Save(const M12_InputMap* map) {
    char path[FSP_PATH_MAX];
    char configDir[FSP_PATH_MAX];
    FILE* fp;
    int i;

    if (!map) return 0;
    if (!m12_keybindings_path(path, sizeof(path))) return 0;

    /* Ensure config directory exists */
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        FSP_CreateDirectoryRecursive(configDir);
    }

    fp = fopen(path, "w");
    if (!fp) return 0;

    fprintf(fp, "# Firestaff key bindings\n");
    fprintf(fp, "# Format: action_name = primary_key [, secondary_key]\n");
    fprintf(fp, "# Delete this file to reset to defaults.\n\n");

    for (i = 0; i < M12_ACTION_COUNT; ++i) {
        const M12_KeyBinding* b = &map->bindings[i];
        const char* primaryName   = m12_keycode_to_name(b->primary);
        const char* secondaryName = m12_keycode_to_name(b->secondary);

        if (b->secondary != 0) {
            fprintf(fp, "%s = %s, %s\n", s_action_names[i], primaryName, secondaryName);
        } else {
            fprintf(fp, "%s = %s\n", s_action_names[i], primaryName);
        }
    }

    fclose(fp);
    return 1;
}

SDL_Keycode M12_InputMap_GetKey(const M12_InputMap* map, M12_InputAction action) {
    if (!map || action < 0 || action >= M12_ACTION_COUNT) return 0;
    return map->bindings[action].primary;
}

SDL_Keycode M12_InputMap_GetSecondaryKey(const M12_InputMap* map, M12_InputAction action) {
    if (!map || action < 0 || action >= M12_ACTION_COUNT) return 0;
    return map->bindings[action].secondary;
}

M12_InputAction M12_InputMap_ActionForKey(const M12_InputMap* map, SDL_Keycode key) {
    int i;
    if (!map || key == 0) return M12_ACTION_COUNT;
    for (i = 0; i < M12_ACTION_COUNT; ++i) {
        if (map->bindings[i].primary == key || map->bindings[i].secondary == key) {
            return (M12_InputAction)i;
        }
    }
    return M12_ACTION_COUNT;
}

const char* M12_InputAction_Name(M12_InputAction action) {
    if (action < 0 || action >= M12_ACTION_COUNT) return "UNKNOWN";
    return s_action_display_names[action];
}

const char* M12_Keycode_Name(SDL_Keycode key) {
    return m12_keycode_to_name(key);
}

/* ── Remap helpers ──────────────────────────────────────────────────── */

void M12_Remap_Begin(M12_RemapState* state, M12_InputAction action, int slot) {
    if (!state) return;
    state->active       = 1;
    state->targetAction = action;
    state->targetSlot   = slot;
}

int M12_Remap_HandleKey(M12_RemapState* state, M12_InputMap* map, SDL_Keycode key) {
    if (!state || !state->active || !map) return 0;

    /* Escape cancels the remap */
    if (key == SDLK_ESCAPE) {
        M12_Remap_Cancel(state);
        return 0;
    }

    /* Unbind this key from any other action to avoid duplicates */
    {
        int i;
        for (i = 0; i < M12_ACTION_COUNT; ++i) {
            if (map->bindings[i].primary == key)   map->bindings[i].primary   = 0;
            if (map->bindings[i].secondary == key)  map->bindings[i].secondary = 0;
        }
    }

    /* Assign */
    if (state->targetSlot == 0) {
        map->bindings[state->targetAction].primary = key;
    } else {
        map->bindings[state->targetAction].secondary = key;
    }

    state->active = 0;
    return 1;
}

void M12_Remap_Cancel(M12_RemapState* state) {
    if (!state) return;
    state->active = 0;
}
