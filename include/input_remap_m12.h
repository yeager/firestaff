#ifndef FIRESTAFF_INPUT_REMAP_M12_H
#define FIRESTAFF_INPUT_REMAP_M12_H

/*
 * input_remap_m12 — Configurable key bindings for the Firestaff launcher.
 *
 * Maps abstract game actions (M12_InputAction) to SDL keycodes.
 * Bindings persist to a config file alongside startup-menu.toml.
 * The remap UI lets the player press a key to reassign an action.
 */

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Action enum ────────────────────────────────────────────────────── */

typedef enum {
    M12_ACTION_MOVE_FORWARD = 0,
    M12_ACTION_MOVE_BACKWARD,
    M12_ACTION_TURN_LEFT,
    M12_ACTION_TURN_RIGHT,
    M12_ACTION_STRAFE_LEFT,
    M12_ACTION_STRAFE_RIGHT,
    M12_ACTION_ACCEPT,
    M12_ACTION_BACK,
    M12_ACTION_ACTION,
    M12_ACTION_CYCLE_CHAMPION,
    M12_ACTION_REST_TOGGLE,
    M12_ACTION_USE_STAIRS,
    M12_ACTION_PICKUP_ITEM,
    M12_ACTION_DROP_ITEM,
    M12_ACTION_SPELL_RUNE_1,
    M12_ACTION_SPELL_RUNE_2,
    M12_ACTION_SPELL_RUNE_3,
    M12_ACTION_SPELL_RUNE_4,
    M12_ACTION_SPELL_RUNE_5,
    M12_ACTION_SPELL_RUNE_6,
    M12_ACTION_SPELL_CAST,
    M12_ACTION_SPELL_CLEAR,
    M12_ACTION_USE_ITEM,
    M12_ACTION_MAP_TOGGLE,
    M12_ACTION_INVENTORY_TOGGLE,
    M12_ACTION_QUICK_SAVE,
    M12_ACTION_QUICK_LOAD,
    M12_ACTION_COUNT  /* sentinel — must be last */
} M12_InputAction;

/* ── Key binding ────────────────────────────────────────────────────── */

typedef struct {
    M12_InputAction action;
    SDL_Keycode     primary;     /* main binding */
    SDL_Keycode     secondary;   /* alternate binding (0 = none) */
} M12_KeyBinding;

/* ── Binding table ──────────────────────────────────────────────────── */

typedef struct {
    M12_KeyBinding bindings[M12_ACTION_COUNT];
} M12_InputMap;

/* ── Remap state (for the "press a key" UI) ─────────────────────────── */

typedef struct {
    int              active;          /* 1 = waiting for keypress */
    M12_InputAction  targetAction;    /* action being remapped */
    int              targetSlot;      /* 0 = primary, 1 = secondary */
} M12_RemapState;

/* ── API ────────────────────────────────────────────────────────────── */

/* Set all bindings to defaults (arrows + WASD + classic DM keys). */
void M12_InputMap_SetDefaults(M12_InputMap* map);

/* Load bindings from the config directory. Returns 1 on success, 0 on
 * missing/invalid file (defaults are kept). */
int M12_InputMap_Load(M12_InputMap* map);

/* Save bindings to the config directory. Returns 1 on success. */
int M12_InputMap_Save(const M12_InputMap* map);

/* Look up the keycode bound to `action` (primary). Returns 0 if none. */
SDL_Keycode M12_InputMap_GetKey(const M12_InputMap* map, M12_InputAction action);

/* Look up the secondary keycode bound to `action`. Returns 0 if none. */
SDL_Keycode M12_InputMap_GetSecondaryKey(const M12_InputMap* map, M12_InputAction action);

/* Find which action (if any) is bound to `key`. Returns M12_ACTION_COUNT if
 * no match. */
M12_InputAction M12_InputMap_ActionForKey(const M12_InputMap* map, SDL_Keycode key);

/* Human-readable name for an action (English). */
const char* M12_InputAction_Name(M12_InputAction action);

/* Human-readable name for an SDL keycode. Returns a static buffer. */
const char* M12_Keycode_Name(SDL_Keycode key);

/* ── Remap helpers ──────────────────────────────────────────────────── */

/* Begin remapping: sets state to wait for a keypress. */
void M12_Remap_Begin(M12_RemapState* state, M12_InputAction action, int slot);

/* Feed a keypress while remapping. Returns 1 if remap completed, 0 if
 * cancelled (Escape pressed). The binding is assigned in `map`. */
int M12_Remap_HandleKey(M12_RemapState* state, M12_InputMap* map, SDL_Keycode key);

/* Cancel an active remap without changing the binding. */
void M12_Remap_Cancel(M12_RemapState* state);

#ifdef __cplusplus
}
#endif


/* Apply a control scheme preset: 0 = original (A/D turn), 1 = hybrid (A/D strafe). */
void M12_InputMap_ApplyScheme(M12_InputMap* map, int schemeIndex);
#endif /* FIRESTAFF_INPUT_REMAP_M12_H */
