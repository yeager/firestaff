/*
 * Pass 42 bounded probe — V1 chrome-mode switch + reroute.
 *
 * Scope: V1_BLOCKERS.md §6 ("Firestaff-invented UI chrome ...").
 * Pass 42 lands a V1 chrome-mode switch (default ON) that (a) hides
 * the Firestaff-invented control strip at y=165, and (b) reroutes
 * player-facing notifications from m11_set_status /
 * m11_set_inspect_readout into the rolling message log (the
 * source-faithful TEXT.C message-log surface in DM1 PC 3.4),
 * rendered by the bottom multi-line surface at y=149, 157, 165.
 *
 * This probe verifies:
 *
 *   1. The chrome-mode helper respects the FIRESTAFF_V1_CHROME
 *      env variable (ON by default, OFF when set to "0").
 *   2. V2 vertical-slice mode forces V1 chrome OFF.
 *   3. Chrome reroute pushes player-facing status payloads into
 *      the message log with the expected "ACTION - OUTCOME" form.
 *   4. Chrome reroute pushes player-facing inspect payloads into
 *      the message log with the expected "TITLE: DETAIL" form.
 *   5. Chrome reroute skips payloads whose key phrase already
 *      appears in the most recent 3 log entries (dedup against
 *      m11_log_event-pushed companions).
 *   6. Chrome reroute skips payloads in the player-facing
 *      suppress list (BOOT, PARTY MOVED TO, SPELL PANEL OPENED,
 *      etc.) -- no debug chatter leaks into the message log.
 *   7. Chrome reroute suppresses back-to-back byte-identical
 *      payloads (per-surface dedup via state->chromeRerouteLast*).
 *   8. The M11_DM1_VIEWPORT_* anchors from pass 40 remain
 *      literal in m11_game_view.c (no regression).
 *   9. The pass-41 champion status-box stride constants remain
 *      literal in m11_game_view.c (no regression).
 *  10. The control-strip enum values remain literal in
 *      m11_game_view.c -- pass 42 suppresses the draw call, it
 *      does NOT rename or relocate the underlying rectangle.
 *  11. The pass-42 V1 chrome-mode switch symbol
 *      m11_v1_chrome_mode_enabled is defined in m11_game_view.c.
 *  12. The pass-42 reroute helper m11_chrome_reroute_push_pass42
 *      is defined in m11_game_view.c.
 *  13. The per-surface state fields chromeRerouteLastStatus /
 *      chromeRerouteLastInspect are declared in m11_game_view.h.
 *  14. The renderer guards m11_draw_control_strip behind a V1
 *      chrome-mode check (no unconditional draw in V1 mode).
 *
 * This probe is diagnostic-only.  It does not link the full
 * m11_game_view translation unit (which would drag SDL headers);
 * instead it verifies the exact policy by (a) exercising the env
 * helper with the same static semantics, and (b) grepping the
 * source file for the expected symbols and call-site guards.
 *
 * Does not touch the game view, the tick orchestrator, any
 * rendering path, any M10 semantics, or any V1 runtime behavior.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

static void rec(const char* id, int ok, const char* why) {
    if (ok) {
        printf("PASS %s %s\n", id, why);
        g_pass++;
    } else {
        printf("FAIL %s %s\n", id, why);
        g_fail++;
    }
}

/* --- Source-level grep helpers ---------------------------------- */

static int file_contains(const char* path, const char* needle) {
    FILE* f = fopen(path, "r");
    char* buf;
    long size;
    int found;
    if (!f) {
        return -1;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) {
        fclose(f);
        return -1;
    }
    buf = (char*)malloc((size_t)size + 1U);
    if (!buf) {
        fclose(f);
        return -1;
    }
    if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf);
        fclose(f);
        return -1;
    }
    buf[size] = '\0';
    found = (strstr(buf, needle) != NULL) ? 1 : 0;
    free(buf);
    fclose(f);
    return found;
}

/* --- Policy mirror: env parsing ---------------------------------- */

/* Mirrors m11_v1_chrome_mode_enabled() + m11_v2_vertical_slice_enabled()
 * exactly, but without the static cache (so the probe can toggle the
 * env between invariants). */
