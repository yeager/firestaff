#ifndef FIRESTAFF_INPUT_REMAP_M11_H
#define FIRESTAFF_INPUT_REMAP_M11_H

/*
 * input_remap_m11 — Engine-side keyboard rebinding.
 *
 * Provides an SDL_Scancode lookup for ~20 abstract actions used by
 * the m11 runtime input dispatcher.  Bindings persist to
 * ~/.firestaff/keybinds.ini in the form:
 *   action_name = primary_scancode_int [, secondary_scancode_int]
 *
 * Distinct from input_remap_m12 (launcher-side, SDL_Keycode + TOML).
 * The engine cares about scancodes for layout-independent binding.
 */

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_ACTION_MOVE_FORWARD = 0,
    M11_ACTION_MOVE_BACKWARD,
    M11_ACTION_TURN_LEFT,
    M11_ACTION_TURN_RIGHT,
    M11_ACTION_STRAFE_LEFT,
    M11_ACTION_STRAFE_RIGHT,
    M11_ACTION_ATTACK,
    M11_ACTION_SPELL,
    M11_ACTION_INVENTORY,
    M11_ACTION_MAP,
    M11_ACTION_PICKUP,
    M11_ACTION_DROP,
    M11_ACTION_USE_STAIRS,
    M11_ACTION_REST,
    M11_ACTION_CYCLE_CHAMPION,
    M11_ACTION_QUICK_SAVE,
    M11_ACTION_QUICK_LOAD,
    M11_ACTION_PAUSE,
    M11_ACTION_SCREENSHOT,
    M11_ACTION_SCREENSHOT_MODE,
    M11_ACTION_COUNT  /* sentinel */
} M11_InputAction;

typedef struct {
    int          action;     /* M11_InputAction */
    SDL_Scancode primary;
    SDL_Scancode secondary;
} M11_InputBinding;

/* Initialize bindings to compiled-in defaults. */
void M11_Input_SetDefaults(void);

/* Look up the primary scancode bound to action.  Returns SDL_SCANCODE_UNKNOWN
 * if action is out of range. */
SDL_Scancode M11_Input_GetScancode(int action);

/* Look up the secondary scancode (or SDL_SCANCODE_UNKNOWN if unset). */
SDL_Scancode M11_Input_GetSecondaryScancode(int action);

/* Rebind one slot of an action.  slot: 0 = primary, 1 = secondary. */
void M11_Input_SetBinding(int action, int slot, SDL_Scancode sc);

/* Find the action (if any) bound to scancode.  Returns M11_ACTION_COUNT
 * if no match. */
int M11_Input_ActionForScancode(SDL_Scancode sc);

/* English label for an action ("MOVE FORWARD", "ATTACK", ...). */
const char* M11_Input_ActionName(int action);

/* SDL scancode name wrapper (returns "[NONE]" for UNKNOWN). */
const char* M11_Input_ScancodeName(SDL_Scancode sc);

/* Load bindings from ~/.firestaff/keybinds.ini.  Returns 1 on success,
 * 0 if file missing/unreadable (defaults preserved). */
int M11_Input_Load(void);

/* Save current bindings to ~/.firestaff/keybinds.ini.  Returns 1 on success. */
int M11_Input_Save(void);

/* Resolve the ini path used for load/save.  Returns a static buffer. */
const char* M11_Input_GetConfigPath(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_INPUT_REMAP_M11_H */
