/*
 * probes/nexus_v1_launch_smoke_probe.c
 * =====================================
 * Nexus V1 Launch Smoke Test
 *
 * Verifies: nexus_v1_init → nexus_v1_load_level(0) → nexus_v1_tick
 *
 * Run (with real Saturn data):
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_launch_smoke_probe \
 *       "$HOME/.firestaff/data/nexus/"
 *
 * Without data the probe exits 0 (skip).  With real Saturn DGN files
 * it confirms the game engine is alive after boot.
 *
 * Source-lock:
 *   src/nexus/nexus_v1_engine.c  (nexus_v1_init, nexus_v1_load_level, nexus_v1_tick)
 *   src/nexus/nexus_v1_dungeon.c (DGN level loader)
 *   src/nexus/nexus_v1_mechanics.c (nexus_mechanics_tick — ReDMCSB CLIKMENU.C F0366)
 */

#include "nexus_v1_engine.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_game.h"
#include "nexus_v1_mechanics_fwd.h" /* Nexus_MechanicsState forward declaration */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── Probe assertions ─────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

#define PROBE_ASSERT(cond, fmt, ...) do {                   \
    if (cond) {                                             \
        printf("  PASS: " fmt "\n", ##__VA_ARGS__);        \
        g_pass++;                                           \
    } else {                                                \
        printf("  FAIL: " fmt "\n", ##__VA_ARGS__);        \
        g_fail++;                                           \
    }                                                        \
} while (0)

/* ── Main ────────────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    const char *data_dir;

    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Nexus V1 Launch Smoke — source-lock: nexus_v1_engine.c\n");
    printf("═══════════════════════════════════════════════════════════\n");

    /* Data directory from argv[1] (CMake test passes it) */
    if (argc > 1 && argv[1] && argv[1][0]) {
        data_dir = argv[1];
    } else {
        data_dir = getenv("HOME");
        if (data_dir) {
            static char buf[512];
            snprintf(buf, sizeof(buf), "%s/.firestaff/data/nexus", data_dir);
            data_dir = buf;
        }
    }

    printf("\n[Step 1: Engine Init]\n");
    printf("  data_dir: %s\n", data_dir ? data_dir : "(null)");

    /* ── Step 1: Init ─────────────────────────────────────────────── */
    Nexus_V1_Engine engine;
    int rc = nexus_v1_init(&engine, data_dir);
    if (rc < 0) {
        printf("\n  No Nexus data found — probe skipped.\n");
        printf("  (Place Saturn DGN/ISO files under ~/.firestaff/data/nexus/)\n");
        printf("\n═══════════════════════════════════════════════════════════\n");
        printf("  Result: SKIP (%s)\n", data_dir ? data_dir : "no data dir");
        printf("═══════════════════════════════════════════════════════════\n");
        return 0;  /* skip, not fail */
    }

    PROBE_ASSERT(engine.initialized == 1, "engine.initialized == 1 after init");
    PROBE_ASSERT(engine.source != NEXUS_SRC_NONE, "source != NEXUS_SRC_NONE");
    PROBE_ASSERT(engine.mechanics != NULL, "engine.mechanics != NULL (calloc'd)");

    /* ── Step 2: Load Level 0 ─────────────────────────────────────── */
    printf("\n[Step 2: Load Level 0]\n");
    printf("  Loading entrance dungeon (LEV00.DGN)...\n");

    rc = nexus_v1_load_level(&engine, 0);
    PROBE_ASSERT(rc == 0, "nexus_v1_load_level(0) returned 0");

    /* After loading, the current_level and party position should be set */
    PROBE_ASSERT(engine.current_level.width > 0,
                 "level width > 0 (got %d)", engine.current_level.width);
    PROBE_ASSERT(engine.current_level.height > 0,
                 "level height > 0 (got %d)", engine.current_level.height);
    PROBE_ASSERT(engine.game.current_level == 0,
                 "game.current_level == 0 (got %d)", engine.game.current_level);

    /* Party should be at a valid position */
    PROBE_ASSERT(engine.game.party_x >= 0, "party_x >= 0 (got %d)", engine.game.party_x);
    PROBE_ASSERT(engine.game.party_y >= 0, "party_y >= 0 (got %d)", engine.game.party_y);
    PROBE_ASSERT(engine.game.party_dir >= 0 && engine.game.party_dir < 4,
                 "party_dir in [0,3] (got %d)", engine.game.party_dir);

    /* Record tick count before ticking */
    uint32_t tick_before = (uint32_t)engine.game.tick_count;
    printf("\n[Step 3: Advance Game Ticks]\n");
    printf("  tick_count before: %u\n", tick_before);

    /* Advance 5 ticks — enough to exercise creature AI, sensor processing,
     * and the script VM without triggering level change (no stairs stepped on). */
    for (int i = 0; i < 5; i++) {
        nexus_v1_tick(&engine);
    }

    uint32_t tick_after = (uint32_t)engine.game.tick_count;
    printf("  tick_count after 5 ticks: %u\n", tick_after);

    PROBE_ASSERT(tick_after > tick_before,
                 "tick_count advanced after 5 ticks (%u → %u)",
                 tick_before, tick_after);
    PROBE_ASSERT(tick_after == tick_before + 5,
                 "tick_count increased by exactly 5 (%u → %u)",
                 tick_before, tick_after);

    /* Engine should still be alive (no crash during ticks) */
    PROBE_ASSERT(engine.initialized == 1,
                 "engine still initialized after 5 ticks");

    /* ── Step 4: Shutdown ─────────────────────────────────────────── */
    printf("\n[Step 4: Shutdown]\n");
    nexus_v1_shutdown(&engine);
    PROBE_ASSERT(1, "nexus_v1_shutdown completed without crash");

    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Result: %d PASS, %d FAIL\n", g_pass, g_fail);
    printf("═══════════════════════════════════════════════════════════\n");
    return (g_fail == 0) ? 0 : 1;
}
