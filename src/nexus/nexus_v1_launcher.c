/*
 * nexus_v1_launcher.c
 * ===================
 * Nexus V1 launcher — singleton engine lifecycle manager.
 *
 * Owns the Nexus_V1_Engine singleton for the duration of the session.
 * All other Nexus V1 modules are accessed through nexus_v1_engine.c
 * which uses the launcher's engine field.
 *
 * Source: DM Nexus (Saturn) boot flow, NEXUS.C / NEXUS2.C engine
 * lifecycle, ReDMCSB boot/disk loading references.
 */

#include "nexus_v1_launcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ── Singleton ──────────────────────────────────────────────────────── */
static Nexus_V1_Engine s_engine;
static int s_initialized = 0;

/* ── Public API ─────────────────────────────────────────────────────── */

int nexus_v1_launcher_init(const char *data_dir) {
    if (!data_dir) {
        printf("Nexus launcher: NULL data_dir\n");
        return -1;
    }

    /* Already initialized — return success without re-init */
    if (s_initialized) {
        printf("Nexus launcher: already initialized\n");
        return 0;
    }

    /* Init the engine singleton.
     * nexus_v1_init() auto-detects ISO vs extracted files and
     * populates the full engine: ISO reader, game state, mechanics,
     * champions, creatures, sound, and font.
     * Source: nexus_v1_engine.c nexus_v1_init() */
    int rc = nexus_v1_init(&s_engine, data_dir);
    if (rc != 0) {
        printf("Nexus launcher: nexus_v1_init failed for '%s'\n", data_dir);
        return -1;
    }

    s_initialized = 1;
    printf("Nexus launcher: initialized (data_dir='%s')\n", data_dir);
    return 0;
}

int nexus_v1_launcher_load_level(int level) {
    if (!s_initialized) {
        printf("Nexus launcher: not initialized — call nexus_v1_launcher_init first\n");
        return -1;
    }
    if (level < 0 || level > 15) {
        printf("Nexus launcher: invalid level %d (must be 0-15)\n", level);
        return -1;
    }
    int rc = nexus_v1_load_level(&s_engine, level);
    if (rc != 0) {
        printf("Nexus launcher: failed to load level %d\n", level);
        return -1;
    }
    printf("Nexus launcher: loaded level %d\n", level);
    return 0;
}

Nexus_V1_Engine *nexus_v1_launcher_get_engine(void) {
    if (!s_initialized) {
        return NULL;
    }
    return &s_engine;
}

void nexus_v1_launcher_shutdown(void) {
    if (!s_initialized) {
        return;
    }
    nexus_v1_shutdown(&s_engine);
    s_initialized = 0;
    printf("Nexus launcher: shut down\n");
}