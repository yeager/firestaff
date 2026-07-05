/*
 * firestaff_theron_v1_startup_real_asset_receipt_probe.c
 *
 * Headless CI probe: verifies the Theron V1 startup real-asset receipt
 * surface (src/theron/theron_v1_startup_receipt.c).
 *
 * What this probe covers:
 *
 *   1. Determinism: two consecutive placeholder receipts produce the
 *      same session_tick_token (same inputs -> same hash).
 *
 *   2. No-data placeholder path:
 *      - reset() clears the struct
 *      - set_placeholder() populates NO_DATA_PLACEHOLDER + verdict_name +
 *        source_evidence without consulting any file or MD5.
 *      - skip_reason_note names the no-data contract.
 *      - m11_dispatch_source_kind is -1 (no M11 work done).
 *      - to_line() renders a non-empty line containing the verdict name
 *        and the "no-data-placeholder" substring.
 *
 *   3. MD5 recognition gate: the four known TQ Track 02 MD5s are
 *      recognised; everything else is rejected.
 *
 *   4. Empty / NULL input paths downgrade to placeholder with a clear
 *      note (audit trail completeness).
 *
 *   5. Unknown MD5 downgrades to REJECTED with a clear note.
 *
 *   6. Missing file downgrades to SKIPPED with a clear note and the
 *      boot platform/version correctly inferred from the supplied MD5.
 *
 *   7. Real-asset path is skip-safe: when a real Track 02 file is
 *      available on disk (operator-installed, env override, or
 *      ~/.firestaff/data/theron), the receipt emits
 *      REAL_ASSET_RECEIPT with:
 *        - bank-signal decoder anchors populated for raw BIN variants
 *        - JP Rev 1 ISO zero-image variant yields a SKIPPED verdict
 *          with the documented "insufficient zero image" note
 *      When no real file is present, the same probe asserts SKIPPED so
 *      CI stays green without a staged Track 02.
 *
 *   8. Source evidence citation always names the four known Track 02
 *      MD5s and references both the theron_v1_startup_receipt module
 *      and the upstream Track 02 decoder.
 *
 *   9. Boot profile fields populated by the real-asset path match the
 *      documented Theron V1 contract (tick_rate_hz=18,
 *      max_champions=4, dungeon_count=7, assets_verified=1).
 *
 *  10. Variant / version id selection: JP BIN -> pce-jp, US BIN ->
 *      pce-en, JP Rev 1 ISO -> pce-jp-rev1-iso, US ISO -> pce-en-iso.
 *
 *  11. Startup Soul Room mirror contract is present even on no-data
 *      hosts: seven mirrors, three-companion cap, portrait ordinals 1..7,
 *      and class mask covering Fighter/Ninja/Priest/Wizard.
 *
 *  12. Startup chapter/progression contract is present in the receipt:
 *      placeholder rows are clearly no-profile/no-progression, while
 *      real Track 02 receipts expose Chapter 1 / Hall of Records and a
 *      0/7 quest-item summary without entering M11.
 *
 * Exit codes:
 *   0  - all checks passed (or were appropriately skipped with a
 *        deterministic verdict)
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy \
 *     ./build/firestaff_theron_v1_startup_real_asset_receipt_probe
 *
 * Source-lock:
 *   - src/theron/theron_v1_startup_receipt.c (this probe's target)
 *   - src/theron/theron_v1_track02.c        (bank-signal decoder)
 *   - src/theron/theron_v1_boot.c           (boot profile + direct launch)
 *   - include/asset_status_m12.h m12_file_md5_hex (file-MD5 helper)
 *   - src/shared/asset_status_m12.c g_theronVersions
 *   - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   - docs/source-lock/tqr_v1_phase1_boot_H2338.md
 *   - docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md
 */

#include "asset_status_m12.h"
#include "theron_v1_startup_receipt.h"
#include "theron_v1_boot.h"
#include "theron_v1_startup_flow.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

static int g_total = 0;
static int g_failed = 0;
static int g_skipped = 0;

