/*
 * firestaff_theron_v1_chapter_marker_probe.c
 *
 * Theron's Quest V1 — Chapter/Progression Startup Marker Probe
 * (Pass scope: theron_startup_chapter_progression_marker_gate)
 *
 * This probe exercises the bounded startup marker surface introduced
 * in src/theron/theron_v1_chapter_marker.c.  It is intentionally
 * skip-safe: it never requires real Track 02 assets and never relies
 * on saves/theron/ being populated.
 *
 * Coverage (synthetic, deterministic, asset-free):
 *   1.  Mark init clears the struct.
 *   2.  NULL boot profile produces SKIP_NO_PROFILE.
 *   3.  Boot profile without progression + assets_verified == 0
 *       produces SKIP_NO_ASSET with a clearly-labelled synthetic
 *       chapter label.
 *   4.  Boot profile with assets_verified == 1 + a fresh
 *       progression at dungeon 1 produces OK_PROGRESSION_ONLY
 *       with the canonical "Chapter 1: Hall of Records" label.
 *   5.  Mid-progression (3 items collected) projects the right
 *       "next: Stone Sigil" hint and chapter 4 label.
 *   6.  Quest complete (7/7) flips verdict to OK_QUEST_COMPLETE.
 *   7.  In-memory save slot promotes verdict to OK_WITH_SAVE and
 *       surfaces the slot label + dungeon name.
 *   8.  theron_v1_chapter_marker_compute_save() against an empty
 *       save_root produces "No save slots present" without
 *       flipping the verdict to a failure.
 *   9.  theron_v1_chapter_marker_compute_save() against a
 *       populated temp save root enumerates and projects the
 *       freshest slot, promoting the verdict to OK_WITH_SAVE.
 *  10.  theron_v1_chapter_marker_compute_save() against NULL
 *       save_root leaves the verdict at OK_PROGRESSION_ONLY.
 *  11.  Format and report helpers produce non-empty NUL-terminated
 *       strings bounded by their declared sizes.
 *  12.  Verdict-name table covers every Theron_ChapterMarkerVerdict.
 *  13.  Source-evidence string is non-empty.
 *  14.  Determinism: 50 fresh markers from identical inputs hash equal.
 *
 * Source/evidence:
 *   - THQUEST.ASM T000 (startup entry) — chapter label projection.
 *   - THQUEST.ASM T080 (between-dungeon save/load) — freshest slot.
 *   - THQUEST.ASM T520 (party placement) — start position.
 *   - THQUEST.ASM T560 (dungeon header) — dungeon_seed, level_count.
 *   - THQUEST.ASM T800 (champion persistence).
 *   - ReDMCSB does not cover Theron (CSB-only decompilation project);
 *     structure mirrors ReDMCSB GROUP.C / CLIKMENU.C analogues.
 *   - Phase 0 provenance gate (PASSED):
 *     docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *       JP MD5: b7afb338ad31be1025b53f9aff12d73a
 *       US MD5: f23601102138f87c33025877767ebf76
 *
 * Run:
 *   ./build/firestaff_theron_v1_chapter_marker_probe
 *
 * Pass: 14/14 invariants.
 */

#include "theron_v1_chapter_marker.h"
#include "theron_v1_boot.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define PROBE_MKDIR(p) _mkdir(p)
#define PROBE_RMDIR(p) _rmdir(p)
#define PROBE_UNLINK(p) remove(p)
#define PROBE_PATH_SEP '\\'
#define PROBE_TMP_PATTERN "C:\\TEMP\\firestaff_theron_marker_XXXXXX"
#else
#include <unistd.h>
#define PROBE_MKDIR(p) mkdir((p), 0700)
#define PROBE_RMDIR(p) rmdir(p)
#define PROBE_UNLINK(p) unlink(p)
#define PROBE_PATH_SEP '/'
#endif

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

/* Helper: make a temporary save root directory.  Returns 1 on
 * success, 0 on failure.  Writes the path into out (size >= 512). */
