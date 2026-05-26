#ifndef FIRESTAFF_M11_QOL_RUNTIME_H
#define FIRESTAFF_M11_QOL_RUNTIME_H

/*
 * m11_qol_runtime — Gameplay & QoL runtime singleton.
 *
 * Holds the live values for the Speed Control, Minimap, Auto-Map and
 * Combat Log features.  Initialised from M12_Config at startup; the
 * hotkey paths (F6 / F7 / F8 / L) mutate the runtime values without
 * touching persisted config (the config menu still owns persistence).
 */

#include <stddef.h>
#include "config_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialise runtime state from a loaded M12_Config (safe with NULL). */
void M11_QolRuntime_InitFromConfig(const M12_Config* config);

/* Speed control */
int  M11_QolRuntime_GetSpeedMultiplier(void);          /* 50/100/150/200 */
void M11_QolRuntime_SetSpeedMultiplier(int multiplier); /* clamped */
int  M11_QolRuntime_CycleSpeedMultiplier(void);

/* Minimap */
int  M11_QolRuntime_GetMinimapEnabled(void);
void M11_QolRuntime_SetMinimapEnabled(int enabled);
int  M11_QolRuntime_ToggleMinimap(void);
int  M11_QolRuntime_GetMinimapSize(void);
int  M11_QolRuntime_GetMinimapCorner(void);

/* Auto-Map */
int  M11_QolRuntime_GetAutoMapEnabled(void);

/* Combat Log */
int  M11_QolRuntime_GetCombatLogEnabled(void);
void M11_QolRuntime_SetCombatLogEnabled(int enabled);
int  M11_QolRuntime_ToggleCombatLog(void);
int  M11_QolRuntime_GetCombatLogMaxLines(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_QOL_RUNTIME_H */