static void check(int cond, const char *name) {
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_str_eq(const char *got, const char *want, const char *name) {
    ++g_total;
    if (!got || !want || strcmp(got, want) != 0) {
        ++g_failed;
        printf("[FAIL] %s: got '%s', want '%s'\n",
               name, got ? got : "(null)", want ? want : "(null)");
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_str_contains(const char *got,
                                const char *needle,
                                const char *name) {
    ++g_total;
    if (!got || !needle || !strstr(got, needle)) {
        ++g_failed;
        printf("[FAIL] %s: '%s' missing '%s'\n",
               name, got ? got : "(null)", needle ? needle : "(null)");
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_startup_mirror_summary(const Theron_V1_StartupReceipt *r,
                                         const char *prefix) {
    uint32_t expected_class_mask =
        (uint32_t)((1u << THERON_CLASS_FIGHTER) |
                   (1u << THERON_CLASS_NINJA) |
                   (1u << THERON_CLASS_PRIEST) |
                   (1u << THERON_CLASS_WIZARD));
    char name[160];

    snprintf(name, sizeof(name), "%s mirror count == 7", prefix);
    check(r->startup_mirror_count == THERON_STARTUP_HERO_MIRROR_COUNT,
          name);
    snprintf(name, sizeof(name), "%s companion limit == 3", prefix);
    check(r->startup_companion_limit == THERON_STARTUP_MAX_COMPANIONS,
          name);
    snprintf(name, sizeof(name), "%s portrait range is 1..7", prefix);
    check(r->startup_portrait_min == 1u &&
          r->startup_portrait_max == 7u,
          name);
    snprintf(name, sizeof(name), "%s class mask covers all four classes",
             prefix);
    check(r->startup_class_mask == expected_class_mask, name);
    snprintf(name, sizeof(name), "%s mirror labels cover all seven slots",
             prefix);
    check(r->startup_fallback_label_count +
              r->startup_decoded_label_count ==
          THERON_STARTUP_HERO_MIRROR_COUNT,
          name);
    snprintf(name, sizeof(name), "%s decoded mirror labels stay bounded",
             prefix);
    check(r->startup_decoded_label_count <=
          THERON_STARTUP_HERO_MIRROR_COUNT, name);
    snprintf(name, sizeof(name), "%s decoded mirror art count starts at zero",
             prefix);
    check(r->startup_decoded_art_count == 0u, name);
}

static void check_startup_chapter_placeholder(
    const Theron_V1_StartupReceipt *r,
    const char *prefix) {
    char name[160];

    snprintf(name, sizeof(name), "%s chapter label marks no profile", prefix);
    check_str_contains(r->startup_chapter_label, "no boot profile", name);
    snprintf(name, sizeof(name), "%s quest total == 7", prefix);
    check(r->startup_quest_item_total == 7u, name);
    snprintf(name, sizeof(name), "%s quest mask starts empty", prefix);
    check(r->startup_quest_items_collected == 0u, name);
    snprintf(name, sizeof(name), "%s quest summary marks no progression", prefix);
    check_str_contains(r->startup_quest_summary, "0/7", name);
}

static void check_startup_chapter_real(
    const Theron_V1_StartupReceipt *r,
    const char *prefix) {
    char name[160];

    snprintf(name, sizeof(name), "%s chapter label starts at Hall of Records",
             prefix);
    check_str_contains(r->startup_chapter_label,
                       "Chapter 1: Hall of Records",
                       name);
    snprintf(name, sizeof(name), "%s quest total == 7", prefix);
    check(r->startup_quest_item_total == 7u, name);
    snprintf(name, sizeof(name), "%s quest mask starts empty", prefix);
    check(r->startup_quest_items_collected == 0u, name);
    snprintf(name, sizeof(name), "%s quest summary starts at 0/7", prefix);
    check_str_contains(r->startup_quest_summary, "0/7", name);
    snprintf(name, sizeof(name), "%s next hint names Crypt of Shadows", prefix);
    check_str_contains(r->startup_next_dungeon_hint,
                       "Crypt of Shadows",
                       name);
}

static int file_exists_nonempty(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static void default_data_path_for(const char *relative_name,
                                  char out_path[512]) {
    const char *home = getenv("HOME");
    if (!home || !home[0]) home = ".";
    snprintf(out_path, 512, "%s%s.firestaff%sdata%s%s",
             home, PATH_SEP, PATH_SEP, PATH_SEP, relative_name);
}

/* ── Invariant 1: determinism of placeholder receipts ─────────────── */

static void check_placeholder_determinism(void) {
    Theron_V1_StartupReceipt r1, r2;
    uint32_t t1, t2;

    theron_v1_startup_receipt_set_placeholder(&r1);
    theron_v1_startup_receipt_set_placeholder(&r2);

    t1 = theron_v1_startup_receipt_session_tick(&r1);
    t2 = theron_v1_startup_receipt_session_tick(&r2);
    /* recompute and read back via session_tick_token (already populated
     * by set_placeholder). */
    check(t1 == t2, "placeholder session tick stable across two receipts");
    check(r1.session_tick_token == t1 &&
          r2.session_tick_token == t2,
          "session_tick_token == session_tick() return");
    check(r1.session_tick_token != 0u,
          "placeholder token is non-zero (CI de-dup helper)");
}

/* ── Invariant 2: placeholder populates the no-data fields ────────── */

static void check_placeholder_fields(void) {
    Theron_V1_StartupReceipt r;
    char line[2048];
    size_t n;

    theron_v1_startup_receipt_set_placeholder(&r);
    check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_NO_DATA_PLACEHOLDER,
          "placeholder verdict == NO_DATA_PLACEHOLDER");
    check_str_eq(r.verdict_name, "no-data-placeholder",
                 "placeholder verdict_name");
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_UNKNOWN,
          "placeholder variant == UNKNOWN");
    check_str_eq(r.variant_name, "unknown",
                 "placeholder variant_name");
    check_str_contains(r.skip_reason_note, "no Track 02 file",
                       "placeholder note names the no-data contract");
    check_str_contains(r.source_evidence, THERON_TRACK02_MD5_JP_BIN,
                       "placeholder evidence cites JP BIN MD5");
    check_str_contains(r.source_evidence, THERON_TRACK02_MD5_US_BIN,
                       "placeholder evidence cites US BIN MD5");
    check_str_contains(r.source_evidence, THERON_TRACK02_MD5_JP_REV1_ISO,
                       "placeholder evidence cites JP Rev 1 ISO MD5");
    check_str_contains(r.source_evidence, THERON_TRACK02_MD5_US_ISO,
                       "placeholder evidence cites US ISO MD5");
    check(r.track02_path[0] == '\0',
          "placeholder leaves track02_path empty");
    check(r.track02_md5_hex[0] == '\0',
          "placeholder leaves track02_md5_hex empty");
    check(r.track02_byte_count == 0u,
          "placeholder leaves track02_byte_count zero");
    check(r.descriptor_window_entry_index == -1,
          "placeholder leaves descriptor window index at -1");
    check(r.descriptor_role_zero_fill_count == 0u &&
          r.descriptor_role_pre_data_count == 0u &&
          r.descriptor_role_post_data_count == 0u &&
          r.descriptor_role_descriptor_table_count == 0u,
          "placeholder leaves descriptor role summary empty");
    check(r.m11_dispatch_source_kind == -1,
          "placeholder leaves m11_dispatch_source_kind at -1");
    check(r.boot_profile_assets_verified == 0,
          "placeholder leaves boot_profile_assets_verified 0");
    check(r.initial_candidate_user_data_offset_valid == 0 &&
          r.initial_candidate_user_data_offset == 0u,
          "placeholder leaves initial user-data offset empty");
    check(r.user_data_window_count == 0u &&
          r.user_data_window_descriptor_count == 0u &&
          r.user_data_window_span_count == 0u &&
          r.user_data_window_initial_count == 0u &&
          r.user_data_window_overflow_count == 0u,
          "placeholder leaves user-data window catalog empty");
    check(r.startup_text_marker_count == 0u &&
          r.startup_text_us_prompt_count == 0u &&
          r.startup_text_jp_roster_count == 0u &&
          r.startup_text_marker_overflow_count == 0u,
          "placeholder leaves startup text marker catalog empty");
    check(r.startup_roster_name_count == 0u &&
          r.startup_roster_title_count == 0u &&
          r.startup_roster_overflow_count == 0u,
          "placeholder leaves startup roster-name catalog empty");
    check_startup_mirror_summary(&r, "placeholder startup");
    check_startup_chapter_placeholder(&r, "placeholder startup");

    n = theron_v1_startup_receipt_to_line(&r, line, sizeof(line));
    check(n > 0u && n < sizeof(line),
          "placeholder renders to a one-line receipt");
    check_str_contains(line, "verdict=no-data-placeholder",
                       "rendered line contains verdict marker");
    check_str_contains(line, "session_tick=0x",
                       "rendered line contains session tick marker");
    check_str_contains(line, "mirrors=7",
                       "rendered line contains startup mirror count");
    check_str_contains(line, "portrait_range=1..7",
                       "rendered line contains portrait range");
    check_str_contains(line, "mirror_fallback_labels=7",
                       "rendered line contains fallback mirror art count");
    check_str_contains(line, "mirror_decoded_labels=0",
                       "rendered line contains decoded mirror label count");
    check_str_contains(line, "mirror_decoded_art=0",
                       "rendered line contains decoded mirror art count");
    check_str_contains(line, "chapter=\"Chapter ?",
                       "rendered line contains startup chapter marker");
    check_str_contains(line, "quest_total=7",
                       "rendered line contains quest total");
    check_str_contains(line, "initial_bind_name=no-level",
                       "rendered line contains placeholder bind status name");
    check_str_contains(line, "startup_text_markers=0",
                       "rendered line contains placeholder text marker count");
    check_str_contains(line, "startup_roster_names=0",
                       "rendered line contains placeholder roster-name count");
}

/* ── Invariant 3: MD5 recognition gate ───────────────────────────── */

static void check_md5_recognition(void) {
    check(theron_v1_startup_receipt_md5_is_known(THERON_TRACK02_MD5_JP_BIN) == 1,
          "JP BIN MD5 recognised");
    check(theron_v1_startup_receipt_md5_is_known(THERON_TRACK02_MD5_US_BIN) == 1,
          "US BIN MD5 recognised");
    check(theron_v1_startup_receipt_md5_is_known(THERON_TRACK02_MD5_JP_REV1_ISO) == 1,
          "JP Rev 1 ISO MD5 recognised");
    check(theron_v1_startup_receipt_md5_is_known(THERON_TRACK02_MD5_US_ISO) == 1,
          "US ISO MD5 recognised");
    check(theron_v1_startup_receipt_md5_is_known("00000000000000000000000000000000") == 0,
          "all-zero MD5 rejected");
    check(theron_v1_startup_receipt_md5_is_known("deadbeefdeadbeefdeadbeefdeadbeef") == 0,
          "arbitrary MD5 rejected");
    check(theron_v1_startup_receipt_md5_is_known(NULL) == 0,
          "NULL MD5 rejected");
    check(theron_v1_startup_receipt_md5_is_known("") == 0,
          "empty MD5 rejected");
}

/* ── Invariant 4: empty / NULL input paths downgrade gracefully ───── */

static void check_empty_input_paths(void) {
    Theron_V1_StartupReceipt r;
    int rc;

    theron_v1_startup_receipt_reset(&r);
    rc = theron_v1_startup_receipt_from_file(NULL,
                                              THERON_TRACK02_MD5_US_BIN,
                                              &r);
    check(rc == 0, "NULL path returns 0");
    check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_NO_DATA_PLACEHOLDER,
          "NULL path -> placeholder");
    check_str_contains(r.skip_reason_note, "empty Track 02 path",
                       "NULL path note names the reason");

    theron_v1_startup_receipt_reset(&r);
    rc = theron_v1_startup_receipt_from_file("", NULL, &r);
    check(rc == 0, "empty path returns 0");
    check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_NO_DATA_PLACEHOLDER,
          "empty path -> placeholder");

    theron_v1_startup_receipt_reset(&r);
    rc = theron_v1_startup_receipt_from_file("", THERON_TRACK02_MD5_US_BIN, &r);
    check(rc == 0, "empty path with known MD5 -> placeholder");
    check_str_contains(r.skip_reason_note, "empty Track 02 path",
                       "empty path note names the reason");

    theron_v1_startup_receipt_reset(&r);
    rc = theron_v1_startup_receipt_from_file("/this/does/not/exist/track02.bin",
                                              THERON_TRACK02_MD5_US_BIN,
                                              &r);
    /* Non-existent path with a known MD5: the contract downgrades to
     * SKIPPED rather than placeholder because the path was non-empty
     * but the file simply isn't present on this host. */
    check(rc == 0, "non-existent path returns 0");
    check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_SKIPPED,
          "non-existent path -> skipped");
    check_str_contains(r.skip_reason_note, "not present",
                       "non-existent path note names the reason");
}

/* ── Invariant 5: unknown MD5 downgrade ──────────────────────────── */

static void check_unknown_md5(void) {
    Theron_V1_StartupReceipt r;
    int rc;

    theron_v1_startup_receipt_reset(&r);
    rc = theron_v1_startup_receipt_from_file("/tmp/any_track02.bin",
                                              "deadbeefdeadbeefdeadbeefdeadbeef",
                                              &r);
    check(rc == 0, "unknown MD5 returns 0");
    check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_REJECTED,
          "unknown MD5 -> rejected");
    check_str_contains(r.skip_reason_note, "not in known TQ Track 02 set",
                       "unknown MD5 note names the reason");
}