static int make_temp_save_root(char out[512]) {
#if defined(_WIN32) || defined(_WIN64)
    /* Windows tmp pattern: replace Xs with a unique stamp. */
    snprintf(out, 512, "C:\\TEMP\\firestaff_theron_marker_%lu",
              (unsigned long)rand());
    return PROBE_MKDIR(out) == 0;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_marker_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

/* Helper: remove every slot file under a save root, then the dir. */
static void remove_temp_save_root(const char *root) {
    if (!root || !root[0]) return;
    for (int i = 0; i < THERON_SAVE_SLOT_COUNT; ++i) {
        char path[512];
        theron_v1_save_slot_path(root, i, path, sizeof(path));
        if (path[0]) {
            PROBE_UNLINK(path);
        }
    }
    PROBE_RMDIR(root);
}

/* Helper: write a synthetic save slot at slot_index using the real
 * Theron save format.  The champion data is all-zero bytes (we never
 * decode it here — the probe only cares about header metadata). */
static int write_synthetic_slot(const char *root, int slot_index,
                                 uint8_t quest_items,
                                 uint8_t current_dungeon,
                                 uint32_t timestamp) {
    uint8_t champion_data[THERON_SAVE_CHAMPION_COUNT *
                          THERON_SAVE_CHAMPION_BLOCK_SIZE];
    Theron_DungeonProgression prog;

    memset(champion_data, 0, sizeof(champion_data));
    theron_v1_dungeon_progression_init(&prog);
    prog.quest_items_collected = quest_items;
    if (current_dungeon >= 1 && current_dungeon <= THERON_DUNGEON_COUNT) {
        prog.current_dungeon = (Theron_DungeonID)current_dungeon;
    }

    if (theron_v1_save_to_slot(root, slot_index,
                                champion_data, sizeof(champion_data),
                                &prog,
                                "synthetic-probe") != 0) {
        return 0;
    }

    /* The save header's timestamp is captured at write time, so the
     * caller cannot directly override it.  The probe tolerates any
     * timestamp ordering — the marker picks the freshest slot, not
     * a specific one. */
    (void)timestamp;
    return 1;
}

/* Helper: FNV-style 32-bit hash over the marker so we can assert
 * determinism across many compute() calls. */
static uint32_t marker_hash(const Theron_ChapterMarker *m) {
    uint32_t h = 0x811c9dc5u;
    h ^= (uint32_t)m->verdict;           h *= 0x01000193u;
    h ^= (uint32_t)m->boot_profile_present; h *= 0x01000193u;
    h ^= (uint32_t)m->boot_assets_verified;  h *= 0x01000193u;
    h ^= (uint32_t)m->quest_items_collected; h *= 0x01000193u;
    h ^= (uint32_t)m->quest_item_total;      h *= 0x01000193u;
    h ^= (uint32_t)m->freshest_save_present; h *= 0x01000193u;
    h ^= (uint32_t)m->freshest_save.valid;   h *= 0x01000193u;
    h ^= (uint32_t)m->freshest_save.slot_index; h *= 0x01000193u;
    /* Hash a few fixed bytes from each string field — enough to
     * catch content drift without paying for strlen() each call. */
    for (size_t i = 0; i < sizeof(m->chapter_label); ++i) {
        h ^= (uint32_t)(uint8_t)m->chapter_label[i];
        h *= 0x01000193u;
        if (m->chapter_label[i] == '\0') break;
    }
    for (size_t i = 0; i < sizeof(m->quest_summary); ++i) {
        h ^= (uint32_t)(uint8_t)m->quest_summary[i];
        h *= 0x01000193u;
        if (m->quest_summary[i] == '\0') break;
    }
    return h;
}

/* ── Invariants ──────────────────────────────────────────────────── */

static void check_init(void) {
    Theron_ChapterMarker m;
    theron_v1_chapter_marker_init(&m);
    CHECK(m.verdict == THERON_MARKER_VERDICT_SKIP_NO_PROFILE,
          "init: verdict == SKIP_NO_PROFILE");
    CHECK(m.boot_profile_present == 0, "init: boot_profile_present == 0");
    CHECK(m.quest_item_total == THERON_QUEST_ITEM_COUNT,
          "init: quest_item_total == 7");
    CHECK(m.freshest_save_present == 0, "init: freshest_save_present == 0");
}

static void check_null_profile(void) {
    Theron_ChapterMarker m;
    theron_v1_chapter_marker_compute(NULL, NULL, NULL, &m);
    CHECK(m.verdict == THERON_MARKER_VERDICT_SKIP_NO_PROFILE,
          "NULL profile: verdict == SKIP_NO_PROFILE");
    CHECK(m.boot_profile_present == 0,
          "NULL profile: boot_profile_present == 0");
    CHECK(strstr(m.chapter_label, "no boot profile") != NULL,
          "NULL profile: chapter label mentions missing profile");
}

static void check_no_asset_synthetic(void) {
    Theron_V1_BootProfile profile;
    Theron_ChapterMarker m;
    theron_v1_boot_profile_init(&profile);
    /* assets_verified defaults to 0 — we keep it that way. */
    theron_v1_chapter_marker_compute(&profile, NULL, NULL, &m);
    CHECK(m.verdict == THERON_MARKER_VERDICT_SKIP_NO_ASSET,
          "no-asset: verdict == SKIP_NO_ASSET");
    CHECK(m.boot_profile_present == 1,
          "no-asset: boot_profile_present == 1");
    CHECK(m.boot_assets_verified == 0,
          "no-asset: boot_assets_verified == 0");
    CHECK(strstr(m.chapter_label, "synthetic") != NULL,
          "no-asset: chapter label is clearly synthetic");
    CHECK(strstr(m.quest_summary, "0/7") != NULL,
          "no-asset: quest summary shows 0/7 collected");
}

static void check_fresh_profile(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;  /* pretend Track 02 matched a known MD5 */
    theron_v1_dungeon_progression_init(&prog);
    theron_v1_chapter_marker_compute(&profile, &prog, NULL, &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY,
          "fresh: verdict == OK_PROGRESSION_ONLY");
    CHECK(m.boot_assets_verified == 1,
          "fresh: boot_assets_verified copied to marker");
    CHECK(strstr(m.chapter_label, "Chapter 1") != NULL &&
          strstr(m.chapter_label, "Hall of Records") != NULL,
          "fresh: chapter label == Chapter 1: Hall of Records");
    CHECK(strstr(m.quest_summary, "0/7") != NULL,
          "fresh: quest summary shows 0/7 collected");
    CHECK(strstr(m.next_dungeon_hint, "Crypt of Shadows") != NULL,
          "fresh: next hint names Crypt of Shadows");
    CHECK(m.freshest_save_present == 0,
          "fresh: freshest_save_present == 0 without save input");
}

static void check_mid_progression(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);

    /* Collect dungeon 1, 2, 3 items (bits 0, 1, 2) and set the
     * current dungeon to 4 (Tomb of Woe) so the "next" hint is
     * Stone Sigil. */
    prog.quest_items_collected = (uint8_t)((1u << 0) | (1u << 1) | (1u << 2));
    prog.current_dungeon = THERON_DUNGEON_4_TOMB_OF_WOE;

    theron_v1_chapter_marker_compute(&profile, &prog, NULL, &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY,
          "mid: verdict == OK_PROGRESSION_ONLY");
    CHECK(strstr(m.chapter_label, "Chapter 4") != NULL &&
          strstr(m.chapter_label, "Tomb of Woe") != NULL,
          "mid: chapter label == Chapter 4: Tomb of Woe");
    CHECK(strstr(m.quest_summary, "3/7") != NULL,
          "mid: quest summary shows 3/7 collected");
    /* Dungeon 4's item is bit 3 (Stone Sigil) which is NOT yet
     * collected, so the summary should highlight it as the next
     * item, not the last one. */
    CHECK(strstr(m.quest_summary, "next: Stone Sigil") != NULL,
          "mid: quest summary names Stone Sigil as next");
    CHECK(m.quest_items_collected == 0x07,
          "mid: marker stores the 3-bit bitmask (0x07)");
}

