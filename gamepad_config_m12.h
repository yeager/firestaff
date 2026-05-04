#ifndef FIRESTAFF_GAMEPAD_CONFIG_M12_H
#define FIRESTAFF_GAMEPAD_CONFIG_M12_H

/*
 * gamepad_config_m12 — Controller binding UI for the Firestaff launcher.
 *
 * Maps abstract game actions (M12_InputAction from input_remap_m12.h) to
 * SDL gamepad buttons and axes.  Supports button mapping, axis controls
 * (dead-zone, sensitivity, inversion), connection status monitoring,
 * and named profile save/load.
 *
 * Bindings persist to gamepad.toml in the Firestaff config directory.
 * Profiles are stored as gamepad-<name>.toml.
 */

#include "input_remap_m12.h"  /* M12_InputAction, M12_ACTION_COUNT */

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Limits ─────────────────────────────────────────────────────────── */

enum {
    M12_GAMEPAD_MAX_PROFILES     = 8,
    M12_GAMEPAD_PROFILE_NAME_MAX = 32,
    M12_GAMEPAD_AXIS_DEADZONE_MIN = 500,
    M12_GAMEPAD_AXIS_DEADZONE_MAX = 16000,
    M12_GAMEPAD_AXIS_DEADZONE_DEFAULT = 4000,
    M12_GAMEPAD_SENSITIVITY_MIN  = 1,
    M12_GAMEPAD_SENSITIVITY_MAX  = 10,
    M12_GAMEPAD_SENSITIVITY_DEFAULT = 5
};

/* ── Gamepad button binding ─────────────────────────────────────────── */

typedef struct {
    M12_InputAction    action;
    SDL_GamepadButton  primary;     /* main button binding (-1 = none) */
    SDL_GamepadButton  secondary;   /* alternate button binding (-1 = none) */
} M12_GamepadButtonBinding;

/* ── Axis role enum ─────────────────────────────────────────────────── */

typedef enum {
    M12_AXIS_ROLE_NONE = 0,
    M12_AXIS_ROLE_MOVE,       /* left stick → movement (fwd/back/strafe) */
    M12_AXIS_ROLE_TURN,       /* right stick X → turning */
    M12_AXIS_ROLE_LOOK,       /* right stick Y → look up/down (future) */
    M12_AXIS_ROLE_COUNT
} M12_AxisRole;

/* ── Per-axis configuration ─────────────────────────────────────────── */

typedef struct {
    M12_AxisRole  role;
    int           deadzone;     /* absolute threshold (0–32767 range) */
    int           sensitivity;  /* 1–10 scale */
    int           inverted;     /* 0 = normal, 1 = inverted */
} M12_GamepadAxisConfig;

/* ── Full gamepad map ───────────────────────────────────────────────── */

typedef struct {
    M12_GamepadButtonBinding buttons[M12_ACTION_COUNT];
    M12_GamepadAxisConfig    axes[SDL_GAMEPAD_AXIS_COUNT];
    int                      enabled;  /* 0 = gamepad input disabled */
} M12_GamepadMap;

/* ── Connection status ──────────────────────────────────────────────── */

typedef struct {
    int              connected;      /* 1 = gamepad is present */
    SDL_JoystickID   instanceId;     /* SDL instance id (0 = none) */
    char             name[128];      /* human-readable controller name */
    SDL_Gamepad*     handle;         /* open SDL handle (NULL = none) */
} M12_GamepadStatus;

/* ── Profile ────────────────────────────────────────────────────────── */

typedef struct {
    char          name[M12_GAMEPAD_PROFILE_NAME_MAX];
    M12_GamepadMap map;
} M12_GamepadProfile;

/* ── Remap state (for the "press a button" UI) ──────────────────────── */

typedef struct {
    int              active;          /* 1 = waiting for button press */
    M12_InputAction  targetAction;    /* action being remapped */
    int              targetSlot;      /* 0 = primary, 1 = secondary */
} M12_GamepadRemapState;

/* ── API: defaults & lifecycle ──────────────────────────────────────── */

