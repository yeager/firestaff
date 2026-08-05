/*
 * firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe.c
 *
 * Pass 20260628181326758026000: CSB quickplay/startup dungeon-handle
 * handoff probe.
 *
 * Narrow runtime-boundary probe that proves the verified DUNGEON.DAT handle
 * survives the boot->runtime handoff AND is cleared on a failed rescan.
 *
 * One real-asset execution path: argv[1], FIRESTAFF_CSB_DATA, the legacy
 * FIRESTAFF_CSB_PC_DATA, or ~/.firestaff/data/csb may point at any
 * hash-recognised CSB package. This exercises the verified dungeon path
 * end-to-end: csb_v1_boot_scan_assets() recognises the package,
 * csb_v1_boot_enter_game() loads its verified DUNGEON.DAT into
 * profile->runtime.dungeon_handle, and the global singleton
 * csb_v1_dungeon_get_current() points at that same handle. The probe skips
 * cleanly when no original package is supplied; it never seeds a verified
 * profile or dungeon from substitute bytes.
 *
 * Invariants (all must PASS):
 *
 *   H1 — handle handoff: enter_game() on a verified profile sets
 *        profile->runtime.dungeon_handle to a non-NULL heap-owned dungeon
 *        whose level_count is at least 1 and which is exposed through
 *        csb_v1_dungeon_get_square_type().
 *   H2 — singleton identity: csb_v1_dungeon_get_current() returns the
 *        same pointer as profile->runtime.dungeon_handle (the handoff
 *        reach the dungeon-layer accessors).
 *   H3 — current-level reset: csb_v1_dungeon_get_current_level() == 0
 *        (new-game map 0, source-locked).
 *   H4 — runtime state: profile->runtime.state == CSB_STATE_TITLE and
 *        profile->state == CSB_V1_BOOT_STATE_RUNTIME_READY.
 *   H5 — failed rescan clears handle: a rescan that does NOT verify CSB
 *        assets (no data dir or a non-existent dir) clears
 *        profile->runtime.dungeon_handle to NULL and resets
 *        csb_v1_dungeon_get_current() to NULL.
 *   H6 — failed rescan clears profile: after a failed rescan
 *        profile->state == CSB_V1_BOOT_STATE_PROFILE_READY, paths/md5s
 *        are empty, variant_id is UNKNOWN.
 *   H7 — failed rescan blocks re-launch: csb_v1_boot_enter_game() returns
 *        -1 on the cleared profile, confirming the next launch goes
 *        through a fresh verification cycle.
 *   H8 — successful rescan re-uses path: a second rescan back into a
 *        directory that DOES carry the CSB pair succeeds, and a subsequent
 *        enter_game() re-establishes a non-NULL dungeon_handle.
 *   H9 — boot cleanup clears singleton: csb_v1_boot_cleanup() resets
 *        csb_v1_dungeon_get_current() to NULL.
 *   H10 — idempotent cleanup: a second csb_v1_boot_cleanup() on the
 *         already-cleared profile does not crash.
 *
 * Source-lock boundary (ReDMCSB + CSBWin):
 *   - ENTRANCE.C F0806 lines 409-441: CSB entrance and C28_ENTRANCE_CSB.
 *   - LOADSAVE.C F0435 lines 1936-1944: new-game dungeon load and map 0.
 *   - DUNGEON.C F0237: dungeon load entry.
 *   - DUNGEON.C F0173/F0174 lines 2724-2755: dungeon release path.
 *
 * Skip-safe on hosts without user-supplied CSB data.
 *
 * Exit code: 0 if every invariant passes, 1 otherwise.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                          \
    ++g_checks;                                                        \
    if (cond) {                                                        \
        printf("  PASS: %s\n", msg);                                   \
    } else {                                                           \
        ++g_failures;                                                  \
        printf("  FAIL: %s\n", msg);                                   \
    }                                                                  \
} while (0)

/* Resolve an original CSB data dir from argv[1] / env / default. */
static const char *csb_data_dir(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];

    env = getenv("FIRESTAFF_CSB_DATA");
    /* An explicitly empty value is the CTest opt-out: do not recursively
     * scan a developer's whole data root during a skip-safe test run. */
    if (env) return env[0] != '\0' ? env : NULL;

    env = getenv("FIRESTAFF_CSB_PC_DATA");
    if (env && env[0] != '\0') return env;

    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data/csb", home);
    return buf;
}