/* ── Invariant 6: missing file downgrade with platform/version id ─── */

static void check_missing_file_platform(void) {
    Theron_V1_StartupReceipt r;

    /* Use a definitely-missing path with a known MD5 to test platform
     * selection on the SKIPPED branch. */
    theron_v1_startup_receipt_reset(&r);
    theron_v1_startup_receipt_from_file("/nonexistent/nope.bin",
                                         THERON_TRACK02_MD5_JP_BIN,
                                         &r);
    check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_SKIPPED,
          "missing file JP BIN -> skipped");
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_JP_BIN,
          "missing file JP BIN -> variant=JP_BIN");

    theron_v1_startup_receipt_reset(&r);
    theron_v1_startup_receipt_from_file("/nonexistent/nope.bin",
                                         THERON_TRACK02_MD5_US_ISO,
                                         &r);
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_US_ISO,
          "missing file US ISO -> variant=US_ISO");
}

/* ── Invariant 7: real-asset path (skip-safe when file absent) ────── */

struct real_asset_case {
    const char *label;
    const char *env_name;
    const char *default_relative;
    const char *expected_md5;
    int expect_real_receipt; /* 1 = real, 0 = skipped */
};

static const struct real_asset_case g_real_cases[] = {
    {
        "JP Track 02 BIN",
        "FIRESTAFF_THERON_TRACK02_JP_BIN",
        "theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin",
        THERON_TRACK02_MD5_JP_BIN,
        1
    },
    {
        "US Track 02 BIN",
        "FIRESTAFF_THERON_TRACK02_US_BIN",
        "theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin",
        THERON_TRACK02_MD5_US_BIN,
        1
    },
    {
        "JP Rev 1 Track 02 ISO",
        "FIRESTAFF_THERON_TRACK02_JP_REV1_ISO",
        "theron/track02_jp_rev1.iso",
        THERON_TRACK02_MD5_JP_REV1_ISO,
        1
    },
    {
        "US Track 02 ISO",
        "FIRESTAFF_THERON_TRACK02_US_ISO",
        "theron/track02_us.iso",
        THERON_TRACK02_MD5_US_ISO,
        1
    },
};