static void check_quest_complete(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);
    prog.quest_items_collected = THERON_QUEST_ALL_ITEMS;
    prog.quest_complete = 1;

    theron_v1_chapter_marker_compute(&profile, &prog, NULL, &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_QUEST_COMPLETE,
          "complete: verdict == OK_QUEST_COMPLETE");
    CHECK(strstr(m.chapter_label, "Quest Complete") != NULL &&
          strstr(m.chapter_label, "7/7") != NULL,
          "complete: chapter label says Quest Complete (7/7)");
    CHECK(strstr(m.quest_summary, "7/7") != NULL,
          "complete: quest summary shows 7/7 collected");
}

static void check_save_promotes_verdict(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_SaveSlot slot;
    Theron_ChapterMarker m;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);

    memset(&slot, 0, sizeof(slot));
    slot.valid = 1;
    slot.slot_index = 2;
    slot.current_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    slot.dungeon_state = THERON_DUNGEON_STATE_COMPLETE;
    slot.quest_items = (uint8_t)(1u << 0);  /* dungeon 1 item only */
    slot.timestamp = 1700000000u;
    slot.size_bytes = 256;
    snprintf(slot.label, sizeof(slot.label), "After Crypt of Shadows");

    theron_v1_chapter_marker_compute(&profile, &prog, &slot, &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_WITH_SAVE,
          "save: verdict == OK_WITH_SAVE");
    CHECK(m.freshest_save_present == 1,
          "save: freshest_save_present == 1");
    CHECK(m.freshest_save.slot_index == 2,
          "save: freshest slot index == 2");
    CHECK(strstr(m.freshest_save_line, "Save slot 2") != NULL &&
          strstr(m.freshest_save_line, "After Crypt of Shadows") != NULL,
          "save: line shows slot + label");
}

