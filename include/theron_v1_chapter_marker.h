/*
 * theron_v1_chapter_marker.h — Theron's Quest V1 chapter/progression
 * startup marker (narrow binding for boot/runtime readiness evidence).
 *
 * This header is intentionally bounded: it does NOT claim full
 * playability, full save-decode, or full Track 02 parsing.  It just
 * projects the *first* chapter label, quest-item summary, and the
 * freshest available between-dungeon save into a single readiness
 * marker that the M12 launcher / boot profile can surface to the
 * user before M11 takes over.
 *
 * Design contract:
 *   - The marker is skip-safe: it never opens Track 02 and never
 *     stat()s a save file unless the caller asks for a real save
 *     lookup.  The default synthetic path (zeroed progression +
 *     boot_profile_init) is always usable, so CI is deterministic.
 *   - Real-asset path is opt-in: pass a non-NULL save_root to
 *     theron_v1_chapter_marker_compute_save() and the marker will
 *     enumerate saves/theron/ for the freshest slot.  If no save
 *     is found the marker falls back to "no saves" without failing.
 *   - Source-locked to THQUEST.ASM T000 (startup entry), T080
 *     (between-dungeon save/load), and T800 (champion persistence).
 *
 * Pass scope: theron_startup_chapter_progression_marker_gate
 *   Bound to one marker (chapter label + quest summary + save slot
 *   summary), not the full 7-dungeon progression path.
 */

#ifndef THERON_V1_CHAPTER_MARKER_H
#define THERON_V1_CHAPTER_MARKER_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_boot.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_save_load.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Marker verdict ─────────────────────────────────────────────── */

typedef enum {
    /* Boot profile absent or NULL.  Caller must hand in a real
     * boot profile.  No save lookup, no chapter computation. */
    THERON_MARKER_VERDICT_SKIP_NO_PROFILE     = 0,

    /* Boot profile present but assets_verified == 0 and no
     * progression supplied.  Marker still produces a synthetic
     * "Chapter 1: Hall of Records" label so the launcher has
     * something to display. */
    THERON_MARKER_VERDICT_SKIP_NO_ASSET       = 1,

    /* Boot profile + progression supplied.  No save enumeration
     * (caller did not request save lookup).  Marker reflects the
     * current dungeon and quest-item bitmask. */
    THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY = 2,

    /* Boot profile + progression + at least one valid save
     * enumerated.  Marker includes the freshest between-dungeon
     * save slot's chapter label + timestamp. */
    THERON_MARKER_VERDICT_OK_WITH_SAVE        = 3,

    /* All 7 quest items collected.  Marker surfaces the quest-
     * complete state without claiming full playability. */
    THERON_MARKER_VERDICT_OK_QUEST_COMPLETE   = 4,

    THERON_MARKER_VERDICT_COUNT
} Theron_ChapterMarkerVerdict;

/* ── Marker payload ─────────────────────────────────────────────── */

/* The startup marker holds the projected label, quest summary, and
 * a brief one-line description of the freshest save slot (if any).
 *
 * All string fields are bounded to THERON_CHAPTER_MARKER_LABEL_MAX
 * and zero-terminated so callers can paste them into launcher UI,
 * diagnostic logs, or phase-gate manifests without re-sizing. */
#define THERON_CHAPTER_MARKER_LABEL_MAX 96