static int csb_data_present(const char *dir)
{
    CSB_V1_BootProfile profile;
    if (!dir || dir[0] == '\0') return 0;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, dir) == 0;
}

/* ── Probe the handle-survival + rescan-clearing invariants on a
 *    profile that already passed csb_v1_boot_enter_game(). */
static void probe_handle_after_handoff(CSB_V1_BootProfile *profile,
                                        const char *label)
{
    const CSB_V1_DungeonData *current;
    char msg[160];

    /* H1 — handle handoff */
    CHECK(profile->runtime.dungeon_handle != NULL,
          "H1a: runtime owns a non-NULL dungeon handle after enter_game");
    current = csb_v1_dungeon_get_current();
    snprintf(msg, sizeof(msg),
             "H1b: %s: loaded dungeon has at least one level", label);
    CHECK(current != NULL && current->level_count >= 1, msg);
    if (current && current->level_count >= 1) {
        int sq = csb_v1_dungeon_get_square_type(current, 0, 0, 0);
        snprintf(msg, sizeof(msg),
                 "H1c: %s: square_type(0,0,0) returns a parsed value", label);
        CHECK(sq >= 0, msg);
    }

    /* H2 — singleton identity */
    snprintf(msg, sizeof(msg),
             "H2: %s: csb_v1_dungeon_get_current() == runtime.dungeon_handle",
             label);
    CHECK(current == profile->runtime.dungeon_handle, msg);

    /* H3 — current-level reset */
    snprintf(msg, sizeof(msg),
             "H3: %s: current dungeon level is map 0 (new-game)",
             label);
    CHECK(csb_v1_dungeon_get_current_level() == 0, msg);

    /* H4 — runtime state */
    snprintf(msg, sizeof(msg),
             "H4a: %s: boot profile is RUNTIME_READY", label);
    CHECK(profile->state == CSB_V1_BOOT_STATE_RUNTIME_READY, msg);
    snprintf(msg, sizeof(msg),
             "H4b: %s: runtime starts at CSB_STATE_TITLE", label);
    CHECK(profile->runtime.state == CSB_STATE_TITLE, msg);
}