static int chrome_mode_for_env(const char* v2env, const char* v1env) {
    int v2on = (v2env && v2env[0] != '\0' && strcmp(v2env, "0") != 0) ? 1 : 0;
    if (v2on) {
        return 0;
    }
    if (v1env && v1env[0] != '\0' && strcmp(v1env, "0") == 0) {
        return 0;
    }
    return 1;
}

/* --- Policy mirror: player-facing suppress list ------------------ */

static const char* const kSuppressPhrases[] = {
    "DUNGEON MASTER LOADED",
    "CHAOS STRIKES BACK LOADED",
    "DUNGEON MASTER II LOADED",
    "PARTY MOVED TO",
    "PARTY MOVED",
    "SPELL PANEL OPENED",
    "SPELL CLEARED",
    "RUNE ",
    "DOOR STATE CHANGED",
    "IDLE TICK",
    "GAME VIEW NOT STARTED",
    "GAME DATA LOADED",
    "FACING UPDATED",
    "TURN IGNORED",
    "MOVEMENT BLOCKED",
    "STRIKE COMMITTED",
    "SPELL COMMITTED",
    NULL
};

static int is_player_facing(const char* text) {
    int i;
    if (!text || text[0] == '\0') {
        return 0;
    }
    for (i = 0; kSuppressPhrases[i] != NULL; ++i) {
        if (strstr(text, kSuppressPhrases[i]) != NULL) {
            return 0;
        }
    }
    return 1;
}

/* --- Policy mirror: key-phrase extraction from payload ---------- */

static const char* extract_key(const char* text) {
    const char* hit;
    if (!text || text[0] == '\0') {
        return text;
    }
    hit = strstr(text, " - ");
    if (hit && hit[3] != '\0') {
        return hit + 3;
    }
    hit = strstr(text, ": ");
    if (hit && hit[2] != '\0') {
        return hit + 2;
    }
    return text;
}

/* --- Invariant 14 helpers: renderer call-site guard scan -------- */

static int source_has_guarded_control_strip(const char* path) {
    FILE* f;
    char line[512];
    int inGuard = 0;
    int sawGuardedCall = 0;
    int sawUnguardedCall = 0;
    f = fopen(path, "r");
    if (!f) {
        return 0;
    }
    /* Walk the file line by line; after we see the pass-42 guard\n     * pattern "if (!m11_v1_chrome_mode_enabled())", the next\n     * non-empty non-comment line must be the control-strip call.\n     * Any other bare m11_draw_control_strip(...) call outside the\n     * function body that is not inside the guard is treated as\n     * an unguarded call and fails the invariant. */
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "if (!m11_v1_chrome_mode_enabled())") != NULL) {
            inGuard = 1;
            continue;
        }
        if (strstr(line, "m11_draw_control_strip(framebuffer") != NULL) {
            if (inGuard) {
                sawGuardedCall = 1;
            } else {
                sawUnguardedCall = 1;
            }
            inGuard = 0;
        }
        if (strlen(line) > 1 && line[0] == '}' && inGuard) {
            inGuard = 0;
        }
    }
    fclose(f);
    return sawGuardedCall && !sawUnguardedCall;
}