typedef struct {
    /* ── Verdict ────────────────────────────────────────── */
    Theron_ChapterMarkerVerdict verdict;

    /* ── Boot context (copied for diagnostics) ──────────── */
    int      boot_profile_present;       /* 1 if profile != NULL */
    int      boot_assets_verified;        /* copy of profile->assets_verified */
    char     boot_version_id[16];         /* e.g. "pce-jp"        */
    char     boot_platform_label[32];     /* e.g. "PC Engine JP"   */

    /* ── Chapter label ─────────────────────────────────── */
    char     chapter_label[THERON_CHAPTER_MARKER_LABEL_MAX];
    /* e.g. "Chapter 1: Hall of Records"
     *      "Chapter 7: Tower of Epilogue (final)"
     *      "Quest Complete (7/7 items)"
     *      "Chapter ? (synthetic — no progression)"
     */

    /* ── Quest-item summary ─────────────────────────────── */
    uint8_t  quest_items_collected;       /* 7-bit bitmask */
    uint8_t  quest_item_total;            /* 7              */
    char     quest_summary[THERON_CHAPTER_MARKER_LABEL_MAX];
    /* e.g. "1/7 items collected — Sacred Amplifier"
     *      "3/7 items collected — next: Stone Sigil"
     *      "7/7 items collected — quest complete"
     */

    /* ── Next-dungeon hint (only meaningful mid-progression) */
    char     next_dungeon_hint[THERON_CHAPTER_MARKER_LABEL_MAX];
    /* e.g. "Next: Crypt of Shadows"
     *      "Next: (quest complete)"
     *      "Next: (unknown — no progression)"
     */

    /* ── Freshest save slot (if save_root was provided and a
     *    valid slot was enumerated).  valid == 0 means no save
     *    was found or save lookup was not requested. ─────── */
    Theron_SaveSlot freshest_save;
    int             freshest_save_present;  /* 1 if freshest_save.valid == 1 */
    char            freshest_save_line[THERON_CHAPTER_MARKER_LABEL_MAX];
    /* e.g. "Save slot 2 — After Crypt of Shadows (label=...)"
     *      "No save slots present"
     *      "Save lookup skipped (no save_root supplied)"
     */
} Theron_ChapterMarker;

/* ── Marker API ─────────────────────────────────────────────────── */

/* Initialise marker to a known "synthetic, no profile" state so the
 * struct is always safe to inspect after return. */
void theron_v1_chapter_marker_init(Theron_ChapterMarker *marker);

/* Project a chapter marker from a boot profile + optional dungeon
 * progression.  Does NOT enumerate saves — the caller can pass a
 * Theron_SaveSlot directly via theron_v1_chapter_marker_attach_save
 * or request a save lookup via theron_v1_chapter_marker_compute_save.
 *
 * profile     - input, may be NULL (then marker is SKIP_NO_PROFILE)
 * progression - input, may be NULL (then marker uses synthetic
 *               "Chapter 1: Hall of Records" defaults from the boot
 *               profile)
 * save_slot   - input, may be NULL (then freshest_save is empty)
 *
 * All other marker fields are computed from these three inputs plus
 * static metadata in theron_v1_dungeon_progression.c.
 *
 * Returns 0 on success, -1 on NULL marker. */
int theron_v1_chapter_marker_compute(const Theron_V1_BootProfile *profile,
                                     const Theron_DungeonProgression *progression,
                                     const Theron_SaveSlot *save_slot,
                                     Theron_ChapterMarker *marker);

/* Enumerate saves/theron/ under save_root and pick the freshest
 * valid slot, then call theron_v1_chapter_marker_compute().
 *
 * save_root may be NULL or empty, in which case the function skips
 * save enumeration and produces a SKIP_NO_PROFILE / SKIP_NO_ASSET
 * marker (depending on profile + progression).
 *
 * Returns 0 on success, -1 on NULL marker.  Save-enumeration errors
 * (no saves present, all corrupt) are reflected in the marker
 * verdict, not in the return code. */
int theron_v1_chapter_marker_compute_save(const Theron_V1_BootProfile *profile,
                                          const Theron_DungeonProgression *progression,
                                          const char *save_root,
                                          Theron_ChapterMarker *marker);

/* Render a one-line human-readable summary of the marker into buf.
 * Always NUL-terminates when buf_size > 0.  Returns bytes written
 * (excluding the NUL byte). */
size_t theron_v1_chapter_marker_format(const Theron_ChapterMarker *marker,
                                        char *buf, size_t buf_size);

/* Render a multi-line readiness report suitable for launcher popups
 * and gate manifest dumps.  Bounded by THERON_CHAPTER_MARKER_REPORT_MAX. */
#define THERON_CHAPTER_MARKER_REPORT_MAX 1024

size_t theron_v1_chapter_marker_report(const Theron_ChapterMarker *marker,
                                        char *buf, size_t buf_size);

/* Verdict -> short human label (e.g. "OK+SAVE", "SKIP_NO_ASSET"). */
const char *theron_v1_chapter_marker_verdict_name(
    Theron_ChapterMarkerVerdict verdict);

/* Source evidence citation string — used in assert comments and
 * probe headers so the gate is traceable to THQUEST.ASM. */
const char *theron_v1_chapter_marker_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_CHAPTER_MARKER_H */
