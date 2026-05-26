/*
 * input_remap_m11.c — Engine-side keyboard rebinding.
 *
 * Stores per-action primary + secondary SDL_Scancodes in a static
 * table.  Persists to ~/.firestaff/keybinds.ini as a simple
 * key = primary[,secondary] flat file (one action per line).
 */

#include "input_remap_m11.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0755)
#endif

/* ── Action metadata ────────────────────────────────────────────────── */

typedef struct {
    int           action;
    const char*   key;        /* ini key (lowercase_with_underscores) */
    const char*   label;      /* "MOVE FORWARD" */
    SDL_Scancode  defPrimary;
    SDL_Scancode  defSecondary;
} ActionDef;

static const ActionDef s_defs[] = {
    { M11_ACTION_MOVE_FORWARD,    "move_forward",    "MOVE FORWARD",    SDL_SCANCODE_UP,     SDL_SCANCODE_W       },
    { M11_ACTION_MOVE_BACKWARD,   "move_backward",   "MOVE BACK",       SDL_SCANCODE_DOWN,   SDL_SCANCODE_S       },
    { M11_ACTION_TURN_LEFT,       "turn_left",       "TURN LEFT",       SDL_SCANCODE_LEFT,   SDL_SCANCODE_A       },
    { M11_ACTION_TURN_RIGHT,      "turn_right",      "TURN RIGHT",      SDL_SCANCODE_RIGHT,  SDL_SCANCODE_D       },
    { M11_ACTION_STRAFE_LEFT,     "strafe_left",     "STRAFE LEFT",     SDL_SCANCODE_Q,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_STRAFE_RIGHT,    "strafe_right",    "STRAFE RIGHT",    SDL_SCANCODE_E,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_ATTACK,          "attack",          "ATTACK",          SDL_SCANCODE_SPACE,  SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_SPELL,           "spell",           "CAST SPELL",      SDL_SCANCODE_C,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_INVENTORY,       "inventory",       "INVENTORY",       SDL_SCANCODE_I,      SDL_SCANCODE_V       },
    { M11_ACTION_MAP,             "map",             "MAP",             SDL_SCANCODE_M,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_PICKUP,          "pickup",          "PICK UP",         SDL_SCANCODE_G,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_DROP,            "drop",            "DROP",            SDL_SCANCODE_P,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_USE_STAIRS,      "use_stairs",      "USE STAIRS",      SDL_SCANCODE_X,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_REST,            "rest",            "REST",            SDL_SCANCODE_R,      SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_CYCLE_CHAMPION,  "cycle_champion",  "CYCLE CHAMPION",  SDL_SCANCODE_TAB,    SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_QUICK_SAVE,      "quick_save",      "QUICK SAVE",      SDL_SCANCODE_F5,     SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_QUICK_LOAD,      "quick_load",      "QUICK LOAD",      SDL_SCANCODE_F9,     SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_PAUSE,           "pause",           "PAUSE",           SDL_SCANCODE_PAUSE,  SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_SCREENSHOT,      "screenshot",      "SCREENSHOT",      SDL_SCANCODE_F12,    SDL_SCANCODE_UNKNOWN },
    { M11_ACTION_SCREENSHOT_MODE, "screenshot_mode", "SCREENSHOT MODE", SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN },
};

#define S_DEF_COUNT ((int)(sizeof(s_defs) / sizeof(s_defs[0])))

static M11_InputBinding s_bindings[M11_ACTION_COUNT];
static int              s_initialized = 0;

/* ── Internal helpers ───────────────────────────────────────────────── */

static const ActionDef* find_def_by_action(int action) {
    int i;
    for (i = 0; i < S_DEF_COUNT; i++) {
        if (s_defs[i].action == action) return &s_defs[i];
    }
    return NULL;
}

static const ActionDef* find_def_by_key(const char* key) {
    int i;
    if (!key) return NULL;
    for (i = 0; i < S_DEF_COUNT; i++) {
        if (strcmp(s_defs[i].key, key) == 0) return &s_defs[i];
    }
    return NULL;
}

static void ensure_initialized(void) {
    if (!s_initialized) {
        M11_Input_SetDefaults();
    }
}

/* Build "<home>/.firestaff/keybinds.ini".  Buffer is static, not
 * thread-safe — fine for our single-threaded launcher path. */
static const char* config_path_internal(void) {
    static char path[1024];
    const char* home;
    char dir[1024];

    home = getenv("HOME");
    if (!home || !*home) home = ".";

    snprintf(dir, sizeof(dir), "%s/.firestaff", home);
    /* best-effort mkdir; ignore failure */
    (void)MKDIR(dir);

    snprintf(path, sizeof(path), "%s/.firestaff/keybinds.ini", home);
    return path;
}

