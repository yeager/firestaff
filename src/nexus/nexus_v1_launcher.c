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
#include "nexus_v1_mechanics.h"
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

void nexus_v1_launcher_boot_receipt_clear(
    Nexus_V1_LauncherBootReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_launch_receipt_clear(&receipt->startup_receipt);
}

static void nexus_v1_launcher_fill_boot_receipt(
    const char *data_dir,
    Nexus_V1_Engine *engine,
    int title_loaded,
    Nexus_V1_LauncherBootReceipt *receipt)
{
    if (!receipt || !engine) {
        return;
    }
    receipt->engine = engine;
    receipt->level_loaded = engine->level_loaded;
    receipt->party_x = engine->game.party_x;
    receipt->party_y = engine->game.party_y;
    receipt->party_dir = engine->game.party_dir;
    receipt->tick_count = engine->game.tick_count;
    receipt->title_loaded = title_loaded ? 1 : 0;
    snprintf(receipt->dungeon_path,
             sizeof(receipt->dungeon_path),
             "%s/LEV00.DGN",
             data_dir ? data_dir : "");
}

int nexus_v1_launcher_boot_level0_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherBootReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_Engine *engine;
    int title_loaded = 0;

    if (out_receipt) {
        nexus_v1_launcher_boot_receipt_clear(out_receipt);
    }
    if (!data_dir || !out_receipt) {
        return 0;
    }
    if (nexus_v1_launcher_init(data_dir) != 0) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        return 0;
    }
    if (nexus_v1_launcher_load_level(0) != 0) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_LEVEL_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        nexus_v1_launcher_shutdown();
        return 0;
    }
    engine = nexus_v1_launcher_get_engine();
    if (!engine) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        return 0;
    }

    /* New selected-entry boots always start from Nexus defaults; save
     * resume applies persisted party/tick state in the M11 resume path. */
    nexus_v1_game_init(&engine->game, engine->data_dir);
    engine->game.current_level = 0;
    nexus_v1_champions_init(&engine->champions);
    if (engine->mechanics) {
        nexus_mechanics_init(engine->mechanics,
                             engine->game.party_x,
                             engine->game.party_y,
                             engine->game.party_dir);
        engine->mechanics->map_index = 0;
    }

    if (title) {
        title_loaded = nexus_title_load(title, engine) == 0 && title->loaded;
    }

    memset(&facts, 0, sizeof(facts));
    facts.champion_pool = &engine->champions;
    if (!nexus_v1_startup_launch_from_host_facts_with_receipt(
            &facts,
            &out_receipt->startup_receipt)) {
        nexus_v1_launcher_shutdown();
        return 0;
    }
    nexus_v1_launcher_fill_boot_receipt(data_dir,
                                        engine,
                                        title_loaded,
                                        out_receipt);
    return 1;
}

void nexus_v1_launcher_shutdown(void) {
    if (!s_initialized) {
        return;
    }
    nexus_v1_shutdown(&s_engine);
    s_initialized = 0;
    printf("Nexus launcher: shut down\n");
}