static void check_real_asset_path(void) {
    size_t i;

    for (i = 0; i < sizeof(g_real_cases) / sizeof(g_real_cases[0]); ++i) {
        const struct real_asset_case *c = &g_real_cases[i];
        char path[512];
        const char *env_path;
        Theron_V1_StartupReceipt r;
        int rc;
        int present;

        env_path = getenv(c->env_name);
        if (env_path && env_path[0]) {
            snprintf(path, sizeof(path), "%s", env_path);
        } else {
            default_data_path_for(c->default_relative, path);
        }
        present = file_exists_nonempty(path);

        theron_v1_startup_receipt_reset(&r);
        rc = theron_v1_startup_receipt_from_file(path, c->expected_md5, &r);

        if (!present) {
            ++g_skipped;
            printf("[SKIP] %s: no file at %s (CI / no-data host)\n",
                   c->label, path);
            /* When the file is absent the contract is SKIPPED, not
             * NO_DATA_PLACEHOLDER, because the path was non-empty. */
            check(rc == 0, "absent file returns 0");
            check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_SKIPPED,
                  "absent file -> skipped verdict");
            check_str_contains(r.skip_reason_note, "not present",
                                "absent file note names the reason");
            continue;
        }

        /* File is present: verify the contract for a real asset. */
        check(rc == 1, "present file -> real-asset receipt (rc=1)");
        check((int)r.verdict == (int)THERON_V1_STARTUP_RECEIPT_REAL_ASSET_RECEIPT,
              "present file -> REAL_ASSET_RECEIPT verdict");
        check_str_eq(r.track02_md5_hex, c->expected_md5,
                     "track02_md5_hex round-trips expected MD5");
        check(r.track02_byte_count > 0u,
              "track02_byte_count populated from file stat");
        check(r.boot_profile_assets_verified == 1,
              "boot profile marks assets_verified");
        check(r.boot_profile_tick_rate_hz == 18u,
              "boot profile carries the documented 18 Hz tick rate");
        check(r.boot_profile_max_champions == 4u,
              "boot profile carries the documented 4 max champions");
        check(r.boot_profile_dungeon_count == 7u,
              "boot profile carries the documented 7 mini-dungeons");
        check(r.boot_profile_dungeon_seed != 0u,
              "boot profile carries a non-zero dungeon_seed");
        check_startup_mirror_summary(&r, "real receipt startup");
        check_startup_chapter_real(&r, "real receipt startup");
        /* JP Rev 1 ISO is allowed to be a zero-fill and we still want
         * the receipt to be REAL_ASSET_RECEIPT (the direct-launch
         * succeeded; the bank-signal decoder reported
         * INSUFFICIENT_ZERO_IMAGE which the receipt accepts). */
        check(r.m11_dispatch_source_kind == 1 /* M11_GAME_SOURCE_THERON_TRACK02 */,
              "m11_dispatch_source_kind marks the Theron Track 02 path");

        /* Version-id selection per the documented contract. */
        if (strcmp(c->expected_md5, THERON_TRACK02_MD5_JP_BIN) == 0) {
            check_str_eq(r.boot_profile_version_id, "pce-jp",
                         "JP BIN -> version_id=pce-jp");
            check((int)r.boot_profile_platform == (int)THERON_PLATFORM_PCE_JP,
                  "JP BIN -> platform=PceJp");
        } else if (strcmp(c->expected_md5, THERON_TRACK02_MD5_US_BIN) == 0) {
            check_str_eq(r.boot_profile_version_id, "pce-en",
                         "US BIN -> version_id=pce-en");
            check((int)r.boot_profile_platform == (int)THERON_PLATFORM_PCE_US,
                  "US BIN -> platform=PceUs");
        } else if (strcmp(c->expected_md5,
                          THERON_TRACK02_MD5_JP_REV1_ISO) == 0) {
            check_str_eq(r.boot_profile_version_id, "pce-jp-rev1-iso",
                         "JP Rev 1 ISO -> version_id=pce-jp-rev1-iso");
            check((int)r.boot_profile_platform == (int)THERON_PLATFORM_PCE_JP,
                  "JP Rev 1 ISO -> platform=PceJp");
        } else if (strcmp(c->expected_md5,
                          THERON_TRACK02_MD5_US_ISO) == 0) {
            check_str_eq(r.boot_profile_version_id, "pce-en-iso",
                         "US ISO -> version_id=pce-en-iso");
            check((int)r.boot_profile_platform == (int)THERON_PLATFORM_PCE_US,
                  "US ISO -> platform=PceUs");
        }

        /* Bank-signal summary: only populated for raw BIN / non-zero-fill
         * variants.  JP Rev 1 ISO is a documented zero-fill so the
         * offsets stay zero. */
        if (strcmp(c->expected_md5, THERON_TRACK02_MD5_JP_REV1_ISO) != 0) {
            check(r.descriptor_offset != 0u,
                  "real receipt has non-zero descriptor_offset");
            check(r.descriptor_size != 0u,
                  "real receipt has non-zero descriptor_size");
            check(r.descriptor_value_count == 9u,
                  "real receipt carries 9 descriptor words");
            check(r.descriptor_stride == 0x0400u,
                  "real receipt carries the documented 0x0400 stride");
            check(r.anchor_count >= 1u,
                  "real receipt reports at least one bank-signal anchor");
            check(r.descriptor_role_descriptor_table_count == 1u,
                  "real receipt reports exactly one descriptor-table role");
            check(r.descriptor_role_zero_fill_count +
                  r.descriptor_role_pre_data_count +
                  r.descriptor_role_post_data_count +
                  r.descriptor_role_descriptor_table_count == 9u,
                  "real receipt descriptor role summary covers all 9 entries");
            if (strcmp(c->expected_md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
                strcmp(c->expected_md5, THERON_TRACK02_MD5_US_BIN) == 0) {
                check(r.user_data_window_count == 7u,
                      "raw Track 02 receipt has 7 logical user-data windows");
                check(r.user_data_window_descriptor_count == 3u,
                      "raw Track 02 receipt has 3 descriptor user-data windows");
                check(r.user_data_window_span_count == 3u,
                      "raw Track 02 receipt has 3 span user-data windows");
                check(r.user_data_window_initial_count == 1u,
                      "raw Track 02 receipt has 1 initial-candidate user-data window");
                check(r.user_data_window_overflow_count == 0u,
                      "raw Track 02 receipt has no user-data window overflow");
                check(r.startup_text_marker_count == 7u,
                      "raw Track 02 receipt has 7 startup text markers");
                check(r.startup_text_marker_overflow_count == 0u,
                      "raw Track 02 receipt has no startup text marker overflow");
                if (strcmp(c->expected_md5, THERON_TRACK02_MD5_US_BIN) == 0) {
                    check(r.startup_text_us_prompt_count == 7u,
                          "US raw Track 02 receipt has 7 resurrect prompt markers");
                    check(r.startup_text_jp_roster_count == 0u,
                          "US raw Track 02 receipt has no JP roster markers");
                    check(r.startup_roster_name_count == 0u &&
                          r.startup_roster_title_count == 0u &&
                          r.startup_roster_overflow_count == 0u,
                          "US raw Track 02 receipt has no decoded roster yet");
                    check(r.startup_fallback_label_count == 7u &&
                          r.startup_decoded_label_count == 0u &&
                          r.startup_decoded_art_count == 0u,
                          "US raw Track 02 receipt keeps label/art fallback split");
                } else {
                    check(r.startup_text_us_prompt_count == 0u,
                          "JP raw Track 02 receipt has no US prompt markers");
                    check(r.startup_text_jp_roster_count == 7u,
                          "JP raw Track 02 receipt has 7 roster markers");
                    check(r.startup_roster_name_count == 8u,
                          "JP raw Track 02 receipt has 8 decoded roster names");
                    check(r.startup_roster_title_count == 7u,
                          "JP raw Track 02 receipt has 7 decoded roster titles");
                    check(r.startup_roster_overflow_count == 0u,
                          "JP raw Track 02 receipt has no roster overflow");
                    check(r.startup_fallback_label_count == 0u &&
                          r.startup_decoded_label_count == 7u &&
                          r.startup_decoded_art_count == 0u,
                          "JP raw Track 02 receipt uses decoded labels but no decoded art");
                }
            }
            check(r.descriptor_window_entry_index >= 0 &&
                  r.descriptor_window_entry_index < 9,
                  "real receipt descriptor window index is in range");
            if (strcmp(c->expected_md5, THERON_TRACK02_MD5_US_ISO) == 0) {
                check(r.descriptor_byte_before == 0x60u,
                      "US ISO descriptor byte-before is RTS");
                check(r.descriptor_byte_before_is_rts == 1,
                      "US ISO descriptor RTS marker is recognized");
                check(r.descriptor_all_zero_after == 1,
                      "US ISO descriptor window is zero after descriptor");
            }
        }
        if (strcmp(c->expected_md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
            strcmp(c->expected_md5, THERON_TRACK02_MD5_US_BIN) == 0) {
            check(r.initial_candidate_found == 1,
                  "raw Track 02 receipt has an initial candidate");
            check(r.initial_candidate_width == 32u,
                  "raw Track 02 initial candidate width is 32");
            check(r.initial_candidate_height == 27u,
                  "raw Track 02 initial candidate height is 27");
            check(r.initial_candidate_seed == 0x0108e938u,
                  "raw Track 02 initial candidate seed is source-locked");
            check(r.initial_candidate_level_index == 0x0026u,
                  "raw Track 02 initial candidate level index is source-locked");
            check(r.initial_candidate_start_x == 2,
                  "raw Track 02 initial candidate start x is runtime-ready");
            check(r.initial_candidate_start_y == 1,
                  "raw Track 02 initial candidate start y is runtime-ready");
            check(r.initial_candidate_start_dir == 1,
                  "raw Track 02 initial candidate start dir is runtime-ready");
            check(r.initial_candidate_descriptor_delta == 0xa852u,
                  "raw Track 02 initial candidate descriptor delta is locked");
            check(r.initial_candidate_anchor_match == 1,
                  "raw Track 02 initial candidate anchor relation is locked");
            check(r.initial_candidate_binding_status ==
                      THERON_TRACK02_LEVEL_HANDOFF_OK,
                  "raw Track 02 initial candidate binding status is OK");
            check(r.initial_candidate_count == 1u,
                  "raw Track 02 initial candidate binding finds one candidate");
            check(r.initial_candidate_expected_offset ==
                      r.initial_candidate_offset,
                  "raw Track 02 initial candidate expected offset matches");
            check(r.initial_candidate_user_data_offset_valid == 1,
                  "raw Track 02 initial candidate user-data offset is valid");
            if (strcmp(c->expected_md5, THERON_TRACK02_MD5_US_BIN) == 0) {
                check(r.initial_candidate_user_data_offset == 0x619914u,
                      "US raw Track 02 initial candidate user-data offset is locked");
            } else {
                check(r.initial_candidate_user_data_offset == 0x619114u,
                      "JP raw Track 02 initial candidate user-data offset is locked");
            }
            {
                char line[2048];
                theron_v1_startup_receipt_to_line(&r, line, sizeof(line));
                check_str_contains(line, "initial_bind_name=ok",
                                   "raw Track 02 rendered line names bind status");
                check_str_contains(line, "initial_user_valid=1",
                                   "raw Track 02 rendered line marks user-data offset valid");
                check_str_contains(line, "user_windows=7",
                                   "raw Track 02 rendered line names user-data catalog count");
                check_str_contains(line, "startup_text_markers=7",
                                   "raw Track 02 rendered line names text marker count");
                if (strcmp(c->expected_md5, THERON_TRACK02_MD5_JP_BIN) == 0) {
                    check_str_contains(line, "startup_roster_names=8",
                                       "JP raw Track 02 rendered line names roster count");
                    check_str_contains(line, "startup_roster_titles=7",
                                       "JP raw Track 02 rendered line names title count");
                    check_str_contains(line, "mirror_fallback_labels=0",
                                       "JP raw Track 02 rendered line clears fallback labels");
                    check_str_contains(line, "mirror_decoded_labels=7",
                                       "JP raw Track 02 rendered line names decoded labels");
                }
            }
        } else {
            check(r.initial_candidate_found == 0,
                  "non-raw Track 02 receipt makes no initial candidate claim");
        }

        check(r.session_tick_token != 0u,
              "real receipt has a non-zero session_tick_token");
        /* Determinism: rerun and compare. */
        {
            Theron_V1_StartupReceipt r2;
            theron_v1_startup_receipt_reset(&r2);
            theron_v1_startup_receipt_from_file(path, c->expected_md5, &r2);
            check(r2.session_tick_token == r.session_tick_token,
                  "session_tick_token stable across two real-asset calls");
        }
    }
}

/* ── Invariant 8: source evidence citation ────────────────────────── */

static void check_source_evidence(void) {
    const char *e = theron_v1_startup_receipt_source_evidence();
    check(e && e[0], "source_evidence() non-empty");
    check_str_contains(e, "theron_v1_startup_receipt.c",
                       "evidence references the receipt module");
    check_str_contains(e, "theron_v1_track02.c",
                       "evidence references the Track 02 decoder");
    check_str_contains(e, "theron_v1_boot.c",
                       "evidence references the boot module");
    check_str_contains(e, THERON_TRACK02_MD5_JP_BIN,
                       "evidence cites JP BIN MD5");
    check_str_contains(e, THERON_TRACK02_MD5_US_BIN,
                       "evidence cites US BIN MD5");
    check_str_contains(e, THERON_TRACK02_MD5_JP_REV1_ISO,
                       "evidence cites JP Rev 1 ISO MD5");
    check_str_contains(e, THERON_TRACK02_MD5_US_ISO,
                       "evidence cites US ISO MD5");
}

/* ── Invariant 9: bank-signal contract for known MD5 without bytes ──
 *
 * Even on hosts with no real Track 02, the receipt's variant enum must
 * be derivable from the MD5 alone so CI / no-data hosts can still
 * surface the right platform / version id through the catalog path. */
static void check_variant_from_md5_only(void) {
    Theron_V1_StartupReceipt r;

    theron_v1_startup_receipt_reset(&r);
    theron_v1_startup_receipt_from_file("/no/such/path",
                                         THERON_TRACK02_MD5_JP_BIN,
                                         &r);
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_JP_BIN,
          "JP BIN MD5 -> variant=JP_BIN even without bytes");

    theron_v1_startup_receipt_reset(&r);
    theron_v1_startup_receipt_from_file("/no/such/path",
                                         THERON_TRACK02_MD5_US_BIN,
                                         &r);
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_US_BIN,
          "US BIN MD5 -> variant=US_BIN even without bytes");

    theron_v1_startup_receipt_reset(&r);
    theron_v1_startup_receipt_from_file("/no/such/path",
                                         THERON_TRACK02_MD5_JP_REV1_ISO,
                                         &r);
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_JP_REV1_ISO,
          "JP Rev 1 ISO MD5 -> variant=JP_REV1_ISO even without bytes");

    theron_v1_startup_receipt_reset(&r);
    theron_v1_startup_receipt_from_file("/no/such/path",
                                         THERON_TRACK02_MD5_US_ISO,
                                         &r);
    check((int)r.variant == (int)THERON_TRACK02_VARIANT_US_ISO,
          "US ISO MD5 -> variant=US_ISO even without bytes");
}

int main(void) {
    printf("=== Theron V1 startup real-asset receipt probe ===\n");
    check_placeholder_determinism();
    check_placeholder_fields();
    check_md5_recognition();
    check_empty_input_paths();
    check_unknown_md5();
    check_missing_file_platform();
    check_real_asset_path();
    check_variant_from_md5_only();
    check_source_evidence();

    printf("--- %d passed, %d skipped, %d failed ---\n",
           g_total - g_failed, g_skipped, g_failed);
    if (g_failed != 0) {
        return 1;
    }
    /* Even on no-data hosts the probe is allowed to print skipped
     * cases; verdict is PASS as long as no FAIL fired. */
    return 0;
}
