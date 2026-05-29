#ifndef NEXUS_V1_LAUNCHER_H
#define NEXUS_V1_LAUNCHER_H

/*
 * nexus_v1_launcher.h
 * ===================
 * Nexus V1 launcher — singleton engine lifecycle manager.
 *
 * Owns the Nexus_V1_Engine singleton. Provides:
 *   - launcher_init()       — discover game data, init engine
 *   - launcher_load_level()  — load a dungeon level (0-15)
 *   - launcher_get_engine()  — access singleton (for M11 render loop)
 *
 * Design rationale:
 *   The Nexus V1 engine is a self-contained object with its own game
 *   state, mechanics, and resource management. The launcher acts as a
 *   thin facade that owns the engine pointer and routes M12/M11 calls
 *   into it. This separates launcher concerns (data discovery, profile
 *   validation) from engine concerns (tick, render, save/load).
 *
 * Source: DM Nexus (Saturn) boot flow, NEXUS.C / NEXUS2.C engine
 * lifecycle, ReDMCSB boot/disk loading references.
 */

#include "nexus_v1_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Public API ─────────────────────────────────────────────────────── */

/* Initialize the Nexus V1 engine singleton.
 * - Scans data_dir for CUE/BIN (Saturn CD) or extracted files.
 * - Calls nexus_v1_init() on the singleton.
 * - Returns 0 on success, -1 on failure.
 * - Safe to call multiple times; only first call has effect
 *   (subsequent calls return 0 if already initialized). */
int nexus_v1_launcher_init(const char *data_dir);

/* Load a dungeon level (0-15) into the engine.
 * Calls nexus_v1_load_level() on the singleton.
 * Returns 0 on success, -1 if launcher not init'd or level load failed. */
int nexus_v1_launcher_load_level(int level);

/* Get the current Nexus V1 engine singleton.
 * Returns NULL if launcher not initialized.
 * The returned pointer is owned by the launcher — do not free it. */
Nexus_V1_Engine *nexus_v1_launcher_get_engine(void);

/* Shutdown the launcher and free the engine singleton.
 * Safe to call multiple times. */
void nexus_v1_launcher_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_LAUNCHER_H */