/* ── Probe the failed-rescan invariants. */
static void probe_handle_after_failed_rescan(CSB_V1_BootProfile *profile,
                                             const char *failed_dir,
                                             const char *label)
{
    int rc = csb_v1_boot_scan_assets(profile, failed_dir);
    char msg[160];

    snprintf(msg, sizeof(msg),
             "H5a: %s: failed rescan returns -1 (no CSB assets)", label);
    CHECK(rc == -1, msg);

    /* H5 — failed rescan clears handle */
    snprintf(msg, sizeof(msg),
             "H5b: %s: runtime.dungeon_handle is NULL after failed rescan",
             label);
    CHECK(profile->runtime.dungeon_handle == NULL, msg);
    snprintf(msg, sizeof(msg),
             "H5c: %s: csb_v1_dungeon_get_current() is NULL after failed rescan",
             label);
    CHECK(csb_v1_dungeon_get_current() == NULL, msg);

    /* H6 — failed rescan clears profile */
    snprintf(msg, sizeof(msg),
             "H6a: %s: profile.state == PROFILE_READY after failed rescan",
             label);
    CHECK(profile->state == CSB_V1_BOOT_STATE_PROFILE_READY, msg);
    snprintf(msg, sizeof(msg),
             "H6b: %s: graphics_path cleared after failed rescan", label);
    CHECK(profile->graphics_path[0] == '\0', msg);
    snprintf(msg, sizeof(msg),
             "H6c: %s: dungeon_path cleared after failed rescan", label);
    CHECK(profile->dungeon_path[0] == '\0', msg);
    snprintf(msg, sizeof(msg),
             "H6d: %s: graphics_md5 cleared after failed rescan", label);
    CHECK(profile->graphics_md5[0] == '\0', msg);
    snprintf(msg, sizeof(msg),
             "H6e: %s: dungeon_md5 cleared after failed rescan", label);
    CHECK(profile->dungeon_md5[0] == '\0', msg);
    snprintf(msg, sizeof(msg),
             "H6f: %s: variant_id reset to UNKNOWN after failed rescan",
             label);
    CHECK(profile->variant_id == CSB_V1_VARIANT_UNKNOWN, msg);

    /* H7 — failed rescan blocks re-launch */
    snprintf(msg, sizeof(msg),
             "H7: %s: csb_v1_boot_enter_game() refuses to launch the "
             "cleared profile", label);
    CHECK(csb_v1_boot_enter_game(profile) == -1, msg);
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir = csb_data_dir(argc, argv, default_dir, sizeof(default_dir));
    CSB_V1_BootProfile profile;

    printf("=== CSB V1 quickplay dungeon handle probe ===\n\n");
    printf("data_dir=%s\n", dir ? dir : "(none)");

    if (!csb_data_present(dir)) {
        printf("SKIP: no hash-recognised CSB package; no synthetic profile is used\n");
        return 0;
    }
    {
        printf("Path: real hash-verified CSB package\n");
        csb_v1_boot_profile_init(&profile);
        CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
              "CSB assets scan by hash");
        CHECK(profile.assets_verified == 1 &&
              profile.graphics_verified == 1 &&
              profile.dungeon_verified == 1,
              "CSB boot profile is fully verified");
        CHECK(profile.variant_id != CSB_V1_VARIANT_UNKNOWN,
              "CSB variant detection selects a supported package");

        CHECK(csb_v1_boot_enter_game(&profile) == 0,
              "enter_game accepts the verified CSB profile");

        probe_handle_after_handoff(&profile, "real-asset");

        /* H5-H7 — failed rescan must clear the handoff handle. */
        probe_handle_after_failed_rescan(
            &profile,
            "/tmp/firestaff-csb-quickplay-dungeon-handle-empty",
            "real-asset");

        /* H8 — successful rescan back into the verified dir re-establishes
         * the handle. */
        if (csb_data_present(dir)) {
            CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
                  "H8a: re-rescan back into verified dir succeeds");
            CHECK(profile.assets_verified == 1 &&
                  profile.dungeon_verified == 1,
                  "H8b: re-rescan restores all verification flags");
            CHECK(profile.runtime.dungeon_handle == NULL,
                  "H8c: re-rescan into verified dir releases the previous "
                  "handle before re-launch (no double ownership)");
            CHECK(csb_v1_dungeon_get_current() == NULL,
                  "H8d: re-rescan into verified dir resets the singleton");
            CHECK(csb_v1_boot_enter_game(&profile) == 0,
                  "H8e: enter_game re-establishes a fresh runtime handle");
            probe_handle_after_handoff(&profile, "real-asset (re-launch)");
        }

        /* H9-H10 — boot cleanup. */
        csb_v1_boot_cleanup(&profile);
        CHECK(csb_v1_dungeon_get_current() == NULL,
              "H9: boot_cleanup clears the singleton");
        csb_v1_boot_cleanup(&profile);
        CHECK(1,
              "H10: second boot_cleanup is idempotent (does not crash)");

    }

    printf("\nchecks=%d failures=%d path=real-assets\n", g_checks, g_failures);

    return g_failures == 0 ? 0 : 1;
}