/* ── Public API ─────────────────────────────────────────────────────── */

void M11_Input_SetDefaults(void) {
    int i;
    for (i = 0; i < M11_ACTION_COUNT; i++) {
        s_bindings[i].action    = i;
        s_bindings[i].primary   = SDL_SCANCODE_UNKNOWN;
        s_bindings[i].secondary = SDL_SCANCODE_UNKNOWN;
    }
    for (i = 0; i < S_DEF_COUNT; i++) {
        int a = s_defs[i].action;
        if (a >= 0 && a < M11_ACTION_COUNT) {
            s_bindings[a].action    = a;
            s_bindings[a].primary   = s_defs[i].defPrimary;
            s_bindings[a].secondary = s_defs[i].defSecondary;
        }
    }
    s_initialized = 1;
}

SDL_Scancode M11_Input_GetScancode(int action) {
    ensure_initialized();
    if (action < 0 || action >= M11_ACTION_COUNT) return SDL_SCANCODE_UNKNOWN;
    return s_bindings[action].primary;
}

SDL_Scancode M11_Input_GetSecondaryScancode(int action) {
    ensure_initialized();
    if (action < 0 || action >= M11_ACTION_COUNT) return SDL_SCANCODE_UNKNOWN;
    return s_bindings[action].secondary;
}

void M11_Input_SetBinding(int action, int slot, SDL_Scancode sc) {
    ensure_initialized();
    if (action < 0 || action >= M11_ACTION_COUNT) return;
    if (slot == 0) {
        s_bindings[action].primary = sc;
    } else if (slot == 1) {
        s_bindings[action].secondary = sc;
    }
}

int M11_Input_ActionForScancode(SDL_Scancode sc) {
    int i;
    ensure_initialized();
    if (sc == SDL_SCANCODE_UNKNOWN) return M11_ACTION_COUNT;
    for (i = 0; i < M11_ACTION_COUNT; i++) {
        if (s_bindings[i].primary == sc || s_bindings[i].secondary == sc) {
            return i;
        }
    }
    return M11_ACTION_COUNT;
}

const char* M11_Input_ActionName(int action) {
    const ActionDef* def = find_def_by_action(action);
    return def ? def->label : "UNKNOWN";
}

const char* M11_Input_ScancodeName(SDL_Scancode sc) {
    const char* name;
    if (sc == SDL_SCANCODE_UNKNOWN) return "[NONE]";
    name = SDL_GetScancodeName(sc);
    if (!name || !*name) return "[NONE]";
    return name;
}

const char* M11_Input_GetConfigPath(void) {
    return config_path_internal();
}

int M11_Input_Load(void) {
    const char* path;
    FILE* f;
    char line[256];

    ensure_initialized();
    path = config_path_internal();
    f = fopen(path, "r");
    if (!f) return 0;

    while (fgets(line, sizeof(line), f)) {
        char key[64];
        int  primary = 0, secondary = 0;
        char* p;
        const ActionDef* def;

        /* strip leading whitespace */
        p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '#' || *p == ';' || *p == '\0') continue;

        /* parse "key = primary[,secondary]" */
        if (sscanf(p, " %63[a-z_] = %d , %d", key, &primary, &secondary) == 3 ||
            sscanf(p, " %63[a-z_] = %d", key, &primary) >= 2) {
            def = find_def_by_key(key);
            if (def && def->action >= 0 && def->action < M11_ACTION_COUNT) {
                s_bindings[def->action].primary   = (SDL_Scancode)primary;
                s_bindings[def->action].secondary = (SDL_Scancode)secondary;
            }
        }
    }

    fclose(f);
    return 1;
}

int M11_Input_Save(void) {
    const char* path;
    FILE* f;
    int i;

    ensure_initialized();
    path = config_path_internal();
    f = fopen(path, "w");
    if (!f) return 0;

    fprintf(f, "# Firestaff engine key bindings (M11)\n");
    fprintf(f, "# Format: action = primary_scancode[,secondary_scancode]\n");
    fprintf(f, "# Scancode values are SDL_Scancode integers (see SDL_scancode.h).\n\n");

    for (i = 0; i < S_DEF_COUNT; i++) {
        const ActionDef* def = &s_defs[i];
        int a = def->action;
        int pri, sec;
        if (a < 0 || a >= M11_ACTION_COUNT) continue;
        pri = (int)s_bindings[a].primary;
        sec = (int)s_bindings[a].secondary;
        if (sec != (int)SDL_SCANCODE_UNKNOWN) {
            fprintf(f, "%s = %d,%d\n", def->key, pri, sec);
        } else {
            fprintf(f, "%s = %d\n", def->key, pri);
        }
    }

    fclose(f);
    return 1;
}