/* Set all button bindings and axis configs to sensible defaults. */
void M12_GamepadMap_SetDefaults(M12_GamepadMap* map);

/* ── API: persistence ───────────────────────────────────────────────── */

/* Load gamepad config from the config directory. Returns 1 on success,
 * 0 on missing/invalid file (defaults are kept). */
int M12_GamepadMap_Load(M12_GamepadMap* map);

/* Save gamepad config to the config directory. Returns 1 on success. */
int M12_GamepadMap_Save(const M12_GamepadMap* map);

/* ── API: button queries ────────────────────────────────────────────── */

/* Look up the primary button bound to `action`. Returns -1 if none. */
SDL_GamepadButton M12_GamepadMap_GetButton(const M12_GamepadMap* map,
                                           M12_InputAction action);

/* Look up the secondary button bound to `action`. Returns -1 if none. */
SDL_GamepadButton M12_GamepadMap_GetSecondaryButton(const M12_GamepadMap* map,
                                                    M12_InputAction action);

/* Find which action is bound to `button`. Returns M12_ACTION_COUNT if
 * no match. */
M12_InputAction M12_GamepadMap_ActionForButton(const M12_GamepadMap* map,
                                               SDL_GamepadButton button);

/* ── API: axis queries ──────────────────────────────────────────────── */

/* Get the axis config for a given SDL axis. */
const M12_GamepadAxisConfig* M12_GamepadMap_GetAxisConfig(
    const M12_GamepadMap* map, SDL_GamepadAxis axis);

/* Apply dead-zone and sensitivity to a raw axis value.
 * Returns the processed value in -32768..32767 range. */
int M12_GamepadAxis_Process(const M12_GamepadAxisConfig* cfg, int rawValue);

/* ── API: connection monitoring ─────────────────────────────────────── */

/* Poll for the first connected gamepad. Updates `status`. Call this
 * on SDL_EVENT_GAMEPAD_ADDED / REMOVED or periodically. */
void M12_GamepadStatus_Update(M12_GamepadStatus* status);

/* Close the open gamepad handle (e.g. on shutdown). */
void M12_GamepadStatus_Close(M12_GamepadStatus* status);

/* ── API: profile management ────────────────────────────────────────── */

/* Save the current map as a named profile. Returns 1 on success. */
int M12_GamepadProfile_Save(const char* profileName,
                            const M12_GamepadMap* map);

/* Load a named profile into `map`. Returns 1 on success. */
int M12_GamepadProfile_Load(const char* profileName,
                            M12_GamepadMap* map);

/* Delete a named profile file. Returns 1 on success. */
int M12_GamepadProfile_Delete(const char* profileName);

/* List available profile names. Writes up to `maxProfiles` names into
 * `namesOut`. Returns the number of profiles found. */
int M12_GamepadProfile_List(char namesOut[][M12_GAMEPAD_PROFILE_NAME_MAX],
                            int maxProfiles);

/* ── API: human-readable names ──────────────────────────────────────── */

/* Human-readable name for an SDL gamepad button. */
const char* M12_GamepadButton_Name(SDL_GamepadButton button);

/* Human-readable name for an SDL gamepad axis. */
const char* M12_GamepadAxis_Name(SDL_GamepadAxis axis);

/* Human-readable name for an axis role. */
const char* M12_AxisRole_Name(M12_AxisRole role);

/* ── API: remap helpers ─────────────────────────────────────────────── */

/* Begin remapping: sets state to wait for a button press. */
void M12_GamepadRemap_Begin(M12_GamepadRemapState* state,
                            M12_InputAction action, int slot);

/* Feed a button press while remapping. Returns 1 if remap completed,
 * 0 if cancelled (Back/Start pressed). The binding is assigned in `map`. */
int M12_GamepadRemap_HandleButton(M12_GamepadRemapState* state,
                                  M12_GamepadMap* map,
                                  SDL_GamepadButton button);

/* Cancel an active remap without changing the binding. */
void M12_GamepadRemap_Cancel(M12_GamepadRemapState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_GAMEPAD_CONFIG_M12_H */
