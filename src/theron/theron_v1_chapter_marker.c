/*
 * theron_v1_chapter_marker.c — Theron's Quest V1 chapter/progression
 * startup marker implementation.
 *
 * Pass scope: theron_startup_chapter_progression_marker_gate.
 *   - Bounded to one startup marker (chapter label + quest summary
 *     + freshest save slot summary).  NOT a full progression
 *     decoder, NOT a Track 02 parser, NOT a save-import path.
 *   - Skip-safe by design: no asset required, no save_root required.
 *     Real-asset path is opt-in via theron_v1_chapter_marker_compute_save.
 *   - Source-locked to THQUEST.ASM T000 (startup entry), T080
 *     (between-dungeon save/load), T520 (party placement), T560
 *     (dungeon header parsing), T800 (champion persistence).
 *
 * The marker is what an M12 launcher / boot-profile consumer reads
 * to surface "what chapter are we on?" without doing any I/O beyond
 * what the boot profile already produced.  It is intentionally cheap
 * and deterministic so CI can exercise it without game data.
 */

#include "theron_v1_chapter_marker.h"
#include "theron_v1_world.h"
#if !defined(FIRESTAFF_THERON_PRODUCTION)
#include "theron_v1_track02_dungeon_text.h"
#endif

#include <stdio.h>
#include <string.h>
#if defined(FIRESTAFF_THERON_PRODUCTION)
#include "firestaff_cp932.h"
#endif

/* Quest item display names come from the authenticated US Track 02 retrieval
 * table (UD 0x27715B-0x277272). Keep one source-owned order for both the
 * progression model and this launcher marker; a duplicated host table had
 * previously swapped the middle five treasures. */

/* ── Local helpers ──────────────────────────────────────────────── */

static int bit_count(uint8_t v) {
    int c = 0;
    while (v) { c += (int)(v & 1u); v = (uint8_t)(v >> 1); }
    return c;
}

static int next_unset_bit(uint8_t mask, uint8_t total) {
    /* Returns the 1-based bit index of the next uncollected quest
     * item, or 0 if every bit is set.  total caps the search so
     * stray high bits do not produce a phantom "next item". */
    for (uint8_t i = 0; i < total && i < 8; ++i) {
        if ((mask & (uint8_t)(1u << i)) == 0) {
            return (int)(i + 1);
        }
    }
    return 0;
}

