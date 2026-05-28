/*
 * Firestaff DM1 V1 Capture Reference Check Probe
 *
 * Headless deterministic probe: verifies canonical DM1 PC 3.4 game data SHA256,
 * checks for the existence of original DOSBox capture directories, and reports
 * the SHA256 of any found capture files.
 *
 * This probe does NOT require Firestaff to be built or game files to be present.
 * It only checks paths and computes SHA256 using `openssl sha256`.
 *
 * Compile:
 *   cc -std=c99 -Wall -Wextra -pedantic \
 *       probes/dm1/firestaff_dm1_v1_capture_reference_check_probe.c \
 *       -o /tmp/capture_ref_check
 *
 * Run:
 *   /tmp/capture_ref_check
 *
 * Canonical game data expected:
 *   DUNGEON.DAT SHA256: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
 *   GRAPHICS.DAT SHA256: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
 *
 * Expected original capture directory (from pass94, 2026-04-28):
 *   ~/.openclaw/data/firestaff-release-v0.3.28/verification-m11/
 *     lane4-original-overlay-20260428-0917/pass94-diagnostic/viewport_224x136/
 *
 * IMPORTANT NOTE on pass94 captures (2026-04-28):
 *   These captures are IMPAIRED and not suitable as parity evidence.
 *   The DOSBox input automation failed to enter the dungeon - frames are
 *   classified as "entrance_menu" or "wall_closeup" instead of "dungeon_gameplay".
 *   Frames 03-06 have IDENTICAL SHA256 (701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c)
 *   indicating the game state did not advance between those captures.
 *   A new capture session with a working dungeon-entry input sequence is required.
 *
 * See:
 *   docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md
 *   docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_PATH 1024

/* -- SHA256 via openssl ------------------------------------------------ */
static int sha256_file(const char *path, char out_hex[65])
{
    char cmd[ MAX_PATH + 64 ];
    snprintf(cmd, sizeof(cmd), "openssl sha256 -r '%s' 2>/dev/null | awk '{print $1}'", path);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        out_hex[0] = '\0';
        return 0;
    }

    char buf[128] = {0};
    if (!fgets(buf, sizeof(buf), fp)) {
        pclose(fp);
        out_hex[0] = '\0';
        return 0;
    }
    pclose(fp);

    /* Remove trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';

    /* openssl -r format: "sha256 <hex>" - buf should be the 64-char hex */
    if (strlen(buf) != 64) {
        out_hex[0] = '\0';
        return 0;
    }

    memcpy(out_hex, buf, 65);
    return 1;
}

/* -- File existence --------------------------------------------------- */
static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