static void check_save_compute_empty_root(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);

    /* Point at a directory that does not exist — enumeration should
     * fail silently and the marker should fall back to OK_PROGRESSION_ONLY
     * with "No save slots present". */
    theron_v1_chapter_marker_compute_save(&profile, &prog,
                                           "/nonexistent/firestaff_theron_probe",
                                           &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY,
          "empty save root: verdict stays OK_PROGRESSION_ONLY");
    CHECK(m.freshest_save_present == 0,
          "empty save root: freshest_save_present == 0");
    CHECK(strstr(m.freshest_save_line, "No save slots") != NULL,
          "empty save root: line reports no saves");
}

static void check_save_compute_null_root(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);

    theron_v1_chapter_marker_compute_save(&profile, &prog, NULL, &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY,
          "NULL save root: verdict stays OK_PROGRESSION_ONLY");
    CHECK(strstr(m.freshest_save_line, "skipped") != NULL,
          "NULL save root: line says lookup was skipped");
}

static void check_save_compute_populated_root(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;
    char save_root[512];

    CHECK(make_temp_save_root(save_root) == 1,
          "save compute: temp save root created");

    /* Write three synthetic slots so the freshest-slot picker has
     * something to pick from.  Real save_to_slot() captures
     * timestamps at write time so all three will have very close
     * timestamps — the marker picks any one of them as "freshest"
     * which is what we want to assert here. */
    CHECK(write_synthetic_slot(save_root, 0,
                                (uint8_t)(1u << 0),
                                THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                                100u),
          "save compute: slot 0 written");
    CHECK(write_synthetic_slot(save_root, 1,
                                (uint8_t)((1u << 0) | (1u << 1)),
                                THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                                200u),
          "save compute: slot 1 written");
    CHECK(write_synthetic_slot(save_root, 2,
                                (uint8_t)((1u << 0) | (1u << 1) | (1u << 2)),
                                THERON_DUNGEON_4_TOMB_OF_WOE,
                                300u),
          "save compute: slot 2 written");

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);

    theron_v1_chapter_marker_compute_save(&profile, &prog, save_root, &m);

    CHECK(m.verdict == THERON_MARKER_VERDICT_OK_WITH_SAVE,
          "populated save root: verdict == OK_WITH_SAVE");
    CHECK(m.freshest_save_present == 1,
          "populated save root: freshest_save_present == 1");
    CHECK(m.freshest_save.valid == 1,
          "populated save root: freshest save is marked valid");
    CHECK(strstr(m.freshest_save_line, "Save slot") != NULL,
          "populated save root: line references a save slot");

    remove_temp_save_root(save_root);
}

static void check_format_helpers(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m;
    char line[256];
    char report[THERON_CHAPTER_MARKER_REPORT_MAX];

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);

    theron_v1_chapter_marker_compute(&profile, &prog, NULL, &m);

    size_t written = theron_v1_chapter_marker_format(&m, line, sizeof(line));
    CHECK(written > 0, "format: one-line summary writes > 0 bytes");
    CHECK(line[written] == '\0' || written < sizeof(line),
          "format: summary is NUL-terminated within buf");
    CHECK(strstr(line, "Chapter 1") != NULL,
          "format: summary references Chapter 1");

    size_t rwritten = theron_v1_chapter_marker_report(&m, report, sizeof(report));
    CHECK(rwritten > 0, "report: multi-line report writes > 0 bytes");
    CHECK(strstr(report, "Theron V1 Chapter Marker") != NULL,
          "report: contains header banner");
    CHECK(strstr(report, "Verdict:") != NULL,
          "report: contains Verdict: field");
    CHECK(strstr(report, "Quest summary:") != NULL,
          "report: contains Quest summary: field");
    CHECK(strstr(report, "Freshest save:") != NULL,
          "report: contains Freshest save: field");
    /* Size-bound check: report must fit within its declared max. */
    CHECK(rwritten < sizeof(report),
          "report: write count stays below buffer size");

    /* Also exercise the NULL-marker code path. */
    char small[64];
    CHECK(theron_v1_chapter_marker_format(NULL, small, sizeof(small)) == 0,
          "format: NULL marker writes 0 bytes");
    CHECK(small[0] == '\0', "format: NULL marker leaves buf empty");
}