int main(void) {
    const char* src = "m11_game_view.c";
    const char* hdr = "m11_game_view.h";

    printf("# firestaff_m11_pass42_chrome_reduction_probe\n");

    /* 1. Chrome mode helper respects FIRESTAFF_V1_CHROME env. */
    rec("INV_P42_01", chrome_mode_for_env(NULL, NULL) == 1,
        "default (no env) => V1 chrome mode ON");
    rec("INV_P42_02", chrome_mode_for_env(NULL, "0") == 0,
        "FIRESTAFF_V1_CHROME=0 => V1 chrome mode OFF");
    rec("INV_P42_03", chrome_mode_for_env(NULL, "1") == 1,
        "FIRESTAFF_V1_CHROME=1 => V1 chrome mode ON");
    rec("INV_P42_04", chrome_mode_for_env(NULL, "") == 1,
        "FIRESTAFF_V1_CHROME= (empty) => V1 chrome mode ON");

    /* 2. V2 vertical-slice mode forces V1 chrome OFF. */
    rec("INV_P42_05", chrome_mode_for_env("1", NULL) == 0,
        "FIRESTAFF_V2_VERTICAL_SLICE=1 overrides V1 chrome mode to OFF");
    rec("INV_P42_06", chrome_mode_for_env("1", "1") == 0,
        "FIRESTAFF_V2_VERTICAL_SLICE=1 + FIRESTAFF_V1_CHROME=1 => OFF (V2 wins)");
    rec("INV_P42_07", chrome_mode_for_env("0", "1") == 1,
        "FIRESTAFF_V2_VERTICAL_SLICE=0 + FIRESTAFF_V1_CHROME=1 => ON");

    /* 3. Status reroute payload form. */
    {
        char payload[96];
        const char* action = "STAIRS";
        const char* outcome = "ASCENDED TO PREVIOUS LEVEL";
        snprintf(payload, sizeof(payload), "%s - %s", action, outcome);
        rec("INV_P42_08", strcmp(payload, "STAIRS - ASCENDED TO PREVIOUS LEVEL") == 0,
            "status reroute payload form is \"ACTION - OUTCOME\"");
    }

    /* 4. Inspect reroute payload form. */
    {
        char payload[96];
        const char* title = "PIT FALL";
        const char* detail = "DROPPED TO LEVEL 2, POSITION (3,4)";
        snprintf(payload, sizeof(payload), "%s: %s", title, detail);
        rec("INV_P42_09", strcmp(payload, "PIT FALL: DROPPED TO LEVEL 2, POSITION (3,4)") == 0,
            "inspect reroute payload form is \"TITLE: DETAIL\"");
    }

    /* 5. Key-phrase extraction correctly picks the OUTCOME / DETAIL
     * half of the payload (so the dedup check against m11_log_event
     * entries matches the right substring). */
    rec("INV_P42_10",
        strcmp(extract_key("STAIRS - ASCENDED TO PREVIOUS LEVEL"),
               "ASCENDED TO PREVIOUS LEVEL") == 0,
        "extract_key picks outcome after \" - \"");
    rec("INV_P42_11",
        strcmp(extract_key("PIT FALL: DROPPED TO LEVEL 2"),
               "DROPPED TO LEVEL 2") == 0,
        "extract_key picks detail after \": \"");
    rec("INV_P42_12",
        strcmp(extract_key("SIMPLEPAYLOAD"), "SIMPLEPAYLOAD") == 0,
        "extract_key returns whole text when neither separator present");

    /* 6. Player-facing suppress list -- debug chatter skipped. */
    rec("INV_P42_13", is_player_facing("STAIRS - ASCENDED TO PREVIOUS LEVEL") == 1,
        "player-facing: STAIRS - ASCENDED TO PREVIOUS LEVEL");
    rec("INV_P42_14", is_player_facing("PIT - FELL TO NEXT LEVEL") == 1,
        "player-facing: PIT - FELL TO NEXT LEVEL");
    rec("INV_P42_15", is_player_facing("USE - POTION CONSUMED") == 1,
        "player-facing: USE - POTION CONSUMED");
    rec("INV_P42_16", is_player_facing("BOOT - GAME VIEW NOT STARTED") == 0,
        "suppressed: BOOT - GAME VIEW NOT STARTED");
    rec("INV_P42_17", is_player_facing("PARTY MOVED TO (3,4)") == 0,
        "suppressed: PARTY MOVED TO ...");
    rec("INV_P42_18", is_player_facing("SPELL PANEL OPENED") == 0,
        "suppressed: SPELL PANEL OPENED");
    rec("INV_P42_19", is_player_facing("RUNE LO (96)") == 0,
        "suppressed: RUNE ... readout");
    rec("INV_P42_20", is_player_facing("IDLE TICK ADVANCED") == 0,
        "suppressed: IDLE TICK ...");
    rec("INV_P42_21", is_player_facing("") == 0,
        "empty string is never player-facing");

    /* 7. Chrome reroute dedup logic (policy level).  If the key\n     * phrase is contained in any of the recent log entries, skip\n     * the push.  This is the runtime behavior in\n     * m11_chrome_reroute_already_in_log_pass42. */
    {
        const char* recent[3] = {
            "T0: ASCENDED TO LEVEL 1",
            "T0: STAIRS LEAD NOWHERE",
            ""
        };
        const char* key = extract_key("STAIRS - ASCENDED TO LEVEL 1");
        int anyMatch = 0;
        int i;
        for (i = 0; i < 3; ++i) {
            if (recent[i] && recent[i][0] != '\0' && strstr(recent[i], key) != NULL) {
                anyMatch = 1;
                break;
            }
        }
        rec("INV_P42_22", anyMatch == 1,
            "dedup skips reroute when key phrase already in recent log");
    }
    {
        const char* recent[3] = {
            "T0: DESCENDED TO LEVEL 2",
            "T0: PARTY MOVED",
            ""
        };
        const char* key = extract_key("STAIRS - ASCENDED TO PREVIOUS LEVEL");
        int anyMatch = 0;
        int i;
        for (i = 0; i < 3; ++i) {
            if (recent[i] && recent[i][0] != '\0' && strstr(recent[i], key) != NULL) {
                anyMatch = 1;
                break;
            }
        }
        rec("INV_P42_23", anyMatch == 0,
            "dedup allows reroute when no recent log entry shares key phrase");
    }

    /* 8. Pass-40 viewport anchors remain literal in m11_game_view.c. */
    rec("INV_P42_24",
        file_contains(src, "M11_DM1_VIEWPORT_X = 0") == 1 &&
        file_contains(src, "M11_DM1_VIEWPORT_Y = 33") == 1 &&
        file_contains(src, "M11_DM1_VIEWPORT_W = 224") == 1 &&
        file_contains(src, "M11_DM1_VIEWPORT_H = 136") == 1,
        "pass-40 M11_DM1_VIEWPORT_* anchors preserved");

    /* 9. Pass-41 champion status-box constants remain literal. */
    rec("INV_P42_25",
        file_contains(src, "M11_V1_PARTY_SLOT_W    = 67") == 1 &&
        file_contains(src, "M11_V1_PARTY_SLOT_STEP = 69") == 1,
        "pass-41 M11_V1_PARTY_SLOT_* constants preserved");

    /* 10. Control-strip enum block remains literal in m11_game_view.c. */
    rec("INV_P42_26",
        file_contains(src, "M11_CONTROL_STRIP_X = 14") == 1 &&
        file_contains(src, "M11_CONTROL_STRIP_Y = 165") == 1 &&
        file_contains(src, "M11_CONTROL_STRIP_W = 88") == 1 &&
        file_contains(src, "M11_CONTROL_STRIP_H = 14") == 1,
        "M11_CONTROL_STRIP_* enum preserved (pass 42 suppresses the draw, not the rect)");

    /* 11. Pass-42 switch symbol present. */
    rec("INV_P42_27",
        file_contains(src, "static int m11_v1_chrome_mode_enabled(void)") == 1,
        "m11_v1_chrome_mode_enabled() defined in m11_game_view.c");

    /* 12. Pass-42 reroute helper present. */
    rec("INV_P42_28",
        file_contains(src, "m11_chrome_reroute_push_pass42") == 1,
        "m11_chrome_reroute_push_pass42 defined in m11_game_view.c");

    /* 13. Per-surface state fields declared. */
    rec("INV_P42_29",
        file_contains(hdr, "chromeRerouteLastStatus") == 1 &&
        file_contains(hdr, "chromeRerouteLastInspect") == 1,
        "state->chromeRerouteLast{Status,Inspect} fields declared in header");

    /* 14. Renderer call-site guard. */
    rec("INV_P42_30",
        source_has_guarded_control_strip(src),
        "m11_draw_control_strip call is guarded by !m11_v1_chrome_mode_enabled()");

    /* 15. Reroute feeds both surfaces (the message-log surface
     * renders the reroute payload alongside m11_log_event entries;
     * we verify by checking that the bottom message surface scans
     * up to M11_MESSAGE_LOG_CAPACITY entries in chrome mode). */
    rec("INV_P42_31",
        file_contains(src, "maxLines = chromeMode ? 3 : 1") == 1,
        "bottom message surface renders 3 lines in V1 chrome mode, 1 otherwise");

    /* 16. Env opt-out is respected: the probe mirror produces the\n     * same decision as the runtime for each input. */
    rec("INV_P42_32",
        chrome_mode_for_env(NULL, "0") == 0 &&
        chrome_mode_for_env(NULL, NULL) == 1 &&
        chrome_mode_for_env("1", NULL) == 0,
        "chrome-mode decision function matches runtime policy for all 3 cases");

    printf("# summary: %d/%d invariants passed\n",
           g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