/* -- Main --------------------------------------------------------------- */
int main(void)
{
    int failures = 0;
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = "/home/trv2";

    printf("probe=firestaff_dm1_v1_capture_reference_check\n");
    printf("probe_description=Headless canonical-ref + capture-existence check; no game files needed\n\n");

    /* -- Section 1: Canonical game data -------------------------------- */
    printf("[1] Canonical game data\n\n");

    char dungeon_path[MAX_PATH];
    char graphics_path[MAX_PATH];
    char title_path[MAX_PATH];

    snprintf(dungeon_path, sizeof(dungeon_path),
             "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT", home);
    snprintf(graphics_path, sizeof(graphics_path),
             "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT", home);
    snprintf(title_path, sizeof(title_path),
             "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE", home);

    struct {
        const char *label;
        const char *path;
        const char *expected_sha256;
    } items[] = {
        { "DUNGEON.DAT", dungeon_path,   "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85" },
        { "GRAPHICS.DAT", graphics_path, "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e" },
    };

    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
        char sha[65];
        int ok = sha256_file(items[i].path, sha);
        if (!ok) {
            printf("  %-12s: ABSENT  (%s)\n", items[i].label, items[i].path);
            failures++;
        } else {
            int match = (strcmp(sha, items[i].expected_sha256) == 0);
            printf("  %-12s: %s  (expected: %.16s..., got: %.16s...)\n",
                   items[i].label,
                   match ? "MATCH" : "MISMATCH",
                   items[i].expected_sha256,
                   sha);
            if (!match) failures++;
        }
    }

    printf("  %-12s: %s\n", "TITLE", file_exists(title_path) ? "PRESENT" : "ABSENT");
    printf("\n");

    /* -- Section 2: pass94 original capture directory ----------------- */
    printf("[2] pass94 original captures (2026-04-28)\n");
    printf("    Source: firestaff-release-v0.3.28/verification-m11/\n");
    printf("            lane4-original-overlay-20260428-0917/pass94-diagnostic/\n\n");

    char pass94_base[MAX_PATH];
    snprintf(pass94_base, sizeof(pass94_base),
             "%s/.openclaw/data/firestaff-release-v0.3.28/verification-m11/"
             "lane4-original-overlay-20260428-0917/pass94-diagnostic/viewport_224x136", home);

    if (!file_exists(pass94_base)) {
        printf("  ABSENT: %s\n", pass94_base);
        printf("  -> A new DOSBox capture session is required.\n");
        printf("     See: docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md\n");
        failures++;
    } else {
        printf("  EXISTS: %s\n\n", pass94_base);

        struct {
            const char *filename;
            const char *expected_sha256;
            const char *note;
        } frames[] = {
            { "01_ingame_start_original_viewport_224x136.ppm",
              "358136006c6d53d112d1cfea3d4bd0fa0902df0ad8b7130cf68778e298a24aa9",
              "" },
            { "02_ingame_turn_right_original_viewport_224x136.ppm",
              "ea845264f9229fed624079892da4c51653b7a101b2c0fa3f95aea4a4621f7edb",
              "" },
            { "03_ingame_move_forward_original_viewport_224x136.ppm",
              "701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c",
              " KNOWN IMPAIRED (sha256 shared with frames 04-06)" },
            { "04_ingame_spell_panel_original_viewport_224x136.ppm",
              "701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c",
              " KNOWN IMPAIRED (sha256 shared with frames 03,05-06)" },
            { "05_ingame_after_cast_original_viewport_224x136.ppm",
              "701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c",
              " KNOWN IMPAIRED (sha256 shared with frames 03-04,06)" },
            { "06_ingame_inventory_panel_original_viewport_224x136.ppm",
              "701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c",
              " KNOWN IMPAIRED (sha256 shared with frames 03-05)" },
        };

        int present = 0, sha_match = 0, impaired = 0;

        for (size_t i = 0; i < sizeof(frames) / sizeof(frames[0]); i++) {
            char fpath[MAX_PATH];
            char sha[65];
            snprintf(fpath, sizeof(fpath), "%s/%s", pass94_base, frames[i].filename);

            if (!file_exists(fpath)) {
                printf("  frame %zu: ABSENT  (%s)\n", i + 1, frames[i].filename);
                failures++;
                continue;
            }

            if (!sha256_file(fpath, sha)) {
                printf("  frame %zu: SHA256_FAILED\n", i + 1);
                failures++;
                continue;
            }

            present++;
            int match = (strcmp(sha, frames[i].expected_sha256) == 0);
            int is_impaired = (strcmp(sha, "701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c") == 0);

            if (match) sha_match++;
            if (is_impaired) impaired++;

            printf("  frame %zu: %s  (%.16s...%s)\n",
                   i + 1,
                   match ? "MATCH" : "MISMATCH",
                   sha,
                   frames[i].note);
            if (!match) {
                printf("           expected: %.16s...\n", frames[i].expected_sha256);
                printf("           got:      %.16s...\n", sha);
            }
        }

        printf("\n");
        printf("  Summary: present=%d/6, sha_match=%d/6, impaired=%d/6\n", present, sha_match, impaired);
        printf("  Classification (pass80 audit):\n");
        printf("    frame 01: unclassified (not dungeon_gameplay)\n");
        printf("    frame 02: entrance_menu (not dungeon_gameplay)\n");
        printf("    frame 03: entrance_menu (not dungeon_gameplay)\n");
        printf("    frame 04: entrance_menu (not dungeon_gameplay)\n");
        printf("    frame 05: wall_closeup (not dungeon_gameplay)\n");
        printf("    frame 06: wall_closeup (not dungeon_gameplay)\n");
        printf("\n");

        if (impaired > 0 || sha_match < 6) {
            printf("  ASSESSMENT: IMPAIRED - DOSBox input route failed to enter dungeon.\n");
            printf("              No dungeon_gameplay frames among the 6 captures.\n");
            printf("              New DOSBox capture session required.\n");
            printf("              See: docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md\n");
            printf("              See: docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md\n");
            failures++;
        } else {
            printf("  ASSESSMENT: All frames intact (no impairment detected).\n");
        }
    }

    printf("\n");

    /* -- Section 3: Firestaff V1 captures ---------------------------- */
    printf("[3] Firestaff V1 captures (lane3-inventory-followup, 2026-04-28)\n\n");

    char lane3[MAX_PATH];
    snprintf(lane3, sizeof(lane3),
             "%s/.openclaw/data/firestaff-release-v0.3.28/verification-m11/"
             "lane3-inventory-followup-20260428-0914", home);

    if (!file_exists(lane3)) {
        /* Try workspace copy */
        snprintf(lane3, sizeof(lane3),
                 "%s/.openclaw/workspace-main/firestaff-v2-gap-manifest/verification-m11/"
                 "lane3-inventory-followup-20260428-0914", home);
    }

    if (file_exists(lane3)) {
        printf("  EXISTS: %s\n", lane3);
        printf("  NOTE: These are Firestaff-only captures (no paired originals).\n");
        printf("        Not usable as parity evidence without paired original captures.\n");
    } else {
        printf("  ABSENT: lane3 capture directory not found.\n");
    }

    printf("\n");

    /* -- Section 4: Minimum required captures ------------------------ */
    printf("[4] Minimum required paired captures for MATCHED parity\n");
    printf("    See: docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md section 5\n\n");
    printf("  %-20s | %-9s | %-9s | %-s\n", "Area", "Present", "Impaired", "Status");
    printf("  %-20s-|%-9s-|%-9s-|%-s\n", "--------------------", "---------", "---------", "------------------------");
    printf("  %-20s | %-9s | %-9s | %s\n", "Viewport", "6/6", "4/6", "BLOCKED (DOSBox route failed)");
    printf("  %-20s | %-9s | %-9s | %s\n", "Wall", "0/3", "-", "BLOCKED (none exist)");
    printf("  %-20s | %-9s | %-9s | %s\n", "Collision", "0/2", "-", "BLOCKED (none exist)");
    printf("  %-20s | %-9s | %-9s | %s\n", "Creature-chain", "0/2", "-", "BLOCKED (none exist)");
    printf("  %-20s | %-9s | %-9s | %s\n", "Champion-panel", "0/2", "-", "BLOCKED (none exist)");
    printf("\n");

    /* -- Result -------------------------------------------------------- */
    printf("result=%s\n", failures == 0 ? "PASS" : "FAIL");
    printf("failures=%d\n", failures);
    printf("\n");
    if (failures > 0) {
        printf("Blockers:\n");
        printf("  1. pass94 DOSBox route failed - no dungeon_gameplay frames.\n");
        printf("     New DOSBox capture session needed.\n");
        printf("     See: docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md\n");
        printf("  2. Wall, collision, creature-chain, champion-panel: no original captures.\n");
        printf("     See: docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md section 2\n");
    }

    return failures > 0 ? 1 : 0;
}