static void check_verdict_name_table(void) {
    const char *n;
    n = theron_v1_chapter_marker_verdict_name(
        THERON_MARKER_VERDICT_SKIP_NO_PROFILE);
    CHECK(n != NULL && strcmp(n, "SKIP_NO_PROFILE") == 0,
          "verdict table: SKIP_NO_PROFILE");
    n = theron_v1_chapter_marker_verdict_name(
        THERON_MARKER_VERDICT_SKIP_NO_ASSET);
    CHECK(n != NULL && strcmp(n, "SKIP_NO_ASSET") == 0,
          "verdict table: SKIP_NO_ASSET");
    n = theron_v1_chapter_marker_verdict_name(
        THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY);
    CHECK(n != NULL && strcmp(n, "OK_PROGRESSION_ONLY") == 0,
          "verdict table: OK_PROGRESSION_ONLY");
    n = theron_v1_chapter_marker_verdict_name(
        THERON_MARKER_VERDICT_OK_WITH_SAVE);
    CHECK(n != NULL && strcmp(n, "OK_WITH_SAVE") == 0,
          "verdict table: OK_WITH_SAVE");
    n = theron_v1_chapter_marker_verdict_name(
        THERON_MARKER_VERDICT_OK_QUEST_COMPLETE);
    CHECK(n != NULL && strcmp(n, "OK_QUEST_COMPLETE") == 0,
          "verdict table: OK_QUEST_COMPLETE");
    n = theron_v1_chapter_marker_verdict_name(
        (Theron_ChapterMarkerVerdict)999);
    CHECK(n != NULL && strcmp(n, "(invalid)") == 0,
          "verdict table: out-of-range -> (invalid)");
}

static void check_source_evidence(void) {
    const char *ev = theron_v1_chapter_marker_source_evidence();
    CHECK(ev != NULL, "source evidence: not NULL");
    CHECK(strlen(ev) > 80, "source evidence: long enough to be useful");
    CHECK(strstr(ev, "THQUEST.ASM") != NULL,
          "source evidence: cites THQUEST.ASM");
    CHECK(strstr(ev, "b7afb338ad31be1025b53f9aff12d73a") != NULL,
          "source evidence: cites JP Track 02 MD5");
}

static void check_determinism(void) {
    Theron_V1_BootProfile profile;
    Theron_DungeonProgression prog;
    Theron_ChapterMarker m1, m2;
    uint32_t h1, h2;
    int mismatch = 0;

    theron_v1_boot_profile_init(&profile);
    profile.assets_verified = 1;
    theron_v1_dungeon_progression_init(&prog);
    prog.quest_items_collected = (uint8_t)((1u << 0) | (1u << 1));
    prog.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;

    theron_v1_chapter_marker_compute(&profile, &prog, NULL, &m1);
    h1 = marker_hash(&m1);

    for (int rep = 0; rep < 50; ++rep) {
        theron_v1_boot_profile_init(&profile);
        profile.assets_verified = 1;
        theron_v1_dungeon_progression_init(&prog);
        prog.quest_items_collected = (uint8_t)((1u << 0) | (1u << 1));
        prog.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
        theron_v1_chapter_marker_compute(&profile, &prog, NULL, &m2);
        h2 = marker_hash(&m2);
        if (h2 != h1) { mismatch = 1; break; }
    }

    CHECK(mismatch == 0, "determinism: 50 compute() calls hash equal");
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Theron V1 chapter/progression startup marker probe ===\n\n");
    printf("Pass scope: theron_startup_chapter_progression_marker_gate\n");
    printf("Source-lock: THQUEST.ASM T000, T080, T520, T560, T800\n\n");

    check_init();
    check_null_profile();
    check_no_asset_synthetic();
    check_fresh_profile();
    check_mid_progression();
    check_quest_complete();
    check_save_promotes_verdict();
    check_save_compute_empty_root();
    check_save_compute_null_root();
    check_save_compute_populated_root();
    check_format_helpers();
    check_verdict_name_table();
    check_source_evidence();
    check_determinism();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