static void copy_bounded(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    size_t n = strlen(src);
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static const char *quest_item_source_name(
    const Theron_V1_World *world,
    unsigned int index,
    char *buffer,
    size_t buffer_size) {
#if defined(FIRESTAFF_THERON_PRODUCTION)
    const uint8_t *bytes = NULL;
    size_t size = 0u;
    size_t i;
    const Theron_Track02ItemNameSource *source;
    unsigned int item_index;
    if (!world || !buffer || buffer_size == 0u ||
        index >= THERON_DUNGEON_COUNT)
        return NULL;
    source = &world->track02_item_names[index];
    if (!source->valid || !source->object_item_index_relation_proven ||
        source->dungeon_id != index + 1u)
        return NULL;
    item_index = index == 0u ? 41u : index == 1u ? 63u :
                 index == 2u ? 45u : index == 3u ? 43u :
                 index == 4u ? 44u : index == 5u ? 43u : 7u;
    if (item_index >= source->count ||
        source->raw_name_sizes[item_index] == 0u)
        return NULL;
    bytes = source->raw_names[item_index];
    size = source->raw_name_sizes[item_index];
    if (source->variant == 2) {
        if (size >= buffer_size) return NULL;
        for (i = 0u; i < size; ++i) {
            if (bytes[i] < 0x20u || bytes[i] > 0x7eu) return NULL;
        }
        memcpy(buffer, bytes, size);
        buffer[size] = '\0';
        return buffer;
    }
    if (source->variant != 1) return NULL;
    for (i = 0u; i < size;) {
        uint8_t lead = bytes[i++];
        if (lead >= 0x20u && lead <= 0x7eu) continue;
        if (lead >= 0xa1u && lead <= 0xdfu) continue;
        if (!((lead >= 0x81u && lead <= 0x9fu) ||
              (lead >= 0xe0u && lead <= 0xfcu)) || i >= size)
            return NULL;
        {
            uint8_t trail = bytes[i++];
            if (trail < 0x40u || trail > 0xfcu || trail == 0x7fu)
                return NULL;
        }
    }
    return firestaff_cp932_to_utf8((const char *)bytes, size,
                                   buffer, buffer_size) >= 0
        ? buffer : NULL;
#else
    (void)world;
    (void)buffer;
    (void)buffer_size;
    return theron_v1_track02_us_treasure_name(index);
#endif
}

/* ── Public API: init / verdict label / source evidence ─────────── */

void theron_v1_chapter_marker_init(Theron_ChapterMarker *marker) {
    if (!marker) return;
    memset(marker, 0, sizeof(*marker));
    marker->verdict = THERON_MARKER_VERDICT_SKIP_NO_PROFILE;
    marker->quest_item_total = (uint8_t)THERON_QUEST_ITEM_COUNT;
    copy_bounded(marker->chapter_label,
                  sizeof(marker->chapter_label),
                  "(unset)");
    copy_bounded(marker->quest_summary,
                  sizeof(marker->quest_summary),
                  "0/7 items collected");
    copy_bounded(marker->next_dungeon_hint,
                  sizeof(marker->next_dungeon_hint),
                  "Next: (unknown — no progression)");
    copy_bounded(marker->freshest_save_line,
                  sizeof(marker->freshest_save_line),
                  "Save lookup not requested");
}

const char *theron_v1_chapter_marker_verdict_name(
        Theron_ChapterMarkerVerdict verdict) {
    switch (verdict) {
    case THERON_MARKER_VERDICT_SKIP_NO_PROFILE:     return "SKIP_NO_PROFILE";
    case THERON_MARKER_VERDICT_SKIP_NO_ASSET:       return "SKIP_NO_ASSET";
    case THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY: return "OK_PROGRESSION_ONLY";
    case THERON_MARKER_VERDICT_OK_WITH_SAVE:        return "OK_WITH_SAVE";
    case THERON_MARKER_VERDICT_OK_QUEST_COMPLETE:   return "OK_QUEST_COMPLETE";
    default:                                        return "(invalid)";
    }
}

const char *theron_v1_chapter_marker_source_evidence(void) {
    return
        "Theron V1 Chapter/Progression Startup Marker — source-lock\n"
        "THQUEST.ASM T000  — title/startup entry (chapter label projection)\n"
        "THQUEST.ASM T080  — between-dungeon save/load (freshest slot)\n"
        "THQUEST.ASM T520  — party placement / start position\n"
        "THQUEST.ASM T560  — dungeon header parsing (dungeon_seed, level_count)\n"
        "THQUEST.ASM T800  — champion persistence (per-dungeon reset)\n"
        "Marker scope: ONE chapter label + quest summary + freshest save.\n"
        "Does NOT parse Track 02 BIN, does NOT decode full save body.\n"
        "Phase 0 provenance: docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md\n"
        "  JP MD5: b7afb338ad31be1025b53f9aff12d73a\n"
        "  US MD5: f23601102138f87c33025877767ebf76";
}

/* ── Public API: compute ────────────────────────────────────────── */

static int theron_v1_chapter_marker_compute_internal(
    const Theron_V1_BootProfile *profile,
    const Theron_DungeonProgression *progression,
    const Theron_SaveSlot *save_slot,
    const Theron_V1_World *world,
    Theron_ChapterMarker *marker) {
    if (!marker) return -1;
    theron_v1_chapter_marker_init(marker);

    /* ── Verdict gate ──────────────────────────────────── */
    if (!profile) {
        marker->verdict = THERON_MARKER_VERDICT_SKIP_NO_PROFILE;
        copy_bounded(marker->chapter_label, sizeof(marker->chapter_label),
                      "Chapter ? (no boot profile)");
        copy_bounded(marker->quest_summary, sizeof(marker->quest_summary),
                      "0/7 items collected (no progression)");
        return 0;
    }

    marker->boot_profile_present  = 1;
    marker->boot_assets_verified  = profile->assets_verified ? 1 : 0;
    copy_bounded(marker->boot_version_id,
                  sizeof(marker->boot_version_id),
                  profile->version_id);
    copy_bounded(marker->boot_platform_label,
                  sizeof(marker->boot_platform_label),
                  profile->platform_label);

    /* ── Progression projection ────────────────────────── */
    uint8_t items = 0;
    Theron_DungeonID current = THERON_DUNGEON_1_AKUTUBA;
    int has_progression = (progression != NULL);

    if (has_progression) {
        items = progression->quest_items_collected;
        if (progression->current_dungeon >= 1 &&
            progression->current_dungeon <= THERON_DUNGEON_COUNT) {
            current = progression->current_dungeon;
        }
    }

    int collected_count = bit_count(items);
    int all_collected   = (items == THERON_QUEST_ALL_ITEMS);

    /* ── Chapter label ─────────────────────────────────── */
#if defined(THERON_STARTUP_RECEIPT_FIXTURE_PROFILE_ONLY)
    if (!has_progression && marker->boot_assets_verified == 0) {
#else
    if (!has_progression) {
#endif
        /* A verified media identity is not progression evidence. Without a
         * decoded progression/save record, never fabricate Chapter 1 or an
         * empty quest counter for the user. */
        marker->verdict = THERON_MARKER_VERDICT_SKIP_NO_ASSET;
        copy_bounded(marker->chapter_label, sizeof(marker->chapter_label),
                      "Chapter unavailable (no verified progression)");
        copy_bounded(marker->quest_summary, sizeof(marker->quest_summary),
                      "Quest progress unavailable");
        copy_bounded(marker->next_dungeon_hint,
                      sizeof(marker->next_dungeon_hint),
                      "Next dungeon unavailable");
    } else if (all_collected) {
        marker->verdict = THERON_MARKER_VERDICT_OK_QUEST_COMPLETE;
        copy_bounded(marker->chapter_label, sizeof(marker->chapter_label),
                      "Quest Complete (7/7 items)");
        copy_bounded(marker->quest_summary, sizeof(marker->quest_summary),
                      "7/7 items collected — quest complete");
        copy_bounded(marker->next_dungeon_hint,
                      sizeof(marker->next_dungeon_hint),
                      "Next: (quest complete)");
    } else {
        marker->verdict = THERON_MARKER_VERDICT_OK_PROGRESSION_ONLY;
        const Theron_DungeonMeta *meta = theron_v1_dungeon_meta(current);
        const char *current_name =
            meta ? meta->name : "(unknown dungeon)";

        snprintf(marker->chapter_label, sizeof(marker->chapter_label),
                  "Chapter %d: %s",
                  (int)current, current_name);
        marker->chapter_label[sizeof(marker->chapter_label) - 1] = '\0';

        /* Quest summary: "<collected>/<total> items — last: <name>"
         * or "<collected>/<total> items — next: <name>" depending
         * on whether the current chapter's quest item is in hand. */
        int cur_bit = (int)current - 1;
        int have_current_item =
            (cur_bit >= 0 && cur_bit < THERON_DUNGEON_COUNT) &&
            ((items & (uint8_t)(1u << cur_bit)) != 0);

        if (have_current_item) {
            char source_name[THERON_CHAPTER_MARKER_LABEL_MAX];
            const char *name =
                (cur_bit >= 0 && cur_bit < THERON_DUNGEON_COUNT)
                ? quest_item_source_name(
                    world, (unsigned int)cur_bit, source_name,
                    sizeof(source_name)) : NULL;
            if (name) {
                snprintf(marker->quest_summary,
                         sizeof(marker->quest_summary),
                         "%d/7 items collected — last: %s",
                         collected_count, name);
            } else {
                snprintf(marker->quest_summary,
                         sizeof(marker->quest_summary),
                         "%d/7 items collected — source name unavailable",
                         collected_count);
            }
        } else {
            int next_bit = next_unset_bit(items, (uint8_t)THERON_QUEST_ITEM_COUNT);
            char source_name[THERON_CHAPTER_MARKER_LABEL_MAX];
            const char *next_name = "(unknown item)";
            if (next_bit >= 1 && next_bit <= THERON_DUNGEON_COUNT) {
                next_name = quest_item_source_name(
                    world, (unsigned int)(next_bit - 1), source_name,
                    sizeof(source_name));
            }
            if (next_name) {
                snprintf(marker->quest_summary,
                         sizeof(marker->quest_summary),
                         "%d/7 items collected — next: %s",
                         collected_count, next_name);
            } else {
                snprintf(marker->quest_summary,
                         sizeof(marker->quest_summary),
                         "%d/7 items collected — source name unavailable",
                         collected_count);
            }
        }
        marker->quest_summary[sizeof(marker->quest_summary) - 1] = '\0';

        /* Next-dungeon hint */
        Theron_DungeonID next_dungeon = theron_v1_dungeon_next(current);
        if (next_dungeon != THERON_DUNGEON_INVALID) {
            const Theron_DungeonMeta *next_meta =
                theron_v1_dungeon_meta(next_dungeon);
            snprintf(marker->next_dungeon_hint,
                      sizeof(marker->next_dungeon_hint),
                      "Next: %s",
                      next_meta ? next_meta->name : "(unknown)");
        } else {
            copy_bounded(marker->next_dungeon_hint,
                          sizeof(marker->next_dungeon_hint),
                          "Next: (end of sequence)");
        }
        marker->next_dungeon_hint[sizeof(marker->next_dungeon_hint) - 1] = '\0';
    }

    marker->quest_items_collected = items;

    /* ── Save-slot projection (optional in-memory input) ─ */
    if (save_slot && save_slot->valid) {
        marker->freshest_save_present = 1;
        marker->freshest_save = *save_slot;
        marker->verdict =
            (marker->verdict == THERON_MARKER_VERDICT_OK_QUEST_COMPLETE)
            ? THERON_MARKER_VERDICT_OK_QUEST_COMPLETE
            : THERON_MARKER_VERDICT_OK_WITH_SAVE;

        const char *save_dungeon_name = "(unknown)";
        if (save_slot->current_dungeon >= 1 &&
            save_slot->current_dungeon <= THERON_DUNGEON_COUNT) {
            save_dungeon_name =
                theron_v1_dungeon_name((Theron_DungeonID)save_slot->current_dungeon);
        }

        /* Label may be empty for auto-saves.  Fall back to dungeon
         * name so the launcher always has something to show. */
        const char *slot_label =
            (save_slot->label[0] != '\0') ? save_slot->label : save_dungeon_name;

        snprintf(marker->freshest_save_line,
                  sizeof(marker->freshest_save_line),
                  "Save slot %d — %s (%s)",
                  save_slot->slot_index,
                  slot_label,
                  save_dungeon_name);
        marker->freshest_save_line[sizeof(marker->freshest_save_line) - 1] = '\0';
    } else {
        marker->freshest_save_present = 0;
        copy_bounded(marker->freshest_save_line,
                      sizeof(marker->freshest_save_line),
                      "No save slot provided");
    }

    return 0;
}

int theron_v1_chapter_marker_compute(const Theron_V1_BootProfile *profile,
                                      const Theron_DungeonProgression *progression,
                                      const Theron_SaveSlot *save_slot,
                                      Theron_ChapterMarker *marker) {
    return theron_v1_chapter_marker_compute_internal(
        profile, progression, save_slot, NULL, marker);
}

int theron_v1_chapter_marker_compute_world(
    const Theron_V1_BootProfile *profile,
    const Theron_V1_World *world,
    const Theron_SaveSlot *save_slot,
    Theron_ChapterMarker *marker) {
    return theron_v1_chapter_marker_compute_internal(
        profile, world ? &world->progression : NULL, save_slot, world, marker);
}

int theron_v1_chapter_marker_compute_save(const Theron_V1_BootProfile *profile,
                                           const Theron_DungeonProgression *progression,
                                           const char *save_root,
                                           Theron_ChapterMarker *marker) {
    if (!marker) return -1;

    /* First pass: compute from profile + progression only.
     * This keeps the verdict sane even when save enumeration
     * fails or is not requested. */
    if (theron_v1_chapter_marker_compute(profile, progression, NULL, marker) != 0) {
        return -1;
    }

#if defined(FIRESTAFF_THERON_PRODUCTION)
    (void)save_root;
    copy_bounded(marker->freshest_save_line,
                 sizeof(marker->freshest_save_line),
                 "Original Backup RAM Continue is not decoded");
    return 0;
#else

    /* If no save_root, leave freshest_save empty.  The init-step
     * already filled freshest_save_line with the "not requested"
     * sentinel, and the verdict is OK_PROGRESSION_ONLY at most. */
    if (!save_root || save_root[0] == '\0') {
        copy_bounded(marker->freshest_save_line,
                      sizeof(marker->freshest_save_line),
                      "Save lookup skipped (no save_root supplied)");
        return 0;
    }

    /* Enumerate save slots under save_root and pick the freshest
     * valid one (by timestamp).  If none valid, we keep the
     * "no save slots present" message without flipping verdict
     * to SKIP_NO_PROFILE — the caller might still want to see
     * the chapter/quest projection on a fresh install. */
    Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
    memset(slots, 0, sizeof(slots));
    int found = theron_v1_save_enum_slots(save_root, slots,
                                           THERON_SAVE_SLOT_COUNT);

    int best_idx = -1;
    uint32_t best_ts = 0;
    for (int i = 0; i < found; ++i) {
        if (!slots[i].valid) continue;
        if (best_idx < 0 || slots[i].timestamp > best_ts) {
            best_idx = i;
            best_ts  = slots[i].timestamp;
        }
    }

    if (best_idx < 0) {
        marker->freshest_save_present = 0;
        copy_bounded(marker->freshest_save_line,
                      sizeof(marker->freshest_save_line),
                      "No save slots present");
        /* Keep current verdict (OK_PROGRESSION_ONLY or
         * OK_QUEST_COMPLETE) — absence of saves is not a failure. */
        return 0;
    }

    marker->freshest_save_present = 1;
    marker->freshest_save = slots[best_idx];

    const Theron_SaveSlot *slot = &slots[best_idx];
    const char *save_dungeon_name = "(unknown)";
    if (slot->current_dungeon >= 1 &&
        slot->current_dungeon <= THERON_DUNGEON_COUNT) {
        save_dungeon_name =
            theron_v1_dungeon_name((Theron_DungeonID)slot->current_dungeon);
    }
    const char *slot_label =
        (slot->label[0] != '\0') ? slot->label : save_dungeon_name;

    snprintf(marker->freshest_save_line,
              sizeof(marker->freshest_save_line),
              "Save slot %d — %s (%s)",
              slot->slot_index, slot_label, save_dungeon_name);
    marker->freshest_save_line[sizeof(marker->freshest_save_line) - 1] = '\0';

    /* Promote verdict if we found a usable save. */
    if (marker->verdict != THERON_MARKER_VERDICT_OK_QUEST_COMPLETE) {
        marker->verdict = THERON_MARKER_VERDICT_OK_WITH_SAVE;
    }

    return 0;
#endif
}

/* ── Public API: format ─────────────────────────────────────────── */

size_t theron_v1_chapter_marker_format(const Theron_ChapterMarker *marker,
                                        char *buf, size_t buf_size) {
    if (!buf || buf_size == 0) return 0;
    if (!marker) {
        buf[0] = '\0';
        return 0;
    }

    int n = snprintf(buf, buf_size,
                      "[%s] %s | %s",
                      theron_v1_chapter_marker_verdict_name(marker->verdict),
                      marker->chapter_label,
                      marker->quest_summary);
    if (n < 0) {
        buf[0] = '\0';
        return 0;
    }
    if ((size_t)n >= buf_size) {
        return buf_size > 0 ? buf_size - 1 : 0;
    }
    return (size_t)n;
}

size_t theron_v1_chapter_marker_report(const Theron_ChapterMarker *marker,
                                        char *buf, size_t buf_size) {
    if (!buf || buf_size == 0) return 0;
    if (!marker) {
        buf[0] = '\0';
        return 0;
    }

    int n = snprintf(buf, buf_size,
        "=== Theron V1 Chapter Marker ===\n"
        "Verdict:        %s\n"
        "Boot profile:    %s%s\n"
        "Platform:        %s (%s)\n"
        "Chapter:        %s\n"
        "Quest summary:  %s\n"
        "Next hint:      %s\n"
        "Freshest save:  %s\n",
        theron_v1_chapter_marker_verdict_name(marker->verdict),
        marker->boot_profile_present ? "present" : "absent",
        (marker->boot_profile_present && marker->boot_assets_verified)
            ? " (assets_verified)" : "",
        marker->boot_platform_label,
        marker->boot_version_id,
        marker->chapter_label,
        marker->quest_summary,
        marker->next_dungeon_hint,
        marker->freshest_save_line);

    if (n < 0) {
        buf[0] = '\0';
        return 0;
    }
    if ((size_t)n >= buf_size) {
        return buf_size > 0 ? buf_size - 1 : 0;
    }
    return (size_t)n;
}
