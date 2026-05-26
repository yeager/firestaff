#ifndef FIRESTAFF_AMBIENT_LAYER_M11_H
#define FIRESTAFF_AMBIENT_LAYER_M11_H

/*
 * ambient_layer_m11 — optional ambient sound layer driven by room
 * context.
 *
 * The gameplay code calls M11_Ambient_SetContext when the party enters
 * a new room or transitions between situations (combat, quiet,
 * underground). The layer caches a resolved file path for the current
 * context and exposes that path to the SDL audio backend, which mixes
 * it under the regular SFX/music streams when enabled. When no asset
 * file is found the layer stays silent (no crash, no synthesis).
 *
 * Source: no ReDMCSB equivalent — Firestaff audio extra. Default mode
 * is disabled, so V1 launches stay bit-identical to the original.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_AMBIENT_CONTEXT_NONE        = 0,
    M11_AMBIENT_CONTEXT_UNDERGROUND = 1, /* water drips, cavern echoes */
    M11_AMBIENT_CONTEXT_COMBAT      = 2, /* heartbeat, steel clashes */
    M11_AMBIENT_CONTEXT_QUIET       = 3, /* wind, distant whispers */
    M11_AMBIENT_CONTEXT_COUNT
};

/* Stable English label for the given context (0..3). */
const char* M11_Ambient_GetContextLabel(int context);

/* Enable / disable the ambient layer at runtime. enabled is 0 or 1;
 * volume is 0..100 (percent). Defaults: enabled=0, volume=40. */
void M11_Ambient_SetEnabled(int enabled);
int  M11_Ambient_GetEnabled(void);
void M11_Ambient_SetVolume(int volumePercent);
int  M11_Ambient_GetVolume(void);

/* Set the active gameplay context. The implementation re-resolves the
 * ambient file path for the new context. context is clamped to a valid
 * range; NONE = no track. */
void M11_Ambient_SetContext(int context);
int  M11_Ambient_GetContext(void);

/* Read back the currently resolved ambient file path (UTF-8, NUL
 * terminated). Returns a pointer to the static internal buffer, never
 * NULL. Empty string means no track is currently selected (either the
 * layer is off, context is NONE, or no asset was found on disk). */
const char* M11_Ambient_GetActivePath(void);

/* Override the directory the resolver probes. Defaults to
 * ~/.firestaff/ambient. dirPath may be NULL/empty to reset to the
 * default. */
void M11_Ambient_SetDirectory(const char* dirPath);
const char* M11_Ambient_GetDirectory(void);

/* Resolve a context-specific filename: writes "underground.ogg" (etc.)
 * into outName, returning 1 on success. */
int M11_Ambient_ResolveContextName(int context, char* outName, int outSize);

/* Tick the layer: call once per frame from the main loop. The default
 * implementation is a no-op — kept here so callers can wire the hook
 * even before the SDL audio mixer grows ambient support. */
void M11_Ambient_Tick(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AMBIENT_LAYER_M11_H */
