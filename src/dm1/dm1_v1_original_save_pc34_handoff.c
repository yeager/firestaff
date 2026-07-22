#include "dm1_v1_original_save_pc34_handoff.h"

#include "dm1_v1_c15_layout_pc34_compat.h"

#include "dm1_v1_resurrection_pc34_compat.h"

#include "memory_door_action_pc34_compat.h"
#include "memory_savegame_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_original_pc34_file_bytes(
    const char *path,
    uint8_t **out_bytes,
    size_t *out_size);

int dm1_v1_original_save_pc34_f0415_read_bytes(
    const uint8_t* source, size_t source_size, size_t* io_cursor,
    uint8_t* destination, size_t byte_count)
{
    size_t cursor;
    if (!io_cursor) return 0;
    if (byte_count == 0u) return 1;
    if (!source || !destination) return 0;
    cursor = *io_cursor;
    if (cursor > source_size || byte_count > source_size - cursor) return 0;
    memcpy(destination, source + cursor, byte_count);
    *io_cursor = cursor + byte_count;
    return 1;
}

int dm1_v1_original_save_pc34_f0416_write_bytes(
    uint8_t* destination, size_t destination_size, size_t* io_cursor,
    const uint8_t* source, size_t byte_count)
{
    size_t cursor;
    if (!io_cursor) return 0;
    if (byte_count == 0u) return 1;
    if (!destination || !source) return 0;
    cursor = *io_cursor;
    if (cursor > destination_size || byte_count > destination_size - cursor) return 0;
    memcpy(destination + cursor, source, byte_count);
    *io_cursor = cursor + byte_count;
    return 1;
}

static uint32_t original_pc34_timeline_runtime_fingerprint(
    const struct TimelineQueue_Compat *timeline)
{
    uint32_t hash = 2166136261u;
    int i;
    int field;

    if (!timeline || timeline->count < 0 ||
        timeline->count > TIMELINE_QUEUE_CAPACITY) {
        return 0u;
    }
    for (field = 0; field < 4; ++field) {
        hash ^= (uint8_t)(((uint32_t)timeline->count >> (field * 8)) & 0xffu);
        hash *= 16777619u;
    }
    for (i = 0; i < timeline->count; ++i) {
        const struct TimelineEvent_Compat *event = &timeline->events[i];
        const uint32_t values[] = {
            event->fireAtTick, (uint32_t)event->kind,
            (uint32_t)event->mapIndex, (uint32_t)event->mapX,
            (uint32_t)event->mapY, (uint32_t)event->cell,
            (uint32_t)event->aux0, (uint32_t)event->aux1,
            (uint32_t)event->aux2, (uint32_t)event->aux3,
            (uint32_t)event->aux4
        };
        size_t value_index;
        for (value_index = 0u; value_index < sizeof(values) / sizeof(values[0]);
             ++value_index) {
            for (field = 0; field < 4; ++field) {
                hash ^= (uint8_t)((values[value_index] >> (field * 8)) & 0xffu);
                hash *= 16777619u;
            }
        }
    }
    return hash;
}

int dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
    const uint8_t* source,
    size_t source_size,
    size_t* io_cursor,
    uint8_t* destination,
    size_t byte_count,
    uint16_t* io_running_checksum)
{
    uint16_t checksum;
    size_t i;

    if (!source || !io_cursor || !destination || !io_running_checksum) {
        return 0;
    }
    /* READWRIT.C F0421 first completes F0415, then sums the bytes read and
     * adds that local 16-bit sum to the caller's running checksum. */
    if (!dm1_v1_original_save_pc34_f0415_read_bytes(
            source, source_size, io_cursor, destination, byte_count)) {
        return 0;
    }
    checksum = 0u;
    for (i = 0u; i < byte_count; ++i) {
        checksum = (uint16_t)(checksum + destination[i]);
    }
    *io_running_checksum = (uint16_t)(*io_running_checksum + checksum);
    return 1;
}

int dm1_v1_original_save_pc34_f0422_write_bytes_with_checksum(
    uint8_t* destination,
    size_t destination_size,
    size_t* io_cursor,
    const uint8_t* source,
    size_t byte_count,
    uint16_t* io_running_checksum)
{
    uint16_t checksum;
    size_t i;

    if (!destination || !io_cursor || !source || !io_running_checksum) {
        return 0;
    }
    /* READWRIT.C F0422 first completes F0416, then sums the exact bytes it
     * wrote into the caller-owned tail checksum. Validate the full range
     * before either the destination, cursor, or checksum can change. */
    if (!dm1_v1_original_save_pc34_f0416_write_bytes(
            destination, destination_size, io_cursor, source, byte_count)) {
        return 0;
    }
    checksum = 0u;
    for (i = 0u; i < byte_count; ++i) {
        checksum = (uint16_t)(checksum + source[i]);
    }
    *io_running_checksum = (uint16_t)(*io_running_checksum + checksum);
    return 1;
}

static int dm1_original_save_backup_path(const char *path,
                                         char out_path[DM1_ORIGINAL_SAVE_PATH_MAX])
{
    size_t length;
    if (!path || !path[0] || !out_path) return 0;
    length = strlen(path);
    if (length + 4u >= DM1_ORIGINAL_SAVE_PATH_MAX) return 0;
    memcpy(out_path, path, length);
    memcpy(out_path + length, ".bak", 5u);
    return 1;
}

static int dm1_original_save_file_opens_for_read(const char *path)
{
    FILE *file;
    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

/* A corpus round-trip is evidence for external original bytes only. The
 * Firestaff exporter deliberately stamps AdditionalData with LSV01RDM, which
 * ReDMCSB ignores but must not certify its own output as an original-save
 * corpus. CSBWin's 512-byte GAMEBLOCK1 also cannot pass this gate by header
 * shape alone: the classifier has already required F0435/F7057's five
 * length-prefixed, keyed, checksummed parts before this provenance check. */
static int dm1_original_save_corpus_external_pc34_file(
    const char *path,
    int *out_firestaff_manifest)
{
    uint8_t *bytes = NULL;
    size_t size = 0u;
    int manifest_result;
    int result;

    if (out_firestaff_manifest) {
        *out_firestaff_manifest = 0;
    }
    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return 0;
    }
    if (size > (size_t)((int)0x7fffffff)) {
        free(bytes);
        return 0;
    }
    manifest_result = F0799_SAVEGAME_PC34PeekManifest_Compat(
        bytes, (int)size, NULL, NULL, NULL);
    free(bytes);
    if (manifest_result == SAVEGAME_PC34_MANIFEST_ERR_NOT_PRESENT) {
        return 1;
    }
    if (manifest_result == SAVEGAME_PC34_MANIFEST_OK &&
        out_firestaff_manifest) {
        *out_firestaff_manifest = 1;
    }
    return 0;
}

static uint32_t dm1_original_save_corpus_fingerprint_mix(uint32_t hash,
                                                          uint32_t value)
{
    unsigned int shift;
    for (shift = 0u; shift < 32u; shift += 8u) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t dm1_original_save_corpus_hash_step(uint32_t hash,
                                                   uint32_t value)
{
    return dm1_original_save_corpus_fingerprint_mix(hash, value);
}

static uint32_t dm1_original_save_visible_runtime_epoch_fingerprint(
    uint32_t source_hash,
    uint32_t game_tick,
    uint32_t queue_game_tick,
    int queue_event_count,
    int queue_first_unused_index,
    uint32_t party_state_fingerprint,
    uint32_t timeline_fingerprint)
{
    uint32_t fingerprint = 2166136261u;

    fingerprint = dm1_original_save_corpus_hash_step(fingerprint,
                                                      source_hash);
    fingerprint = dm1_original_save_corpus_hash_step(fingerprint,
                                                      game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(fingerprint,
                                                      queue_game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)queue_event_count);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)queue_first_unused_index);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, party_state_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, timeline_fingerprint);
    return fingerprint ? fingerprint : 1u;
}

static uint32_t dm1_original_save_hash_bytes(const uint8_t *bytes,
                                             size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes && byte_count != 0u) {
        return 0u;
    }
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int dm1_original_save_special_events_adoptable(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const struct GameWorld_Compat *world);
static int validate_original_pc34_tail_runtime_receipt(
    DM1OriginalSavePC34HandoffReport *report,
    const struct GameWorld_Compat *world);
static int validate_original_pc34_c13_party_runtime_receipt(
    DM1OriginalSavePC34HandoffReport *report,
    const struct GameWorld_Compat *world);
static int original_pc34_runtime_queue_matches_world(
    const struct GameWorld_Compat *world,
    const struct DM1_EventQueue_V1 *queue);

static int dm1_original_save_c13_runtime_event_matches(
    const struct DM1_Event_V1 *source,
    const struct TimelineEvent_Compat *runtime)
{
    if (!source || !runtime ||
        source->type != DM1_EVENT_VI_ALTAR_REBIRTH) {
        return 0;
    }
    return runtime->kind == TIMELINE_EVENT_VI_ALTAR_REBIRTH &&
           runtime->fireAtTick == (source->map_time & 0x00ffffffu) &&
           runtime->mapIndex == (int)((source->map_time >> 24) & 0xffu) &&
           runtime->mapX == source->b_mapX &&
           runtime->mapY == source->b_mapY &&
           runtime->cell == source->c_cell &&
           runtime->aux0 == DM1_EVENT_VI_ALTAR_REBIRTH &&
           runtime->aux1 == source->c_effect &&
           runtime->aux4 == source->priority;
}

/* A tail-backed C13 is admissible only if its F0255-owned state reaches the
 * runtime timeline unchanged. This operates on externally supplied source
 * records after F0435 has materialized the saved dungeon; it never creates a
 * C13 candidate or treats a host timeline event as corpus evidence. */
static int dm1_original_save_c13_runtime_receipt(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const struct GameWorld_Compat *world,
    int *out_admitted_count,
    uint32_t *out_fingerprint)
{
    int consumed[TIMELINE_QUEUE_CAPACITY] = {0};
    uint32_t fingerprint = 2166136261u;
    int source_count = 0;
    int i;

    if (out_admitted_count) *out_admitted_count = 0;
    if (out_fingerprint) *out_fingerprint = 0u;
    if (!source_report || !world ||
        source_report->decoded_event_count < 0 ||
        source_report->decoded_event_count > DM1_EVENT_MAX_COUNT ||
        world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }
    for (i = 0; i < source_report->decoded_event_count; ++i) {
        const struct DM1_Event_V1 *source = &source_report->events[i];
        int runtime_index;

        if (source->type != DM1_EVENT_VI_ALTAR_REBIRTH) continue;
        ++source_count;
        for (runtime_index = 0; runtime_index < world->timeline.count;
             ++runtime_index) {
            if (!consumed[runtime_index] &&
                dm1_original_save_c13_runtime_event_matches(
                    source, &world->timeline.events[runtime_index])) {
                consumed[runtime_index] = 1;
                fingerprint = dm1_original_save_corpus_hash_step(
                    fingerprint, source->map_time);
                fingerprint = dm1_original_save_corpus_hash_step(
                    fingerprint, source->priority);
                fingerprint = dm1_original_save_corpus_hash_step(
                    fingerprint, source->b_mapX);
                fingerprint = dm1_original_save_corpus_hash_step(
                    fingerprint, source->b_mapY);
                fingerprint = dm1_original_save_corpus_hash_step(
                    fingerprint, source->c_cell);
                fingerprint = dm1_original_save_corpus_hash_step(
                    fingerprint, source->c_effect);
                break;
            }
        }
        if (runtime_index == world->timeline.count) return 0;
    }
    if (out_admitted_count) *out_admitted_count = source_count;
    if (out_fingerprint && source_count > 0) {
        *out_fingerprint = fingerprint ? fingerprint : 1u;
    }
    return 1;
}

/* ReDMCSB LOADSAVE.C F0435 authenticates each length-prefixed part before it
 * reaches later runtime materializers. A corpus failure needs that boundary
 * for diagnosis, but this probe owns only local staging objects. */
static void dm1_original_save_corpus_receipt_source_handoff(
    const uint8_t *bytes,
    size_t size,
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    struct SaveGame_Compat staged_state;
    struct PartyState_Compat staged_party;
    struct TimelineQueue_Compat staged_timeline;
    DM1OriginalSavePC34HandoffReport handoff_report;

    if (!bytes || !receipt || size == 0u || size > UINT32_MAX) {
        return;
    }
    memset(&staged_state, 0, sizeof(staged_state));
    memset(&staged_party, 0, sizeof(staged_party));
    memset(&staged_timeline, 0, sizeof(staged_timeline));
    memset(&handoff_report, 0, sizeof(handoff_report));
    staged_state.party = &staged_party;
    staged_state.timeline = &staged_timeline;
    receipt->source_handoff_result =
        dm1_v1_original_save_pc34_handoff_bytes(
            bytes, size, &staged_state, &handoff_report);
    receipt->source_importer_result = handoff_report.importer_result;
    receipt->source_part_checksum_ok_count =
        handoff_report.part_checksum_ok_count;
}

/* ReDMCSB: LOADSAVE.C F0435 lines 2721-2826 and 2932-2934 restore PARTY,
 * EVENTS, TIMELINE, portraits, then the optional saved dungeon before the
 * game returns live. Stage the immutable corpus snapshot through that exact
 * candidate-world order, with no start-world backing. A tail-less save may
 * still authenticate its parts, but cannot claim runtime readiness by
 * borrowing host dungeon data. */
static void dm1_original_save_corpus_receipt_runtime_stage(
    const uint8_t *bytes,
    size_t size,
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    struct GameWorld_Compat staged_world;
    struct GameWorld_Compat adopted_world;
    struct GameWorld_Compat visible_world;
    struct GameWorld_Compat next_visible_world;
    struct DM1_EventQueue_V1 staged_queue;
    struct DM1_EventQueue_V1 adopted_queue;
    struct DM1_EventQueue_V1 visible_queue;
    struct DM1_EventQueue_V1 next_visible_queue;
    DM1OriginalSavePC34HandoffReport staged_report;
    DM1OriginalSavePC34HandoffReport visible_report;
    int i;

    if (!bytes || !receipt) {
        return;
    }
    memset(&staged_world, 0, sizeof(staged_world));
    memset(&adopted_world, 0, sizeof(adopted_world));
    memset(&visible_world, 0, sizeof(visible_world));
    memset(&next_visible_world, 0, sizeof(next_visible_world));
    memset(&staged_queue, 0, sizeof(staged_queue));
    memset(&adopted_queue, 0, sizeof(adopted_queue));
    memset(&visible_queue, 0, sizeof(visible_queue));
    memset(&next_visible_queue, 0, sizeof(next_visible_queue));
    memset(&staged_report, 0, sizeof(staged_report));
    memset(&visible_report, 0, sizeof(visible_report));
    receipt->source_runtime_stage_attempted = 1;
    receipt->source_runtime_stage_input_byte_count = (uint32_t)size;
    receipt->source_runtime_stage_input_hash =
        dm1_original_save_hash_bytes(bytes, size);
    receipt->source_runtime_stage_result =
        dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
            bytes, size, &staged_world, &staged_queue, &staged_report);
    if (receipt->source_runtime_stage_result ==
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        receipt->source_runtime_stage_owns_dungeon = staged_world.ownsDungeon &&
            staged_world.dungeon != NULL && staged_world.things != NULL;
        receipt->source_runtime_stage_event_count = staged_world.timeline.count;
        receipt->source_runtime_stage_timeline_count = staged_queue.eventCount;
        receipt->source_runtime_stage_party_champion_count =
            staged_world.party.championCount;
        receipt->source_runtime_stage_active_champion_index =
            staged_world.party.activeChampionIndex;
        receipt->source_runtime_stage_timeline_fingerprint =
            original_pc34_timeline_runtime_fingerprint(&staged_world.timeline);
        for (i = 0; i < staged_world.timeline.count; ++i) {
            if (staged_world.timeline.events[i].kind ==
                TIMELINE_EVENT_VI_ALTAR_REBIRTH) {
                ++receipt->source_runtime_stage_c13_event_count;
            }
        }
        receipt->source_runtime_stage_c13_admission_ok =
            dm1_original_save_c13_runtime_receipt(
                &staged_report, &staged_world,
                &receipt->source_runtime_stage_c13_admitted_count,
                &receipt->source_runtime_stage_c13_fingerprint);
        if (receipt->source_runtime_stage_c13_admission_ok &&
            validate_original_pc34_tail_runtime_receipt(&staged_report,
                                                         &staged_world)) {
            receipt->source_runtime_stage_c13_party_receipt_valid =
                validate_original_pc34_c13_party_runtime_receipt(
                    &staged_report, &staged_world);
            receipt->source_runtime_stage_party_metadata_fingerprint =
                staged_report.c13_party_runtime_metadata_fingerprint;
            receipt->source_runtime_stage_party_state_fingerprint =
                staged_report.c13_party_runtime_state_fingerprint;
        }
        receipt->source_runtime_stage_committed =
            receipt->source_runtime_stage_owns_dungeon &&
            receipt->source_runtime_stage_c13_admission_ok &&
            receipt->source_runtime_stage_c13_party_receipt_valid;
        if (receipt->source_runtime_stage_committed) {
            receipt->source_runtime_adopt_attempted = 1;
            receipt->source_runtime_adopt_result =
                dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
                    &adopted_world, &adopted_queue,
                    &staged_world, &staged_queue);
            if (receipt->source_runtime_adopt_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                receipt->source_runtime_adopt_input_byte_count =
                    receipt->source_runtime_stage_input_byte_count;
                receipt->source_runtime_adopt_input_hash =
                    receipt->source_runtime_stage_input_hash;
                receipt->source_runtime_adopt_owns_dungeon =
                    adopted_world.ownsDungeon && adopted_world.dungeon != NULL &&
                    adopted_world.things != NULL;
                receipt->source_runtime_adopt_event_count =
                    adopted_world.timeline.count;
                receipt->source_runtime_adopt_party_champion_count =
                    adopted_world.party.championCount;
                receipt->source_runtime_adopt_active_champion_index =
                    adopted_world.party.activeChampionIndex;
                receipt->source_runtime_adopt_timeline_fingerprint =
                    original_pc34_timeline_runtime_fingerprint(
                        &adopted_world.timeline);
                receipt->source_runtime_adopt_c13_admission_ok =
                    dm1_original_save_c13_runtime_receipt(
                        &staged_report, &adopted_world,
                        &receipt->source_runtime_adopt_c13_admitted_count,
                        &receipt->source_runtime_adopt_c13_fingerprint);
                if (receipt->source_runtime_adopt_c13_admission_ok) {
                    receipt->source_runtime_adopt_c13_party_receipt_valid =
                        validate_original_pc34_c13_party_runtime_receipt(
                            &staged_report, &adopted_world);
                    receipt->source_runtime_adopt_party_metadata_fingerprint =
                        staged_report.c13_party_runtime_metadata_fingerprint;
                    receipt->source_runtime_adopt_party_state_fingerprint =
                        staged_report.c13_party_runtime_state_fingerprint;
                }
                receipt->source_runtime_adopted =
                    receipt->source_runtime_adopt_owns_dungeon &&
                    receipt->source_runtime_adopt_c13_admission_ok &&
                    receipt->source_runtime_adopt_c13_party_receipt_valid;
                receipt->source_runtime_adopt_timeline_count =
                    adopted_queue.eventCount;
                if (receipt->source_runtime_adopted) {
                    receipt->source_runtime_adopt_queue_committed = 1;
                    receipt->source_runtime_adopt_queue_event_count =
                        adopted_queue.eventCount;
                    receipt->source_runtime_adopt_queue_first_unused_index =
                        adopted_queue.firstUnusedIndex;
                    receipt->source_runtime_adopt_queue_matches_world =
                        original_pc34_runtime_queue_matches_world(
                            &adopted_world, &adopted_queue);
                    /* This is the production handoff boundary that presents
                     * restored state to a live runtime, not a copy of the
                     * candidate world used for corpus diagnostics. */
                    receipt->source_runtime_visible_handoff_attempted = 1;
                    receipt->source_runtime_visible_handoff_result =
                        dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
                            bytes, size, &visible_world, &visible_queue,
                            &visible_report);
                    if (receipt->source_runtime_visible_handoff_result ==
                        DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                        receipt->source_runtime_visible_party_champion_count =
                            visible_world.party.championCount;
                        receipt->source_runtime_visible_active_champion_index =
                            visible_world.party.activeChampionIndex;
                        receipt->source_runtime_visible_timeline_event_count =
                            visible_world.timeline.count;
                        receipt->source_runtime_visible_queue_event_count =
                            visible_queue.eventCount;
                        receipt->source_runtime_visible_queue_first_unused_index =
                            visible_queue.firstUnusedIndex;
                        receipt->source_runtime_visible_queue_matches_world =
                            original_pc34_runtime_queue_matches_world(
                                &visible_world, &visible_queue);
                        receipt->source_runtime_visible_party_state_fingerprint =
                            visible_report.c13_party_runtime_state_fingerprint;
                        receipt->source_runtime_visible_timeline_fingerprint =
                            original_pc34_timeline_runtime_fingerprint(
                                &visible_world.timeline);
                        receipt->source_runtime_visible_game_tick =
                            visible_world.gameTick;
                        receipt->source_runtime_visible_queue_game_tick =
                            visible_queue.gameTick;
                        receipt->source_runtime_visible_provenance_fingerprint =
                            dm1_original_save_visible_runtime_epoch_fingerprint(
                                receipt->source_runtime_stage_input_hash,
                                visible_world.gameTick, visible_queue.gameTick,
                                visible_queue.eventCount,
                                visible_queue.firstUnusedIndex,
                                receipt->source_runtime_visible_party_state_fingerprint,
                                receipt->source_runtime_visible_timeline_fingerprint);
                        receipt->source_runtime_visible_handoff_accepted =
                            visible_report.c13_party_runtime_receipt_valid &&
                            visible_report.dungeon_tail_runtime_receipt_valid &&
                            receipt->source_runtime_visible_queue_matches_world;
                        /* The diagnostic runtime is local. Advance a shallow
                         * epoch copy only: F0238's clock, WORLD clock and
                         * timeline clock must stay coupled without mutating
                         * the handed-off state or inventing a save. */
                        if (visible_world.gameTick != UINT32_MAX &&
                            visible_queue.gameTick != UINT32_MAX) {
                            next_visible_world = visible_world;
                            next_visible_queue = visible_queue;
                            ++next_visible_world.gameTick;
                            next_visible_world.timeline.nowTick =
                                next_visible_world.gameTick;
                            dm1v1_event_advance_tick(&next_visible_queue);
                            receipt->source_runtime_visible_next_game_tick =
                                next_visible_world.gameTick;
                            receipt->source_runtime_visible_next_queue_game_tick =
                                next_visible_queue.gameTick;
                            receipt->source_runtime_visible_next_queue_matches_world =
                                original_pc34_runtime_queue_matches_world(
                                    &next_visible_world, &next_visible_queue);
                            receipt->source_runtime_visible_next_party_state_fingerprint =
                                visible_report.c13_party_runtime_state_fingerprint;
                            receipt->source_runtime_visible_next_timeline_fingerprint =
                                original_pc34_timeline_runtime_fingerprint(
                                    &next_visible_world.timeline);
                            receipt->source_runtime_visible_next_provenance_fingerprint =
                                dm1_original_save_visible_runtime_epoch_fingerprint(
                                    receipt->source_runtime_stage_input_hash,
                                    next_visible_world.gameTick,
                                    next_visible_queue.gameTick,
                                    next_visible_queue.eventCount,
                                    next_visible_queue.firstUnusedIndex,
                                    receipt->source_runtime_visible_party_state_fingerprint,
                                    receipt->source_runtime_visible_timeline_fingerprint);
                        }
                    }
                }
            }
        }
    }
    F0883_WORLD_Free_Compat(&staged_world);
    F0883_WORLD_Free_Compat(&adopted_world);
    F0883_WORLD_Free_Compat(&visible_world);
}

/* C13/C24/C25 are subtype receipts. Core corpus proof instead requires the
 * complete C3 EVENT and C4 TIMELINE parts plus the optional raw F0433 tail. */
static int dm1_original_save_corpus_receipt_has_core_roundtrip_evidence(
    const DM1OriginalSavePC34CorpusReceipt *receipt)
{
    if (!receipt || !receipt->header_part_shape_receipt_available ||
        !receipt->header_identity_preservation_ok ||
        !receipt->part_byte_count_preservation_ok ||
        !receipt->c3_event_layout_receipt_available ||
        !receipt->c3_event_byte_preservation_ok ||
        !receipt->c4_timeline_layout_receipt_available ||
        !receipt->c4_timeline_byte_preservation_ok ||
        !receipt->dungeon_tail_byte_receipt_available ||
        !receipt->dungeon_tail_byte_preservation_ok ||
        receipt->source_f7057_envelope_end_offset <=
            SAVEGAME_PC34_DM_SAVE_HEADER_SIZE ||
        receipt->source_f7057_envelope_end_offset +
                receipt->source_f7057_trailing_byte_count !=
            receipt->source_byte_count ||
        receipt->source_c3_event_byte_count !=
            receipt->source_c3_event_record_count *
                10u ||
        receipt->exported_c3_event_byte_count !=
            receipt->exported_c3_event_record_count *
                10u ||
        receipt->source_c4_timeline_byte_count !=
            receipt->source_c4_timeline_index_count * sizeof(uint16_t) ||
        receipt->exported_c4_timeline_byte_count !=
            receipt->exported_c4_timeline_index_count * sizeof(uint16_t)) {
        return 0;
    }
    if (receipt->source_c13_event_count > 0 &&
        (!receipt->c13_roundtrip_emission_receipt_available ||
         !receipt->c13_roundtrip_emission_valid ||
         receipt->c13_roundtrip_emission_fingerprint == 0u ||
         !receipt->c13_roundtrip_input_admission_available ||
         !receipt->c13_roundtrip_input_admission_valid ||
         !receipt->source_discovery_admission_receipt_available ||
         !receipt->source_discovery_admission_valid ||
         receipt->source_discovery_admission_fingerprint == 0u ||
         !receipt->c13_raw_capture_receipt_available ||
         !receipt->c13_raw_capture_byte_preservation_ok ||
         !receipt->c13_corpus_capture_admission_receipt_available ||
         !receipt->c13_corpus_capture_admission_valid ||
         receipt->c13_corpus_capture_admission_fingerprint == 0u ||
         !receipt->c13_runtime_handoff_provenance_receipt_available ||
         !receipt->c13_runtime_handoff_provenance_valid ||
         receipt->c13_runtime_handoff_provenance_fingerprint == 0u ||
         !receipt->c13_active_runtime_state_receipt_available ||
         !receipt->c13_active_runtime_state_valid ||
         receipt->c13_active_runtime_party_state_fingerprint == 0u ||
         receipt->c13_active_runtime_timeline_fingerprint == 0u ||
         !receipt->c13_active_runtime_consumption_receipt_available ||
         !receipt->c13_active_runtime_consumption_valid ||
         receipt->c13_active_runtime_consumption_fingerprint == 0u ||
         !receipt->c13_runtime_frame.receipt_available ||
         !receipt->c13_runtime_frame.valid ||
         receipt->c13_runtime_frame.revoked ||
         receipt->c13_runtime_frame.fingerprint == 0u ||
         !receipt->c13_runtime_frame_lifecycle.receipt_available ||
         !receipt->c13_runtime_frame_lifecycle.active_visible_handoff ||
         !receipt->c13_runtime_frame_lifecycle.valid ||
         receipt->c13_runtime_frame_lifecycle.clear_output ||
         receipt->c13_runtime_frame_lifecycle.revoke_output ||
         receipt->c13_runtime_frame_lifecycle.fingerprint == 0u ||
         !receipt->c13_runtime_frame_m11_bridge.receipt_available ||
         !receipt->c13_runtime_frame_m11_bridge.active_visible_handoff ||
         !receipt->c13_runtime_frame_m11_bridge.deliver_frame ||
         receipt->c13_runtime_frame_m11_bridge.clear_output ||
         receipt->c13_runtime_frame_m11_bridge.revoke_output ||
         receipt->c13_runtime_frame_m11_bridge.fingerprint == 0u ||
         !receipt->c13_m11_runtime_capture.receipt_available ||
         !receipt->c13_m11_runtime_capture.active_visible_handoff ||
         !receipt->c13_m11_runtime_capture.capture_admitted ||
         receipt->c13_m11_runtime_capture.clear_output ||
         receipt->c13_m11_runtime_capture.revoke_output ||
         receipt->c13_m11_runtime_capture.fingerprint == 0u ||
         receipt->exported_c13_event_count !=
             receipt->source_c13_event_count)) {
        return 0;
    }
    return 1;
}

static int dm1_original_save_c13_corpus_admit_roundtrip_input(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_roundtrip_input_admission_available = 1;
    receipt->c13_roundtrip_input_admission_valid =
        receipt->classified_loader_envelope && receipt->external_original &&
        receipt->source_byte_count != 0u && receipt->source_hash != 0u &&
        receipt->c13_roundtrip_input_byte_count ==
            receipt->source_byte_count &&
        receipt->c13_roundtrip_input_hash == receipt->source_hash &&
        receipt->c13_roundtrip_input_c3_byte_count != 0u &&
        receipt->c13_roundtrip_input_c3_fingerprint != 0u &&
        receipt->c13_roundtrip_input_c3_byte_offset >=
            SAVEGAME_PC34_DM_SAVE_HEADER_SIZE &&
        receipt->c13_roundtrip_input_c3_byte_offset <=
            receipt->source_byte_count &&
        receipt->c13_roundtrip_input_c3_byte_count <=
            receipt->source_byte_count -
                receipt->c13_roundtrip_input_c3_byte_offset &&
        receipt->c13_roundtrip_input_c3_byte_count ==
            receipt->source_c3_event_byte_count;
    return receipt->c13_roundtrip_input_admission_valid;
}

/* Bind the classifier-qualified first read to raw C13 C3 rows. Nothing here
 * reconstructs bytes or admits an exporter-created save as corpus evidence. */
static int dm1_original_save_c13_corpus_admit_raw_capture(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_corpus_capture_admission_receipt_available = 1;
    receipt->c13_corpus_capture_admission_valid =
        receipt->external_original &&
        receipt->source_discovery_admission_valid &&
        receipt->c13_roundtrip_input_admission_valid &&
        receipt->c13_raw_capture_receipt_available &&
        receipt->c13_raw_capture_byte_preservation_ok &&
        receipt->source_c13_raw_capture_count ==
            receipt->source_c13_event_count &&
        receipt->exported_c13_raw_capture_count ==
            receipt->exported_c13_event_count &&
        receipt->source_c13_raw_capture_byte_count ==
            (uint32_t)receipt->source_c13_raw_capture_count *
                GAMEWORLD_PC34_ORIGINAL_C3_EVENT_BYTE_COUNT &&
        receipt->exported_c13_raw_capture_byte_count ==
            (uint32_t)receipt->exported_c13_raw_capture_count *
                GAMEWORLD_PC34_ORIGINAL_C3_EVENT_BYTE_COUNT &&
        receipt->source_c13_raw_capture_fingerprint != 0u &&
        receipt->source_c13_raw_capture_fingerprint ==
            receipt->exported_c13_raw_capture_fingerprint;
    if (!receipt->c13_corpus_capture_admission_valid) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_discovery_admission_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_roundtrip_input_hash);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_c13_raw_capture_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_c13_raw_capture_byte_count);
    receipt->c13_corpus_capture_admission_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

static int dm1_original_save_c13_corpus_bind_runtime_handoff(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_runtime_handoff_provenance_receipt_available = 1;
    receipt->c13_runtime_handoff_provenance_valid =
        receipt->c13_roundtrip_input_admission_available &&
        receipt->c13_roundtrip_input_admission_valid &&
        receipt->source_runtime_stage_attempted &&
        receipt->source_runtime_stage_result ==
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
        receipt->source_runtime_stage_committed &&
        receipt->source_runtime_stage_owns_dungeon &&
        receipt->source_runtime_stage_c13_admission_ok &&
        receipt->source_runtime_stage_c13_party_receipt_valid &&
        receipt->source_runtime_stage_c13_admitted_count ==
            receipt->source_c13_event_count &&
        receipt->source_runtime_stage_input_byte_count ==
            receipt->c13_roundtrip_input_byte_count &&
        receipt->source_runtime_stage_input_hash ==
            receipt->c13_roundtrip_input_hash &&
        receipt->source_runtime_adopt_attempted &&
        receipt->source_runtime_adopt_result ==
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
        receipt->source_runtime_adopted &&
        receipt->source_runtime_adopt_owns_dungeon &&
        receipt->source_runtime_adopt_c13_admission_ok &&
        receipt->source_runtime_adopt_c13_party_receipt_valid &&
        receipt->source_runtime_adopt_c13_admitted_count ==
            receipt->source_c13_event_count &&
        receipt->source_runtime_adopt_input_byte_count ==
            receipt->c13_roundtrip_input_byte_count &&
        receipt->source_runtime_adopt_input_hash ==
            receipt->c13_roundtrip_input_hash &&
        receipt->source_runtime_adopt_c13_fingerprint ==
            receipt->source_runtime_stage_c13_fingerprint;
    if (!receipt->c13_runtime_handoff_provenance_valid) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_roundtrip_input_hash);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_stage_c13_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_adopt_c13_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_stage_party_state_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_adopt_party_state_fingerprint);
    receipt->c13_runtime_handoff_provenance_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

/* The staging fields above remain diagnostic evidence. This is the sole
 * publication point for a C13-bearing active runtime: no party or timeline
 * state is exposed as accepted until the external input identity and the
 * candidate-to-active ownership route have both passed. */
static int dm1_original_save_c13_publish_active_runtime_state(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_active_runtime_state_receipt_available = 1;
    receipt->c13_active_runtime_state_valid =
        receipt->c13_roundtrip_input_admission_valid &&
        receipt->c13_runtime_handoff_provenance_valid &&
        receipt->source_runtime_stage_party_champion_count >= 0 &&
        receipt->source_runtime_stage_party_champion_count <=
            CHAMPION_MAX_PARTY &&
        receipt->source_runtime_adopt_party_champion_count ==
            receipt->source_runtime_stage_party_champion_count &&
        receipt->source_runtime_adopt_active_champion_index ==
            receipt->source_runtime_stage_active_champion_index &&
        receipt->source_runtime_adopt_event_count ==
            receipt->source_runtime_stage_event_count &&
        receipt->source_runtime_adopt_timeline_count ==
            receipt->source_runtime_stage_timeline_count &&
        receipt->source_runtime_adopt_timeline_fingerprint != 0u &&
        receipt->source_runtime_adopt_timeline_fingerprint ==
            receipt->source_runtime_stage_timeline_fingerprint &&
        receipt->source_runtime_adopt_party_state_fingerprint != 0u &&
        receipt->source_runtime_adopt_party_state_fingerprint ==
            receipt->source_runtime_stage_party_state_fingerprint;
    if (!receipt->c13_active_runtime_state_valid) {
        return 0;
    }
    receipt->c13_active_runtime_party_champion_count =
        receipt->source_runtime_adopt_party_champion_count;
    receipt->c13_active_runtime_active_champion_index =
        receipt->source_runtime_adopt_active_champion_index;
    receipt->c13_active_runtime_timeline_event_count =
        receipt->source_runtime_adopt_event_count;
    receipt->c13_active_runtime_party_state_fingerprint =
        receipt->source_runtime_adopt_party_state_fingerprint;
    receipt->c13_active_runtime_timeline_fingerprint =
        receipt->source_runtime_adopt_timeline_fingerprint;
    return 1;
}

/* A completed adoption alone is not visible-runtime consumption. Before
 * exposing C13's active party/timeline state, bind it to the final F0238
 * queue and compare every source-owned fingerprint again. This prevents a
 * later candidate or a detached queue from consuming stale acceptance. */
static int dm1_original_save_c13_consume_active_runtime_state(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_active_runtime_consumption_receipt_available = 1;
    receipt->c13_active_runtime_consumption_valid =
        receipt->c13_active_runtime_state_valid &&
        receipt->c13_runtime_handoff_provenance_valid &&
        receipt->source_runtime_adopted &&
        receipt->source_runtime_adopt_queue_committed &&
        receipt->source_runtime_adopt_queue_matches_world &&
        receipt->c13_active_runtime_party_champion_count ==
            receipt->source_runtime_adopt_party_champion_count &&
        receipt->c13_active_runtime_active_champion_index ==
            receipt->source_runtime_adopt_active_champion_index &&
        receipt->c13_active_runtime_timeline_event_count ==
            receipt->source_runtime_adopt_event_count &&
        receipt->c13_active_runtime_timeline_event_count ==
            receipt->source_runtime_adopt_queue_event_count &&
        receipt->source_runtime_adopt_queue_first_unused_index >=
            receipt->source_runtime_adopt_queue_event_count &&
        receipt->c13_active_runtime_party_state_fingerprint ==
            receipt->source_runtime_adopt_party_state_fingerprint &&
        receipt->c13_active_runtime_timeline_fingerprint ==
            receipt->source_runtime_adopt_timeline_fingerprint;
    if (!receipt->c13_active_runtime_consumption_valid) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_runtime_handoff_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_active_runtime_party_state_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_active_runtime_timeline_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)receipt->c13_active_runtime_timeline_event_count);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)receipt->source_runtime_adopt_queue_first_unused_index);
    receipt->c13_active_runtime_consumed_event_count =
        receipt->c13_active_runtime_timeline_event_count;
    receipt->c13_active_runtime_consumption_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

/* The corpus candidate is never itself the visible runtime. Re-enter the
 * public F0435 resume boundary and require its party/timeline plus F0238
 * queue to be the already-consumed state before issuing this final receipt. */
static int dm1_original_save_c13_handoff_consumption_to_visible_runtime(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_visible_runtime_handoff_receipt_available = 1;
    receipt->c13_visible_runtime_handoff_valid =
        receipt->c13_active_runtime_consumption_valid &&
        receipt->c13_runtime_handoff_provenance_valid &&
        receipt->source_runtime_visible_handoff_attempted &&
        receipt->source_runtime_visible_handoff_result ==
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
        receipt->source_runtime_visible_handoff_accepted &&
        receipt->source_runtime_visible_queue_matches_world &&
        receipt->source_runtime_visible_party_champion_count ==
            receipt->c13_active_runtime_party_champion_count &&
        receipt->source_runtime_visible_active_champion_index ==
            receipt->c13_active_runtime_active_champion_index &&
        receipt->source_runtime_visible_timeline_event_count ==
            receipt->c13_active_runtime_timeline_event_count &&
        receipt->source_runtime_visible_queue_event_count ==
            receipt->c13_active_runtime_consumed_event_count &&
        receipt->source_runtime_visible_queue_first_unused_index >=
            receipt->source_runtime_visible_queue_event_count &&
        receipt->source_runtime_visible_party_state_fingerprint ==
            receipt->c13_active_runtime_party_state_fingerprint &&
        receipt->source_runtime_visible_timeline_fingerprint ==
            receipt->c13_active_runtime_timeline_fingerprint;
    if (!receipt->c13_visible_runtime_handoff_valid) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_active_runtime_consumption_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_party_state_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_timeline_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)receipt->source_runtime_visible_queue_first_unused_index);
    receipt->c13_visible_runtime_handoff_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

/* The visible handoff must not certify a C13 state after its provenance or
 * F0238 epoch has gone stale. This validates the next logical game tick on a
 * local copy because corpus inspection never owns a live game loop. */
static int dm1_original_save_c13_visible_runtime_lifecycle(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;
    const uint32_t expected_current =
        dm1_original_save_visible_runtime_epoch_fingerprint(
            receipt ? receipt->c13_roundtrip_input_hash : 0u,
            receipt ? receipt->source_runtime_visible_game_tick : 0u,
            receipt ? receipt->source_runtime_visible_queue_game_tick : 0u,
            receipt ? receipt->source_runtime_visible_queue_event_count : 0,
            receipt ? receipt->source_runtime_visible_queue_first_unused_index : 0,
            receipt ? receipt->source_runtime_visible_party_state_fingerprint : 0u,
            receipt ? receipt->source_runtime_visible_timeline_fingerprint : 0u);
    const uint32_t expected_next =
        dm1_original_save_visible_runtime_epoch_fingerprint(
            receipt ? receipt->c13_roundtrip_input_hash : 0u,
            receipt ? receipt->source_runtime_visible_next_game_tick : 0u,
            receipt ? receipt->source_runtime_visible_next_queue_game_tick : 0u,
            receipt ? receipt->source_runtime_visible_queue_event_count : 0,
            receipt ? receipt->source_runtime_visible_queue_first_unused_index : 0,
            receipt ? receipt->source_runtime_visible_party_state_fingerprint : 0u,
            receipt ? receipt->source_runtime_visible_timeline_fingerprint : 0u);

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_visible_runtime_lifecycle_receipt_available = 1;
    receipt->c13_visible_runtime_lifecycle_valid =
        receipt->c13_visible_runtime_handoff_valid &&
        receipt->c13_runtime_handoff_provenance_valid &&
        receipt->c13_runtime_handoff_provenance_fingerprint != 0u &&
        receipt->source_runtime_visible_queue_matches_world &&
        receipt->source_runtime_visible_next_queue_matches_world &&
        receipt->source_runtime_visible_game_tick != UINT32_MAX &&
        receipt->source_runtime_visible_queue_game_tick != UINT32_MAX &&
        receipt->source_runtime_visible_game_tick ==
            receipt->source_runtime_visible_queue_game_tick &&
        receipt->source_runtime_visible_next_game_tick ==
            receipt->source_runtime_visible_game_tick + 1u &&
        receipt->source_runtime_visible_next_queue_game_tick ==
            receipt->source_runtime_visible_queue_game_tick + 1u &&
        receipt->source_runtime_visible_provenance_fingerprint ==
            expected_current &&
        receipt->source_runtime_visible_next_provenance_fingerprint ==
            expected_next;
    if (!receipt->c13_visible_runtime_lifecycle_valid) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_handoff_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_runtime_handoff_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_next_provenance_fingerprint);
    receipt->c13_visible_runtime_lifecycle_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

/* M11's renderer/session owner is intentionally outside the original-save
 * module. Expose only a bounded admission receipt: it names the exact source
 * epoch M11 may consume, and cannot be issued until the F0238/world pair has
 * survived the next logical tick. */
static int dm1_original_save_c13_visible_runtime_m11_handoff(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_visible_runtime_m11_handoff_receipt_available = 1;
    receipt->c13_visible_runtime_m11_handoff_valid =
        receipt->c13_visible_runtime_lifecycle_valid &&
        receipt->c13_visible_runtime_lifecycle_fingerprint != 0u &&
        receipt->c13_runtime_handoff_provenance_valid &&
        receipt->c13_runtime_handoff_provenance_fingerprint != 0u &&
        receipt->source_runtime_visible_handoff_accepted &&
        receipt->source_runtime_visible_queue_matches_world &&
        receipt->source_runtime_visible_next_queue_matches_world &&
        receipt->source_runtime_visible_game_tick != UINT32_MAX &&
        receipt->source_runtime_visible_queue_game_tick != UINT32_MAX &&
        receipt->source_runtime_visible_game_tick ==
            receipt->source_runtime_visible_queue_game_tick &&
        receipt->source_runtime_visible_next_game_tick ==
            receipt->source_runtime_visible_game_tick + 1u &&
        receipt->source_runtime_visible_next_queue_game_tick ==
            receipt->source_runtime_visible_queue_game_tick + 1u &&
        receipt->source_runtime_visible_party_champion_count ==
            receipt->c13_active_runtime_party_champion_count &&
        receipt->source_runtime_visible_active_champion_index ==
            receipt->c13_active_runtime_active_champion_index &&
        receipt->source_runtime_visible_timeline_event_count ==
            receipt->c13_active_runtime_timeline_event_count &&
        receipt->source_runtime_visible_provenance_fingerprint != 0u &&
        receipt->source_runtime_visible_next_provenance_fingerprint != 0u;
    if (!receipt->c13_visible_runtime_m11_handoff_valid) {
        return 0;
    }
    receipt->c13_visible_runtime_m11_handoff_game_tick =
        receipt->source_runtime_visible_game_tick;
    receipt->c13_visible_runtime_m11_handoff_queue_game_tick =
        receipt->source_runtime_visible_queue_game_tick;
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_lifecycle_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_runtime_handoff_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_next_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_m11_handoff_game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_m11_handoff_queue_game_tick);
    receipt->c13_visible_runtime_m11_handoff_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

/* Keep M11 admission revocable. The source-owned probe observes the next
 * F0238 epoch from the same immutable PC34 bytes; any drift revokes rather
 * than extending the lifetime of an old world/provenance receipt. */
static int dm1_original_save_c13_visible_runtime_m11_lifecycle(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    receipt->c13_visible_runtime_m11_lifecycle_receipt_available = 1;
    receipt->c13_visible_runtime_m11_admission_revoked = 0;
    receipt->c13_visible_runtime_m11_revoke_reason =
        DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_NONE;
    if (!receipt->c13_visible_runtime_m11_handoff_valid ||
        !receipt->c13_runtime_handoff_provenance_valid ||
        receipt->c13_runtime_handoff_provenance_fingerprint == 0u ||
        receipt->source_runtime_visible_provenance_fingerprint == 0u ||
        receipt->source_runtime_visible_next_provenance_fingerprint == 0u) {
        receipt->c13_visible_runtime_m11_revoke_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_PROVENANCE;
    } else if (receipt->source_runtime_visible_game_tick == UINT32_MAX ||
               receipt->source_runtime_visible_next_game_tick !=
                   receipt->source_runtime_visible_game_tick + 1u) {
        receipt->c13_visible_runtime_m11_revoke_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_WORLD_TICK;
    } else if (receipt->source_runtime_visible_queue_game_tick == UINT32_MAX ||
               receipt->source_runtime_visible_next_queue_game_tick !=
                   receipt->source_runtime_visible_queue_game_tick + 1u ||
               receipt->source_runtime_visible_next_queue_game_tick !=
                   receipt->source_runtime_visible_next_game_tick) {
        receipt->c13_visible_runtime_m11_revoke_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_QUEUE_TICK;
    } else if (!receipt->source_runtime_visible_queue_matches_world ||
               !receipt->source_runtime_visible_next_queue_matches_world) {
        receipt->c13_visible_runtime_m11_revoke_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_QUEUE_WORLD;
    } else if (receipt->source_runtime_visible_next_party_state_fingerprint !=
                   receipt->source_runtime_visible_party_state_fingerprint ||
               receipt->source_runtime_visible_next_timeline_fingerprint !=
                   receipt->source_runtime_visible_timeline_fingerprint ||
               receipt->source_runtime_visible_party_state_fingerprint !=
                   receipt->c13_active_runtime_party_state_fingerprint ||
               receipt->source_runtime_visible_timeline_fingerprint !=
                   receipt->c13_active_runtime_timeline_fingerprint) {
        receipt->c13_visible_runtime_m11_revoke_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_C13_STATE;
    }
    if (receipt->c13_visible_runtime_m11_revoke_reason !=
        DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_NONE) {
        receipt->c13_visible_runtime_m11_admission_revoked = 1;
        receipt->c13_visible_runtime_m11_lifecycle_valid = 0;
        return 0;
    }
    receipt->c13_visible_runtime_m11_lifecycle_valid = 1;
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_m11_handoff_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_runtime_handoff_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_next_provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_next_party_state_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_next_timeline_fingerprint);
    receipt->c13_visible_runtime_m11_lifecycle_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

/* Consolidate the visible F0435 runtime and M11 admission evidence into one
 * immutable current-tick receipt. The lifecycle probe remains the revocation
 * fence, but its next-tick facts are intentionally not exposed as frame data. */
static int dm1_original_save_c13_build_runtime_frame(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    DM1OriginalSavePC34C13RuntimeFrameReceipt *frame;
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    frame = &receipt->c13_runtime_frame;
    memset(frame, 0, sizeof(*frame));
    frame->receipt_available = 1;
    frame->revoked = receipt->c13_visible_runtime_m11_admission_revoked;
    frame->revoke_reason = receipt->c13_visible_runtime_m11_revoke_reason;
    frame->game_tick = receipt->source_runtime_visible_game_tick;
    frame->queue_game_tick = receipt->source_runtime_visible_queue_game_tick;
    frame->party_champion_count =
        receipt->source_runtime_visible_party_champion_count;
    frame->active_champion_index =
        receipt->source_runtime_visible_active_champion_index;
    frame->timeline_event_count =
        receipt->source_runtime_visible_timeline_event_count;
    frame->queue_event_count = receipt->source_runtime_visible_queue_event_count;
    frame->queue_first_unused_index =
        receipt->source_runtime_visible_queue_first_unused_index;
    frame->party_state_fingerprint =
        receipt->source_runtime_visible_party_state_fingerprint;
    frame->timeline_fingerprint =
        receipt->source_runtime_visible_timeline_fingerprint;
    frame->provenance_fingerprint =
        receipt->c13_runtime_handoff_provenance_fingerprint;
    frame->valid =
        receipt->c13_visible_runtime_handoff_valid &&
        receipt->c13_visible_runtime_m11_handoff_valid &&
        receipt->c13_visible_runtime_m11_lifecycle_valid &&
        !frame->revoked &&
        frame->revoke_reason == DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_NONE &&
        frame->game_tick == frame->queue_game_tick &&
        frame->party_champion_count ==
            receipt->c13_active_runtime_party_champion_count &&
        frame->active_champion_index ==
            receipt->c13_active_runtime_active_champion_index &&
        frame->timeline_event_count ==
            receipt->c13_active_runtime_timeline_event_count &&
        frame->queue_event_count ==
            receipt->c13_active_runtime_consumed_event_count &&
        frame->queue_first_unused_index >= frame->queue_event_count &&
        frame->party_state_fingerprint ==
            receipt->c13_active_runtime_party_state_fingerprint &&
        frame->timeline_fingerprint ==
            receipt->c13_active_runtime_timeline_fingerprint &&
        frame->provenance_fingerprint != 0u;
    if (!frame->valid) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_handoff_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_m11_handoff_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_visible_runtime_m11_lifecycle_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, frame->game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, frame->queue_game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, frame->party_state_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, frame->timeline_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, frame->provenance_fingerprint);
    frame->fingerprint = fingerprint ? fingerprint : 1u;
    return 1;
}

/* Frame lifetime belongs to the original-save route. A frame is retained only
 * for its active visible F0435 handoff; an absent handoff clears it, while a
 * changed source identity or stale F0238/world state revokes it. */
static int dm1_original_save_c13_runtime_frame_lifecycle(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    DM1OriginalSavePC34C13RuntimeFrameLifecycleReceipt *lifecycle;
    uint32_t fingerprint = 2166136261u;
    int stale_source;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    lifecycle = &receipt->c13_runtime_frame_lifecycle;
    memset(lifecycle, 0, sizeof(*lifecycle));
    lifecycle->receipt_available = 1;
    lifecycle->active_visible_handoff =
        receipt->source_runtime_visible_handoff_attempted &&
        receipt->source_runtime_visible_handoff_result ==
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
        receipt->source_runtime_visible_handoff_accepted;
    lifecycle->current_game_tick =
        receipt->c13_runtime_frame.game_tick;
    lifecycle->next_game_tick =
        receipt->source_runtime_visible_next_game_tick;
    stale_source =
        receipt->source_hash == 0u ||
        receipt->source_runtime_stage_input_hash != receipt->source_hash ||
        receipt->source_runtime_adopt_input_hash != receipt->source_hash ||
        receipt->c13_roundtrip_input_hash != receipt->source_hash ||
        receipt->c13_runtime_frame.provenance_fingerprint !=
            receipt->c13_runtime_handoff_provenance_fingerprint;
    if (!lifecycle->active_visible_handoff) {
        lifecycle->clear_output = 1;
        lifecycle->clear_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_NO_ACTIVE_VISIBLE_HANDOFF;
    } else if (stale_source) {
        lifecycle->clear_output = 1;
        lifecycle->revoke_output = 1;
        lifecycle->clear_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_STALE_SOURCE;
    } else if (receipt->c13_visible_runtime_m11_admission_revoked) {
        lifecycle->clear_output = 1;
        lifecycle->revoke_output = 1;
        lifecycle->clear_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_REVOKED_M11_ADMISSION;
    } else if (receipt->source_runtime_visible_next_game_tick !=
                   receipt->c13_runtime_frame.game_tick + 1u ||
               receipt->source_runtime_visible_next_queue_game_tick !=
                   receipt->c13_runtime_frame.queue_game_tick + 1u ||
               !receipt->source_runtime_visible_next_queue_matches_world) {
        lifecycle->clear_output = 1;
        lifecycle->revoke_output = 1;
        lifecycle->clear_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_NEXT_TICK;
    } else if (!receipt->c13_runtime_frame.valid ||
               receipt->c13_runtime_frame.revoked ||
               receipt->c13_runtime_frame.fingerprint == 0u) {
        lifecycle->clear_output = 1;
        lifecycle->revoke_output = 1;
        lifecycle->clear_reason =
            DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_CURRENT_FRAME;
    }
    if (lifecycle->clear_output || lifecycle->revoke_output) {
        return 0;
    }
    lifecycle->valid = 1;
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->c13_runtime_frame.fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_hash);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, lifecycle->current_game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, lifecycle->next_game_tick);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_runtime_visible_next_provenance_fingerprint);
    lifecycle->fingerprint = fingerprint ? fingerprint : 1u;
    return 1;
}

/* Convert the source-owned frame lifecycle into the only payload M11 needs:
 * a current frame identity, or a clear/revoke decision. The bridge never
 * fabricates a fallback frame and cannot revive a rejected source snapshot. */
static int dm1_original_save_c13_runtime_frame_m11_bridge(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    DM1OriginalSavePC34C13M11RuntimeFrameBridgeReceipt *bridge;
    const DM1OriginalSavePC34C13RuntimeFrameLifecycleReceipt *lifecycle;
    const DM1OriginalSavePC34C13RuntimeFrameReceipt *frame;
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    bridge = &receipt->c13_runtime_frame_m11_bridge;
    lifecycle = &receipt->c13_runtime_frame_lifecycle;
    frame = &receipt->c13_runtime_frame;
    memset(bridge, 0, sizeof(*bridge));
    bridge->receipt_available = 1;
    bridge->active_visible_handoff = lifecycle->active_visible_handoff;
    bridge->game_tick = frame->game_tick;
    bridge->frame_fingerprint = frame->fingerprint;
    bridge->provenance_fingerprint = frame->provenance_fingerprint;
    bridge->clear_output = lifecycle->clear_output;
    bridge->revoke_output = lifecycle->revoke_output;
    bridge->clear_reason = lifecycle->clear_reason;
    bridge->deliver_frame =
        bridge->active_visible_handoff &&
        lifecycle->valid &&
        !bridge->clear_output &&
        !bridge->revoke_output &&
        frame->valid &&
        !frame->revoked &&
        frame->revoke_reason == DM1_ORIGINAL_SAVE_PC34_C13_M11_REVOKE_NONE &&
        receipt->c13_visible_runtime_m11_handoff_valid &&
        receipt->c13_visible_runtime_m11_lifecycle_valid &&
        !receipt->c13_visible_runtime_m11_admission_revoked &&
        bridge->game_tick == receipt->c13_visible_runtime_m11_handoff_game_tick &&
        bridge->provenance_fingerprint ==
            receipt->c13_runtime_handoff_provenance_fingerprint &&
        bridge->frame_fingerprint != 0u;
    if (!bridge->deliver_frame) {
        if (!bridge->clear_output) {
            bridge->clear_output = 1;
            bridge->clear_reason =
                DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_CURRENT_FRAME;
        }
        if (receipt->c13_visible_runtime_m11_admission_revoked ||
            lifecycle->revoke_output || frame->revoked) {
            bridge->revoke_output = 1;
        }
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, lifecycle->fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, bridge->frame_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, bridge->provenance_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, bridge->game_tick);
    bridge->fingerprint = fingerprint ? fingerprint : 1u;
    return 1;
}

/* Admit a discovered C13 capture to the M11-facing frame boundary only when
 * the exact discovery buffer, C3 span, raw C13 bytes and current frame share
 * one provenance chain. This is proof-only: M11 receives no save bytes here. */
static int dm1_original_save_c13_discovered_capture_to_m11_runtime(
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    DM1OriginalSavePC34C13M11RuntimeCaptureReceipt *capture;
    const DM1OriginalSavePC34C13M11RuntimeFrameBridgeReceipt *bridge;
    int source_identity_valid;
    int c3_span_valid;
    int raw_capture_valid;
    uint32_t fingerprint = 2166136261u;

    if (!receipt || receipt->source_c13_event_count <= 0) {
        return 1;
    }
    capture = &receipt->c13_m11_runtime_capture;
    bridge = &receipt->c13_runtime_frame_m11_bridge;
    memset(capture, 0, sizeof(*capture));
    capture->receipt_available = 1;
    capture->active_visible_handoff = bridge->active_visible_handoff;
    capture->clear_output = bridge->clear_output;
    capture->revoke_output = bridge->revoke_output;
    capture->clear_reason = bridge->clear_reason;
    capture->source_byte_count = receipt->source_byte_count;
    capture->source_hash = receipt->source_hash;
    capture->c3_byte_offset = receipt->c13_roundtrip_input_c3_byte_offset;
    capture->c3_byte_count = receipt->c13_roundtrip_input_c3_byte_count;
    capture->c3_fingerprint = receipt->c13_roundtrip_input_c3_fingerprint;
    capture->c13_capture_fingerprint =
        receipt->source_c13_raw_capture_fingerprint;
    capture->runtime_frame_fingerprint = bridge->frame_fingerprint;
    capture->provenance_fingerprint = bridge->provenance_fingerprint;
    source_identity_valid =
        receipt->external_original &&
        receipt->source_discovery_admission_receipt_available &&
        receipt->source_discovery_admission_valid &&
        receipt->source_discovery_admission_fingerprint != 0u &&
        receipt->c13_roundtrip_input_admission_available &&
        receipt->c13_roundtrip_input_admission_valid &&
        receipt->source_byte_count != 0u &&
        receipt->source_hash != 0u &&
        receipt->c13_roundtrip_input_byte_count == receipt->source_byte_count &&
        receipt->c13_roundtrip_input_hash == receipt->source_hash;
    c3_span_valid =
        capture->c3_byte_offset >= SAVEGAME_PC34_DM_SAVE_HEADER_SIZE &&
        capture->c3_byte_offset <= capture->source_byte_count &&
        capture->c3_byte_count <=
            capture->source_byte_count - capture->c3_byte_offset &&
        capture->c3_byte_count == receipt->source_c3_event_byte_count &&
        capture->c3_fingerprint != 0u;
    raw_capture_valid =
        receipt->c13_raw_capture_receipt_available &&
        receipt->c13_raw_capture_byte_preservation_ok &&
        receipt->c13_corpus_capture_admission_receipt_available &&
        receipt->c13_corpus_capture_admission_valid &&
        receipt->source_c13_raw_capture_count ==
            receipt->source_c13_event_count &&
        receipt->exported_c13_raw_capture_count ==
            receipt->exported_c13_event_count &&
        receipt->source_c13_raw_capture_byte_count ==
            (uint32_t)receipt->source_c13_raw_capture_count *
                GAMEWORLD_PC34_ORIGINAL_C3_EVENT_BYTE_COUNT &&
        receipt->source_c13_raw_capture_fingerprint != 0u &&
        receipt->source_c13_raw_capture_fingerprint ==
            receipt->exported_c13_raw_capture_fingerprint;
    capture->capture_admitted =
        source_identity_valid && c3_span_valid && raw_capture_valid &&
        bridge->deliver_frame && !capture->clear_output &&
        !capture->revoke_output &&
        capture->active_visible_handoff &&
        capture->runtime_frame_fingerprint != 0u &&
        capture->runtime_frame_fingerprint ==
            receipt->c13_runtime_frame.fingerprint &&
        capture->provenance_fingerprint != 0u &&
        capture->provenance_fingerprint ==
            receipt->c13_runtime_frame.provenance_fingerprint &&
        capture->provenance_fingerprint ==
            receipt->c13_runtime_handoff_provenance_fingerprint;
    if (!capture->capture_admitted) {
        capture->clear_output = 1;
        if (!source_identity_valid || !c3_span_valid || !raw_capture_valid) {
            capture->revoke_output = 1;
            capture->clear_reason =
                DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_STALE_SOURCE;
        } else if (capture->clear_reason ==
                   DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_NONE) {
            capture->clear_reason =
                DM1_ORIGINAL_SAVE_PC34_C13_FRAME_CLEAR_CURRENT_FRAME;
        }
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, receipt->source_discovery_admission_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->source_byte_count);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->source_hash);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->c3_byte_offset);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->c3_byte_count);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->c3_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->c13_capture_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->runtime_frame_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, capture->provenance_fingerprint);
    capture->fingerprint = fingerprint ? fingerprint : 1u;
    return 1;
}

static void dm1_original_save_corpus_record_discovery(
    DM1OriginalSavePC34CorpusRoundtripReport *report,
    const DM1OriginalSaveClassifyResult *result,
    const char *path)
{
    DM1OriginalSavePC34CorpusDiscoveryReceipt *receipt;
    int firestaff_manifest = 0;

    if (!report || !result ||
        report->discovery_receipt_count >=
            (int)DM1_ORIGINAL_SAVE_PC34_CORPUS_RECEIPT_CAP) {
        return;
    }
    receipt = &report->discovery_receipts[report->discovery_receipt_count++];
    memset(receipt, 0, sizeof(*receipt));
    receipt->source_byte_count = (uint32_t)result->size_bytes;
    receipt->header_prefix_fingerprint = result->prefix_checksum32;
    receipt->shape = (int)result->shape;
    receipt->readiness = (int)result->readiness;
    receipt->save_format_id = result->format_id;
    receipt->save_platform = result->platform;
    receipt->save_dungeon_id = result->dungeon_id;
    receipt->save_game_id = result->game_id;
    receipt->pc34_version_platform_identity_ok =
        result->shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 &&
        result->format_id == 5u &&
        result->platform == 1u &&
        result->dungeon_id == 0u;
    receipt->pc34_importer_candidate = result->pc34_importer_candidate;
    receipt->pc34_loader_part_envelope_candidate =
        result->pc34_loader_part_envelope_candidate;
    receipt->f7057_envelope_end_offset =
        result->save_part_loader_envelope_payload_bytes
            ? (uint32_t)SAVEGAME_PC34_DM_SAVE_HEADER_SIZE +
                  result->save_part_loader_envelope_payload_bytes
            : 0u;
    receipt->f7057_trailing_byte_count =
        result->size_bytes > receipt->f7057_envelope_end_offset
            ? (uint32_t)(result->size_bytes -
                         receipt->f7057_envelope_end_offset)
            : 0u;
    receipt->external_original =
        result->pc34_loader_part_envelope_candidate &&
        dm1_original_save_corpus_external_pc34_file(
            path, &firestaff_manifest);
    receipt->roundtrip_eligible = receipt->external_original;
    receipt->result = result->pc34_loader_part_envelope_candidate
        ? DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK
        : DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    if (path) {
        snprintf(receipt->path, sizeof(receipt->path), "%s", path);
    }
    if (firestaff_manifest) {
        snprintf(receipt->reason, sizeof(receipt->reason),
                 "firestaff-export-manifest");
    } else if (!result->pc34_loader_part_envelope_candidate) {
        snprintf(receipt->reason, sizeof(receipt->reason),
                 "%s", result->reason);
    } else if (!receipt->external_original) {
        snprintf(receipt->reason, sizeof(receipt->reason),
                 "nonexternal-original-envelope");
    } else {
        snprintf(receipt->reason, sizeof(receipt->reason),
                 "external-original-pc34");
    }
}

/* The corpus scanner classifies file paths first, then this fence confirms
 * that the single byte buffer admitted to F0435 is still that discovery row.
 * This prevents a second path read from supplying substitute C13 bytes. */
static int dm1_original_save_corpus_admit_discovered_bytes(
    const uint8_t *bytes,
    size_t size,
    const DM1OriginalSaveClassifyResult *discovered,
    DM1OriginalSavePC34CorpusReceipt *receipt)
{
    DM1OriginalSaveClassifyResult current;
    uint32_t fingerprint = 2166136261u;

    if (!bytes || !discovered || !receipt || size > UINT32_MAX) {
        return 0;
    }
    memset(&current, 0, sizeof(current));
    receipt->source_discovery_admission_receipt_available = 1;
    if (!dm1_v1_original_save_classify_bytes(bytes, size, &current) ||
        current.shape != DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 ||
        !current.pc34_importer_candidate ||
        !current.pc34_loader_part_envelope_candidate ||
        current.size_bytes != discovered->size_bytes ||
        current.game_id != discovered->game_id ||
        current.format_id != discovered->format_id ||
        current.platform != discovered->platform ||
        current.dungeon_id != discovered->dungeon_id ||
        current.prefix_checksum32 != discovered->prefix_checksum32 ||
        current.save_part_loader_envelope_payload_bytes !=
            discovered->save_part_loader_envelope_payload_bytes) {
        return 0;
    }
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)current.size_bytes);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)(current.size_bytes >> 32));
    fingerprint = dm1_original_save_corpus_hash_step(fingerprint,
        current.game_id);
    fingerprint = dm1_original_save_corpus_hash_step(fingerprint,
        current.prefix_checksum32);
    fingerprint = dm1_original_save_corpus_hash_step(fingerprint,
        current.save_part_loader_envelope_payload_bytes);
    receipt->source_discovery_admission_valid = 1;
    receipt->source_discovery_admission_fingerprint =
        fingerprint ? fingerprint : 1u;
    return 1;
}

#define DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT 319u
#define DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT 128u
#define DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT \
    ((DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY) + \
     DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT)
#define DM1_PC34_PARTY_INFO_MAGICAL_LIGHT_OFFSET 0u
#define DM1_PC34_PARTY_INFO_THIEVES_EYE_COUNT_OFFSET 2u
#define DM1_PC34_PARTY_INFO_FOOTPRINTS_COUNT_OFFSET 3u
#define DM1_PC34_PARTY_INFO_SHIELD_DEFENSE_OFFSET 4u
#define DM1_PC34_PARTY_INFO_FIRE_SHIELD_DEFENSE_OFFSET 6u
#define DM1_PC34_PARTY_INFO_SPELL_SHIELD_DEFENSE_OFFSET 8u
#define DM1_PC34_PARTY_INFO_SCENT_COUNT_OFFSET 10u
#define DM1_PC34_PARTY_INFO_FREEZE_LIFE_TICKS_OFFSET 11u
#define DM1_PC34_PARTY_INFO_FIRST_SCENT_INDEX_OFFSET 84u
#define DM1_PC34_ORIGINAL_PARTY_INFO_MAGICAL_LIGHT_AMOUNT_OFFSET DM1_PC34_PARTY_INFO_MAGICAL_LIGHT_OFFSET
#define DM1_PC34_ORIGINAL_PARTY_INFO_EVENT73_COUNT_OFFSET DM1_PC34_PARTY_INFO_THIEVES_EYE_COUNT_OFFSET
#define DM1_PC34_ORIGINAL_PARTY_INFO_EVENT79_COUNT_OFFSET DM1_PC34_PARTY_INFO_FOOTPRINTS_COUNT_OFFSET
#define DM1_PC34_ORIGINAL_PARTY_INFO_SHIELD_DEFENSE_OFFSET DM1_PC34_PARTY_INFO_SHIELD_DEFENSE_OFFSET
#define DM1_PC34_ORIGINAL_PARTY_INFO_FIRE_SHIELD_DEFENSE_OFFSET DM1_PC34_PARTY_INFO_FIRE_SHIELD_DEFENSE_OFFSET
#define DM1_PC34_ORIGINAL_PARTY_INFO_SPELL_SHIELD_DEFENSE_OFFSET DM1_PC34_PARTY_INFO_SPELL_SHIELD_DEFENSE_OFFSET
#define DM1_PC34_ORIGINAL_PARTY_INFO_EVENT71_COUNT_OFFSET 86u
#define DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT 16u
#define DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT 10u
#define DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT 3u
#define DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT 4u

#define DM1_PC34_CHAMPION_NAME_OFFSET 0u
#define DM1_PC34_CHAMPION_TITLE_OFFSET 8u
#define DM1_PC34_CHAMPION_DIRECTION_OFFSET 28u
#define DM1_PC34_CHAMPION_ATTRIBUTES_OFFSET 48u
#define DM1_PC34_CHAMPION_WOUNDS_OFFSET 50u
#define DM1_PC34_CHAMPION_POISON_EVENT_COUNT_OFFSET 42u
#define DM1_PC34_CHAMPION_ACTION_INDEX_OFFSET 32u
#define DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET 52u
#define DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET 54u
#define DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET 56u
#define DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET 58u
#define DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET 60u
#define DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET 62u
#define DM1_PC34_CHAMPION_FOOD_OFFSET 66u
#define DM1_PC34_CHAMPION_WATER_OFFSET 68u
#define DM1_PC34_CHAMPION_STATISTICS_OFFSET 70u
#define DM1_PC34_CHAMPION_SKILLS_OFFSET 91u
#define DM1_PC34_CHAMPION_SLOTS_OFFSET 211u
#define DM1_PC34_CHAMPION_LOAD_OFFSET 271u

#define DM1_PC34_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET 30u
#define DM1_PC34_GLOBAL_EVENT_COUNT_OFFSET 24u
#define DM1_PC34_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET 26u
#define DM1_PC34_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET 28u
#define DM1_PC34_GLOBAL_MAXIMUM_ACTIVE_GROUP_COUNT_OFFSET 46u

#define DM1_PC34_THING_TYPE_GROUP 4
#define DM1_PC34_THING_INDEX_MASK 0x03ffu
#define DM1_PC34_THING_TYPE_SHIFT 10u
#define DM1_PC34_THING_TYPE_MASK 0x000fu

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void write_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static void write_u32_le(uint8_t *p, uint32_t v)
{
    write_u16_le(p, (uint16_t)(v & 0xffffu));
    write_u16_le(p + 2u, (uint16_t)((v >> 16) & 0xffffu));
}

static int16_t read_i16_le(const uint8_t *p)
{
    return (int16_t)read_u16_le(p);
}

static int read_original_pc34_file_bytes(
    const char *path,
    uint8_t **out_bytes,
    size_t *out_size)
{
    FILE *file;
    long file_size;
    uint8_t *bytes;
    size_t read_count;

    if (!path || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    *out_bytes = NULL;
    *out_size = 0u;

    file = fopen(path, "rb");
    if (!file) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    file_size = ftell(file);
    if (file_size <= 0 ||
        file_size > (long)SAVEGAME_PC34_MAX_FILE_SIZE ||
        file_size > (long)((int)0x7fffffff)) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    bytes = (uint8_t *)malloc((size_t)file_size);
    if (!bytes) {
        fclose(file);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    read_count = fread(bytes, 1u, (size_t)file_size, file);
    fclose(file);
    if (read_count != (size_t)file_size) {
        free(bytes);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    *out_bytes = bytes;
    *out_size = (size_t)file_size;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static uint16_t original_pc34_header_first_half_checksum(const uint8_t *header)
{
    uint16_t acc = 0u;
    size_t i;
    for (i = 0u; i < 32u; ++i) {
        acc = (uint16_t)(acc + read_u16_le(header + (i * 8u) + 0u));
        acc = (uint16_t)(acc ^ read_u16_le(header + (i * 8u) + 2u));
        acc = (uint16_t)(acc - read_u16_le(header + (i * 8u) + 4u));
        acc = (uint16_t)(acc ^ read_u16_le(header + (i * 8u) + 6u));
    }
    return acc;
}

static uint16_t original_pc34_header_second_half_plain_sum(const uint8_t *header)
{
    uint16_t sum = 0u;
    size_t i;
    for (i = SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; i < 256u; ++i) {
        sum = (uint16_t)(sum + read_u16_le(header + (i * 2u)));
    }
    return sum;
}

static void original_pc34_fix_header_noise_checksum(uint8_t *header,
                                                     uint16_t expected)
{
    uint16_t acc = 0u;
    int group;

    for (group = 0; group < 31; ++group) {
        size_t base = (size_t)group * 8u;
        acc = (uint16_t)(acc + read_u16_le(header + base));
        acc = (uint16_t)(acc ^ read_u16_le(header + base + 2u));
        acc = (uint16_t)(acc - read_u16_le(header + base + 4u));
        acc = (uint16_t)(acc ^ read_u16_le(header + base + 6u));
    }
    acc = (uint16_t)(acc + read_u16_le(header + 248u));
    acc = (uint16_t)(acc ^ read_u16_le(header + 250u));
    acc = (uint16_t)(acc - read_u16_le(header + 252u));
    write_u16_le(header + 254u, (uint16_t)(acc ^ expected));
    acc = original_pc34_header_first_half_checksum(header);
    if (acc != expected) {
        write_u16_le(header + 254u,
                     (uint16_t)(read_u16_le(header + 254u) ^ acc ^
                                expected));
    }
}

static uint32_t read_skill_experience_le(const uint8_t *p)
{
    return read_u32_le(p + 2u);
}

static uint16_t skill_level_from_base_experience(uint32_t experience)
{
    uint16_t level = 1u;

    /* ReDMCSB CHAMPION.C F0303 lines 765-769: base skill level starts
     * at 1 and halves experience while it remains >= 500. Object
     * modifiers and temporary experience are not folded into saved base
     * runtime levels here; they remain live-runtime concerns. */
    while (experience >= 500u) {
        experience >>= 1;
        ++level;
    }
    return level;
}

int dm1_v1_original_save_pc34_write_part_f0420(
    uint8_t *dst,
    size_t dst_capacity,
    const uint8_t *plain,
    size_t byte_count,
    uint16_t key,
    uint16_t *out_checksum)
{
    uint16_t checksum;

    if (!dst || !plain || !out_checksum) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    if ((byte_count & 1u) != 0u ||
        byte_count > 0xffffu ||
        dst_capacity < 2u + byte_count) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }

    write_u16_le(dst, (uint16_t)byte_count);
    memcpy(dst + 2u, plain, byte_count);
    checksum = F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        dst + 2u, byte_count / 2u, key);
    *out_checksum = checksum;
    return (int)(2u + byte_count);
}

static void write_original_pc34_fixture_champion(uint8_t *dst,
                                                 const char *name,
                                                 const char *title,
                                                 int direction,
                                                 int hp_current,
                                                 int hp_maximum,
                                                 int stamina_current,
                                                 int stamina_maximum,
                                                 int mana_current,
                                                 int mana_maximum,
                                                 int food,
                                                 int water,
                                                 uint16_t wounds,
                                                 uint16_t hand_item)
{
    int i;

    memset(dst, 0, DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT);
    memset(dst + DM1_PC34_CHAMPION_NAME_OFFSET, ' ', CHAMPION_NAME_LENGTH);
    memset(dst + DM1_PC34_CHAMPION_TITLE_OFFSET, ' ', CHAMPION_TITLE_LENGTH);
    if (name) {
        size_t n = strlen(name);
        if (n > CHAMPION_NAME_LENGTH) n = CHAMPION_NAME_LENGTH;
        memcpy(dst + DM1_PC34_CHAMPION_NAME_OFFSET, name, n);
    }
    if (title) {
        size_t n = strlen(title);
        if (n > CHAMPION_TITLE_LENGTH) n = CHAMPION_TITLE_LENGTH;
        memcpy(dst + DM1_PC34_CHAMPION_TITLE_OFFSET, title, n);
    }
    dst[DM1_PC34_CHAMPION_DIRECTION_OFFSET] = (uint8_t)(direction & 3);
    dst[DM1_PC34_CHAMPION_ACTION_INDEX_OFFSET] = 0xffu;
    write_u16_le(dst + DM1_PC34_CHAMPION_ATTRIBUTES_OFFSET, 0x1234u);
    write_u16_le(dst + DM1_PC34_CHAMPION_WOUNDS_OFFSET, wounds);
    write_u16_le(dst + DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET, (uint16_t)hp_current);
    write_u16_le(dst + DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET, (uint16_t)hp_maximum);
    write_u16_le(dst + DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET, (uint16_t)stamina_current);
    write_u16_le(dst + DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET, (uint16_t)stamina_maximum);
    write_u16_le(dst + DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET, (uint16_t)mana_current);
    write_u16_le(dst + DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET, (uint16_t)mana_maximum);
    write_u16_le(dst + DM1_PC34_CHAMPION_FOOD_OFFSET, (uint16_t)food);
    write_u16_le(dst + DM1_PC34_CHAMPION_WATER_OFFSET, (uint16_t)water);
    for (i = 0; i < 7; ++i) {
        uint8_t *stat = dst + DM1_PC34_CHAMPION_STATISTICS_OFFSET +
                        (size_t)i * 3u;
        stat[0] = (uint8_t)(40 + i);
        stat[1] = (uint8_t)(30 + i);
        stat[2] = (uint8_t)(10 + i);
    }
    for (i = 0; i < 20; ++i) {
        uint8_t *skill = dst + DM1_PC34_CHAMPION_SKILLS_OFFSET +
                         (size_t)i * 6u;
        write_u16_le(skill, (uint16_t)(0x0100u + (uint16_t)i));
        write_u32_le(skill + 2u, 1000u + (uint32_t)i * 111u);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        write_u16_le(dst + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                     (size_t)i * 2u, 0xffffu);
    }
    write_u16_le(dst + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                 (size_t)CHAMPION_SLOT_HAND_RIGHT * 2u, hand_item);
    write_u16_le(dst + DM1_PC34_CHAMPION_LOAD_OFFSET, 345u);
}

static void write_original_pc34_fixture_active_group(uint8_t *dst,
                                                     uint16_t group_thing_index,
                                                     int directions,
                                                     int cells,
                                                     int target_x,
                                                     int target_y)
{
    write_u16_le(dst + 0u, group_thing_index);
    dst[2u] = (uint8_t)directions;
    dst[3u] = (uint8_t)cells;
    dst[4u] = 12u;
    dst[5u] = 3u;
    dst[6u] = (uint8_t)target_x;
    dst[7u] = (uint8_t)target_y;
    dst[8u] = 5u;
    dst[9u] = 6u;
    dst[10u] = 7u;
    dst[11u] = 8u;
    dst[12u] = 0x41u;
    dst[13u] = 0x42u;
    dst[14u] = 0x43u;
    dst[15u] = 0x44u;
}

static void write_original_pc34_fixture_event(uint8_t *dst,
                                              uint32_t map_time,
                                              int type,
                                              int priority,
                                              int map_x,
                                              int map_y,
                                              int cell,
                                              int effect)
{
    write_u32_le(dst + 0u, map_time);
    dst[4u] = (uint8_t)type;
    dst[5u] = (uint8_t)priority;
    dst[6u] = (uint8_t)map_x;
    dst[7u] = (uint8_t)map_y;
    dst[8u] = (uint8_t)cell;
    dst[9u] = (uint8_t)effect;
}

int dm1_v1_original_save_pc34_read_part_f0419(
    const uint8_t *bytes,
    size_t size,
    size_t *cursor,
    uint16_t key,
    uint16_t expected_checksum,
    uint8_t *out_plain,
    size_t out_capacity,
    size_t *out_size,
    uint16_t *out_actual_checksum)
{
    uint16_t byte_count;
    uint16_t actual_checksum;

    if (!bytes || !cursor || !out_plain || !out_size) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    *out_size = 0u;
    if (out_actual_checksum) {
        *out_actual_checksum = 0u;
    }
    if (*cursor > size || size - *cursor < 2u) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    byte_count = read_u16_le(bytes + *cursor);
    *cursor += 2u;
    if ((byte_count & 1u) != 0u ||
        *cursor > size ||
        (size_t)byte_count > size - *cursor ||
        (size_t)byte_count > out_capacity) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    /* ReDMCSB F0419 validates the exact stored part before it decrypts the
     * staged buffer. Keep F0418 non-mutating over the source-owned span, then
     * let F0417 own the separate plaintext copy. */
    actual_checksum = F0418_SAVEUTIL_GetChecksumPC34_Compat(
        bytes + *cursor, (size_t)byte_count / 2u, key);
    memcpy(out_plain, bytes + *cursor, (size_t)byte_count);
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        out_plain, (size_t)byte_count / 2u, key);
    *cursor += (size_t)byte_count;
    *out_size = (size_t)byte_count;
    if (out_actual_checksum) {
        *out_actual_checksum = actual_checksum;
    }
    if (actual_checksum != expected_checksum) {
        return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
    }
    return SAVEGAME_PC34_OK;
}

static void import_original_pc34_champion(const uint8_t *src,
                                          int slot,
                                          struct ChampionState_Compat *champ)
{
    int i;

    F0600_CHAMPION_InitEmpty_Compat(champ);
    champ->present = 1;
    champ->portraitIndex = slot;
    memcpy(champ->name, src + DM1_PC34_CHAMPION_NAME_OFFSET,
           CHAMPION_NAME_LENGTH);
    memcpy(champ->title, src + DM1_PC34_CHAMPION_TITLE_OFFSET,
           CHAMPION_TITLE_LENGTH);
    champ->direction = src[DM1_PC34_CHAMPION_DIRECTION_OFFSET];
    champ->actionIndex = src[DM1_PC34_CHAMPION_ACTION_INDEX_OFFSET];
    champ->poisonDose = src[DM1_PC34_CHAMPION_POISON_EVENT_COUNT_OFFSET];
    champ->wounds = read_u16_le(src + DM1_PC34_CHAMPION_WOUNDS_OFFSET);
    champ->hp.current =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET);
    champ->hp.maximum =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET);
    champ->hp.shifted = (uint16_t)(champ->hp.maximum << 1);
    champ->stamina.current =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET);
    champ->stamina.maximum =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET);
    champ->stamina.shifted = (uint16_t)(champ->stamina.maximum << 1);
    champ->mana.current =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET);
    champ->mana.maximum =
        (uint16_t)read_i16_le(src + DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET);
    champ->mana.shifted = (uint16_t)(champ->mana.maximum << 1);
    champ->food = read_i16_le(src + DM1_PC34_CHAMPION_FOOD_OFFSET);
    champ->water = read_i16_le(src + DM1_PC34_CHAMPION_WATER_OFFSET);

    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        const uint8_t *stat = src + DM1_PC34_CHAMPION_STATISTICS_OFFSET +
                              (size_t)(i + 1) * 3u;
        champ->attributeMaximums[i] = stat[0];
        champ->attributes[i] = stat[1];
    }

    /* ReDMCSB DEFS.H lines 608-622 stores each SKILL as
     * TemporaryExperience + Experience. Firestaff currently exposes four
     * base skill experience buckets, matching DEFS.H lines 756-760. */
    for (i = 0; i < CHAMPION_SKILL_COUNT; ++i) {
        const uint8_t *skill = src + DM1_PC34_CHAMPION_SKILLS_OFFSET +
                               (size_t)i * 6u;
        champ->skillExperience[i] = read_skill_experience_le(skill);
        champ->skillLevels[i] =
            skill_level_from_base_experience((uint32_t)champ->skillExperience[i]);
    }
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = read_u16_le(src + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                                          (size_t)i * 2u);
    }
    champ->load = read_u16_le(src + DM1_PC34_CHAMPION_LOAD_OFFSET);
}

static int validate_original_pc34_champion_block(const uint8_t *src)
{
    static const size_t current_offsets[3] = {
        DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET,
        DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET,
        DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET
    };
    static const size_t maximum_offsets[3] = {
        DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET,
        DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET,
        DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET
    };
    int i;

    if (!src || src[DM1_PC34_CHAMPION_DIRECTION_OFFSET] > 3u) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    /* ReDMCSB DEFS.H stores Current/MaximumHealth, Stamina and Mana as
     * int16_t. LOADSAVE.C F0435 copies those fixed CHAMPION records, so
     * reject impossible signed values before Firestaff's uint16_t state can
     * turn a negative source value into a huge live vital. */
    for (i = 0; i < 3; ++i) {
        int current = (int)read_i16_le(src + current_offsets[i]);
        int maximum = (int)read_i16_le(src + maximum_offsets[i]);

        if (current < 0 || maximum < 0 || current > maximum) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
    }
    return SAVEGAME_PC34_OK;
}

static uint32_t original_pc34_party_metadata_hash_step(uint32_t hash,
                                                       uint32_t value)
{
    int byte_index;

    for (byte_index = 0; byte_index < 4; ++byte_index) {
        hash ^= (uint8_t)(value >> (byte_index * 8));
        hash *= 16777619u;
    }
    return hash;
}

/* F0435 receives the C2 M516 prefix as source bytes. Keep the ownership
 * boundary narrow: party cardinality/leader identity and each live
 * champion's name, facing and inventory slots are enough to prove the C13
 * Priority target was not remapped while materializing the tail. */
static uint32_t original_pc34_source_party_metadata_fingerprint(
    const uint8_t *part,
    int champion_count,
    int active_champion_index)
{
    uint32_t fingerprint = 2166136261u;
    int champion_index;

    if (!part || champion_count < 0 || champion_count > CHAMPION_MAX_PARTY ||
        active_champion_index < -1 || active_champion_index >= champion_count) {
        return 0u;
    }
    fingerprint = original_pc34_party_metadata_hash_step(
        fingerprint, (uint32_t)champion_count);
    fingerprint = original_pc34_party_metadata_hash_step(
        fingerprint, (uint32_t)(active_champion_index + 1));
    for (champion_index = 0; champion_index < champion_count;
         ++champion_index) {
        const uint8_t *champion = part +
            (size_t)champion_index * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT;
        int byte_index;
        int slot_index;

        for (byte_index = 0; byte_index < CHAMPION_NAME_LENGTH;
             ++byte_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, champion[DM1_PC34_CHAMPION_NAME_OFFSET + byte_index]);
        }
        for (byte_index = 0; byte_index < CHAMPION_TITLE_LENGTH;
             ++byte_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, champion[DM1_PC34_CHAMPION_TITLE_OFFSET + byte_index]);
        }
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion[DM1_PC34_CHAMPION_DIRECTION_OFFSET]);
        for (slot_index = 0; slot_index < CHAMPION_SLOT_COUNT; ++slot_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, read_u16_le(champion + DM1_PC34_CHAMPION_SLOTS_OFFSET +
                                         (size_t)slot_index * 2u));
        }
    }
    return fingerprint ? fingerprint : 1u;
}

static uint32_t original_pc34_runtime_party_metadata_fingerprint(
    const struct PartyState_Compat *party)
{
    uint32_t fingerprint = 2166136261u;
    int champion_index;

    if (!party || party->championCount < 0 ||
        party->championCount > CHAMPION_MAX_PARTY ||
        party->activeChampionIndex < -1 ||
        party->activeChampionIndex >= party->championCount) {
        return 0u;
    }
    fingerprint = original_pc34_party_metadata_hash_step(
        fingerprint, (uint32_t)party->championCount);
    fingerprint = original_pc34_party_metadata_hash_step(
        fingerprint, (uint32_t)(party->activeChampionIndex + 1));
    for (champion_index = 0; champion_index < party->championCount;
         ++champion_index) {
        const struct ChampionState_Compat *champion =
            &party->champions[champion_index];
        int byte_index;
        int slot_index;

        if (!champion->present) {
            return 0u;
        }
        for (byte_index = 0; byte_index < CHAMPION_NAME_LENGTH;
             ++byte_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, (uint8_t)champion->name[byte_index]);
        }
        for (byte_index = 0; byte_index < CHAMPION_TITLE_LENGTH;
             ++byte_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, (uint8_t)champion->title[byte_index]);
        }
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, (uint32_t)champion->direction);
        for (slot_index = 0; slot_index < CHAMPION_SLOT_COUNT; ++slot_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, champion->inventory[slot_index]);
        }
    }
    return fingerprint ? fingerprint : 1u;
}

static uint32_t original_pc34_source_party_state_fingerprint(
    const uint8_t *part,
    int champion_count,
    int active_champion_index)
{
    static const size_t vital_offsets[] = {
        DM1_PC34_CHAMPION_WOUNDS_OFFSET,
        DM1_PC34_CHAMPION_CURRENT_HEALTH_OFFSET,
        DM1_PC34_CHAMPION_MAXIMUM_HEALTH_OFFSET,
        DM1_PC34_CHAMPION_CURRENT_STAMINA_OFFSET,
        DM1_PC34_CHAMPION_MAXIMUM_STAMINA_OFFSET,
        DM1_PC34_CHAMPION_CURRENT_MANA_OFFSET,
        DM1_PC34_CHAMPION_MAXIMUM_MANA_OFFSET,
        DM1_PC34_CHAMPION_FOOD_OFFSET,
        DM1_PC34_CHAMPION_WATER_OFFSET
    };
    uint32_t fingerprint = original_pc34_source_party_metadata_fingerprint(
        part, champion_count, active_champion_index);
    int champion_index;

    if (fingerprint == 0u) {
        return 0u;
    }
    for (champion_index = 0; champion_index < champion_count;
         ++champion_index) {
        const uint8_t *champion = part +
            (size_t)champion_index * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT;
        int vital_index;
        int attribute_index;

        for (vital_index = 0;
             vital_index < (int)(sizeof(vital_offsets) / sizeof(vital_offsets[0]));
             ++vital_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, read_u16_le(champion + vital_offsets[vital_index]));
        }
        for (attribute_index = 0; attribute_index < CHAMPION_ATTR_COUNT;
             ++attribute_index) {
            const uint8_t *stat = champion + DM1_PC34_CHAMPION_STATISTICS_OFFSET +
                (size_t)(attribute_index + 1) * 3u;

            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, stat[0]);
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, stat[1]);
        }
    }
    return fingerprint ? fingerprint : 1u;
}

static uint32_t original_pc34_runtime_party_state_fingerprint(
    const struct PartyState_Compat *party)
{
    uint32_t fingerprint = original_pc34_runtime_party_metadata_fingerprint(
        party);
    int champion_index;

    if (fingerprint == 0u) {
        return 0u;
    }
    for (champion_index = 0; champion_index < party->championCount;
         ++champion_index) {
        const struct ChampionState_Compat *champion =
            &party->champions[champion_index];
        int attribute_index;

        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->wounds);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->hp.current);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->hp.maximum);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->stamina.current);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->stamina.maximum);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->mana.current);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, champion->mana.maximum);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, (uint16_t)champion->food);
        fingerprint = original_pc34_party_metadata_hash_step(
            fingerprint, (uint16_t)champion->water);
        for (attribute_index = 0; attribute_index < CHAMPION_ATTR_COUNT;
             ++attribute_index) {
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, champion->attributeMaximums[attribute_index]);
            fingerprint = original_pc34_party_metadata_hash_step(
                fingerprint, champion->attributes[attribute_index]);
        }
    }
    return fingerprint ? fingerprint : 1u;
}

static int import_original_pc34_party_part(const uint8_t *part,
                                           size_t part_size,
                                           struct SaveGame_Compat *out_state,
                                           DM1OriginalSavePC34HandoffReport *out_report)
{
    int slot_count;
    int i;

    /* ReDMCSB LOADSAVE.C F0435:2766-2777 reads one fixed PC34 PARTY
     * save part: M516_CHAMPIONS (4 * 319 bytes) followed by PARTY_INFO
     * (128 bytes). Do not treat excess bytes as a private extension. */
    if (part_size != DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (out_report) {
        /* ReDMCSB DEFS.H PARTY_INFO: MagicalLightAmount is the first
         * int16, followed by Event73Count_ThievesEye/Event79Count_Footprints;
         * ShieldDefense is the following signed int16 at byte 4;
         * FireShieldDefense is the next signed int16 at byte 6;
         * SpellShieldDefense is the following signed int16 at byte 8;
         * Event71Count_Invisibility follows 24 two-byte SCENTs and 24 scent
         * strengths at byte 86. LOADSAVE.C F0435
         * copies this fixed block before it resumes TIMELINE.C, so retain
         * the persisted active-state byte instead of inferring it from a
         * pending C73 timeout. */
        out_report->imported_event73_count_thieves_eye =
            part[DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
                 DM1_PC34_ORIGINAL_PARTY_INFO_EVENT73_COUNT_OFFSET];
        out_report->imported_magical_light_amount = read_i16_le(
            part + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
            DM1_PC34_ORIGINAL_PARTY_INFO_MAGICAL_LIGHT_AMOUNT_OFFSET);
        out_report->imported_event79_count_footprints =
            part[DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
                 DM1_PC34_ORIGINAL_PARTY_INFO_EVENT79_COUNT_OFFSET];
        out_report->imported_event71_count_invisibility =
            part[DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
                 DM1_PC34_ORIGINAL_PARTY_INFO_EVENT71_COUNT_OFFSET];
        out_report->imported_party_shield_defense = read_i16_le(
            part + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
            DM1_PC34_ORIGINAL_PARTY_INFO_SHIELD_DEFENSE_OFFSET);
        out_report->imported_party_fire_shield_defense = read_i16_le(
            part + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
            DM1_PC34_ORIGINAL_PARTY_INFO_FIRE_SHIELD_DEFENSE_OFFSET);
        out_report->imported_party_spell_shield_defense = read_i16_le(
            part + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY +
            DM1_PC34_ORIGINAL_PARTY_INFO_SPELL_SHIELD_DEFENSE_OFFSET);
    }
    if (!out_state->party) {
        return SAVEGAME_PC34_OK;
    }
    memcpy(out_state->party->pc34PartyInfoBytes,
           part + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY,
           DM1_PC34_ORIGINAL_PARTY_INFO_BYTE_COUNT);
    out_state->party->pc34PartyInfoBytesValid = 1;

    slot_count = out_state->party->championCount;
    if (slot_count < 0) {
        slot_count = 0;
    }
    if (slot_count > CHAMPION_MAX_PARTY) {
        slot_count = CHAMPION_MAX_PARTY;
    }
    for (i = 0; i < slot_count; ++i) {
        int result = validate_original_pc34_champion_block(
            part + (size_t)i * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT);
        if (result != SAVEGAME_PC34_OK) {
            return result;
        }
    }
    for (i = 0; i < slot_count; ++i) {
        import_original_pc34_champion(
            part + (size_t)i * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT,
            i,
            &out_state->party->champions[i]);
    }
    for (; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&out_state->party->champions[i]);
    }
    if (out_report) {
        out_report->imported_champion_block_count = CHAMPION_MAX_PARTY;
        out_report->imported_champion_slot_count = slot_count;
        out_report->imported_skill_level_count =
            slot_count * CHAMPION_SKILL_COUNT;
        out_report->source_party_champion_metadata_fingerprint =
            original_pc34_source_party_metadata_fingerprint(
                part, slot_count, out_state->party->activeChampionIndex);
        out_report->source_party_champion_state_fingerprint =
            original_pc34_source_party_state_fingerprint(
                part, slot_count, out_state->party->activeChampionIndex);
    }
    return SAVEGAME_PC34_OK;
}

static void decode_original_pc34_active_group(
    const uint8_t *src,
    DM1OriginalSavePC34ActiveGroupRecord *dst)
{
    int i;

    dst->group_thing_index = read_i16_le(src + 0u);
    dst->directions = src[2u];
    dst->cells = src[3u];
    dst->last_move_time = src[4u];
    dst->delay_fleeing_from_target = src[5u];
    dst->target_map_x = src[6u];
    dst->target_map_y = src[7u];
    dst->prior_map_x = src[8u];
    dst->prior_map_y = src[9u];
    dst->home_map_x = src[10u];
    dst->home_map_y = src[11u];
    for (i = 0; i < 4; ++i) {
        dst->aspect[i] = src[12u + (size_t)i];
    }
}

static int import_original_pc34_active_group_part(
    const uint8_t *part,
    size_t part_size,
    int maximum_active_group_count,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    size_t expected_size;
    int i;
    int report_count;

    if (maximum_active_group_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    expected_size = (size_t)maximum_active_group_count *
                    DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT;
    if (part_size != expected_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_report) {
        return SAVEGAME_PC34_OK;
    }

    out_report->decoded_active_group_count = maximum_active_group_count;
    report_count = maximum_active_group_count;
    if (report_count > DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP) {
        report_count = DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP;
    }
    out_report->reported_active_group_count = report_count;
    out_report->active_group_decode_truncated_count =
        maximum_active_group_count - report_count;
    for (i = 0; i < report_count; ++i) {
        decode_original_pc34_active_group(
            part + (size_t)i * DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
            &out_report->active_groups[i]);
    }
    return SAVEGAME_PC34_OK;
}

static void decode_original_pc34_event(const uint8_t *src,
                                       struct DM1_Event_V1 *dst)
{
    dst->map_time = read_u32_le(src + 0u);
    dst->type = src[4u];
    dst->priority = src[5u];
    dst->b_mapX = src[6u];
    dst->b_mapY = src[7u];
    dst->c_cell = src[8u];
    dst->c_effect = src[9u];
}

static int import_original_pc34_events_part(
    const uint8_t *part,
    size_t part_size,
    int event_maximum_count,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    size_t expected_size;
    int i;
    int decode_count;

    if (event_maximum_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    expected_size = (size_t)event_maximum_count *
                    DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT;
    if (part_size != expected_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_report) {
        return SAVEGAME_PC34_OK;
    }

    decode_count = event_maximum_count;
    if (decode_count > DM1_EVENT_MAX_COUNT) {
        decode_count = DM1_EVENT_MAX_COUNT;
    }
    out_report->decoded_event_count = decode_count;
    out_report->event_decode_truncated_count =
        event_maximum_count - decode_count;
    if (part_size > sizeof(out_report->c3_raw_event_bytes)) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    memcpy(out_report->c3_raw_event_bytes, part, part_size);
    out_report->c3_raw_event_byte_count = (uint32_t)part_size;
    out_report->c3_raw_event_fingerprint = dm1_v1_c15_layout_fingerprint_pc34(
        part, part_size);
    for (i = 0; i < decode_count; ++i) {
        decode_original_pc34_event(
            part + (size_t)i * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
            &out_report->events[i]);
    }
    return SAVEGAME_PC34_OK;
}

static int import_original_pc34_timeline_part(
    const uint8_t *part,
    size_t part_size,
    int event_maximum_count,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    size_t expected_size;
    int i;
    int decode_count;

    if (event_maximum_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    expected_size = (size_t)event_maximum_count * 2u;
    if (part_size != expected_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (!out_report) {
        return SAVEGAME_PC34_OK;
    }

    decode_count = event_maximum_count;
    if (decode_count > DM1_EVENT_MAX_COUNT) {
        decode_count = DM1_EVENT_MAX_COUNT;
    }
    out_report->decoded_timeline_index_count = decode_count;
    if (part_size > sizeof(out_report->c4_raw_heap_bytes)) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    memcpy(out_report->c4_raw_heap_bytes, part, part_size);
    out_report->c4_raw_heap_byte_count = (uint32_t)part_size;
    out_report->c4_raw_heap_fingerprint = dm1_v1_c15_layout_fingerprint_pc34(
        part, part_size);
    for (i = 0; i < decode_count; ++i) {
        out_report->timeline_indices[i] = read_u16_le(part + (size_t)i * 2u);
    }
    return SAVEGAME_PC34_OK;
}

static int validate_original_pc34_timeline_membership(
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t seen[DM1_EVENT_MAX_COUNT];
    int i;

    if (!out_report || out_report->original_event_count < 0 ||
        out_report->original_event_count > out_report->decoded_event_count ||
        out_report->original_event_count >
            out_report->decoded_timeline_index_count) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (out_report->original_first_unused_event_index >= 0 &&
        out_report->original_first_unused_event_index <
            out_report->decoded_event_count &&
        out_report->events[out_report->original_first_unused_event_index].type !=
            DM1_EVENT_NONE) {
        out_report->first_unused_event_index_points_to_active = 1;
        out_report->first_unused_event_index_event_type =
            out_report->events[
                out_report->original_first_unused_event_index].type;
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    memset(seen, 0, sizeof(seen));
    for (i = 0; i < out_report->original_event_count; ++i) {
        uint16_t event_index = out_report->timeline_indices[i];

        if (event_index >= (uint16_t)out_report->decoded_event_count) {
            out_report->timeline_invalid_slot = i;
            out_report->timeline_invalid_event_index = event_index;
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        if (out_report->events[event_index].type == DM1_EVENT_NONE) {
            out_report->timeline_invalid_slot = i;
            out_report->timeline_invalid_event_index = event_index;
            out_report->timeline_invalid_event_is_none = 1;
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        if (seen[event_index]) {
            int first_slot;

            for (first_slot = 0; first_slot < i; ++first_slot) {
                if (out_report->timeline_indices[first_slot] == event_index) {
                    break;
                }
            }
            out_report->timeline_duplicate_first_slot = first_slot;
            out_report->timeline_duplicate_slot = i;
            out_report->timeline_duplicate_event_index = event_index;
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        seen[event_index] = 1u;
    }
    /* ReDMCSB LOADSAVE.C F0433 writes the live EVENT heap as a paired C3/C4
     * transaction. F0435 restores C4 before the M10 materializer consumes
     * it; accepting a live C13 absent from C4 would silently lose its rebirth
     * timer. Reject every such active orphan with source-slot provenance. */
    for (i = 0; i < out_report->decoded_event_count; ++i) {
        if (out_report->events[i].type != DM1_EVENT_NONE && !seen[i]) {
            out_report->timeline_orphan_active_event_index = i;
            out_report->timeline_orphan_active_event_type =
                out_report->events[i].type;
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
    }
    return SAVEGAME_PC34_OK;
}

static int original_pc34_timeline_event_is_before(
    const struct DM1_Event_V1 *left,
    int left_index,
    const struct DM1_Event_V1 *right,
    int right_index)
{
    uint32_t left_time;
    uint32_t right_time;
    uint16_t left_type_priority;
    uint16_t right_type_priority;

    if (!left || !right) {
        return 0;
    }
    left_time = left->map_time & 0x00ffffffu;
    right_time = right->map_time & 0x00ffffffu;
    if (left_time != right_time) {
        return left_time < right_time;
    }
    left_type_priority = (uint16_t)(((uint16_t)left->type << 8) |
                                    left->priority);
    right_type_priority = (uint16_t)(((uint16_t)right->type << 8) |
                                     right->priority);
    if (left_type_priority != right_type_priority) {
        return left_type_priority > right_type_priority;
    }
    return left_index <= right_index;
}

static int validate_original_pc34_timeline_heap(
    DM1OriginalSavePC34HandoffReport *out_report)
{
    int child_slot;

    if (!out_report) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    for (child_slot = 1;
         child_slot < out_report->original_event_count;
         ++child_slot) {
        int parent_slot = (child_slot - 1) / 2;
        int parent_index = out_report->timeline_indices[parent_slot];
        int child_index = out_report->timeline_indices[child_slot];

        /* ReDMCSB TIMELINE.C F0234 compares low-24-bit time first, then
         * Type/Priority descending, and finally EVENT array address. F0236
         * preserves that relation in C4; F0240 consumes C4[0] directly. */
        if (!original_pc34_timeline_event_is_before(
                &out_report->events[parent_index], parent_index,
                &out_report->events[child_index], child_index)) {
            out_report->timeline_heap_invalid_parent_slot = parent_slot;
            out_report->timeline_heap_invalid_child_slot = child_slot;
            out_report->timeline_heap_invalid_parent_event_index =
                parent_index;
            out_report->timeline_heap_invalid_child_event_index = child_index;
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
    }
    return SAVEGAME_PC34_OK;
}

static int timeline_kind_from_original_pc34_event_type(int type)
{
    switch (type) {
    case DM1_EVENT_DOOR_ANIMATION:
        return TIMELINE_EVENT_DOOR_ANIMATE;
    case DM1_EVENT_DOOR_DESTRUCTION:
        return TIMELINE_EVENT_DOOR_DESTRUCTION;
    case DM1_EVENT_MOVE_PROJECTILE:
    case DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS:
        return TIMELINE_EVENT_PROJECTILE_MOVE;
    case DM1_EVENT_EXPLOSION:
        return TIMELINE_EVENT_EXPLOSION_ADVANCE;
    case DM1_EVENT_LIGHT:
        return TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
    case DM1_EVENT_ENABLE_CHAMPION_ACTION:
        return TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    case DM1_EVENT_HIDE_DAMAGE_RECEIVED:
        return TIMELINE_EVENT_STATUS_TIMEOUT;
    case DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE:
    case DM1_EVENT_GROUP_REACTION_HIT_BY_PROJECTILE:
    case DM1_EVENT_GROUP_REACTION_PARTY_IS_ADJACENT:
    case DM1_EVENT_UPDATE_ASPECT_GROUP:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_0:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_1:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_2:
    case DM1_EVENT_UPDATE_ASPECT_CREATURE_3:
    case DM1_EVENT_UPDATE_BEHAVIOR_GROUP:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_1:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_2:
    case DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3:
        return TIMELINE_EVENT_CREATURE_REACTION;
    case DM1_EVENT_ENABLE_GROUP_GENERATOR:
        return TIMELINE_EVENT_GROUP_GENERATOR;
    case DM1_EVENT_REMOVE_FLUXCAGE:
        return TIMELINE_EVENT_REMOVE_FLUXCAGE;
    case DM1_EVENT_PLAY_SOUND:
        return TIMELINE_EVENT_PLAY_SOUND;
    case DM1_EVENT_CPSE:
        return TIMELINE_EVENT_CPSE_CHECK;
    case DM1_EVENT_WATCHDOG:
        return TIMELINE_EVENT_WATCHDOG;
    case DM1_EVENT_MOVE_GROUP_SILENT:
        return TIMELINE_EVENT_MOVE_GROUP_SILENT;
    case DM1_EVENT_MOVE_GROUP_AUDIBLE:
        return TIMELINE_EVENT_MOVE_GROUP_AUDIBLE;
    case DM1_EVENT_CORRIDOR:
    case DM1_EVENT_WALL:
    case DM1_EVENT_FAKEWALL:
    case DM1_EVENT_TELEPORTER:
    case DM1_EVENT_PIT:
    case DM1_EVENT_DOOR:
        return TIMELINE_EVENT_SQUARE_STATE;
    case DM1_EVENT_INVISIBILITY:
    case DM1_EVENT_CHAMPION_SHIELD:
    case DM1_EVENT_THIEVES_EYE:
    case DM1_EVENT_PARTY_SHIELD:
    case DM1_EVENT_POISON_CHAMPION:
    case DM1_EVENT_SPELLSHIELD:
    case DM1_EVENT_FIRESHIELD:
    case DM1_EVENT_FOOTPRINTS:
        return TIMELINE_EVENT_STATUS_TIMEOUT;
    default:
        return TIMELINE_EVENT_INVALID;
    }
}

static int validate_original_pc34_timeline_references(
    const DM1OriginalSavePC34HandoffReport *report)
{
    unsigned char seen[DM1_EVENT_MAX_COUNT];
    int i;

    if (!report || report->original_event_count < 0 ||
        report->original_event_count > DM1_EVENT_MAX_COUNT ||
        report->original_event_count > report->decoded_event_count ||
        report->original_event_count > report->decoded_timeline_index_count) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(seen, 0, sizeof(seen));
    for (i = 0; i < report->original_event_count; ++i) {
        uint16_t source_index = report->timeline_indices[i];

        /* ReDMCSB LOADSAVE.C F0435 reads the persisted heap of EVENT array
         * indexes before TIMELINE.C resumes it. Each live heap entry names
         * one EVENT slot, so a duplicate would schedule one source event
         * twice even though each index is individually in range. */
        if (source_index >= (uint16_t)report->decoded_event_count ||
            seen[source_index]) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        seen[source_index] = 1u;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int original_pc34_event_type_is_status_timeout(int type)
{
    return type == DM1_EVENT_INVISIBILITY ||
           type == DM1_EVENT_CHAMPION_SHIELD ||
           type == DM1_EVENT_THIEVES_EYE ||
           type == DM1_EVENT_PARTY_SHIELD ||
           type == DM1_EVENT_POISON_CHAMPION ||
           type == DM1_EVENT_SPELLSHIELD ||
           type == DM1_EVENT_FIRESHIELD ||
           type == DM1_EVENT_FOOTPRINTS;
}

static uint16_t original_pc34_next_thing(
    const struct DungeonThings_Compat *things,
    uint16_t thing)
{
    int type;
    int index;

    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (index < 0) {
        return THING_ENDOFLIST;
    }
    switch (type) {
    case THING_TYPE_DOOR:
        return (things->doors && index < things->doorCount) ?
            things->doors[index].next : THING_ENDOFLIST;
    case THING_TYPE_TELEPORTER:
        return (things->teleporters && index < things->teleporterCount) ?
            things->teleporters[index].next : THING_ENDOFLIST;
    case THING_TYPE_TEXTSTRING:
        return (things->textStrings && index < things->textStringCount) ?
            things->textStrings[index].next : THING_ENDOFLIST;
    case THING_TYPE_SENSOR:
        return (things->sensors && index < things->sensorCount) ?
            things->sensors[index].next : THING_ENDOFLIST;
    case THING_TYPE_GROUP:
        return (things->groups && index < things->groupCount) ?
            things->groups[index].next : THING_ENDOFLIST;
    case THING_TYPE_WEAPON:
        return (things->weapons && index < things->weaponCount) ?
            things->weapons[index].next : THING_ENDOFLIST;
    case THING_TYPE_ARMOUR:
        return (things->armours && index < things->armourCount) ?
            things->armours[index].next : THING_ENDOFLIST;
    case THING_TYPE_SCROLL:
        return (things->scrolls && index < things->scrollCount) ?
            things->scrolls[index].next : THING_ENDOFLIST;
    case THING_TYPE_POTION:
        return (things->potions && index < things->potionCount) ?
            things->potions[index].next : THING_ENDOFLIST;
    case THING_TYPE_CONTAINER:
        return (things->containers && index < things->containerCount) ?
            things->containers[index].next : THING_ENDOFLIST;
    case THING_TYPE_JUNK:
        return (things->junks && index < things->junkCount) ?
            things->junks[index].next : THING_ENDOFLIST;
    case THING_TYPE_PROJECTILE:
        return (things->projectiles && index < things->projectileCount) ?
            things->projectiles[index].next : THING_ENDOFLIST;
    case THING_TYPE_EXPLOSION:
        return (things->explosions && index < things->explosionCount) ?
            things->explosions[index].next : THING_ENDOFLIST;
    default:
        return THING_ENDOFLIST;
    }
}

static int original_pc34_event_type_is_group_reaction(int type)
{
    return type >= DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE &&
           type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3;
}

static int original_pc34_group_on_square(
    const struct GameWorld_Compat *world,
    int map_index,
    int map_x,
    int map_y,
    int *out_group_index)
{
    uint16_t thing;
    int matches = 0;
    int group_index = -1;
    int safety = 0;

    if (out_group_index) *out_group_index = -1;
    if (!world || !world->dungeon || !world->things ||
        !world->things->groups || map_index < 0 ||
        map_index >= (int)world->dungeon->header.mapCount ||
        map_x < 0 || map_y < 0 ||
        map_x >= (int)world->dungeon->maps[map_index].width ||
        map_y >= (int)world->dungeon->maps[map_index].height) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, map_index, map_x, map_y);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int index = (int)THING_GET_INDEX(thing);
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP &&
            index >= 0 && index < world->things->groupCount) {
            group_index = index;
            ++matches;
        }
        thing = original_pc34_next_thing(world->things, thing);
    }
    if (matches != 1) {
        return 0;
    }
    if (out_group_index) *out_group_index = group_index;
    return 1;
}

static int original_pc34_explosion_event_slot_is_valid(
    const struct DM1_Event_V1 *src,
    const struct GameWorld_Compat *world,
    int require_fluxcage,
    int *out_source_index)
{
    uint16_t source_thing;
    uint16_t thing;
    int source_index;
    int map_index;
    int matches = 0;
    int safety = 0;

    if (out_source_index) *out_source_index = -1;
    if (!src || !world || !world->dungeon || !world->things ||
        !world->things->explosions) {
        return 0;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    source_thing = read_u16_le(&src->c_cell);
    source_index = (int)THING_GET_INDEX(source_thing);
    if (THING_GET_TYPE(source_thing) != THING_TYPE_EXPLOSION ||
        source_index < 0 || source_index >= world->things->explosionCount ||
        map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height) {
        return 0;
    }
    if (require_fluxcage &&
        world->things->explosions[source_index].type != C050_EXPLOSION_FLUXCAGE) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, map_index, src->b_mapX, src->b_mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_EXPLOSION &&
            (int)THING_GET_INDEX(thing) == source_index) {
            ++matches;
        }
        thing = original_pc34_next_thing(world->things, thing);
    }
    if (matches != 1) {
        return 0;
    }
    if (out_source_index) *out_source_index = source_index;
    return 1;
}

/* ReDMCSB F0435 restores the C3/C4 pair before F0651 republishes its live
 * queue. C13 keeps its champion-rebirth union, while C24/C25 retain a C15
 * explosion owner. Check those source-specific relationships immediately
 * before runtime adoption rather than accepting a merely count-matched host
 * timeline. */
static int dm1_original_save_special_events_adoptable(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const struct GameWorld_Compat *world)
{
    int consumed[TIMELINE_QUEUE_CAPACITY] = {0};
    int source_index;

    if (!source_report || !world ||
        source_report->decoded_event_count < 0 ||
        source_report->decoded_event_count > DM1_EVENT_MAX_COUNT ||
        world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY) {
        return 0;
    }

    for (source_index = 0;
         source_index < source_report->decoded_event_count;
         ++source_index) {
        const struct DM1_Event_V1 *source =
            &source_report->events[source_index];
        const uint32_t fire_at_tick = source->map_time & 0x00ffffffu;
        const int map_index = (int)((source->map_time >> 24) & 0xffu);
        int runtime_index;

        if (source->type != DM1_EVENT_VI_ALTAR_REBIRTH &&
            source->type != DM1_EVENT_EXPLOSION &&
            source->type != DM1_EVENT_REMOVE_FLUXCAGE) {
            continue;
        }
        for (runtime_index = 0; runtime_index < world->timeline.count;
             ++runtime_index) {
            const struct TimelineEvent_Compat *runtime =
                &world->timeline.events[runtime_index];
            int source_explosion_index;
            uint16_t source_thing;
            const struct DungeonExplosion_Compat *source_explosion;
            const struct ExplosionInstance_Compat *runtime_explosion;

            if (consumed[runtime_index]) {
                continue;
            }
            if (source->type == DM1_EVENT_VI_ALTAR_REBIRTH) {
                if (!dm1_original_save_c13_runtime_event_matches(source,
                                                                  runtime)) {
                    continue;
                }
                consumed[runtime_index] = 1;
                break;
            }
            /* F0435 has already validated C15 ownership before F0213/F0224
             * materialize these events. That materialization may relink the
             * live chain, so adoption verifies the authenticated C.Slot and
             * the recreated runtime owner rather than rescanning a mutated
             * C15 chain a second time. */
            source_thing = read_u16_le(&source->c_cell);
            source_explosion_index = (int)THING_GET_INDEX(source_thing);
            if (THING_GET_TYPE(source_thing) != THING_TYPE_EXPLOSION ||
                !world->things || !world->things->explosions ||
                source_explosion_index < 0 ||
                source_explosion_index >= world->things->explosionCount ||
                (source->type == DM1_EVENT_REMOVE_FLUXCAGE &&
                 world->things->explosions[source_explosion_index].type !=
                     C050_EXPLOSION_FLUXCAGE) ||
                runtime->aux0 < 0 || runtime->aux0 >= EXPLOSION_LIST_CAPACITY) {
                return 0;
            }
            source_explosion = &world->things->explosions[source_explosion_index];
            runtime_explosion = &world->explosions.entries[runtime->aux0];
            if (runtime_explosion->reserved0 != 1 ||
                runtime_explosion->scheduledAtTick != (int)fire_at_tick ||
                runtime_explosion->mapIndex != map_index ||
                runtime_explosion->mapX != source->b_mapX ||
                runtime_explosion->mapY != source->b_mapY ||
                runtime_explosion->attack != source_explosion->attack) {
                continue;
            }
            if (source->type == DM1_EVENT_EXPLOSION) {
                if (runtime->kind != TIMELINE_EVENT_EXPLOSION_ADVANCE ||
                    runtime->fireAtTick != fire_at_tick ||
                    runtime->mapIndex != map_index ||
                    runtime->mapX != source->b_mapX ||
                    runtime->mapY != source->b_mapY ||
                    runtime->cell != (int)THING_GET_CELL(source_thing) ||
                    runtime->aux1 != source_explosion->type ||
                    runtime->aux2 != source_explosion->attack ||
                    runtime->aux4 != source->priority ||
                    runtime_explosion->explosionType != source_explosion->type) {
                    continue;
                }
            } else {
                if (runtime->kind != TIMELINE_EVENT_REMOVE_FLUXCAGE ||
                    runtime->fireAtTick != fire_at_tick ||
                    runtime->mapIndex != map_index ||
                    runtime->mapX != source->b_mapX ||
                    runtime->mapY != source->b_mapY ||
                    runtime->cell != 0 ||
                    runtime->aux1 != C050_EXPLOSION_FLUXCAGE ||
                    runtime->aux2 != (int)source_thing || runtime->aux4 != 0 ||
                    runtime_explosion->explosionType != C050_EXPLOSION_FLUXCAGE) {
                    continue;
                }
            }
            consumed[runtime_index] = 1;
            break;
        }
        if (runtime_index == world->timeline.count) {
            return 0;
        }
    }
    return 1;
}

int dm1_v1_original_save_pc34_handoff_projectile_event_plan(
    const struct DM1_Event_V1 *src,
    int source_index,
    const struct DungeonThings_Compat *things,
    DM1OriginalSavePC34ProjectileEventPlan *out_plan)
{
    const struct DungeonProjectile_Compat *source_projectile;
    uint16_t source_thing;
    uint16_t projectile_motion;
    int projectile_index;
    int projectile_type;

    if (!src || !things || !out_plan ||
        (src->type != DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS &&
         src->type != DM1_EVENT_MOVE_PROJECTILE)) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));

    /* ReDMCSB DEFS.H EVENT stores B.Slot as a C14 THING and packs
     * C.Projectile as MapX:5, MapY:5, Direction:2, StepEnergy:4.  An
     * original C48/C49 must bind both records before it can enter M10. */
    source_thing = read_u16_le(&src->b_mapX);
    projectile_motion = read_u16_le(&src->c_cell);
    if (THING_GET_TYPE(source_thing) != THING_TYPE_PROJECTILE) {
        return 0;
    }
    projectile_index = (int)THING_GET_INDEX(source_thing);
    if (projectile_index < 0 || projectile_index >= PROJECTILE_LIST_CAPACITY ||
        projectile_index >= things->projectileCount || !things->projectiles) {
        return 0;
    }
    source_projectile = &things->projectiles[projectile_index];
    if ((int)source_projectile->eventIndex != source_index) {
        return 0;
    }
    projectile_type = THING_GET_TYPE(source_projectile->slot);
    out_plan->valid = 1;
    out_plan->source_event_type = src->type;
    out_plan->source_event_index = source_index;
    out_plan->projectile_index = projectile_index;
    out_plan->projectile_category =
        projectile_type == THING_TYPE_EXPLOSION
            ? PROJECTILE_CATEGORY_MAGICAL : PROJECTILE_CATEGORY_KINETIC;
    out_plan->projectile_subtype =
        projectile_type == THING_TYPE_EXPLOSION
            ? (int)(source_projectile->slot & 0xffu)
            : PROJECTILE_SUBTYPE_KINETIC_ARROW;
    out_plan->map_index = (int)((src->map_time >> 24) & 0xffu);
    out_plan->map_x = (int)(projectile_motion & 0x001fu);
    out_plan->map_y = (int)((projectile_motion >> 5) & 0x001fu);
    out_plan->cell = (int)THING_GET_CELL(source_thing);
    out_plan->direction = (int)((projectile_motion >> 10) & 0x03u);
    out_plan->step_energy = (int)((projectile_motion >> 12) & 0x0fu);
    out_plan->first_move_grace =
        src->type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS;
    out_plan->kinetic_energy = source_projectile->kineticEnergy;
    out_plan->attack = source_projectile->attack;
    out_plan->associated_thing = source_projectile->slot;
    return 1;
}

int dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
    const struct DM1_Event_V1 *src,
    int source_event_index,
    DM1OriginalSavePC34ViAltarRebirthEventPlan *out_plan)
{
    if (!src || !out_plan || source_event_index < 0 ||
        src->type != DM1_EVENT_VI_ALTAR_REBIRTH ||
        src->priority >= CHAMPION_MAX_PARTY || src->c_cell > 3u ||
        src->c_effect > 2u) {
        return 0;
    }

    /* ReDMCSB CLIKVIEW.C F0374 lines 179-186 creates C13 from the bones
     * location/cell and ChargeCount champion index. TIMELINE.C F0255 lines
     * 1665-1699 consumes B.Location, C.A.Cell, C.A.Effect and Priority for
     * its exact 2 -> 1 -> 0 sequence. Do not collapse this union into the
     * generic Location/Cell/Effect handoff before that transaction exists. */
    memset(out_plan, 0, sizeof(*out_plan));
    out_plan->valid = 1;
    out_plan->source_event_index = source_event_index;
    out_plan->champion_index = src->priority;
    out_plan->map_index = (int)((src->map_time >> 24) & 0xffu);
    out_plan->map_x = src->b_mapX;
    out_plan->map_y = src->b_mapY;
    out_plan->cell = src->c_cell;
    out_plan->step = src->c_effect;
    out_plan->fire_at_tick = src->map_time & 0x00ffffffu;
    return 1;
}

static int materialize_original_pc34_projectile_event(
    const struct DM1_Event_V1 *src,
    int source_index,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    struct ProjectileInstance_Compat *runtime_projectile;
    DM1OriginalSavePC34ProjectileEventPlan plan;

    if (!src || !world || !world->dungeon || !world->things || !out_event ||
        !dm1_v1_original_save_pc34_handoff_projectile_event_plan(
            src, source_index, world->things, &plan)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (plan.map_index < 0 ||
        plan.map_index >= (int)world->dungeon->header.mapCount ||
        plan.map_x >= (int)world->dungeon->maps[plan.map_index].width ||
        plan.map_y >= (int)world->dungeon->maps[plan.map_index].height) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    runtime_projectile = &world->projectiles.entries[plan.projectile_index];
    if (runtime_projectile->reserved3 != 0) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(runtime_projectile, 0, sizeof(*runtime_projectile));
    runtime_projectile->slotIndex = plan.projectile_index;
    runtime_projectile->projectileCategory = plan.projectile_category;
    runtime_projectile->projectileSubtype = plan.projectile_subtype;
    runtime_projectile->ownerKind = -1;
    runtime_projectile->ownerIndex = -1;
    runtime_projectile->mapIndex = plan.map_index;
    runtime_projectile->mapX = plan.map_x;
    runtime_projectile->mapY = plan.map_y;
    runtime_projectile->cell = plan.cell;
    runtime_projectile->direction = plan.direction;
    runtime_projectile->kineticEnergy = plan.kinetic_energy;
    runtime_projectile->attack = plan.attack;
    runtime_projectile->stepEnergy = plan.step_energy;
    runtime_projectile->firstMoveGraceFlag = plan.first_move_grace;
    runtime_projectile->launchedAtTick =
        (int)((src->map_time & 0x00ffffffu) - 1u);
    runtime_projectile->scheduledAtTick =
        (int)(src->map_time & 0x00ffffffu);
    runtime_projectile->attackTypeCode = COMBAT_ATTACK_BLUNT;
    runtime_projectile->flags = PROJECTILE_FLAG_IGNORE_DOOR_PASS_THROUGH;
    runtime_projectile->launcherStrength = plan.attack;
    runtime_projectile->reserved1 = (int)plan.associated_thing;
    runtime_projectile->reserved3 = 1;
    if (world->projectiles.count <= plan.projectile_index) {
        world->projectiles.count = plan.projectile_index + 1;
    }

    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = plan.map_index;
    out_event->mapX = plan.map_x;
    out_event->mapY = plan.map_y;
    out_event->cell = plan.cell;
    out_event->aux0 = plan.projectile_index;
    /* C48/C49 is a source-owned discriminator. Keep it apart from aux4,
     * which retains EVENT.Priority for the scheduled runtime receipt. */
    out_event->aux2 = src->type;
    out_event->aux3 = runtime_projectile->projectileSubtype;
    out_event->aux4 = src->priority;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_group_reaction_event(
    const struct DM1_Event_V1 *src,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    int group_index;

    if (!src || !world || !out_event ||
        !original_pc34_event_type_is_group_reaction(src->type) ||
        !original_pc34_group_on_square(world,
                                       (int)((src->map_time >> 24) & 0xffu),
                                       (int)src->b_mapX, (int)src->b_mapY,
                                       &group_index)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB TIMELINE.C F0261:1858-1863 extracts C29..C41 and calls
     * GROUP.C F0209 with B.Location and C.Ticks.  Resolve the group from
     * the original SFT chain rather than inventing an M10 group identity. */
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_CREATURE_REACTION;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = (int)((src->map_time >> 24) & 0xffu);
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = group_index;
    out_event->aux1 = world->things->groups[group_index].creatureType;
    out_event->aux2 = src->type;
    out_event->aux3 = (int)read_u16_le(&src->c_cell);
    /* Keep the source byte while marking aux3 as an original C.Ticks
     * payload; M10-generated reactions retain their legacy tick route. */
    out_event->aux4 = (int)src->priority | 0x100;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_explosion_event(
    const struct DM1_Event_V1 *src,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    const struct DungeonExplosion_Compat *source_explosion;
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_event;
    uint16_t source_thing;
    int source_index;
    int runtime_index;
    int map_index;

    if (!src || !world || !world->dungeon || !world->things || !out_event ||
        src->type != DM1_EVENT_EXPLOSION) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    source_thing = read_u16_le(&src->c_cell);

    /* ReDMCSB PROJEXPL.C F0213:157-165 creates C25 with B.Location and
     * C.Slot. TIMELINE.C F0261:1872 forwards that same EVENT to F0220.
     * A C25 cannot be reconstructed from Cell/Effect: its C15 reference
     * must be present in the original square chain before M10 publishes it. */
    if (!original_pc34_explosion_event_slot_is_valid(
            src, world, 0, &source_index)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    source_explosion = &world->things->explosions[source_index];
    memset(&input, 0, sizeof(input));
    input.explosionType = source_explosion->type;
    input.attack = source_explosion->attack;
    input.mapIndex = map_index;
    input.mapX = src->b_mapX;
    input.mapY = src->b_mapY;
    input.cell = (int)THING_GET_CELL(source_thing);
    input.centered = source_explosion->centered;
    input.currentTick = (int)((src->map_time & 0x00ffffffu) - 1u);
    input.ownerKind = -1;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    if (!F0213_EXPLOSION_Create_Compat(&input, &world->explosions,
                                       &runtime_index, &first_event)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    (void)first_event;
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->cell = (int)THING_GET_CELL(source_thing);
    out_event->aux0 = runtime_index;
    out_event->aux1 = source_explosion->type;
    out_event->aux2 = source_explosion->attack;
    out_event->aux3 = (int)dm1_v1_c15_layout_fingerprint_pc34(
        world->things->rawThingData[THING_TYPE_EXPLOSION] +
            (size_t)source_index * s_thingDataByteCount[THING_TYPE_EXPLOSION],
        s_thingDataByteCount[THING_TYPE_EXPLOSION]);
    out_event->aux4 = src->priority;
    world->explosions.entries[runtime_index].scheduledAtTick =
        (int)(src->map_time & 0x00ffffffu);
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_remove_fluxcage_event(
    const struct DM1_Event_V1 *src,
    struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    const struct DungeonExplosion_Compat *source_explosion;
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first_event;
    uint16_t source_thing;
    int source_index;
    int runtime_index;
    int map_index;

    if (!src || !world || !world->dungeon || !world->things || !out_event ||
        src->type != DM1_EVENT_REMOVE_FLUXCAGE || src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    source_thing = read_u16_le(&src->c_cell);

    /* ReDMCSB PROJEXPL.C F0224:983-994 creates C24 only for a newly
     * linked C15 fluxcage: Priority=0, B.Location, C.Slot. TIMELINE.C
     * F0261:1906-1916 later unlinks that exact Thing unless the game is
     * won. Do not reinterpret C.Slot as a host ExplosionList index. */
    if (!original_pc34_explosion_event_slot_is_valid(
            src, world, 1, &source_index)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    source_explosion = &world->things->explosions[source_index];
    memset(&input, 0, sizeof(input));
    input.explosionType = C050_EXPLOSION_FLUXCAGE;
    input.attack = source_explosion->attack;
    input.mapIndex = map_index;
    input.mapX = src->b_mapX;
    input.mapY = src->b_mapY;
    input.cell = 0;
    input.centered = source_explosion->centered;
    input.currentTick = (int)((src->map_time & 0x00ffffffu) - 1u);
    input.ownerKind = -1;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    if (!F0213_EXPLOSION_Create_Compat(&input, &world->explosions,
                                       &runtime_index, &first_event)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    (void)first_event;
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_REMOVE_FLUXCAGE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->cell = 0;
    out_event->aux0 = runtime_index;
    out_event->aux1 = C050_EXPLOSION_FLUXCAGE;
    out_event->aux2 = (int)source_thing;
    out_event->aux4 = 0;
    world->explosions.entries[runtime_index].scheduledAtTick =
        (int)(src->map_time & 0x00ffffffu);
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_light_event(
    const struct DM1_Event_V1 *src,
    struct TimelineEvent_Compat *out_event)
{
    int light_power;
    int abs_power;

    if (!src || !out_event || src->type != DM1_EVENT_LIGHT ||
        src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    light_power = (int)(int16_t)read_u16_le(&src->b_mapX);
    abs_power = light_power < 0 ? -light_power : light_power;
    /* ReDMCSB TIMELINE.C F0257:1747-1765 consumes only B.LightPower,
     * decrements its signed magnitude, and queues C70 at Priority=0.
     * DATA.C G0039 has 16 entries, so zero or magnitudes above 15 are
     * not a live source sequence Firestaff can faithfully materialize. */
    if (light_power == 0 || abs_power > RUNTIME_LIGHT_POWER_MAX) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = (int)((src->map_time >> 24) & 0xffu);
    /* F0802 serializes the generic B union from mapX/mapY. C70's B is
     * LightPower, so retain that source union spelling alongside aux0. */
    out_event->mapX = (int)(uint8_t)(light_power & 0xff);
    out_event->mapY = (int)(uint8_t)((uint16_t)light_power >> 8);
    out_event->aux0 = light_power;
    /* aux0 is live LightPower; retain C70 separately as F0802's receipt. */
    out_event->aux1 = DM1_EVENT_LIGHT;
    out_event->aux4 = 0;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_deferred_group_move_event(
    const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    uint16_t group_thing;
    int map_index;
    int group_index;
    if (!src || !world || !world->dungeon || !world->dungeon->maps ||
        !world->things || !world->things->groups || !out_event ||
        src->type != DM1_EVENT_MOVE_GROUP_SILENT || src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    group_thing = read_u16_le(&src->c_cell);
    group_index = (int)THING_GET_INDEX(group_thing);
    /* ReDMCSB MOVESENS.C F0265:169-192 owns C60/C61 as B.Location plus
     * C.Slot. Bind the raw C04 thing before publishing M10 state. */
    if (map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height ||
        THING_GET_TYPE(group_thing) != THING_TYPE_GROUP ||
        group_index >= world->things->groupCount) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_MOVE_GROUP_SILENT;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = group_index;
    out_event->aux1 = (int)group_thing;
    out_event->aux2 = src->type;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_fakewall_event(const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world, struct TimelineEvent_Compat *out_event) {
    const struct DungeonMapTiles_Compat *tiles;
    int map_index;
    int square_index;
    if (!src || !world || !world->dungeon || !world->dungeon->maps ||
        !world->dungeon->tiles || !out_event ||
        src->type != DM1_EVENT_FAKEWALL || src->c_effect > 2u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        !world->dungeon->tilesLoaded ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    tiles = &world->dungeon->tiles[map_index];
    square_index = (int)src->b_mapX *
        (int)world->dungeon->maps[map_index].height + (int)src->b_mapY;
    /* ReDMCSB TIMELINE.C F0242:820-870 changes only a fakewall square.
     * F0435 must not turn a source-shaped C07 into a generic attribute write. */
    if (!tiles->squareData || square_index < 0 ||
        (tiles->squareCount > 0 && square_index >= tiles->squareCount) ||
        ((tiles->squareData[square_index] & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
            DUNGEON_ELEMENT_FAKEWALL) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(out_event, 0, sizeof(*out_event)); out_event->kind = TIMELINE_EVENT_SQUARE_STATE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu; out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX; out_event->mapY = src->b_mapY; out_event->aux0 = DM1_EVENT_FAKEWALL;
    out_event->aux1 = src->c_effect; out_event->aux2 = DM1_EVENT_FAKEWALL; out_event->aux4 = src->priority;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

/* ReDMCSB TIMELINE.C F0242/F0244/F0245/F0250/F0251 consume C05/C06 and
 * C08-C10 through EVENT.B.Location plus EVENT.C.Cell/Effect.  M10 already
 * owns their live execution as TIMELINE_EVENT_SQUARE_STATE; F0435 must retain
 * that union instead of publishing a generic host event. */
static int materialize_original_pc34_square_state_event(
    const struct DM1_Event_V1 *src,
    struct TimelineEvent_Compat *out_event)
{
    int map_index;

    if (!src || !out_event || src->c_effect > DM1_EFFECT_TOGGLE) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    switch (src->type) {
    case DM1_EVENT_CORRIDOR:
        break;
    case DM1_EVENT_WALL:
        if (src->c_cell > 3u) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        break;
    case DM1_EVENT_TELEPORTER:
        break;
    case DM1_EVENT_PIT:
        break;
    case DM1_EVENT_DOOR:
        break;
    default:
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    map_index = (int)((src->map_time >> 24) & 0xffu);
    /* F0435 restores C05/C06/C08-C10 before TIMELINE.C dispatches them.
     * The established M10 consumer is the source-square validation owner;
     * importing here must not pre-run it or substitute a host transition. */

    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_SQUARE_STATE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->cell = src->c_cell;
    out_event->aux0 = src->type;
    out_event->aux1 = src->c_effect;
    out_event->aux2 = src->type;
    out_event->aux4 = src->priority;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_door_destruction_event(
    const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    const struct DungeonMapTiles_Compat *tiles;
    int map_index;
    int square_index;
    if (!src || !world || !world->dungeon || !world->dungeon->maps ||
        !world->dungeon->tiles || !out_event ||
        src->type != DM1_EVENT_DOOR_DESTRUCTION || src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        !world->dungeon->tilesLoaded ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    tiles = &world->dungeon->tiles[map_index];
    square_index = (int)src->b_mapX *
        (int)world->dungeon->maps[map_index].height + (int)src->b_mapY;
    /* ReDMCSB PROJEXPL.C F0232 creates C02 only for a closed, destructible
     * door, then TIMELINE.C F0243 consumes B.Location.  F0435 must not turn
     * a source-shaped C02 into a generic square-state mutation. */
    if (!tiles->squareData || square_index < 0 ||
        (tiles->squareCount > 0 && square_index >= tiles->squareCount) ||
        ((tiles->squareData[square_index] & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
            DUNGEON_ELEMENT_DOOR) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_DOOR_DESTRUCTION;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = DM1_EVENT_DOOR_DESTRUCTION;
    out_event->aux2 = DM1_EVENT_DOOR_DESTRUCTION;
    out_event->aux4 = src->priority;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_door_animation_event(
    const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    const struct DungeonMapTiles_Compat *tiles;
    int map_index;
    unsigned char square;

    if (!src || !world || !world->dungeon || !world->dungeon->maps ||
        !world->dungeon->tiles || !out_event ||
        src->type != DM1_EVENT_DOOR_ANIMATION ||
        src->c_effect > DOOR_EFFECT_CLEAR) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        !world->dungeon->tilesLoaded ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    tiles = &world->dungeon->tiles[map_index];
    if (!tiles->squareData) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    square = tiles->squareData[(size_t)src->b_mapX *
                               (size_t)world->dungeon->maps[map_index].height +
                               (size_t)src->b_mapY];
    if ((square >> 5) != DUNGEON_ELEMENT_DOOR) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB TIMELINE.C F0244:894-913 resolves C10's toggle to SET or
     * CLEAR before replacing its type with C01.  F0241:749-816 then owns
     * exactly B.Location and C.A.Effect; C.A.Cell is not read.  A saved C01
     * therefore cannot become a generic square event or carry a host cell. */
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_DOOR_ANIMATE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = DM1_EVENT_DOOR_ANIMATION;
    out_event->aux1 = src->c_effect;
    out_event->aux2 = DM1_EVENT_DOOR_ANIMATION;
    out_event->aux4 = src->priority;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_audible_group_move_event(
    const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    uint16_t group_thing;
    int map_index;
    int group_index;

    if (!src || !world || !world->dungeon || !world->dungeon->maps ||
        !world->things || !world->things->groups || !out_event ||
        src->type != DM1_EVENT_MOVE_GROUP_AUDIBLE || src->priority != 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map_index = (int)((src->map_time >> 24) & 0xffu);
    group_thing = read_u16_le(&src->c_cell);
    group_index = (int)THING_GET_INDEX(group_thing);
    if (map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height ||
        THING_GET_TYPE(group_thing) != THING_TYPE_GROUP ||
        group_index >= world->things->groupCount) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    /* ReDMCSB MOVESENS.C F0265 writes C61 as B.Location plus C04 Slot.
     * TIMELINE.C F0252 selects the audible M560 route from its type. */
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_MOVE_GROUP_AUDIBLE;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = group_index;
    out_event->aux1 = (int)group_thing;
    out_event->aux2 = DM1_EVENT_MOVE_GROUP_AUDIBLE;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_generator_reenable_event(
    const struct DM1_Event_V1 *src, const struct GameWorld_Compat *world,
    struct TimelineEvent_Compat *out_event)
{
    uint16_t thing;
    int map_index;
    int sensor_index = -1;
    int safety = 0;
    if (!src || !world || !world->dungeon || !world->things ||
        !world->things->loaded || !world->things->sensors || !out_event ||
        src->type != DM1_EVENT_ENABLE_GROUP_GENERATOR || src->priority != 0u) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    map_index = (int)((src->map_time >> 24) & 0xffu);
    if (map_index < 0 || map_index >= (int)world->dungeon->header.mapCount ||
        src->b_mapX >= world->dungeon->maps[map_index].width ||
        src->b_mapY >= world->dungeon->maps[map_index].height) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    /* ReDMCSB TIMELINE.C F0246:1020-1027 reads B.Location only and
     * re-enables the first disabled sensor found on that exact square. */
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(world->dungeon, world->things,
                                                      map_index, src->b_mapX, src->b_mapY);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int index = (int)THING_GET_INDEX(thing);
        if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR && index >= 0 &&
            index < world->things->sensorCount &&
            world->things->sensors[index].sensorType == RUNTIME_SENSOR_TYPE_DISABLED) {
            sensor_index = index;
            break;
        }
        thing = original_pc34_next_thing(world->things, thing);
    }
    if (sensor_index < 0) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    memset(out_event, 0, sizeof(*out_event));
    out_event->kind = TIMELINE_EVENT_GROUP_GENERATOR;
    out_event->fireAtTick = src->map_time & 0x00ffffffu;
    out_event->mapIndex = map_index;
    out_event->mapX = src->b_mapX;
    out_event->mapY = src->b_mapY;
    out_event->aux0 = GENERATOR_EVENT_AUX0_REENABLE;
    out_event->aux1 = sensor_index;
    out_event->aux2 = DM1_EVENT_ENABLE_GROUP_GENERATOR;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_timeline(
    const DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world,
    struct TimelineQueue_Compat *timeline)
{
    int i;
    uint16_t source_indices[DM1_EVENT_MAX_COUNT];

    if (!report || !world || !timeline) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->original_event_count < 0 ||
        report->original_event_count > DM1_EVENT_MAX_COUNT ||
        report->original_event_count > TIMELINE_QUEUE_CAPACITY ||
        report->original_event_count > report->decoded_event_count ||
        report->original_event_count > report->decoded_timeline_index_count) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (validate_original_pc34_timeline_references(report) !=
        DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines 2780-2800 loads the EVENT array
     * and timeline heap immediately after PARTY, then initializes the
     * optimized timeline management. Mirror that runtime handoff here:
     * the report preserves the raw source EVENT/TIMELINE bytes, while
     * GameWorld_Compat needs the equivalent M10 TimelineQueue. */
    for (i = 0; i < report->original_event_count; ++i) {
        if (report->timeline_indices[i] >=
            (uint16_t)report->decoded_event_count) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        source_indices[i] = report->timeline_indices[i];
    }
    /* C4 is a heap, not F0240's eventual pop order.  M10's compact queue
     * only sorts by tick, so materialize a source-comparator order here to
     * retain TIMELINE.C F0234/F0240's same-tick Type/Priority/index order. */
    for (i = 1; i < report->original_event_count; ++i) {
        uint16_t source_index = source_indices[i];
        int insert_index = i;

        while (insert_index > 0 &&
               original_pc34_timeline_event_is_before(
                   &report->events[source_index], (int)source_index,
                   &report->events[source_indices[insert_index - 1]],
                   (int)source_indices[insert_index - 1])) {
            source_indices[insert_index] = source_indices[insert_index - 1];
            --insert_index;
        }
        source_indices[insert_index] = source_index;
    }
    for (i = 0; i < report->original_event_count; ++i) {
        uint16_t source_index = report->timeline_indices[i];
        const struct DM1_Event_V1 *src = &report->events[source_index];
        if ((src->type == DM1_EVENT_EXPLOSION &&
             !original_pc34_explosion_event_slot_is_valid(
                 src, world, 0, NULL)) ||
            (src->type == DM1_EVENT_REMOVE_FLUXCAGE &&
             !original_pc34_explosion_event_slot_is_valid(
                 src, world, 1, NULL))) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
    }
    (void)F0720_TIMELINE_Init_Compat(timeline, report->original_game_time);
    for (i = 0; i < report->original_event_count; ++i) {
        uint16_t source_index = source_indices[i];
        const struct DM1_Event_V1 *src;
        struct TimelineEvent_Compat ev;
        int kind;
        if (source_index >= (uint16_t)report->decoded_event_count) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        src = &report->events[source_index];
        kind = timeline_kind_from_original_pc34_event_type(src->type);
        if (kind == TIMELINE_EVENT_INVALID) {
            /* ReDMCSB LOADSAVE.C F0435:2781-2800 restores the complete
             * EVENTS/TIMELINE pair, then TIMELINE.C F0651:100-124 rebuilds
             * its live management over every non-NONE event.  Dropping an
             * active source event here would publish a different runtime;
             * reject the candidate world until that event family has a real
             * M10 materialization route. */
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        if (src->type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS ||
            src->type == DM1_EVENT_MOVE_PROJECTILE) {
            if (materialize_original_pc34_projectile_event(
                    src, (int)source_index, world, &ev) !=
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_ENABLE_GROUP_GENERATOR) {
            if (materialize_original_pc34_generator_reenable_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (original_pc34_event_type_is_group_reaction(src->type)) {
            if (materialize_original_pc34_group_reaction_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_EXPLOSION) {
            if (materialize_original_pc34_explosion_event(src, world, &ev) !=
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_REMOVE_FLUXCAGE) {
            if (materialize_original_pc34_remove_fluxcage_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_LIGHT) {
            if (materialize_original_pc34_light_event(src, &ev) !=
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_MOVE_GROUP_SILENT) {
            if (materialize_original_pc34_deferred_group_move_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_FAKEWALL) {
            if (materialize_original_pc34_fakewall_event(src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK || !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            continue;
        }
        if (src->type == DM1_EVENT_CORRIDOR || src->type == DM1_EVENT_WALL ||
            src->type == DM1_EVENT_TELEPORTER || src->type == DM1_EVENT_PIT ||
            src->type == DM1_EVENT_DOOR) {
            if (materialize_original_pc34_square_state_event(
                    src, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_DOOR_DESTRUCTION) {
            if (materialize_original_pc34_door_destruction_event(src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK || !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            continue;
        }
        if (src->type == DM1_EVENT_DOOR_ANIMATION) {
            if (materialize_original_pc34_door_animation_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_MOVE_GROUP_AUDIBLE && src->priority == 0u) {
            if (materialize_original_pc34_audible_group_move_event(
                    src, world, &ev) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
                !F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_INVISIBILITY) {
            /* ReDMCSB MENU.C F0412:1922-1964 creates C71 with Priority
             * zero, while TIMELINE.C C71:1953-1964 consumes no B/C union
             * arm. Keep those bytes outside the live contract. */
            if (src->priority != 0u) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_INVISIBILITY;
            ev.aux2 = DM1_EVENT_INVISIBILITY;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_THIEVES_EYE) {
            /* ReDMCSB MENU.C action F0407:1542-1546 and spell F0412 set
             * C73 Priority to zero. TIMELINE.C C73:1972-1974 consumes no
             * B/C union arm, so those source bytes have no live owner. */
            if (src->priority != 0u) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_THIEVES_EYE;
            ev.aux2 = DM1_EVENT_THIEVES_EYE;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_PARTY_SHIELD) {
            int defense = (int)(int16_t)read_u16_le(&src->b_mapX);
            /* ReDMCSB MENU.C F0412:1922-1992 creates C74 with zero
             * Priority and a positive B.Defense. TIMELINE.C C74:1975-1976
             * consumes only that signed defense; C has no event union arm. */
            if (src->priority != 0u || defense <= 0) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_PARTY_SHIELD;
            ev.aux1 = defense;
            ev.aux2 = DM1_EVENT_PARTY_SHIELD;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_POISON_CHAMPION) {
            int attack = (int)read_u16_le(&src->b_mapX);
            /* ReDMCSB CHAMPION.C F0322:1954-1960 creates C75 with a live
             * champion Priority and positive unsigned B.Attack. TIMELINE.C
             * C75:1991-1993 reads exactly those fields, not C. */
            if (src->priority >= CHAMPION_MAX_PARTY || attack <= 0 ||
                !world->party.champions[src->priority].present) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_POISON_CHAMPION;
            ev.aux1 = attack;
            ev.aux2 = DM1_EVENT_POISON_CHAMPION;
            ev.aux4 = src->priority;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_SPELLSHIELD) {
            int defense = (int)(int16_t)read_u16_le(&src->b_mapX);
            /* ReDMCSB MENU.C F0403:1099-1115 creates C77 with zero
             * Priority and positive B.Defense. TIMELINE.C C77:1985-1986
             * consumes only that defense; C has no event union arm. */
            if (src->priority != 0u || defense <= 0) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_SPELLSHIELD;
            ev.aux1 = defense;
            ev.aux2 = DM1_EVENT_SPELLSHIELD;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_FIRESHIELD) {
            int defense = (int)(int16_t)read_u16_le(&src->b_mapX);
            /* ReDMCSB MENU.C F0403:1099-1115 creates C78 with zero
             * Priority and positive B.Defense. TIMELINE.C C78:1988-1989
             * consumes only that defense; C has no event union arm. */
            if (src->priority != 0u || defense <= 0) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_FIRESHIELD;
            ev.aux1 = defense;
            ev.aux2 = DM1_EVENT_FIRESHIELD;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            continue;
        }
        if (src->type == DM1_EVENT_FOOTPRINTS) {
            /* ReDMCSB MENU.C F0412:1922-1992 creates C79 with zero
             * Priority. TIMELINE.C C79:1998-2000 consumes no B/C union. */
            if (src->priority != 0u) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_FOOTPRINTS;
            ev.aux2 = DM1_EVENT_FOOTPRINTS;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            continue;
        }
        if (src->type == DM1_EVENT_WATCHDOG) {
            /* ReDMCSB TIMELINE.C F0256:1710-1715 creates C53 from only
             * Type and Map_Time, and the C53 dispatch re-arms that same
             * record 300 ticks later.  B/C and Priority are not initialized
             * by this source path, so keep them out of the live receipt. */
            if (((src->map_time >> 24) & 0xffu) != 0u) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_WATCHDOG;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.aux0 = DM1_EVENT_WATCHDOG;
            ev.aux2 = DM1_EVENT_WATCHDOG;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_PLAY_SOUND) {
            int sound_index = (int)(int16_t)read_u16_le(&src->c_cell);

            /* ReDMCSB SOUND.C F0064:1523-1543 rejects negative SoundIndex
             * before it can schedule C20.  The delayed event owns Priority,
             * B.Location and signed C.SoundIndex; C.Cell/Effect is not a
             * separate union arm for this family. */
            if (sound_index < 0) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_PLAY_SOUND;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.aux0 = sound_index;
            ev.aux2 = DM1_EVENT_PLAY_SOUND;
            ev.aux4 = src->priority;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_CPSE) {
            /* ReDMCSB NEWMAP.C:45-77 schedules C22 with Map_Time only.
             * TIMELINE.C:1920-1925 has no game-side action in a
             * NOCOPYPROTECTION build.  Keep its typed receipt so an
             * original save survives the handoff, but never interpret the
             * unowned Priority/B/C bytes as dungeon data. */
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_CPSE_CHECK;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_CPSE;
            ev.aux2 = DM1_EVENT_CPSE;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        if (src->type == DM1_EVENT_CHAMPION_SHIELD) {
            int defense = (int)(int16_t)read_u16_le(&src->b_mapX);
            if (src->priority >= CHAMPION_MAX_PARTY ||
                !world->party.champions[src->priority].present) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            /* ReDMCSB TIMELINE.C C72:1964-1967 consumes Priority and
             * signed B.Defense only. C is not part of this event union. */
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
            ev.fireAtTick = src->map_time & 0x00ffffffu;
            ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
            ev.aux0 = DM1_EVENT_CHAMPION_SHIELD;
            ev.aux1 = defense;
            ev.aux2 = DM1_EVENT_CHAMPION_SHIELD;
            ev.aux4 = src->priority;
            if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        memset(&ev, 0, sizeof(ev));
        ev.kind = kind;
        ev.fireAtTick = src->map_time & 0x00ffffffu;
        ev.mapIndex = (int)((src->map_time >> 24) & 0xffu);
        ev.aux0 = src->type;
        ev.aux4 = src->priority;
        if (src->type == DM1_EVENT_ENABLE_CHAMPION_ACTION) {
            /* ReDMCSB DEFS.H EVENT.B.SlotOrdinal overlays only B's first
             * byte. CHAMPION.C F0330 creates ordinal zero; MENU.C F0407
             * changes that pending C11 to M000_INDEX_TO_ORDINAL(C01), i.e.
             * two, after a successful throw. No other C11 SlotOrdinal
             * producer exists in the PC34 source, so reject every other
             * value rather than mapping raw save bytes into host inventory. */
            if (src->priority >= CHAMPION_MAX_PARTY ||
                (src->b_mapX != 0u && src->b_mapX != 2u)) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            /* B's second byte and C are outside C11's semantic union arm.
             * Retain them for F0433 round-trip, never as live coordinates. */
            ev.aux1 = src->b_mapX;
            ev.aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
            ev.mapX = src->b_mapY;
            ev.mapY = src->c_cell;
            ev.cell = src->c_effect;
        } else if (src->type == DM1_EVENT_HIDE_DAMAGE_RECEIVED) {
            /* ReDMCSB TIMELINE.C F0254 consumes only C12's Map_Time and
             * Priority champion index. B/C have no C12 union owner. */
            ev.mapX = 0;
            ev.mapY = 0;
            ev.cell = 0;
            ev.aux1 = 0;
            ev.aux2 = DM1_EVENT_HIDE_DAMAGE_RECEIVED;
        } else if (src->type >= DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE &&
                   src->type <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            /* ReDMCSB PROJEXPL.C F0231 queues C31 by square. The saved
             * EVENT has no Firestaff group index, so M10 resolves the live
             * group from B.Location when dispatch resumes. */
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.cell = 0;
            ev.aux0 = -1;
            ev.aux1 = -1;
            ev.aux2 = src->type;
        } else if (src->type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS ||
                   src->type == DM1_EVENT_MOVE_PROJECTILE) {
            uint16_t thing = read_u16_le(&src->b_mapX);
            uint16_t projectile_data = read_u16_le(&src->c_cell);
            /* ReDMCSB PROJEXPL.C F0212:82-91 stores a C14 projectile
             * THING in B.Slot, and F0219:682-717 uses both its index and
             * M011_CELL. The source C.Projectile map position is not B. */
            ev.mapX = (int)(projectile_data & 0x001fu);
            ev.mapY = (int)((projectile_data >> 5) & 0x001fu);
            ev.cell = (int)(thing >> 14);
            ev.aux0 =
                (((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                  DM1_PC34_THING_TYPE_MASK) == THING_TYPE_PROJECTILE)
                    ? (int)(thing & DM1_PC34_THING_INDEX_MASK)
                    : -1;
            ev.aux1 = 0;
            if (src->type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS) {
                /* C48 must survive one reload as an impact-suppressed first
                 * move. aux4 is the type marker; retain the source priority
                 * separately because EVENT.A.A.Priority is still persisted. */
                ev.aux3 = src->priority;
                ev.aux4 = DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS;
            }
        } else if (src->type == DM1_EVENT_EXPLOSION) {
            uint16_t thing = read_u16_le(&src->c_cell);
            /* ReDMCSB PROJEXPL.C F0220 lines 803-806 follows C25's
             * EVENT.C.Slot explosion THING. M10 dispatches that instance
             * through aux0, not through the raw C25 type value. */
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.cell = 0;
            ev.aux0 =
                (((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                  DM1_PC34_THING_TYPE_MASK) == THING_TYPE_EXPLOSION)
                    ? (int)(thing & DM1_PC34_THING_INDEX_MASK)
                    : -1;
            ev.aux1 = 0;
        } else if (original_pc34_event_type_is_status_timeout(src->type)) {
            ev.aux1 = (int)read_u16_le(&src->b_mapX);
            ev.cell = src->c_cell;
            ev.aux2 = src->c_effect;
        } else if (src->type == DM1_EVENT_REMOVE_FLUXCAGE) {
            uint16_t thing = read_u16_le(&src->c_cell);
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.cell = EXPLOSION_CELL_CENTERED;
            ev.aux0 =
                (((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                  DM1_PC34_THING_TYPE_MASK) == THING_TYPE_EXPLOSION)
                    ? (int)(thing & DM1_PC34_THING_INDEX_MASK)
                    : (int)thing;
            ev.aux1 = C050_EXPLOSION_FLUXCAGE;
        } else {
            ev.mapX = src->b_mapX;
            ev.mapY = src->b_mapY;
            ev.cell = src->c_cell;
            ev.aux1 = src->c_effect;
        }
        if (!F0721_TIMELINE_Schedule_Compat(timeline, &ev)) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int materialize_original_pc34_party_status(
    const DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world)
{
    int thieves_eye_count;
    int footprints_count;
    int invisibility_count;
    int party_shield_defense;
    int party_fire_shield_defense;
    int party_spell_shield_defense;
    int magical_light_amount;

    if (!report || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    thieves_eye_count = report->imported_event73_count_thieves_eye;
    footprints_count = report->imported_event79_count_footprints;
    invisibility_count = report->imported_event71_count_invisibility;
    party_shield_defense = report->imported_party_shield_defense;
    party_fire_shield_defense = report->imported_party_fire_shield_defense;
    party_spell_shield_defense = report->imported_party_spell_shield_defense;
    magical_light_amount = report->imported_magical_light_amount;
    if (thieves_eye_count < 0 || thieves_eye_count > 0xff ||
        footprints_count < 0 || footprints_count > 0xff ||
        invisibility_count < 0 || invisibility_count > 0xff) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB LOADSAVE.C F0435 restores G0407_s_Party before EVENTS and
     * TIMELINE. DUNVIEW.C F0111/F0127 consumes Event73Count_ThievesEye
     * directly for the wall/door material, while TIMELINE.C decrements C71,
     * C73, and C79 at expiry. Keep Firestaff's two runtime mirrors equal at
     * the transactional handoff boundary; do not synthesize active state from
     * an event that may already be due. */
    world->magic.event71CountInvisibility = invisibility_count;
    world->lifecycle.status.invisibilityCount = (uint16_t)invisibility_count;
    world->magic.partyShieldDefense = party_shield_defense;
    world->lifecycle.status.partyShieldDefense =
        (int16_t)party_shield_defense;
    world->magic.fireShieldDefense = party_fire_shield_defense;
    world->lifecycle.status.partyFireShieldDefense =
        (int16_t)party_fire_shield_defense;
    world->magic.spellShieldDefense = party_spell_shield_defense;
    world->lifecycle.status.partySpellShieldDefense =
        (int16_t)party_spell_shield_defense;
    world->magic.magicalLightAmount = magical_light_amount;
    world->magic.event73CountThievesEye = thieves_eye_count;
    world->lifecycle.status.thievesEyeCount = (uint16_t)thieves_eye_count;
    world->magic.event79CountFootprints = footprints_count;
    world->lifecycle.status.footprintsCount = (uint16_t)footprints_count;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int import_original_pc34_external_portraits(
    const uint8_t *bytes,
    size_t size,
    size_t cursor,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    int slot;
    int count;
    const size_t portrait_bytes = SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;

    /* ReDMCSB LOADSAVE.C F0435 lines ~2810-2816 reads all four fixed
     * 32x29 portrait payloads after the five save parts. Validate the
     * whole section before copying portrait 0, so truncation cannot leave
     * a partially updated party. */
    if (!bytes || cursor > size || portrait_bytes > size - cursor) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    count = (out_state && out_state->party) ? out_state->party->championCount : 0;
    if (count < 0) count = 0;
    if (count > CHAMPION_MAX_PARTY) count = CHAMPION_MAX_PARTY;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (out_state && out_state->party && slot < count) {
            memcpy(out_state->party->champions[slot].portraitBitmap,
                   bytes + cursor,
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
            out_state->party->champions[slot].portraitBitmapValid = 1;
        }
        cursor += CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    }
    if (out_report) {
        out_report->external_portrait_byte_count = (uint32_t)portrait_bytes;
        out_report->external_portrait_byte_offset = (uint32_t)cursor -
            (uint32_t)portrait_bytes;
        out_report->external_portrait_fingerprint =
            dm1_original_save_hash_bytes(
                bytes + out_report->external_portrait_byte_offset,
                portrait_bytes);
        out_report->external_portrait_payload_count = CHAMPION_MAX_PARTY;
        out_report->external_portrait_imported_count = count;
    }
    return SAVEGAME_PC34_OK;
}

static uint32_t original_pc34_tail_fingerprint(const uint8_t *bytes,
                                               size_t count)
{
    uint32_t fingerprint = 2166136261u;
    size_t i;

    for (i = 0u; i < count; ++i) {
        fingerprint ^= bytes[i];
        fingerprint *= 16777619u;
    }
    return fingerprint;
}

/* ReDMCSB LOADSAVE.C F0435:2826 calls F0434 after the five save parts.
 * F0434's dungeon loader rejects map descriptors whose raw-map span falls
 * outside the saved raw-map block. Keep the receipt fail-closed before it
 * can describe a tail that the actual F0434/F0504 materialization rejects. */
static int validate_original_pc34_dungeon_tail_map_spans(
    const uint8_t *tail,
    int map_count,
    size_t map_descriptors_offset,
    size_t raw_map_offset,
    size_t raw_map_byte_count)
{
    int map_index;

    if (!tail || map_count <= 0 || map_count > DUNGEON_MAX_MAPS) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    for (map_index = 0; map_index < map_count; ++map_index) {
        const uint8_t *map = tail + map_descriptors_offset +
            (size_t)map_index * DUNGEON_MAP_DESC_SIZE;
        uint16_t raw_bitfield_a = read_u16_le(map + 8u);
        uint16_t raw_bitfield_c = read_u16_le(map + 12u);
        size_t raw_map_data_offset = (size_t)read_u16_le(map);
        size_t width = (size_t)((raw_bitfield_a >> 6) & 0x1fu) + 1u;
        size_t height = (size_t)((raw_bitfield_a >> 11) & 0x1fu) + 1u;
        size_t creature_type_count = (size_t)((raw_bitfield_c >> 4) & 0x0fu);
        size_t map_span = width * height + creature_type_count;

        if (raw_map_offset > SIZE_MAX - raw_map_data_offset ||
            raw_map_data_offset > raw_map_byte_count ||
            map_span > raw_map_byte_count - raw_map_data_offset) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
    }
    return SAVEGAME_PC34_OK;
}

/* ReDMCSB DUNGEON.C F0160 uses G0280's entry for each map column as the
 * index of that column's first SquareFirstThings row. LOADSAVE.C F0433
 * serializes the table before the SFT payload; F0435 reads it verbatim.
 * Firestaff's M10 lookup can reconstruct this table from raw tiles, but an
 * original-save handoff must first prove the persisted table is equivalent.
 */
static int validate_original_pc34_dungeon_tail_columns(
    const uint8_t *tail,
    int map_count,
    size_t map_descriptors_offset,
    size_t columns_offset,
    size_t raw_map_offset,
    size_t raw_map_byte_count,
    int square_first_thing_count,
    uint32_t *out_terminal_count)
{
    uint32_t cumulative = 0u;
    int map_index;
    int column_index = 0;

    if (!tail || map_count <= 0 || map_count > DUNGEON_MAX_MAPS ||
        square_first_thing_count < 0) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    for (map_index = 0; map_index < map_count; ++map_index) {
        const uint8_t *map = tail + map_descriptors_offset +
            (size_t)map_index * DUNGEON_MAP_DESC_SIZE;
        uint16_t raw_bitfield_a = read_u16_le(map + 8u);
        size_t map_offset = (size_t)read_u16_le(map + 0u);
        size_t width = (size_t)((raw_bitfield_a >> 6) & 0x1fu) + 1u;
        size_t height = (size_t)((raw_bitfield_a >> 11) & 0x1fu) + 1u;
        size_t x;

        if (map_offset > raw_map_byte_count ||
            width > raw_map_byte_count - map_offset ||
            height != 0u && width > SIZE_MAX / height ||
            width * height > raw_map_byte_count - map_offset) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        for (x = 0u; x < width; ++x, ++column_index) {
            size_t y;
            uint16_t saved_cumulative = read_u16_le(
                tail + columns_offset + (size_t)column_index * 2u);
            if (cumulative > UINT16_MAX ||
                saved_cumulative != (uint16_t)cumulative) {
                return SAVEGAME_PC34_ERROR_BAD_SIZE;
            }
            for (y = 0u; y < height; ++y) {
                uint8_t square = tail[raw_map_offset + map_offset +
                                      x * height + y];
                if ((square & DUNGEON_SQUARE_MASK_THING_LIST) != 0u) {
                    ++cumulative;
                }
            }
        }
    }
    /* SquareFirstThingCount is the persisted allocation length. The source
     * table addresses the live thing-list rows and may legitimately leave
     * spare slots at its tail, so only reject a column table that indexes
     * beyond the stored SFT payload. */
    if (cumulative > (uint32_t)square_first_thing_count) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    if (out_terminal_count) {
        *out_terminal_count = cumulative;
    }
    return SAVEGAME_PC34_OK;
}

static int decode_original_pc34_dungeon_tail(
    const uint8_t *bytes,
    size_t size,
    size_t cursor,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    const uint8_t *tail;
    size_t tail_size;
    size_t off;
    int map_count;
    int column_count = 0;
    int thing_data_bytes = 0;
    size_t map_descriptors_offset;
    size_t columns_offset;
    size_t square_first_things_offset;
    size_t text_data_offset;
    size_t thing_data_offset;
    size_t raw_map_offset;
    int type;

    if (!bytes || !out_report || cursor >= size) {
        return SAVEGAME_PC34_OK;
    }
    tail = bytes + cursor;
    tail_size = size - cursor;
    if (tail_size < DUNGEON_HEADER_SIZE + 2u) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }

    map_count = (int)tail[4u];
    if (map_count <= 0 || map_count > DUNGEON_MAX_MAPS) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    off = DUNGEON_HEADER_SIZE;
    map_descriptors_offset = off;
    if (off + (size_t)map_count * DUNGEON_MAP_DESC_SIZE + 2u > tail_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    for (type = 0; type < map_count; ++type) {
        const uint8_t *map = tail + off + (size_t)type * DUNGEON_MAP_DESC_SIZE;
        uint16_t bitfield_a = read_u16_le(map + 8u);
        int width = (int)((bitfield_a >> 6) & 0x1fu) + 1;
        column_count += width;
    }
    off += (size_t)map_count * DUNGEON_MAP_DESC_SIZE;
    if (off + (size_t)column_count * 2u + 2u > tail_size) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    columns_offset = off;
    off += (size_t)column_count * 2u;
    square_first_things_offset = off;

    {
        int square_first_thing_count = (int)read_u16_le(tail + 10u);
        int text_data_word_count = (int)read_u16_le(tail + 6u);
        int raw_map_bytes = (int)read_u16_le(tail + 2u);
        uint16_t expected_checksum;
        uint16_t actual_checksum;
        uint32_t terminal_sft_count = 0u;
        if (square_first_thing_count < 0 || text_data_word_count < 0 ||
            raw_map_bytes < 0) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        if (off + (size_t)square_first_thing_count * 2u +
                  (size_t)text_data_word_count * 2u + 2u > tail_size) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        off += (size_t)square_first_thing_count * 2u;
        text_data_offset = off;
        off += (size_t)text_data_word_count * 2u;
        thing_data_offset = off;
        for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
            int count = (int)read_u16_le(tail + 12u + (size_t)type * 2u);
            int bytes_for_type = count * (int)s_thingDataByteCount[type];
            if (count < 0 || bytes_for_type < 0) {
                return SAVEGAME_PC34_ERROR_BAD_SIZE;
            }
            thing_data_bytes += bytes_for_type;
        }
        if (off + (size_t)thing_data_bytes + 2u > tail_size) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        off += (size_t)thing_data_bytes;
        raw_map_offset = off;
        if (off + (size_t)raw_map_bytes + 2u != tail_size) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        off += (size_t)raw_map_bytes;
        expected_checksum = read_u16_le(tail + off);
        {
            uint8_t* staged_tail = (uint8_t*)malloc(off ? off : 1u);
            size_t read_cursor = 0u;
            uint16_t running_checksum = 0u;
            size_t section_start;

            if (!staged_tail ||
                !dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                    tail, off, &read_cursor, staged_tail + read_cursor,
                    DUNGEON_HEADER_SIZE, &running_checksum) ||
                !dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                    tail, off, &read_cursor, staged_tail + read_cursor,
                    (size_t)map_count * DUNGEON_MAP_DESC_SIZE,
                    &running_checksum) ||
                !dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                    tail, off, &read_cursor, staged_tail + read_cursor,
                    square_first_things_offset - columns_offset,
                    &running_checksum) ||
                !dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                    tail, off, &read_cursor, staged_tail + read_cursor,
                    text_data_offset - square_first_things_offset,
                    &running_checksum) ||
                !dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                    tail, off, &read_cursor, staged_tail + read_cursor,
                    thing_data_offset - text_data_offset, &running_checksum)) {
                free(staged_tail);
                return SAVEGAME_PC34_ERROR_BAD_SIZE;
            }
            section_start = thing_data_offset;
            for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
                size_t type_byte_count = (size_t)read_u16_le(
                    tail + 12u + (size_t)type * 2u) *
                    (size_t)s_thingDataByteCount[type];
                if (!dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                        tail, off, &read_cursor, staged_tail + read_cursor,
                        type_byte_count, &running_checksum)) {
                    free(staged_tail);
                    return SAVEGAME_PC34_ERROR_BAD_SIZE;
                }
                section_start += type_byte_count;
            }
            if (section_start != raw_map_offset ||
                !dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
                    tail, off, &read_cursor, staged_tail + read_cursor,
                    (size_t)raw_map_bytes, &running_checksum) ||
                read_cursor != off) {
                free(staged_tail);
                return SAVEGAME_PC34_ERROR_BAD_SIZE;
            }
            actual_checksum = running_checksum;
            free(staged_tail);
        }
        out_report->dungeon_tail_present = 1;
        out_report->dungeon_tail_byte_count = (uint32_t)tail_size;
        out_report->dungeon_tail_expected_checksum = expected_checksum;
        out_report->dungeon_tail_actual_checksum = actual_checksum;
        out_report->dungeon_tail_checksum_ok =
            (expected_checksum == actual_checksum);
        out_report->dungeon_tail_map_count = map_count;
        out_report->dungeon_tail_column_count = column_count;
        out_report->dungeon_tail_square_first_thing_count =
            square_first_thing_count;
        out_report->dungeon_tail_text_data_word_count =
            text_data_word_count;
        out_report->dungeon_tail_text_string_count = (int)read_u16_le(
            tail + 12u + (size_t)THING_TYPE_TEXTSTRING * 2u);
        out_report->dungeon_tail_thing_data_byte_count =
            (uint32_t)thing_data_bytes;
        out_report->dungeon_tail_raw_map_data_byte_count =
            (uint32_t)raw_map_bytes;
        if (!out_report->dungeon_tail_checksum_ok) {
            return SAVEGAME_PC34_ERROR_BAD_CHECKSUM;
        }
        if (validate_original_pc34_dungeon_tail_columns(
                tail, map_count, map_descriptors_offset, columns_offset,
                raw_map_offset, (size_t)raw_map_bytes,
                square_first_thing_count, &terminal_sft_count) !=
            SAVEGAME_PC34_OK) {
            return SAVEGAME_PC34_ERROR_BAD_SIZE;
        }
        out_report->dungeon_tail_column_table_valid = 1;
        out_report->dungeon_tail_column_terminal_sft_count =
            terminal_sft_count;
        out_report->dungeon_tail_column_table_fingerprint =
            original_pc34_tail_fingerprint(
                tail + columns_offset, (size_t)column_count * 2u);
        out_report->dungeon_tail_square_first_thing_fingerprint =
            original_pc34_tail_fingerprint(
                tail + square_first_things_offset,
                (size_t)square_first_thing_count * 2u);
        out_report->dungeon_tail_fingerprint =
            original_pc34_tail_fingerprint(tail, tail_size);
    }
    return SAVEGAME_PC34_OK;
}

static size_t original_pc34_dungeon_tail_cursor(const uint8_t *bytes,
                                                size_t size)
{
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int part;

    if (!bytes || size < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return 0u;
    }
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t part_size;
        if (cursor + 2u > size) {
            return 0u;
        }
        part_size = read_u16_le(bytes + cursor);
        cursor += 2u;
        if (cursor + (size_t)part_size > size) {
            return 0u;
        }
        cursor += (size_t)part_size;
    }
    if (cursor + CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT >
        size) {
        return 0u;
    }
    cursor += CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT;
    return cursor;
}

static int materialize_original_pc34_dungeon_tail(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    DM1OriginalSavePC34HandoffReport *report)
{
    size_t cursor;
    struct DungeonDatState_Compat *dungeon;
    struct DungeonThings_Compat *things;

    if (!bytes || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    cursor = original_pc34_dungeon_tail_cursor(bytes, size);
    if (cursor == 0u || cursor >= size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
    }
    if (size - cursor > (size_t)((int)0x7fffffff)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    dungeon = (struct DungeonDatState_Compat *)calloc(1u, sizeof(*dungeon));
    things = (struct DungeonThings_Compat *)calloc(1u, sizeof(*things));
    if (!dungeon || !things) {
        free(dungeon);
        free(things);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    if (!F0504_DUNGEON_LoadTailBuffer_Compat(
            bytes + cursor, (int)(size - cursor), dungeon, things)) {
        F0504_DUNGEON_FreeThingData_Compat(things);
        F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
        free(things);
        free(dungeon);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    /* F0504 restores the header/maps/SquareFirstThings but its tail-buffer
     * path does not retain ReDMCSB G0280. F0435 must own that source table:
     * it is written by F0433 immediately after MAP[] and is later consumed
     * by F0160 when resolving square thing chains. */
    if (report && report->dungeon_tail_present) {
        const size_t columns_offset = DUNGEON_HEADER_SIZE +
            (size_t)dungeon->header.mapCount * DUNGEON_MAP_DESC_SIZE;
        const int column_count = (int)report->dungeon_tail_column_count;
        unsigned short *columns;
        int i;

        if (column_count <= 0 ||
            columns_offset > size - cursor ||
            (size_t)column_count >
                (size - cursor - columns_offset) / sizeof(*columns)) {
            F0504_DUNGEON_FreeThingData_Compat(things);
            F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
            free(things);
            free(dungeon);
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        columns = (unsigned short *)calloc((size_t)column_count,
                                           sizeof(*columns));
        if (!columns) {
            F0504_DUNGEON_FreeThingData_Compat(things);
            F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
            free(things);
            free(dungeon);
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
        }
        for (i = 0; i < column_count; ++i) {
            columns[i] = read_u16_le(bytes + cursor + columns_offset +
                                     (size_t)i * sizeof(*columns));
        }
        free(dungeon->columnsCumulativeSquareFirstThingCount);
        dungeon->columnsCumulativeSquareFirstThingCount = columns;
        dungeon->dungeonColumnCount = column_count;
    }
    {
        int i;
        for (i = 0; i < (int)dungeon->header.squareFirstThingCount; ++i) {
            uint16_t thing = things->squareFirstThings[i];
            unsigned int type;
            unsigned int index;

            if (thing == THING_NONE || thing == THING_ENDOFLIST) {
                continue;
            }
            type = THING_GET_TYPE(thing);
            index = THING_GET_INDEX(thing);
            if (type >= DUNGEON_THING_TYPE_COUNT ||
                index >= things->thingCounts[type]) {
                F0504_DUNGEON_FreeThingData_Compat(things);
                F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
                free(things);
                free(dungeon);
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
        }
        for (i = 0; i < things->textStringCount; ++i) {
            if (!things->textStrings ||
                things->textStrings[i].textDataWordOffset >=
                    (uint16_t)things->textDataWordCount) {
                F0504_DUNGEON_FreeThingData_Compat(things);
                F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
                free(things);
                free(dungeon);
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
        }
    }

    if (world->ownsDungeon) {
        F0883_WORLD_Free_Compat(world);
    }
    world->dungeon = dungeon;
    world->things = things;
    world->ownsDungeon = 1;
    world->dungeonFingerprint =
        report ? (((uint32_t)report->dungeon_tail_actual_checksum << 16) ^
                  report->dungeon_tail_byte_count) : 0u;
    if (report) {
        report->dungeon_tail_runtime_imported = 1;
    }
    (void)F0502b_DUNGEON_CheckBug0_08SftOverfill_Compat(dungeon, things);
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static uint32_t original_pc34_u16_array_fingerprint(
    const unsigned short *values,
    int count)
{
    uint32_t fingerprint = 2166136261u;
    int i;

    if (count < 0) {
        return 0u;
    }
    for (i = 0; i < count; ++i) {
        const uint16_t value = values ? values[i] : 0u;
        fingerprint ^= (uint8_t)(value & 0xffu);
        fingerprint *= 16777619u;
        fingerprint ^= (uint8_t)(value >> 8);
        fingerprint *= 16777619u;
    }
    return fingerprint;
}

/* F0433 writes G0280 then SquareFirstThings in the dungeon tail; F0435
 * recreates both before F0651 exposes restored EVENTS/TIMELINE. The earlier
 * parse gate proves byte shape, but adoption must also prove that the actual
 * owned dungeon and its C3/C4-backed timeline are the same source state. */
static int validate_original_pc34_tail_runtime_receipt(
    DM1OriginalSavePC34HandoffReport *report,
    const struct GameWorld_Compat *world)
{
    uint32_t columns_fingerprint;
    uint32_t square_first_things_fingerprint;
    uint32_t timeline_fingerprint;

    if (!report || !world) {
        return 0;
    }
    if (!report->dungeon_tail_present) {
        return 1;
    }
    if (!report->dungeon_tail_checksum_ok ||
        !report->dungeon_tail_column_table_valid ||
        !report->dungeon_tail_runtime_imported || !world->ownsDungeon ||
        !world->dungeon || !world->things ||
        (world->dungeon->header.squareFirstThingCount != 0u &&
         !world->things->squareFirstThings) ||
        world->dungeon->header.mapCount != report->dungeon_tail_map_count ||
        world->dungeon->dungeonColumnCount != report->dungeon_tail_column_count ||
        world->dungeon->header.squareFirstThingCount !=
            (uint16_t)report->dungeon_tail_square_first_thing_count ||
        !world->pc34OriginalC3C4ReceiptValid ||
        world->pc34OriginalC3C4RuntimeEventCount !=
            (uint32_t)world->timeline.count) {
        return 0;
    }

    columns_fingerprint = original_pc34_u16_array_fingerprint(
        world->dungeon->columnsCumulativeSquareFirstThingCount,
        world->dungeon->dungeonColumnCount);
    square_first_things_fingerprint = original_pc34_u16_array_fingerprint(
        world->things->squareFirstThings,
        (int)world->dungeon->header.squareFirstThingCount);
    timeline_fingerprint = original_pc34_timeline_runtime_fingerprint(
        &world->timeline);
    if (columns_fingerprint == 0u || square_first_things_fingerprint == 0u ||
        timeline_fingerprint == 0u ||
        columns_fingerprint != report->dungeon_tail_column_table_fingerprint ||
        square_first_things_fingerprint !=
            report->dungeon_tail_square_first_thing_fingerprint ||
        timeline_fingerprint != report->timeline_runtime_fingerprint ||
        timeline_fingerprint != world->pc34OriginalTimelineFingerprint) {
        return 0;
    }
    report->dungeon_tail_runtime_column_table_fingerprint =
        columns_fingerprint;
    report->dungeon_tail_runtime_square_first_thing_fingerprint =
        square_first_things_fingerprint;
    report->dungeon_tail_runtime_timeline_fingerprint = timeline_fingerprint;
    report->dungeon_tail_runtime_receipt_valid = 1;
    return 1;
}

/* A C13 rebirth timer names M516_CHAMPIONS through Priority.  F0435 has to
 * retain that exact source-selected party while it materializes the tail and
 * then publishes C3/C4.  Count matching is insufficient: leader changes or
 * an inventory-slot remap can leave a timer pointing at the wrong champion. */
static int validate_original_pc34_c13_party_runtime_receipt(
    DM1OriginalSavePC34HandoffReport *report,
    const struct GameWorld_Compat *world)
{
    uint32_t metadata_fingerprint;
    uint32_t state_fingerprint;
    uint32_t timeline_fingerprint;
    uint32_t c13_fingerprint;
    int c13_admitted_count;
    int source_index;

    if (!report || !world) {
        return 0;
    }
    /* A tail-less PC34 save can still resume against the caller's existing
     * dungeon, but it cannot make this source-owned C13 party/tail claim. */
    if (!report->dungeon_tail_present) {
        return 1;
    }
    if (!report->dungeon_tail_runtime_receipt_valid ||
        !report->c3_c4_receipt_valid || !world->ownsDungeon ||
        !world->dungeon || !world->things ||
        report->source_party_champion_metadata_fingerprint == 0u ||
        report->source_party_champion_state_fingerprint == 0u ||
        world->party.championCount != report->imported_champion_count ||
        world->party.activeChampionIndex !=
            report->imported_active_champion_index) {
        return 0;
    }
    metadata_fingerprint = original_pc34_runtime_party_metadata_fingerprint(
        &world->party);
    state_fingerprint = original_pc34_runtime_party_state_fingerprint(
        &world->party);
    timeline_fingerprint = original_pc34_timeline_runtime_fingerprint(
        &world->timeline);
    if (metadata_fingerprint == 0u || state_fingerprint == 0u ||
        timeline_fingerprint == 0u ||
        metadata_fingerprint !=
            report->source_party_champion_metadata_fingerprint ||
        state_fingerprint != report->source_party_champion_state_fingerprint ||
        timeline_fingerprint != report->timeline_runtime_fingerprint ||
        timeline_fingerprint != world->pc34OriginalTimelineFingerprint) {
        return 0;
    }
    for (source_index = 0; source_index < report->decoded_event_count;
         ++source_index) {
        const struct DM1_Event_V1 *event = &report->events[source_index];

        if (event->type == DM1_EVENT_VI_ALTAR_REBIRTH &&
            (event->priority >= (uint16_t)world->party.championCount ||
             !world->party.champions[event->priority].present)) {
            return 0;
        }
    }
    if (!dm1_original_save_c13_runtime_receipt(
            report, world, &c13_admitted_count, &c13_fingerprint)) {
        return 0;
    }
    report->c13_party_runtime_champion_count = world->party.championCount;
    report->c13_party_runtime_active_champion_index =
        world->party.activeChampionIndex;
    report->c13_party_runtime_metadata_fingerprint = metadata_fingerprint;
    report->c13_party_runtime_state_fingerprint = state_fingerprint;
    report->c13_party_runtime_timeline_fingerprint = timeline_fingerprint;
    report->c13_party_runtime_receipt_valid = 1;
    return 1;
}

static int validate_original_pc34_party_inventory_references(
    const struct GameWorld_Compat *world)
{
    int champion_index;

    if (!world || !world->ownsDungeon || !world->things) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
    }
    for (champion_index = 0;
         champion_index < world->party.championCount;
         ++champion_index) {
        const struct ChampionState_Compat *champion =
            &world->party.champions[champion_index];
        int slot;

        for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot) {
            unsigned short thing = champion->inventory[slot];
            unsigned int type;
            unsigned int index;
            unsigned short next;

            if (thing == THING_NONE) {
                continue;
            }
            type = THING_GET_TYPE(thing);
            index = THING_GET_INDEX(thing);
            /* ReDMCSB LOADSAVE.C F0435 restores PARTY before the dungeon
             * tail. Once F0434 has supplied ThingData, a champion slot may
             * name only a live carryable Thing in a declared type table.
             * A zero-count type remains owned by the compatible start
             * runtime, as F0435 tails can be intentionally partial. */
            if (thing == THING_ENDOFLIST || type < THING_TYPE_WEAPON ||
                type > THING_TYPE_JUNK) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            if (world->things->thingCounts[type] == 0) {
                continue;
            }
            if (index >= (unsigned int)world->things->thingCounts[type] ||
                !world->things->rawThingData[type]) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            switch (type) {
            case THING_TYPE_WEAPON:
                next = world->things->weapons[index].next;
                break;
            case THING_TYPE_ARMOUR:
                next = world->things->armours[index].next;
                break;
            case THING_TYPE_SCROLL:
                next = world->things->scrolls[index].next;
                break;
            case THING_TYPE_POTION:
                next = world->things->potions[index].next;
                break;
            case THING_TYPE_CONTAINER:
                next = world->things->containers[index].next;
                break;
            case THING_TYPE_JUNK:
                next = world->things->junks[index].next;
                break;
            default:
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            if (next == THING_NONE) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
        }
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int validate_original_pc34_party_pose_against_dungeon_tail(
    const struct GameWorld_Compat *world)
{
    const struct DungeonDatState_Compat *dungeon;
    const struct DungeonMapDesc_Compat *map;
    int map_index;

    if (!world || !world->ownsDungeon) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
    }
    dungeon = world->dungeon;
    if (!dungeon || !dungeon->maps || dungeon->header.mapCount == 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB LOADSAVE.C F0435 restores GLOBAL_DATA before F0434 reads the
     * saved dungeon.  F0434 then makes PartyMapIndex select G0277_ps_DungeonMaps
     * and the party coordinates select a square in that map.  Keep the same
     * dependency inside the candidate world: a checksum-valid save must not
     * publish a pose that cannot address its own restored dungeon tail. */
    map_index = world->party.mapIndex;
    if (map_index < 0 || map_index >= (int)dungeon->header.mapCount) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    map = &dungeon->maps[map_index];
    if (world->party.mapX < 0 || world->party.mapY < 0 ||
        world->party.mapX >= (int)map->width ||
        world->party.mapY >= (int)map->height) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int import_original_pc34_global_data(
    const uint8_t *bytes,
    size_t size,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t meta[256];
    uint16_t key;
    uint16_t part_keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t part_checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    uint8_t part[SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    size_t part_size = 0u;
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    uint32_t game_id;
    uint16_t actual_checksum = 0u;
    int party_champion_count;
    int active_champion_index;
    int current_active_group_count;
    int maximum_active_group_count = 0;
    int event_maximum_count = 0;
    int i;
    int rc;

    if (size < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }

    key = read_u16_le(bytes + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u);
    memcpy(meta, bytes + 256u, sizeof(meta));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, key);

    game_id = read_u32_le(meta + 50u);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        part_keys[i] = read_u16_le(meta + 54u + (size_t)i * 2u);
        part_checksums[i] = read_u16_le(meta + 86u + (size_t)i * 2u);
    }
    if (out_report) {
        for (i = 0; i < SAVEGAME_PC34_PART_COUNT; ++i) {
            out_report->part_expected_checksums[i] = part_checksums[i];
        }
    }

    rc = dm1_v1_original_save_pc34_read_part_f0419(
        bytes, size, &cursor, part_keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
        part_checksums[SAVEGAME_PC34_PART_GLOBAL_DATA], part, sizeof(part),
        &part_size, &actual_checksum);
    if (out_report) {
        out_report->part_byte_counts[SAVEGAME_PC34_PART_GLOBAL_DATA] =
            (uint32_t)part_size;
        out_report->part_actual_checksums[SAVEGAME_PC34_PART_GLOBAL_DATA] =
            actual_checksum;
        if (rc == SAVEGAME_PC34_OK) {
            out_report->part_checksum_ok_count++;
        }
    }
    if (rc != SAVEGAME_PC34_OK) {
        return rc;
    }
    if (part_size < SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    party_champion_count = (int)read_u16_le(part + 10u);
    active_champion_index = (int)read_i16_le(part + 20u);
    /* ReDMCSB LOADSAVE.C F0435 restores GLOBAL_DATA before copying the
     * fixed four CHAMPION records. Do not clamp a malformed count: PARTY
     * consumers use the count and leader index as array bounds. */
    if (party_champion_count < 0 ||
        party_champion_count > CHAMPION_MAX_PARTY ||
        active_champion_index < -1 ||
        (party_champion_count == 0 && active_champion_index > 0) ||
        (party_champion_count > 0 &&
         active_champion_index >= party_champion_count)) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    maximum_active_group_count = (int)read_u16_le(
        part + DM1_PC34_GLOBAL_MAXIMUM_ACTIVE_GROUP_COUNT_OFFSET);
    current_active_group_count = (int)read_u16_le(
        part + DM1_PC34_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET);
    /* ReDMCSB LOADSAVE.C F0435 reads exactly MaximumActiveGroupCount
     * ACTIVE_GROUP records. A larger current count would make the saved
     * live-set claim entries outside that checksum-validated part. */
    if (current_active_group_count > maximum_active_group_count) {
        return SAVEGAME_PC34_ERROR_BAD_SIZE;
    }
    event_maximum_count = (int)read_u16_le(
        part + DM1_PC34_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET);
    if (out_report) {
        out_report->original_game_time = read_u32_le(part + 0u);
        out_report->original_current_active_group_count =
            current_active_group_count;
        out_report->original_maximum_active_group_count =
            maximum_active_group_count;
        out_report->original_event_count = (int)read_u16_le(
            part + DM1_PC34_GLOBAL_EVENT_COUNT_OFFSET);
        out_report->original_first_unused_event_index = (int)read_u16_le(
            part + DM1_PC34_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET);
        out_report->original_event_maximum_count = event_maximum_count;
    }
    if (out_state->party) {
        out_state->party->championCount = party_champion_count;
        out_state->party->mapX = (int)read_i16_le(part + 12u);
        out_state->party->mapY = (int)read_i16_le(part + 14u);
        out_state->party->direction = (int)read_i16_le(part + 16u);
        out_state->party->mapIndex = (int)read_i16_le(part + 18u);
        out_state->party->activeChampionIndex = active_champion_index;
    }

    for (i = 1; i < SAVEGAME_PC34_PART_COUNT; ++i) {
        size_t part_offset = cursor;

        rc = dm1_v1_original_save_pc34_read_part_f0419(
            bytes, size, &cursor, part_keys[i], part_checksums[i], part,
            sizeof(part), &part_size, &actual_checksum);
        if (out_report) {
            out_report->part_byte_counts[i] = (uint32_t)part_size;
            out_report->part_actual_checksums[i] = actual_checksum;
            if (rc == SAVEGAME_PC34_OK) {
                out_report->part_checksum_ok_count++;
            }
        }
        if (rc != SAVEGAME_PC34_OK) {
            return rc;
        }
        if (out_report && i == SAVEGAME_PC34_PART_PARTY) {
            out_report->pc34_party_part_byte_offset =
                (uint32_t)(part_offset + 2u);
            out_report->pc34_party_part_key = part_keys[i];
        }
        if (i == SAVEGAME_PC34_PART_ACTIVE_GROUP) {
            rc = import_original_pc34_active_group_part(
                part, part_size, maximum_active_group_count, out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_PARTY) {
            rc = import_original_pc34_party_part(part, part_size, out_state,
                                                 out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_EVENTS) {
            rc = import_original_pc34_events_part(
                part, part_size, event_maximum_count, out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_TIMELINE) {
            rc = import_original_pc34_timeline_part(
                part, part_size, event_maximum_count, out_report);
            if (rc != SAVEGAME_PC34_OK) {
                return rc;
            }
        }
        if (i == SAVEGAME_PC34_PART_TIMELINE && out_state->timeline && part_size > 0u) {
            size_t copy_n = part_size;
            if (copy_n > sizeof(*out_state->timeline)) {
                copy_n = sizeof(*out_state->timeline);
            }
            memcpy(out_state->timeline, part, copy_n);
        }
    }

    rc = import_original_pc34_external_portraits(bytes, size, cursor,
                                                 out_state, out_report);
    if (rc != SAVEGAME_PC34_OK) {
        return rc;
    }
    cursor += SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;
    rc = decode_original_pc34_dungeon_tail(bytes, size, cursor, out_report);
    if (rc != SAVEGAME_PC34_OK) {
        return rc;
    }

    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 0] =
        (unsigned char)(game_id & 0xffu);
    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 1] =
        (unsigned char)((game_id >> 8) & 0xffu);
    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 2] =
        (unsigned char)((game_id >> 16) & 0xffu);
    out_state->header.reserved[SAVEGAME_HEADER_RESERVED_GAME_ID_OFFSET + 3] =
        (unsigned char)((game_id >> 24) & 0xffu);

    if (out_report && out_state->party) {
        out_report->imported_champion_count =
            out_state->party->championCount;
        out_report->imported_map_index = out_state->party->mapIndex;
        out_report->imported_map_x = out_state->party->mapX;
        out_report->imported_map_y = out_state->party->mapY;
        out_report->imported_direction = out_state->party->direction;
        out_report->imported_active_champion_index =
            out_state->party->activeChampionIndex;
    }
    return SAVEGAME_PC34_OK;
}

int dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
    const DM1OriginalSavePC34FixtureSpec *spec,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size)
{
    uint8_t header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    uint8_t global[SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT];
    uint8_t active_group[DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT *
                         DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT];
    uint8_t party[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    uint8_t events[DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT *
                   DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT];
    uint8_t timeline[2u * DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT];
    uint8_t portraits[SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int champion_count;
    int current_active_group_count;
    int maximum_active_group_count;
    int event_count;
    int event_maximum_count;
    int rc;
    int i;

    if (!spec || !out_bytes || !out_size) {
        return SAVEGAME_PC34_ERROR_NULL_ARG;
    }
    *out_size = 0u;
    if (out_capacity < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }

    champion_count = spec->champion_count;
    if (champion_count < 0) champion_count = 0;
    if (champion_count > CHAMPION_MAX_PARTY) champion_count = CHAMPION_MAX_PARTY;

    current_active_group_count = spec->current_active_group_count;
    if (current_active_group_count < 0) current_active_group_count = 0;
    if (current_active_group_count > (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT) {
        current_active_group_count = (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT;
    }
    maximum_active_group_count = spec->maximum_active_group_count;
    if (maximum_active_group_count <= 0) {
        maximum_active_group_count =
            (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT;
    }
    if (maximum_active_group_count >
        (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT) {
        maximum_active_group_count =
            (int)DM1_PC34_ORIGINAL_ACTIVE_GROUP_FIXTURE_COUNT;
    }

    event_count = spec->event_count;
    if (event_count <= 0) event_count = 3;
    if (event_count > (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT) {
        event_count = (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT;
    }
    event_maximum_count = spec->event_maximum_count;
    if (event_maximum_count <= 0) {
        event_maximum_count = (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT;
    }
    if (event_maximum_count < event_count) {
        event_maximum_count = event_count;
    }
    if (event_maximum_count > (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT) {
        event_maximum_count = (int)DM1_PC34_ORIGINAL_EVENT_FIXTURE_COUNT;
    }

    memset(out_bytes, 0, out_capacity);
    memset(header, 0, sizeof(header));
    memset(global, 0, sizeof(global));
    memset(active_group, 0, sizeof(active_group));
    memset(party, 0, sizeof(party));
    memset(events, 0, sizeof(events));
    memset(timeline, 0, sizeof(timeline));
    memset(portraits, 0, sizeof(portraits));
    memset(checksums, 0, sizeof(checksums));

    for (i = 0; i < 127; ++i) {
        write_u16_le(header + (size_t)i * 2u,
                     (uint16_t)(0x4321u + (uint16_t)(i * 17u)));
    }
    write_u16_le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u,
                 0x2468u);
    header[298u] = 1u;
    header[299u] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    write_u16_le(header + 304u, 1u);
    write_u32_le(header + 306u,
                 spec->game_id ? spec->game_id : 0x50433334u);

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = (uint16_t)(0x2000u + (uint16_t)(i * 0x101u));
    }

    write_u32_le(global + 0u, spec->game_time ? spec->game_time : 123456u);
    write_u16_le(global + 10u, (uint16_t)champion_count);
    write_u16_le(global + 12u, (uint16_t)spec->map_x);
    write_u16_le(global + 14u, (uint16_t)spec->map_y);
    write_u16_le(global + 16u, (uint16_t)spec->direction);
    write_u16_le(global + 18u, (uint16_t)spec->map_index);
    write_u16_le(global + 20u, (uint16_t)spec->active_champion_index);
    write_u16_le(global + DM1_PC34_GLOBAL_EVENT_COUNT_OFFSET,
                 (uint16_t)event_count);
    write_u16_le(global + DM1_PC34_GLOBAL_FIRST_UNUSED_EVENT_INDEX_OFFSET,
                 (uint16_t)event_count);
    write_u16_le(global + DM1_PC34_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET,
                 (uint16_t)event_maximum_count);
    write_u16_le(global + DM1_PC34_GLOBAL_CURRENT_ACTIVE_GROUP_COUNT_OFFSET,
                 (uint16_t)current_active_group_count);
    write_u16_le(global + DM1_PC34_GLOBAL_MAXIMUM_ACTIVE_GROUP_COUNT_OFFSET,
                 (uint16_t)maximum_active_group_count);

    write_original_pc34_fixture_active_group(active_group + 0u, 0x1001u,
                                             0x5a, 0xc3, 21, 22);
    write_original_pc34_fixture_active_group(
        active_group + DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
        0x1002u, 0x6b, 0xd4, 23, 24);
    write_original_pc34_fixture_active_group(
        active_group + 2u * DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
        0x1003u, 0x7c, 0xe5, 25, 26);

    if (champion_count > 0) {
        write_original_pc34_fixture_champion(
            party + 0u, "TIGGY", "APPRENTICE", spec->direction,
            44, 55, 66, 77, 8, 9, 1500, -32, 0x0021u, 0x1555u);
    }
    if (champion_count > 1) {
        write_original_pc34_fixture_champion(
            party + DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT,
            "WUUF", "BIKA", (spec->direction + 1) & 3,
            88, 99, 111, 122, 33, 44, 1200, 1100, 0x0002u, 0x1666u);
    }
    if (champion_count > 2) {
        write_original_pc34_fixture_champion(
            party + 2u * DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT,
            "HALK", "BARBARIAN", (spec->direction + 2) & 3,
            101, 202, 303, 404, 55, 66, 900, 800, 0x0010u, 0x1777u);
    }

    write_original_pc34_fixture_event(events + 0u,
                                      DM1_MAP_TIME_MAKE(2, 123500u),
                                      DM1_EVENT_MOVE_GROUP_AUDIBLE, 7,
                                      11, 12, 3, DM1_EFFECT_SET);
    write_original_pc34_fixture_event(
        events + DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
        DM1_MAP_TIME_MAKE(2, 123470u),
        DM1_EVENT_DOOR, 4, 21, 22, 1, DM1_EFFECT_TOGGLE);
    write_original_pc34_fixture_event(
        events + 2u * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
        DM1_MAP_TIME_MAKE(1, 123490u),
        DM1_EVENT_LIGHT, 2, 0, 0, 0, 9);
    write_u16_le(timeline + 0u, 1u);
    write_u16_le(timeline + 2u, 2u);
    write_u16_le(timeline + 4u, 0u);
    write_u16_le(timeline + 6u, 3u);
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        memset(portraits + (size_t)i * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT,
               0x30 + i, CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT);
    }

    rc = dm1_v1_original_save_pc34_write_part_f0420(out_bytes + cursor, out_capacity - cursor,
                             global, sizeof(global),
                             keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                             &checksums[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = dm1_v1_original_save_pc34_write_part_f0420(
        out_bytes + cursor, out_capacity - cursor,
        active_group,
        (size_t)maximum_active_group_count *
            DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT,
        keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
        &checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = dm1_v1_original_save_pc34_write_part_f0420(out_bytes + cursor, out_capacity - cursor,
                             party, sizeof(party),
                             keys[SAVEGAME_PC34_PART_PARTY],
                             &checksums[SAVEGAME_PC34_PART_PARTY]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = dm1_v1_original_save_pc34_write_part_f0420(
        out_bytes + cursor, out_capacity - cursor,
        events,
        (size_t)event_maximum_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT,
        keys[SAVEGAME_PC34_PART_EVENTS],
        &checksums[SAVEGAME_PC34_PART_EVENTS]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;
    rc = dm1_v1_original_save_pc34_write_part_f0420(out_bytes + cursor, out_capacity - cursor,
                             timeline, (size_t)event_maximum_count * 2u,
                             keys[SAVEGAME_PC34_PART_TIMELINE],
                             &checksums[SAVEGAME_PC34_PART_TIMELINE]);
    if (rc < 0) return rc;
    cursor += (size_t)rc;

    if (out_capacity - cursor < sizeof(portraits)) {
        return SAVEGAME_PC34_ERROR_BUFFER_TOO_SMALL;
    }
    memcpy(out_bytes + cursor, portraits, sizeof(portraits));
    cursor += sizeof(portraits);

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        write_u16_le(header + 310u + (size_t)i * 2u, keys[i]);
        write_u16_le(header + 342u + (size_t)i * 2u, checksums[i]);
    }
    write_u16_le(header + 374u, SAVEGAME_PC34_PLATFORM_PC);
    write_u16_le(header + 376u, SAVEGAME_PC34_DUNGEON_ID_DM);
    {
        uint16_t second_sum = original_pc34_header_second_half_plain_sum(header);
        uint16_t first_before_last =
            original_pc34_header_first_half_checksum(header);
        uint16_t last =
            (uint16_t)(read_u16_le(header + 254u) ^
                       first_before_last ^
                       second_sum);
        write_u16_le(header + 254u, last);
    }
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        header + 256u, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS,
        read_u16_le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(out_bytes, header, sizeof(header));
    *out_size = cursor;
    return SAVEGAME_PC34_OK;
}

int dm1_v1_original_save_pc34_handoff_bytes(
    const uint8_t *bytes,
    size_t size,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    DM1OriginalSaveClassifyResult classify;
    DM1OriginalSavePC34HandoffReport staged_report;
    struct SaveGame_Compat staged_state;
    struct PartyState_Compat staged_party;
    struct TimelineQueue_Compat staged_timeline;
    int rc;

    memset(&staged_report, 0, sizeof(staged_report));
    staged_report.importer_result = SAVEGAME_PC34_ERROR_INTERNAL;
    staged_report.timeline_duplicate_first_slot = -1;
    staged_report.timeline_duplicate_slot = -1;
    staged_report.timeline_duplicate_event_index = -1;
    staged_report.timeline_invalid_slot = -1;
    staged_report.timeline_invalid_event_index = -1;
    staged_report.timeline_invalid_event_is_none = 0;
    staged_report.timeline_orphan_active_event_index = -1;
    staged_report.timeline_orphan_active_event_type = -1;
    staged_report.timeline_heap_invalid_parent_slot = -1;
    staged_report.timeline_heap_invalid_child_slot = -1;
    staged_report.timeline_heap_invalid_parent_event_index = -1;
    staged_report.timeline_heap_invalid_child_event_index = -1;
    staged_report.first_unused_event_index_event_type = -1;
    if (out_report) {
        memset(out_report, 0, sizeof(*out_report));
        out_report->importer_result = SAVEGAME_PC34_ERROR_INTERNAL;
        out_report->timeline_duplicate_first_slot = -1;
        out_report->timeline_duplicate_slot = -1;
        out_report->timeline_duplicate_event_index = -1;
        out_report->timeline_invalid_slot = -1;
        out_report->timeline_invalid_event_index = -1;
        out_report->timeline_invalid_event_is_none = 0;
        out_report->timeline_orphan_active_event_index = -1;
        out_report->timeline_orphan_active_event_type = -1;
        out_report->timeline_heap_invalid_parent_slot = -1;
        out_report->timeline_heap_invalid_child_slot = -1;
        out_report->timeline_heap_invalid_parent_event_index = -1;
        out_report->timeline_heap_invalid_child_event_index = -1;
        out_report->first_unused_event_index_event_type = -1;
    }
    if (!bytes || !out_state) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (size > (size_t)SAVEGAME_PC34_MAX_FILE_SIZE ||
        size > (size_t)((int)0x7fffffff)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }

    if (!dm1_v1_original_save_classify_bytes(bytes, size, &classify)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (out_report) {
        out_report->classify = classify;
    }
    if (classify.shape != DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 ||
        !classify.pc34_importer_candidate) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines ~2665-2722 reads the original
     * DM_SAVE_HEADER layout from DEFS.H lines 468-480
     * (Noise[149], then metadata at byte 298) before reading
     * GLOBAL_DATA and save parts through READWRIT.C F0419 lines
     * ~232-242. LOADSAVE.C F0435 lines ~2746-2754 reads
     * sizeof(ACTIVE_GROUP) * GLOBAL_DATA.MaximumActiveGroupCount
     * before LOADSAVE.C F0435 lines ~2766-2777 copies the
     * PC34 PARTY block as four 319-byte CHAMPION_EXCLUDING_PORTRAIT
     * records plus 128 PARTY_INFO bytes, then lines ~2810-2816 read
     * four external 32x29 portrait bitmap payloads. F0796 handles Firestaff's
     * PC34 native-export layout;
     * this path handles the real original header envelope classified
     * above. */
    staged_report.classify = classify;
    staged_state = *out_state;
    if (out_state->party) {
        staged_party = *out_state->party;
        staged_state.party = &staged_party;
    }
    if (out_state->timeline) {
        staged_timeline = *out_state->timeline;
        staged_state.timeline = &staged_timeline;
    }
    rc = import_original_pc34_global_data(bytes, size, &staged_state,
                                          &staged_report);
    if (rc == SAVEGAME_PC34_OK) {
        rc = validate_original_pc34_timeline_membership(&staged_report);
    }
    if (rc == SAVEGAME_PC34_OK) {
        rc = validate_original_pc34_timeline_heap(&staged_report);
    }
    staged_report.importer_result = rc;
    if (out_report) *out_report = staged_report;
    if (rc != SAVEGAME_PC34_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    out_state->header = staged_state.header;
    if (out_state->party) *out_state->party = staged_party;
    if (out_state->timeline) *out_state->timeline = staged_timeline;

    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_file(
    const char *path,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !out_state) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    result = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, size, out_state, out_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_handoff_apply_active_groups(
    DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world)
{
    int i;
    int import_count;

    if (!report || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->reported_active_group_count < 0 ||
        report->reported_active_group_count >
            DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    import_count = report->original_current_active_group_count;
    if (import_count < 0) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (import_count > report->reported_active_group_count) {
        import_count = report->reported_active_group_count;
    }
    if (import_count > GAMEWORLD_CREATURE_AI_CAPACITY) {
        import_count = GAMEWORLD_CREATURE_AI_CAPACITY;
    }

    /* Once an F0435 tail supplied GROUP records, every live ACTIVE_GROUP
     * must name a used type-4 Thing in that same bounded table. Validate the
     * full source set before touching the public runtime array. */
    if (world->things && world->things->groupCount > 0) {
        for (i = 0; i < import_count; ++i) {
            const DM1OriginalSavePC34ActiveGroupRecord *src =
                &report->active_groups[i];
            unsigned int thing = (unsigned int)(uint16_t)src->group_thing_index;
            int thing_type = (int)((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                                   DM1_PC34_THING_TYPE_MASK);
            int thing_index = (int)(thing & DM1_PC34_THING_INDEX_MASK);

            if (thing_type != DM1_PC34_THING_TYPE_GROUP ||
                !world->things->groups || thing_index < 0 ||
                thing_index >= world->things->groupCount ||
                world->things->groups[thing_index].next == THING_NONE) {
                return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
        }
    }

    memset(world->creatureAI, 0, sizeof(world->creatureAI));
    memset(world->pc34ActiveGroupDirections, 0,
           sizeof(world->pc34ActiveGroupDirections));
    memset(world->pc34ActiveGroupHomeMapX, 0,
           sizeof(world->pc34ActiveGroupHomeMapX));
    memset(world->pc34ActiveGroupHomeMapY, 0,
           sizeof(world->pc34ActiveGroupHomeMapY));
    world->creatureAICount = import_count;
    /* C04 stores MaximumActiveGroupCount records even when fewer are live.
     * Retain that source capacity so F0433 keeps the authenticated part
     * shape while only the current prefix becomes runtime AI state. */
    world->pc34ActiveGroupSourceCount = report->reported_active_group_count;
    report->active_group_runtime_imported_count = import_count;
    report->active_group_runtime_truncated_count =
        report->original_current_active_group_count - import_count;
    report->active_group_runtime_resolved_count = 0;
    report->active_group_runtime_unresolved_count = 0;
    for (i = 0; i < import_count; ++i) {
        const DM1OriginalSavePC34ActiveGroupRecord *src =
            &report->active_groups[i];
        struct CreatureAIState_Compat *dst = &world->creatureAI[i];
        const struct DungeonGroup_Compat *resolved_group = 0;
        unsigned int thing = (unsigned int)(uint16_t)src->group_thing_index;
        int thing_type = (int)((thing >> DM1_PC34_THING_TYPE_SHIFT) &
                               DM1_PC34_THING_TYPE_MASK);
        int thing_index = (int)(thing & DM1_PC34_THING_INDEX_MASK);
        int first_direction = src->directions & 0x03;

        /* ReDMCSB DEFS.H ACTIVE_GROUP carries GroupThingIndex, packed
         * directions/cells, target/prior/home coordinates, and aspect
         * bytes. DEFS.H THING encodes Bits 13-10 as type and Bits 9-0
         * as index. If Firestaff has decoded DungeonThings_Compat, a type
         * 4 GROUP thing resolves directly to things->groups[index]. */
        if (thing_type == DM1_PC34_THING_TYPE_GROUP &&
            world->things &&
            thing_index >= 0 &&
            thing_index < world->things->groupCount &&
            world->things->groups) {
            resolved_group = &world->things->groups[thing_index];
        }
        dst->stateKind = AI_STATE_WANDER;
        dst->creatureType = resolved_group ? resolved_group->creatureType : -1;
        dst->groupMapIndex = world->partyMapIndex;
        dst->groupMapX = src->prior_map_x;
        dst->groupMapY = src->prior_map_y;
        dst->groupCells = src->cells;
        dst->groupDirection = first_direction;
        dst->targetChampionIndex = -1;
        dst->lastSeenPartyMapX = src->target_map_x;
        dst->lastSeenPartyMapY = src->target_map_y;
        dst->lastSeenPartyTick = src->last_move_time;
        dst->fearCounter = src->delay_fleeing_from_target;
        dst->turnCounter = 0;
        dst->attackCooldownTicks = 0;
        dst->movementCooldownTicks = 0;
        dst->aggressionScore = 0;
        dst->rngCallCount = 0;
        dst->reserved0 = resolved_group ? thing_index : src->group_thing_index;
        memcpy(dst->aspect, src->aspect, sizeof(dst->aspect));
        world->pc34ActiveGroupDirections[i] = (uint8_t)src->directions;
        world->pc34ActiveGroupHomeMapX[i] = (uint8_t)src->home_map_x;
        world->pc34ActiveGroupHomeMapY[i] = (uint8_t)src->home_map_y;
        if (resolved_group) {
            report->active_group_runtime_resolved_count++;
        } else {
            report->active_group_runtime_unresolved_count++;
        }
    }

    return import_count;
}

int dm1_v1_original_save_pc34_handoff_apply_event_queue(
    const DM1OriginalSavePC34HandoffReport *report,
    struct DM1_EventQueue_V1 *queue)
{
    struct DM1_EventQueue_V1 candidate_queue;
    DM1OriginalSavePC34HandoffReport candidate_report;
    int i;

    if (!report || !queue) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (report->original_event_count < 0 ||
        report->original_event_count > DM1_EVENT_MAX_COUNT ||
        report->decoded_event_count < 0 ||
        report->decoded_event_count > DM1_EVENT_MAX_COUNT ||
        report->decoded_timeline_index_count < 0 ||
        report->decoded_timeline_index_count > DM1_EVENT_MAX_COUNT ||
        report->original_event_count > report->decoded_event_count ||
        report->original_event_count > report->decoded_timeline_index_count ||
        report->original_first_unused_event_index < 0 ||
        report->original_first_unused_event_index > DM1_EVENT_MAX_COUNT) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (validate_original_pc34_timeline_references(report) !=
        DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    /* ReDMCSB LOADSAVE.C F0435:2780-2800 reads EVENTS and TIMELINE before
     * F0651 publishes optimized timeline management. Validate and build the
     * complete Firestaff queue first, so a malformed timeline cannot replace
     * the runtime's previously valid queue with a partial load. Keep failure
     * provenance local because this public apply API accepts a const report. */
    candidate_report = *report;
    if (validate_original_pc34_timeline_membership(&candidate_report) !=
            SAVEGAME_PC34_OK ||
        validate_original_pc34_timeline_heap(&candidate_report) !=
            SAVEGAME_PC34_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    if (!dm1v1_event_queue_init(&candidate_queue, report->original_game_time)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    candidate_queue.eventCount = report->original_event_count;
    candidate_queue.firstUnusedIndex = report->original_first_unused_event_index;
    candidate_queue.maxEvents = DM1_EVENT_MAX_COUNT;
    for (i = 0; i < report->decoded_event_count; ++i) {
        candidate_queue.events[i] = report->events[i];
    }
    for (i = 0; i < report->original_event_count; ++i) {
        candidate_queue.timeline[i] = report->timeline_indices[i];
    }
    *queue = candidate_queue;
    return queue->eventCount;
}

int dm1_v1_original_save_pc34_handoff_load_world_from_file(
    const char *path,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, size, world, event_queue, out_report);
    free(bytes);
    return result;
}

/* F0435 has authenticated both parts before this point. Publish the exact
 * plaintext C3/C4 part bytes only after all later world materialization has
 * succeeded, so a rejected tail never leaves a usable receipt behind. */
static int publish_original_pc34_c3_c4_receipt(
    DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world)
{
    uint32_t expected_c3_bytes;
    uint32_t expected_c4_bytes;

    if (!report || !world || report->decoded_event_count < 0 ||
        report->decoded_event_count > DM1_EVENT_MAX_COUNT ||
        report->decoded_timeline_index_count != report->decoded_event_count) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    expected_c3_bytes = (uint32_t)report->decoded_event_count *
        GAMEWORLD_PC34_ORIGINAL_C3_EVENT_BYTE_COUNT;
    expected_c4_bytes = (uint32_t)report->decoded_timeline_index_count *
        GAMEWORLD_PC34_ORIGINAL_C4_HEAP_ENTRY_BYTE_COUNT;
    if (report->c3_raw_event_byte_count != expected_c3_bytes ||
        report->c4_raw_heap_byte_count != expected_c4_bytes ||
        expected_c3_bytes > sizeof(world->pc34OriginalC3RawEventBytes) ||
        expected_c4_bytes > sizeof(world->pc34OriginalC4RawHeapBytes) ||
        report->c3_raw_event_fingerprint != dm1_v1_c15_layout_fingerprint_pc34(
            report->c3_raw_event_bytes, expected_c3_bytes) ||
        report->c4_raw_heap_fingerprint != dm1_v1_c15_layout_fingerprint_pc34(
            report->c4_raw_heap_bytes, expected_c4_bytes)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    memcpy(world->pc34OriginalC3RawEventBytes,
           report->c3_raw_event_bytes, expected_c3_bytes);
    memcpy(world->pc34OriginalC4RawHeapBytes,
           report->c4_raw_heap_bytes, expected_c4_bytes);
    world->pc34OriginalC3RawEventByteCount = expected_c3_bytes;
    world->pc34OriginalC4RawHeapByteCount = expected_c4_bytes;
    world->pc34OriginalC3RawEventFingerprint =
        report->c3_raw_event_fingerprint;
    world->pc34OriginalC4RawHeapFingerprint =
        report->c4_raw_heap_fingerprint;
    world->pc34OriginalC3C4RuntimeEventCount =
        (uint32_t)world->timeline.count;
    world->pc34OriginalTimelineFingerprint =
        original_pc34_timeline_runtime_fingerprint(&world->timeline);
    if (world->pc34OriginalTimelineFingerprint == 0u) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    report->c3_c4_runtime_event_count =
        world->pc34OriginalC3C4RuntimeEventCount;
    report->timeline_runtime_fingerprint =
        world->pc34OriginalTimelineFingerprint;
    for (int i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (world->party.champions[i].poisonDose > 0xffu) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
        }
        world->party.pc34OriginalPoisonEventCounts[i] =
            (uint8_t)world->party.champions[i].poisonDose;
    }
    world->party.pc34PoisonEventCountRuntimeEventCount =
        world->pc34OriginalC3C4RuntimeEventCount;
    world->party.pc34PoisonEventCountTimelineFingerprint =
        world->pc34OriginalTimelineFingerprint;
    world->party.pc34PoisonEventCountReceiptValid = 1;
    world->pc34OriginalC3C4ReceiptValid = 1;
    report->c3_c4_receipt_valid = 1;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int load_world_from_bytes_uncommitted(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *report)
{
    struct SaveGame_Compat state;
    int result;

    if (!bytes || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    memset(&state, 0, sizeof(state));
    state.party = &world->party;
    state.timeline = &world->timeline;

    result = dm1_v1_original_save_pc34_handoff_bytes(
        bytes, size, &state, report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    /* ReDMCSB DEFS.H PARTY_INFO starts with MagicalLightAmount, C73/C79
     * counters, then ShieldDefense/FireShieldDefense/SpellShieldDefense,
     * ScentCount and FreezeLifeTicks. After the opaque scent records and
     * strengths, FirstScentIndex is the existing F0412 Footprints-window
     * owner. Do not infer the scent arrays, LastScentIndex, or BUG0_00
     * bytes. */
    if (!world->party.pc34PartyInfoBytesValid) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    world->magic.magicalLightAmount = read_i16_le(
        world->party.pc34PartyInfoBytes +
        DM1_PC34_PARTY_INFO_MAGICAL_LIGHT_OFFSET);
    world->magic.event73CountThievesEye =
        world->party.pc34PartyInfoBytes[
            DM1_PC34_PARTY_INFO_THIEVES_EYE_COUNT_OFFSET];
    world->magic.event79CountFootprints =
        world->party.pc34PartyInfoBytes[
            DM1_PC34_PARTY_INFO_FOOTPRINTS_COUNT_OFFSET];
    world->magic.magicFootprintsActive =
        world->magic.event79CountFootprints > 0;
    world->magic.partyShieldDefense = read_i16_le(
        world->party.pc34PartyInfoBytes +
        DM1_PC34_PARTY_INFO_SHIELD_DEFENSE_OFFSET);
    world->magic.fireShieldDefense = read_i16_le(
        world->party.pc34PartyInfoBytes +
        DM1_PC34_PARTY_INFO_FIRE_SHIELD_DEFENSE_OFFSET);
    world->magic.spellShieldDefense = read_i16_le(
        world->party.pc34PartyInfoBytes +
        DM1_PC34_PARTY_INFO_SPELL_SHIELD_DEFENSE_OFFSET);
    world->magic.scentCount = world->party.pc34PartyInfoBytes[
        DM1_PC34_PARTY_INFO_SCENT_COUNT_OFFSET];
    world->freezeLifeTicks = world->party.pc34PartyInfoBytes[
        DM1_PC34_PARTY_INFO_FREEZE_LIFE_TICKS_OFFSET];
    world->magic.freezeLifeTicks = world->freezeLifeTicks;
    world->magic.firstScentIndex = world->party.pc34PartyInfoBytes[
        DM1_PC34_PARTY_INFO_FIRST_SCENT_INDEX_OFFSET];
    world->lifecycle.status.partyShieldDefense =
        (int16_t)world->magic.partyShieldDefense;
    world->lifecycle.status.partyFireShieldDefense =
        (int16_t)world->magic.fireShieldDefense;
    world->lifecycle.status.partySpellShieldDefense =
        (int16_t)world->magic.spellShieldDefense;

    world->gameTick = report->original_game_time;
    world->partyMapIndex = world->party.mapIndex;
    world->newPartyMapIndex = world->party.mapIndex;
    world->timeline.nowTick = report->original_game_time;

    result = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        report, world);
    if (result < 0) {
        return result;
    }
    result = materialize_original_pc34_dungeon_tail(
        bytes, size, world, report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    result = validate_original_pc34_party_pose_against_dungeon_tail(world);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    result = validate_original_pc34_party_inventory_references(world);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    result = dm1_v1_original_save_pc34_handoff_apply_active_groups(
        report, world);
    if (result < 0) {
        return result;
    }
    result = materialize_original_pc34_timeline(report, world, &world->timeline);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    result = materialize_original_pc34_party_status(report, world);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    if (event_queue) {
        result = dm1_v1_original_save_pc34_handoff_apply_event_queue(
            report, event_queue);
        if (result < 0) {
            return result;
        }
    }

    result = publish_original_pc34_c3_c4_receipt(report, world);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct GameWorld_Compat candidate_world;
    struct DM1_EventQueue_V1 candidate_queue;
    DM1OriginalSavePC34HandoffReport candidate_report;
    int reuses_existing_dungeon;
    int result;

    if (!bytes || !world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    /* ReDMCSB LOADSAVE.C F0435 restores GLOBAL_DATA, PARTY, EVENT,
     * TIMELINE, portraits, and finally the dungeon before the resumed
     * runtime is exposed. Keep that sequence private to a candidate world:
     * a rejected final tail/checksum must not leak a partially loaded party
     * or event heap into the running HoC session. */
    memset(&candidate_world, 0, sizeof(candidate_world));
    /* The byte-loader also serves callers that have already materialized a
     * start dungeon. It may resolve ACTIVE_GROUP records against that data,
     * but never owns it while validation is still in progress. */
    candidate_world.dungeon = world->dungeon;
    candidate_world.things = world->things;
    candidate_world.ownsDungeon = 0;
    memset(&candidate_queue, 0, sizeof(candidate_queue));
    memset(&candidate_report, 0, sizeof(candidate_report));
    result = load_world_from_bytes_uncommitted(
        bytes, size, &candidate_world,
        event_queue ? &candidate_queue : NULL, &candidate_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return result;
    }

    reuses_existing_dungeon =
        candidate_world.dungeon == world->dungeon &&
        candidate_world.things == world->things;
    if (reuses_existing_dungeon) {
        candidate_world.ownsDungeon = world->ownsDungeon;
        world->dungeon = NULL;
        world->things = NULL;
        world->ownsDungeon = 0;
    }
    F0883_WORLD_Free_Compat(world);
    *world = candidate_world;
    memset(&candidate_world, 0, sizeof(candidate_world));
    if (event_queue) {
        *event_queue = candidate_queue;
    }
    if (out_report) {
        *out_report = candidate_report;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
    const char *path,
    const struct GameWorld_Compat *start_world,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct GameWorld_Compat candidate_world;
    struct DM1_EventQueue_V1 candidate_queue;
    DM1OriginalSavePC34HandoffReport candidate_report;
    char backup_path[DM1_ORIGINAL_SAVE_PATH_MAX];
    const char *load_path = path;
    int resumed_from_backup = 0;
    int result;

    if (!path || !out_world || out_world == start_world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    memset(&candidate_world, 0, sizeof(candidate_world));
    memset(&candidate_queue, 0, sizeof(candidate_queue));
    memset(&candidate_report, 0, sizeof(candidate_report));
    /* ReDMCSB LOADSAVE.C F0435 lines 2560-2583 only opens the backup when
     * the primary cannot be opened. A malformed primary must fail closed;
     * it must never be replaced by an older backup. */
    if (!dm1_original_save_file_opens_for_read(path)) {
        if (!dm1_original_save_backup_path(path, backup_path) ||
            !dm1_original_save_file_opens_for_read(backup_path)) {
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
        }
        load_path = backup_path;
        resumed_from_backup = 1;
    }
    /* M11 reaches this boundary only after its original-save classifier has
     * selected the F0435 route. Keep the final file admission here as well:
     * a Firestaff F0433 export is useful for verification, but is not an
     * external original-save corpus member and must not become a launcher
     * resume source. There is deliberately no native-save/synthetic retry
     * from this route. Apply the same rule to a recovered .bak source. */
    if (!dm1_original_save_corpus_external_pc34_file(load_path, NULL)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }
    /* ReDMCSB LOADSAVE.C F0435 restores a tail-less save against the active
     * DM1 dungeon.  Seed the candidate before C3/C4 materialization so its
     * events resolve against that real backing rather than a host substitute. */
    if (start_world) {
        candidate_world.dungeon = start_world->dungeon;
        candidate_world.things = start_world->things;
        candidate_world.ownsDungeon = 0;
    }
    result = dm1_v1_original_save_pc34_handoff_load_world_from_file(
        load_path, &candidate_world, event_queue ? &candidate_queue : NULL,
        &candidate_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return result;
    }

    /* ReDMCSB LOADSAVE.C F0435 loads the dungeon after PARTY/EVENT/TIMELINE.
     * A PC34 stream without that optional tail is therefore resumed against
     * the already materialized DM1 start dungeon, never a host-made HoC
     * substitute. */
    if (!candidate_world.dungeon && start_world) {
        candidate_world.dungeon = start_world->dungeon;
        candidate_world.things = start_world->things;
        candidate_world.ownsDungeon = 0;
    }
    if (!candidate_world.dungeon || !candidate_world.things) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    candidate_report.resumed_from_backup = resumed_from_backup;
    /* The save source is promoted only after every part, optional dungeon
     * tail, and borrowed-start-dungeon invariant has passed. If promotion
     * fails, leave both the destination world and the backup untouched. */
    if (resumed_from_backup) {
        if (rename(backup_path, path) != 0) {
            F0883_WORLD_Free_Compat(&candidate_world);
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
        }
        candidate_report.backup_promoted_to_primary = 1;
    }
    F0883_WORLD_Free_Compat(out_world);
    *out_world = candidate_world;
    memset(&candidate_world, 0, sizeof(candidate_world));
    if (event_queue) {
        *event_queue = candidate_queue;
    }
    if (out_report) {
        *out_report = candidate_report;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
    const uint8_t *bytes,
    size_t size,
    const struct GameWorld_Compat *start_world,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct GameWorld_Compat candidate_world;
    struct DM1_EventQueue_V1 candidate_queue;
    DM1OriginalSavePC34HandoffReport candidate_report;
    int result;

    if (!bytes || size == 0u || !out_world || out_world == start_world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    memset(&candidate_world, 0, sizeof(candidate_world));
    memset(&candidate_queue, 0, sizeof(candidate_queue));
    memset(&candidate_report, 0, sizeof(candidate_report));
    /* ReDMCSB LOADSAVE.C F0435 restores the optional dungeon only after the
     * save parts. A tail-less immutable import therefore has to borrow the
     * active DM1 backing before C3/C4 materialization, just like the file
     * route; otherwise an M11 import would lose its source dungeon. */
    if (start_world) {
        candidate_world.dungeon = start_world->dungeon;
        candidate_world.things = start_world->things;
        candidate_world.ownsDungeon = 0;
    }
    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, size, &candidate_world,
        event_queue ? &candidate_queue : NULL, &candidate_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return result;
    }
    if (!candidate_world.dungeon || !candidate_world.things) {
        F0883_WORLD_Free_Compat(&candidate_world);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    F0883_WORLD_Free_Compat(out_world);
    *out_world = candidate_world;
    memset(&candidate_world, 0, sizeof(candidate_world));
    if (event_queue) {
        *event_queue = candidate_queue;
    }
    if (out_report) {
        *out_report = candidate_report;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
    struct GameWorld_Compat *runtime_world,
    struct GameWorld_Compat *loaded_world)
{
    int reuses_start_dungeon;

    if (!runtime_world || !loaded_world || runtime_world == loaded_world) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    /* Native quicksaves serialize runtime data but retain the start dungeon
     * outside that blob.  Give them the same DM1-owned materialized backing
     * used by a tail-less original save before transferring ownership. */
    if (!loaded_world->dungeon && runtime_world->dungeon) {
        loaded_world->dungeon = runtime_world->dungeon;
        loaded_world->things = runtime_world->things;
        loaded_world->ownsDungeon = 0;
    }
    reuses_start_dungeon = loaded_world->dungeon == runtime_world->dungeon &&
                         loaded_world->things == runtime_world->things;
    if (reuses_start_dungeon) {
        loaded_world->ownsDungeon = runtime_world->ownsDungeon;
        runtime_world->dungeon = NULL;
        runtime_world->things = NULL;
        runtime_world->ownsDungeon = 0;
    }
    F0883_WORLD_Free_Compat(runtime_world);
    *runtime_world = *loaded_world;
    memset(loaded_world, 0, sizeof(*loaded_world));
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

/* ReDMCSB LOADSAVE.C F0435 restores GameTime, EVENTS, and TIMELINE as one
 * resumed-runtime state before F0651 exposes the queue.  The candidate-world
 * materializer has already checked source C3/C4 bytes; repeat the ownership
 * invariants here so no caller can publish a separately damaged queue after
 * the world has moved. */
static int original_pc34_runtime_queue_matches_world(
    const struct GameWorld_Compat *world,
    const struct DM1_EventQueue_V1 *queue)
{
    uint8_t seen[DM1_EVENT_MAX_COUNT];
    int i;

    if (!world || !queue ||
        queue->maxEvents != DM1_EVENT_MAX_COUNT ||
        queue->eventCount < 0 || queue->eventCount > DM1_EVENT_MAX_COUNT ||
        queue->firstUnusedIndex < 0 ||
        queue->firstUnusedIndex > DM1_EVENT_MAX_COUNT ||
        world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY ||
        world->timeline.nowTick != world->gameTick ||
        queue->gameTick != world->gameTick ||
        queue->eventCount != world->timeline.count) {
        return 0;
    }

    memset(seen, 0, sizeof(seen));
    for (i = 0; i < queue->eventCount; ++i) {
        const uint16_t event_index = queue->timeline[i];

        if (event_index >= DM1_EVENT_MAX_COUNT || seen[event_index] ||
            queue->events[event_index].type == DM1_EVENT_NONE) {
            return 0;
        }
        seen[event_index] = 1;
    }
    return 1;
}

int dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
    struct GameWorld_Compat *runtime_world,
    struct DM1_EventQueue_V1 *runtime_queue,
    struct GameWorld_Compat *loaded_world,
    struct DM1_EventQueue_V1 *loaded_queue)
{
    int result;

    if (!runtime_world || !runtime_queue || !loaded_world || !loaded_queue ||
        runtime_world == loaded_world || runtime_queue == loaded_queue) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (!original_pc34_runtime_queue_matches_world(loaded_world,
                                                   loaded_queue)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    result = dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
        runtime_world, loaded_world);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }
    *runtime_queue = *loaded_queue;
    memset(loaded_queue, 0, sizeof(*loaded_queue));
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *runtime_world,
    struct DM1_EventQueue_V1 *runtime_queue,
    DM1OriginalSavePC34HandoffReport *out_report)
{
    struct GameWorld_Compat loaded_world;
    struct DM1_EventQueue_V1 loaded_queue;
    DM1OriginalSavePC34HandoffReport loaded_report;
    int result;

    if (!bytes || size == 0u || !runtime_world || !runtime_queue) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    memset(&loaded_world, 0, sizeof(loaded_world));
    memset(&loaded_queue, 0, sizeof(loaded_queue));
    memset(&loaded_report, 0, sizeof(loaded_report));

    /* F0435 completes its candidate load against the live DM1 backing before
     * exposing either restored WORLD state or the F0238 event heap. Keep the
     * two ownership transfers together so a valid C3/C4 queue cannot become
     * detached from the world it was decoded for. */
    result = dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
        bytes, size, runtime_world, &loaded_world, &loaded_queue,
        &loaded_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&loaded_world);
        return result;
    }
    if (!dm1_original_save_special_events_adoptable(&loaded_report,
                                                    &loaded_world)) {
        F0883_WORLD_Free_Compat(&loaded_world);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (!validate_original_pc34_tail_runtime_receipt(&loaded_report,
                                                     &loaded_world)) {
        F0883_WORLD_Free_Compat(&loaded_world);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (!validate_original_pc34_c13_party_runtime_receipt(&loaded_report,
                                                           &loaded_world)) {
        F0883_WORLD_Free_Compat(&loaded_world);
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    result = dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
        runtime_world, runtime_queue, &loaded_world, &loaded_queue);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&loaded_world);
        return result;
    }
    if (out_report) {
        *out_report = loaded_report;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

void dm1_v1_original_save_pc34_handoff_normalize_hoc_resume_state(
    const struct GameWorld_Compat *world,
    DM1OriginalSavePC34HoCResumeState *state)
{
    if (!state) {
        return;
    }
    if (!world || !state->candidate_panel_active ||
        state->candidate_mirror_ordinal < 0 ||
        state->candidate_party_index < 0 ||
        state->candidate_party_index >= world->party.championCount ||
        state->candidate_party_index >= CHAMPION_MAX_PARTY ||
        !world->party.champions[state->candidate_party_index].present) {
        state->candidate_mirror_ordinal = -1;
        state->candidate_party_index = -1;
        state->candidate_panel_active = 0;
        /* ReDMCSB LOADSAVE.C F0435 restores PARTY and dungeon state, not
         * Firestaff's transient C040 panel. A rejected/stale candidate must
         * therefore also close its dependent inventory surface; otherwise a
         * quicksave sidecar can paint a false HoC panel after the restored
         * world has no live mirror candidate. */
        state->inventory_panel_active = 0;
        return;
    }
    /* F0280 has already appended the candidate when C040 opens. */
    state->candidate_panel_active = 1;
    state->inventory_panel_active = 1;
}

/* F0802 reconstructs ACTIVE_GROUP from the generic creature-AI projection.
 * That projection has no owner for the upper packed Directions bits or the
 * independent Home location. During an original-save round trip, restore
 * those authenticated DEFS.H record fields after F0433 has written its part,
 * then recompute the source F0417/F0430 receipts. */
static int restore_original_pc34_active_group_records(
    uint8_t *bytes,
    size_t size,
    const uint8_t *source_bytes,
    size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report)
{
    uint8_t *header;
    uint8_t *part;
    uint8_t *party_part;
    uint8_t header_meta[SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS * 2u];
    uint8_t party_receipt[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    size_t cursor;
    uint16_t header_key;
    uint16_t part_key;
    uint16_t checksum;
    uint16_t old_checksum;
    uint16_t party_key;
    uint16_t old_party_key;
    uint16_t party_checksum;
    uint16_t old_party_checksum;
    uint16_t expected_header_checksum;
    uint16_t part_size;
    int count;
    int record_count;
    int i;

    if (!bytes || !source_bytes || !source_report ||
        size < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2u) {
        return 0;
    }
    count = source_report->reported_active_group_count;
    if (source_report->original_current_active_group_count < 0 ||
        source_report->original_current_active_group_count > count ||
        count < 0 ||
        count > DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP) {
        return 0;
    }

    header = bytes;
    header_key = read_u16_le(
        header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u);
    cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    if (cursor + 2u > size ||
        read_u16_le(bytes + cursor) != SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT) {
        goto fail;
    }
    cursor += 2u + SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT;
    if (cursor + 2u > size) {
        goto fail;
    }
    part_size = read_u16_le(bytes + cursor);
    if (part_size != (uint16_t)((size_t)count *
                                DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT) ||
        (size_t)part_size > size - cursor - 2u) {
        goto fail;
    }
    part = bytes + cursor + 2u;
    party_part = part + part_size + 2u;
    if ((size_t)part_size + 4u > size - cursor ||
        size - cursor - 2u - (size_t)part_size < 2u ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT >
            size - cursor - 2u - (size_t)part_size - 2u ||
        read_u16_le(part + part_size) !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        source_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        source_report->pc34_party_part_byte_offset > source_size ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT >
            source_size - source_report->pc34_party_part_byte_offset) {
        goto fail;
    }
    memcpy(header_meta, header + SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS * 2u,
           sizeof(header_meta));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        header_meta, SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, header_key);
    part_key = read_u16_le(header_meta + 54u +
        (size_t)SAVEGAME_PC34_PART_ACTIVE_GROUP * 2u);
    old_checksum = read_u16_le(header_meta + 86u +
        (size_t)SAVEGAME_PC34_PART_ACTIVE_GROUP * 2u);
    old_party_key = read_u16_le(header_meta + 54u +
        (size_t)SAVEGAME_PC34_PART_PARTY * 2u);
    old_party_checksum = read_u16_le(header_meta + 86u +
        (size_t)SAVEGAME_PC34_PART_PARTY * 2u);
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        part, (size_t)part_size / 2u, part_key);
    record_count = (int)((size_t)part_size /
                         DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT);
    for (i = 0; i < record_count; ++i) {
        const DM1OriginalSavePC34ActiveGroupRecord *src =
            &source_report->active_groups[i];
        uint8_t *dst = part + (size_t)i *
            DM1_PC34_ORIGINAL_ACTIVE_GROUP_BYTE_COUNT;

        write_u16_le(dst, (uint16_t)src->group_thing_index);
        dst[2u] = src->directions;
        dst[3u] = src->cells;
        dst[4u] = src->last_move_time;
        dst[5u] = src->delay_fleeing_from_target;
        dst[6u] = src->target_map_x;
        dst[7u] = src->target_map_y;
        dst[8u] = src->prior_map_x;
        dst[9u] = src->prior_map_y;
        dst[10u] = src->home_map_x;
        dst[11u] = src->home_map_y;
        memcpy(dst + 12u, src->aspect, sizeof(src->aspect));
    }
    checksum = F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        part, (size_t)part_size / 2u, part_key);
    /* PARTY is an authenticated original F0417 record. F0802's generic
     * state projection cannot own every PC34 byte, so retain its exact
     * ciphertext/key/checksum at the target's strict PARTY boundary. */
    memcpy(party_part,
           source_bytes + source_report->pc34_party_part_byte_offset,
           DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT);
    write_u16_le(party_part - 2u, DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT);
    party_key = source_report->pc34_party_part_key;
    memcpy(party_receipt, party_part, sizeof(party_receipt));
    party_checksum = F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        party_receipt, sizeof(party_receipt) / 2u, party_key);
    expected_header_checksum = (uint16_t)(
        original_pc34_header_first_half_checksum(header) - old_checksum -
        old_party_key - old_party_checksum + checksum + party_key +
        party_checksum);
    write_u16_le(header_meta + 86u +
                 (size_t)SAVEGAME_PC34_PART_ACTIVE_GROUP * 2u, checksum);
    write_u16_le(header_meta + 54u +
                 (size_t)SAVEGAME_PC34_PART_PARTY * 2u, party_key);
    write_u16_le(header_meta + 86u +
                 (size_t)SAVEGAME_PC34_PART_PARTY * 2u, party_checksum);
    original_pc34_fix_header_noise_checksum(header, expected_header_checksum);
    memcpy(header + SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS * 2u, header_meta,
           sizeof(header_meta));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(
        header + SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS * 2u,
        SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS, header_key);
    return 1;

fail:
    return 0;
}

int dm1_v1_original_save_pc34_roundtrip_world_bytes(
    const uint8_t *bytes,
    size_t size,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34HandoffReport *import_report,
    DM1OriginalSavePC34HandoffReport *verify_report)
{
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 event_queue;
    struct SaveGame_Compat verify_state;
    int written = 0;
    int result;

    if (!bytes || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    *out_size = 0u;
    if (size > (size_t)((int)0x7fffffff) ||
        out_capacity > (size_t)((int)0x7fffffff)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    memset(&world, 0, sizeof(world));
    memset(&event_queue, 0, sizeof(event_queue));

    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        bytes, size, &world, &event_queue, import_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&world);
        return result;
    }

    /* ReDMCSB LOADSAVE.C F0435 materializes GLOBAL_DATA, ACTIVE_GROUP,
     * PARTY, EVENT/TIMELINE, and optional dungeon bytes before runtime
     * resumes; F0433 then writes those same save parts back through
     * READWRIT.C F0420. This helper pins that import-export-import
     * contract for Firestaff's bounded DM1 world handoff. */
    result = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, game_id, out_bytes, (int)out_capacity, &written);
    F0883_WORLD_Free_Compat(&world);
    if (result != SAVEGAME_PC34_OK) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    if (import_report && !restore_original_pc34_active_group_records(
                             out_bytes, (size_t)written, bytes, size,
                             import_report)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }

    memset(&verify_state, 0, sizeof(verify_state));
    result = dm1_v1_original_save_pc34_handoff_bytes(
        out_bytes, (size_t)written, &verify_state, verify_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    *out_size = (size_t)written;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

static int dm1_original_save_external_portraits_match(
    const uint8_t *source_bytes,
    size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes,
    size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report)
{
    uint32_t portrait_bytes = SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;

    if (!source_bytes || !source_report || !exported_bytes ||
        !exported_report ||
        source_report->external_portrait_byte_count != portrait_bytes ||
        exported_report->external_portrait_byte_count != portrait_bytes ||
        source_report->external_portrait_byte_offset > source_size ||
        portrait_bytes > source_size -
            source_report->external_portrait_byte_offset ||
        exported_report->external_portrait_byte_offset > exported_size ||
        portrait_bytes > exported_size -
            exported_report->external_portrait_byte_offset) {
        return 0;
    }
    return memcmp(source_bytes + source_report->external_portrait_byte_offset,
                  exported_bytes + exported_report->external_portrait_byte_offset,
                  portrait_bytes) == 0;
}

static int dm1_original_save_inactive_champion_records_match(
    const uint8_t *source_bytes,
    size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes,
    size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t source_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    uint8_t exported_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    int source_count;
    int exported_count;
    int slot;

    if (!source_bytes || !source_report || !exported_bytes ||
        !exported_report || !out_report ||
        source_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        exported_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        source_report->pc34_party_part_byte_offset > source_size ||
        exported_report->pc34_party_part_byte_offset > exported_size ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT > source_size -
            source_report->pc34_party_part_byte_offset ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT > exported_size -
            exported_report->pc34_party_part_byte_offset) {
        return 0;
    }
    memcpy(source_part,
           source_bytes + source_report->pc34_party_part_byte_offset,
           sizeof(source_part));
    memcpy(exported_part,
           exported_bytes + exported_report->pc34_party_part_byte_offset,
           sizeof(exported_part));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(source_part,
                                   sizeof(source_part) / 2u,
                                   source_report->pc34_party_part_key);
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(exported_part,
                                   sizeof(exported_part) / 2u,
                                   exported_report->pc34_party_part_key);
    source_count = source_report->imported_champion_count;
    exported_count = exported_report->imported_champion_count;
    if (source_count < 0 || source_count > CHAMPION_MAX_PARTY ||
        exported_count != source_count) {
        return 0;
    }
    out_report->inactive_champion_record_byte_receipt_available = 1;
    out_report->inactive_champion_record_count =
        CHAMPION_MAX_PARTY - source_count;
    for (slot = source_count; slot < CHAMPION_MAX_PARTY; ++slot) {
        const size_t offset = (size_t)slot *
            DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT;
        if (memcmp(source_part + offset, exported_part + offset,
                   DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT) == 0) {
            ++out_report->inactive_champion_record_byte_preserved_count;
        }
    }
    out_report->inactive_champion_record_byte_preservation_ok =
        out_report->inactive_champion_record_byte_preserved_count ==
        out_report->inactive_champion_record_count;
    return 1;
}

/* ReDMCSB LOADSAVE.C F0433/F0435 carries the complete fixed M516_CHAMPIONS
 * prefix in C2 before PARTY_INFO. This receipt hashes only those four raw
 * 319-byte records after their own F0417 deobfuscation; it does not infer
 * champion state or alter the importer/exporter. */
static int dm1_original_save_m516_champion_records_match(
    const uint8_t *source_bytes, size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes, size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t source_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    uint8_t exported_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    const size_t m516_bytes = DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT *
        CHAMPION_MAX_PARTY;

    if (!source_bytes || !source_report || !exported_bytes ||
        !exported_report || !out_report ||
        source_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        exported_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        source_report->pc34_party_part_byte_offset > source_size ||
        exported_report->pc34_party_part_byte_offset > exported_size ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT >
            source_size - source_report->pc34_party_part_byte_offset ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT >
            exported_size - exported_report->pc34_party_part_byte_offset) {
        return 0;
    }
    memcpy(source_part, source_bytes + source_report->pc34_party_part_byte_offset,
           sizeof(source_part));
    memcpy(exported_part,
           exported_bytes + exported_report->pc34_party_part_byte_offset,
           sizeof(exported_part));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(source_part, sizeof(source_part) / 2u,
                                   source_report->pc34_party_part_key);
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(exported_part, sizeof(exported_part) / 2u,
                                   exported_report->pc34_party_part_key);
    out_report->m516_champion_record_receipt_available = 1;
    out_report->source_m516_champion_record_count = CHAMPION_MAX_PARTY;
    out_report->exported_m516_champion_record_count = CHAMPION_MAX_PARTY;
    out_report->source_m516_champion_record_byte_count = (uint32_t)m516_bytes;
    out_report->exported_m516_champion_record_byte_count = (uint32_t)m516_bytes;
    out_report->source_m516_champion_record_fingerprint =
        dm1_original_save_hash_bytes(source_part, m516_bytes);
    out_report->exported_m516_champion_record_fingerprint =
        dm1_original_save_hash_bytes(exported_part, m516_bytes);
    out_report->m516_champion_record_byte_preservation_ok = memcmp(
        source_part, exported_part, m516_bytes) == 0;
    return 1;
}

/* ReDMCSB CLIKVIEW.C F0374:179-186 sets C13.Priority from the dropped
 * bones' ChargeCount. That is an index into M516_CHAMPIONS, not a generic
 * host champion identity. F0433/F0435 copy the complete C2 PARTY block, so
 * the C13-selected active 319-byte source record must survive unchanged. */
static int dm1_original_save_c13_champion_records_match(
    const uint8_t *source_bytes,
    size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes,
    size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t source_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    uint8_t exported_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    int source_count;
    int exported_count;
    int i;

    if (!source_bytes || !source_report || !exported_bytes ||
        !exported_report || !out_report ||
        source_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        exported_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        source_report->pc34_party_part_byte_offset > source_size ||
        exported_report->pc34_party_part_byte_offset > exported_size ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT > source_size -
            source_report->pc34_party_part_byte_offset ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT > exported_size -
            exported_report->pc34_party_part_byte_offset) {
        return 0;
    }
    source_count = source_report->imported_champion_count;
    exported_count = exported_report->imported_champion_count;
    if (source_count < 0 || source_count > CHAMPION_MAX_PARTY ||
        exported_count != source_count) {
        return 0;
    }
    memcpy(source_part,
           source_bytes + source_report->pc34_party_part_byte_offset,
           sizeof(source_part));
    memcpy(exported_part,
           exported_bytes + exported_report->pc34_party_part_byte_offset,
           sizeof(exported_part));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(source_part,
                                   sizeof(source_part) / 2u,
                                   source_report->pc34_party_part_key);
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(exported_part,
                                   sizeof(exported_part) / 2u,
                                   exported_report->pc34_party_part_key);
    out_report->c13_champion_record_byte_receipt_available = 1;
    for (i = 0; i < source_report->decoded_event_count; ++i) {
        const struct DM1_Event_V1 *event = &source_report->events[i];
        size_t offset;

        if (event->type != DM1_EVENT_VI_ALTAR_REBIRTH) {
            continue;
        }
        ++out_report->source_c13_champion_record_reference_count;
        if (event->priority >= (uint8_t)source_count) {
            ++out_report->c13_champion_record_byte_mismatch_count;
            continue;
        }
        offset = (size_t)event->priority *
            DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT;
        if (memcmp(source_part + offset, exported_part + offset,
                   DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT) == 0) {
            ++out_report->c13_champion_record_byte_preserved_count;
        } else {
            ++out_report->c13_champion_record_byte_mismatch_count;
        }
    }
    out_report->c13_champion_record_byte_preservation_ok =
        out_report->c13_champion_record_byte_preserved_count ==
        out_report->source_c13_champion_record_reference_count;
    return 1;
}

static int dm1_original_save_party_info_bytes_match(
    const uint8_t *source_bytes,
    size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes,
    size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t source_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    uint8_t exported_part[DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT];
    const size_t party_info_offset =
        DM1_PC34_ORIGINAL_CHAMPION_BYTE_COUNT * CHAMPION_MAX_PARTY;

    if (!source_bytes || !source_report || !exported_bytes ||
        !exported_report || !out_report ||
        source_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        exported_report->part_byte_counts[SAVEGAME_PC34_PART_PARTY] !=
            DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT ||
        source_report->pc34_party_part_byte_offset > source_size ||
        exported_report->pc34_party_part_byte_offset > exported_size ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT > source_size -
            source_report->pc34_party_part_byte_offset ||
        DM1_PC34_ORIGINAL_PARTY_PART_BYTE_COUNT > exported_size -
            exported_report->pc34_party_part_byte_offset) {
        return 0;
    }
    memcpy(source_part,
           source_bytes + source_report->pc34_party_part_byte_offset,
           sizeof(source_part));
    memcpy(exported_part,
           exported_bytes + exported_report->pc34_party_part_byte_offset,
           sizeof(exported_part));
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(source_part,
                                   sizeof(source_part) / 2u,
                                   source_report->pc34_party_part_key);
    (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(exported_part,
                                   sizeof(exported_part) / 2u,
                                   exported_report->pc34_party_part_key);
    out_report->party_info_byte_receipt_available = 1;
    out_report->source_party_info_byte_count = PARTY_PC34_SAVE_INFO_BYTE_COUNT;
    out_report->source_party_info_fingerprint = dm1_original_save_hash_bytes(
        source_part + party_info_offset, PARTY_PC34_SAVE_INFO_BYTE_COUNT);
    out_report->exported_party_info_byte_count = PARTY_PC34_SAVE_INFO_BYTE_COUNT;
    out_report->exported_party_info_fingerprint = dm1_original_save_hash_bytes(
        exported_part + party_info_offset, PARTY_PC34_SAVE_INFO_BYTE_COUNT);
    out_report->party_info_byte_preservation_ok = memcmp(
        source_part + party_info_offset, exported_part + party_info_offset,
        PARTY_PC34_SAVE_INFO_BYTE_COUNT) == 0;
    return 1;
}

static int dm1_original_save_c4_timeline_bytes_match(
    const uint8_t *source_bytes, size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes, size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    const uint8_t *inputs[2] = { source_bytes, exported_bytes };
    const size_t sizes[2] = { source_size, exported_size };
    const DM1OriginalSavePC34HandoffReport *reports[2] = {
        source_report, exported_report };
    uint8_t decoded[2][SAVEGAME_PC34_TIMELINE_BYTE_COUNT];
    size_t byte_counts[2];
    int which;

    if (!source_bytes || !exported_bytes || !source_report || !exported_report ||
        !out_report) return 0;
    for (which = 0; which < 2; ++which) {
        uint8_t meta[256];
        size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
        uint16_t key;
        int part;
        if (sizes[which] < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE ||
            reports[which]->part_byte_counts[SAVEGAME_PC34_PART_TIMELINE] >
                sizeof(decoded[which])) return 0;
        key = read_u16_le(inputs[which] + 20u);
        memcpy(meta, inputs[which] + 256u, sizeof(meta));
        (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(meta,
                                       SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS,
                                       key);
        for (part = 0; part < SAVEGAME_PC34_PART_TIMELINE; ++part) {
            uint16_t n;
            if (cursor + 2u > sizes[which]) return 0;
            n = read_u16_le(inputs[which] + cursor);
            cursor += 2u;
            if (cursor + n > sizes[which]) return 0;
            cursor += n;
        }
        if (cursor + 2u > sizes[which]) return 0;
        byte_counts[which] = read_u16_le(inputs[which] + cursor);
        cursor += 2u;
        if (byte_counts[which] !=
                reports[which]->part_byte_counts[SAVEGAME_PC34_PART_TIMELINE] ||
            (byte_counts[which] & 1u) != 0u ||
            cursor + byte_counts[which] > sizes[which]) return 0;
        memcpy(decoded[which], inputs[which] + cursor, byte_counts[which]);
        (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(decoded[which], byte_counts[which] / 2u,
            read_u16_le(meta + 54u + SAVEGAME_PC34_PART_TIMELINE * 2u));
    }
    out_report->c4_timeline_layout_receipt_available = 1;
    out_report->source_c4_timeline_index_count = (uint32_t)(byte_counts[0] / 2u);
    out_report->source_c4_timeline_byte_count = (uint32_t)byte_counts[0];
    out_report->source_c4_timeline_fingerprint =
        dm1_original_save_hash_bytes(decoded[0], byte_counts[0]);
    out_report->exported_c4_timeline_index_count = (uint32_t)(byte_counts[1] / 2u);
    out_report->exported_c4_timeline_byte_count = (uint32_t)byte_counts[1];
    out_report->exported_c4_timeline_fingerprint =
        dm1_original_save_hash_bytes(decoded[1], byte_counts[1]);
    out_report->c4_timeline_byte_preservation_ok =
        byte_counts[0] == byte_counts[1] &&
        memcmp(decoded[0], decoded[1], byte_counts[0]) == 0;
    return 1;
}

/* ReDMCSB LOADSAVE.C F0433 writes and F0435 reads the complete C3 EVENT
 * allocation, before TIMELINE.C can interpret its indexes. Unlike the
 * canonical per-type receipts, this preserves raw slot order and every byte. */
static int dm1_original_save_c3_event_bytes_match(
    const uint8_t *source_bytes, size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes, size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    const uint8_t *inputs[2] = { source_bytes, exported_bytes };
    const size_t sizes[2] = { source_size, exported_size };
    const DM1OriginalSavePC34HandoffReport *reports[2] = {
        source_report, exported_report };
    uint8_t decoded[2][DM1_EVENT_MAX_COUNT * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT];
    size_t byte_counts[2];
    int which;

    if (!source_bytes || !exported_bytes || !source_report || !exported_report ||
        !out_report) return 0;
    for (which = 0; which < 2; ++which) {
        uint8_t meta[256];
        size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
        uint16_t key;
        int part;

        if (sizes[which] < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE ||
            reports[which]->part_byte_counts[SAVEGAME_PC34_PART_EVENTS] >
                sizeof(decoded[which])) return 0;
        key = read_u16_le(inputs[which] + 20u);
        memcpy(meta, inputs[which] + 256u, sizeof(meta));
        (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(meta,
                                       SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS,
                                       key);
        for (part = 0; part < SAVEGAME_PC34_PART_EVENTS; ++part) {
            uint16_t n;
            if (cursor + 2u > sizes[which]) return 0;
            n = read_u16_le(inputs[which] + cursor);
            cursor += 2u;
            if (cursor + n > sizes[which]) return 0;
            cursor += n;
        }
        if (cursor + 2u > sizes[which]) return 0;
        byte_counts[which] = read_u16_le(inputs[which] + cursor);
        cursor += 2u;
        if (byte_counts[which] !=
                reports[which]->part_byte_counts[SAVEGAME_PC34_PART_EVENTS] ||
            byte_counts[which] % DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT != 0u ||
            cursor + byte_counts[which] > sizes[which]) return 0;
        memcpy(decoded[which], inputs[which] + cursor, byte_counts[which]);
        (void)F0417_SAVEUTIL_GetChecksumAndObfuscatePC34_Compat(decoded[which], byte_counts[which] / 2u,
            read_u16_le(meta + 54u + SAVEGAME_PC34_PART_EVENTS * 2u));
    }
    out_report->c3_event_layout_receipt_available = 1;
    out_report->source_c3_event_record_count =
        (uint32_t)(byte_counts[0] / DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT);
    out_report->source_c3_event_byte_count = (uint32_t)byte_counts[0];
    out_report->source_c3_event_fingerprint =
        dm1_original_save_hash_bytes(decoded[0], byte_counts[0]);
    out_report->exported_c3_event_record_count =
        (uint32_t)(byte_counts[1] / DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT);
    out_report->exported_c3_event_byte_count = (uint32_t)byte_counts[1];
    out_report->exported_c3_event_fingerprint =
        dm1_original_save_hash_bytes(decoded[1], byte_counts[1]);
    out_report->c3_event_byte_preservation_ok =
        byte_counts[0] == byte_counts[1] &&
        memcmp(decoded[0], decoded[1], byte_counts[0]) == 0;
    return 1;
}

/* ReDMCSB LOADSAVE.C F0433:1573-1627 constructs five source save-part
 * descriptors, writes their uint16 byte counts, and stamps FormatID/GameID/
 * Platform/DungeonID before F0430 obfuscates the header. F0435 consumes that
 * same header identity and each raw length prefix. Keys/checksums are fresh
 * F0433 output and AdditionalData is Firestaff's manifest area, so neither
 * belongs in an external-original mirror receipt. */
static int dm1_original_save_header_part_shape_match(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    int i;

    if (!source_report || !exported_report || !out_report ||
        !source_report->classify.header_checksum_ok ||
        !exported_report->classify.header_checksum_ok ||
        source_report->part_checksum_ok_count != SAVEGAME_PC34_PART_COUNT ||
        exported_report->part_checksum_ok_count != SAVEGAME_PC34_PART_COUNT) {
        return 0;
    }
    out_report->header_part_shape_receipt_available = 1;
    out_report->source_header_format_id = source_report->classify.format_id;
    out_report->exported_header_format_id = exported_report->classify.format_id;
    out_report->source_header_platform = source_report->classify.platform;
    out_report->exported_header_platform = exported_report->classify.platform;
    out_report->source_header_dungeon_id = source_report->classify.dungeon_id;
    out_report->exported_header_dungeon_id =
        exported_report->classify.dungeon_id;
    out_report->source_header_game_id = source_report->classify.game_id;
    out_report->exported_header_game_id = exported_report->classify.game_id;
    out_report->header_identity_preservation_ok =
        out_report->source_header_format_id ==
            out_report->exported_header_format_id &&
        out_report->source_header_platform == out_report->exported_header_platform &&
        out_report->source_header_dungeon_id ==
            out_report->exported_header_dungeon_id &&
        out_report->source_header_game_id == out_report->exported_header_game_id;
    out_report->part_byte_count_preservation_ok = 1;
    for (i = 0; i < SAVEGAME_PC34_PART_COUNT; ++i) {
        out_report->source_part_byte_counts[i] =
            source_report->part_byte_counts[i];
        out_report->exported_part_byte_counts[i] =
            exported_report->part_byte_counts[i];
        if (out_report->source_part_byte_counts[i] !=
            out_report->exported_part_byte_counts[i]) {
            out_report->part_byte_count_preservation_ok = 0;
        }
    }
    return 1;
}

/* ReDMCSB LOADSAVE.C F0433 writes the optional loaded dungeon immediately
 * after the four external portraits, and F0435 reads it from the same cursor.
 * Rebuild its body through the bounded F0422 writer before comparing a corpus
 * receipt, so an independently checksum-valid decoded tail cannot borrow an
 * unchecked byte stream. */
static int dm1_original_save_dungeon_tail_f0422_roundtrip_ok(
    const uint8_t *tail,
    size_t tail_size,
    uint16_t expected_checksum)
{
    uint8_t *staged;
    size_t body_size;
    size_t cursor = 0u;
    uint16_t checksum = 0u;
    int ok;

    if (!tail || tail_size < 2u) {
        return 0;
    }
    body_size = tail_size - 2u;
    staged = (uint8_t *)malloc(body_size ? body_size : 1u);
    if (!staged) {
        return 0;
    }
    ok = dm1_v1_original_save_pc34_f0422_write_bytes_with_checksum(
             staged, body_size, &cursor, tail, body_size, &checksum) &&
         cursor == body_size && checksum == expected_checksum &&
         memcmp(staged, tail, body_size) == 0;
    free(staged);
    return ok;
}

/* This is a corpus-only raw-byte receipt: no tail bytes are decoded or
 * promoted here. A tail-less original save must remain tail-less on export. */
static int dm1_original_save_dungeon_tail_bytes_match(
    const uint8_t *source_bytes,
    size_t source_size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    const uint8_t *exported_bytes,
    size_t exported_size,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    size_t source_cursor;
    size_t exported_cursor;

    if (!source_bytes || !source_report || !exported_bytes ||
        !exported_report || !out_report) {
        return 0;
    }
    out_report->source_dungeon_tail_byte_count =
        source_report->dungeon_tail_byte_count;
    out_report->source_dungeon_tail_fingerprint =
        source_report->dungeon_tail_fingerprint;
    out_report->exported_dungeon_tail_byte_count =
        exported_report->dungeon_tail_byte_count;
    out_report->exported_dungeon_tail_fingerprint =
        exported_report->dungeon_tail_fingerprint;
    out_report->dungeon_tail_byte_receipt_available = 1;

    if (!source_report->dungeon_tail_present) {
        out_report->dungeon_tail_byte_preservation_ok =
            !exported_report->dungeon_tail_present;
        return 1;
    }
    if (!source_report->dungeon_tail_checksum_ok ||
        !exported_report->dungeon_tail_present ||
        !exported_report->dungeon_tail_checksum_ok ||
        source_report->dungeon_tail_byte_count == 0u ||
        source_report->dungeon_tail_byte_count !=
            exported_report->dungeon_tail_byte_count) {
        return 1;
    }
    source_cursor = original_pc34_dungeon_tail_cursor(source_bytes, source_size);
    exported_cursor = original_pc34_dungeon_tail_cursor(exported_bytes,
                                                         exported_size);
    if (source_cursor == 0u || exported_cursor == 0u ||
        source_cursor > source_size || exported_cursor > exported_size ||
        source_report->dungeon_tail_byte_count > source_size - source_cursor ||
        exported_report->dungeon_tail_byte_count >
            exported_size - exported_cursor) {
        return 1;
    }
    if (!dm1_original_save_dungeon_tail_f0422_roundtrip_ok(
            source_bytes + source_cursor,
            source_report->dungeon_tail_byte_count,
            source_report->dungeon_tail_expected_checksum) ||
        !dm1_original_save_dungeon_tail_f0422_roundtrip_ok(
            exported_bytes + exported_cursor,
            exported_report->dungeon_tail_byte_count,
            exported_report->dungeon_tail_expected_checksum)) {
        return 1;
    }
    out_report->dungeon_tail_byte_preservation_ok = memcmp(
        source_bytes + source_cursor, exported_bytes + exported_cursor,
        source_report->dungeon_tail_byte_count) == 0;
    return 1;
}

/* ReDMCSB DEFS.H EVENT is written verbatim by LOADSAVE.C F0433. This is a
 * receipt serializer only: F0651 may reorder storage but not payload bytes. */
static void dm1_original_save_c13_event_receipt_bytes(
    const struct DM1_Event_V1 *event,
    uint8_t out_bytes[DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT])
{
    write_u32_le(out_bytes, event->map_time);
    out_bytes[4] = event->type;
    out_bytes[5] = event->priority;
    out_bytes[6] = event->b_mapX;
    out_bytes[7] = event->b_mapY;
    out_bytes[8] = event->c_cell;
    out_bytes[9] = event->c_effect;
}

static void dm1_original_save_sort_c13_receipt_rows(
    uint8_t rows[DM1_EVENT_MAX_COUNT][DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT],
    int count)
{
    int i;

    for (i = 1; i < count; ++i) {
        uint8_t row[DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT];
        int j = i;

        memcpy(row, rows[i], sizeof(row));
        while (j > 0 && memcmp(rows[j - 1], row, sizeof(row)) > 0) {
            memcpy(rows[j], rows[j - 1], sizeof(row));
            --j;
        }
        memcpy(rows[j], row, sizeof(row));
    }
}

/* This is deliberately an emission receipt, not a reconstructed event
 * assertion. F0435 accepted C13 from authenticated C3 bytes; F0433 must put
 * the same DEFS.H EVENT rows back into C3 before the unchanged C4/tail can
 * be certified. Sorting only removes storage-slot order from the receipt. */
static int dm1_original_save_c13_event_emission_bytes_match(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t rows[2][DM1_EVENT_MAX_COUNT][DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT];
    int counts[2] = {0, 0};
    const DM1OriginalSavePC34HandoffReport *reports[2] = {
        source_report, exported_report };
    int which;

    if (!source_report || !exported_report || !out_report) {
        return 0;
    }
    for (which = 0; which < 2; ++which) {
        int event_index;

        if (reports[which]->decoded_event_count < 0 ||
            reports[which]->decoded_event_count > DM1_EVENT_MAX_COUNT) {
            return 0;
        }
        for (event_index = 0;
             event_index < reports[which]->decoded_event_count;
             ++event_index) {
            const struct DM1_Event_V1 *event =
                &reports[which]->events[event_index];

            if (event->type != DM1_EVENT_VI_ALTAR_REBIRTH) {
                continue;
            }
            dm1_original_save_c13_event_receipt_bytes(
                event, rows[which][counts[which]++]);
        }
        dm1_original_save_sort_c13_receipt_rows(rows[which], counts[which]);
    }
    out_report->source_c13_event_count = counts[0];
    out_report->exported_c13_event_count = counts[1];
    out_report->c13_event_byte_preservation_ok =
        counts[0] == counts[1] &&
        memcmp(rows[0], rows[1],
               (size_t)counts[0] * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT) == 0;
    return 1;
}

/* Capture C13 at its source-owned C3 slot, after the original part has been
 * deobfuscated. EVENT semantic equality alone cannot prove these raw rows
 * survived F0435 -> F0433 without a substitution or slot rewrite. */
static int dm1_original_save_c13_raw_capture_bytes_match(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const DM1OriginalSavePC34HandoffReport *exported_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t source_rows[DM1_EVENT_MAX_COUNT][DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT];
    uint8_t exported_rows[DM1_EVENT_MAX_COUNT][DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT];
    int source_count = 0;
    int exported_count = 0;
    int preserved = 0;
    int mismatches = 0;
    int index;

    if (!source_report || !exported_report || !out_report ||
        source_report->decoded_event_count < 0 ||
        source_report->decoded_event_count > DM1_EVENT_MAX_COUNT ||
        exported_report->decoded_event_count !=
            source_report->decoded_event_count ||
        source_report->c3_raw_event_byte_count !=
            (uint32_t)source_report->decoded_event_count *
                DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT ||
        exported_report->c3_raw_event_byte_count !=
            (uint32_t)exported_report->decoded_event_count *
                DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT) {
        return 0;
    }
    for (index = 0; index < source_report->decoded_event_count; ++index) {
        const uint8_t *source_row = source_report->c3_raw_event_bytes +
            (size_t)index * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT;
        const uint8_t *exported_row = exported_report->c3_raw_event_bytes +
            (size_t)index * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT;
        const int source_is_c13 =
            source_report->events[index].type == DM1_EVENT_VI_ALTAR_REBIRTH;
        const int exported_is_c13 =
            exported_report->events[index].type == DM1_EVENT_VI_ALTAR_REBIRTH;

        if (source_is_c13) {
            memcpy(source_rows[source_count++], source_row,
                   DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT);
        }
        if (exported_is_c13) {
            memcpy(exported_rows[exported_count++], exported_row,
                   DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT);
        }
        if (source_is_c13 != exported_is_c13 ||
            (source_is_c13 && memcmp(source_row, exported_row,
                                     DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT) != 0)) {
            ++mismatches;
        } else if (source_is_c13) {
            ++preserved;
        }
    }
    if (source_count == 0) {
        return 1;
    }
    out_report->c13_raw_capture_receipt_available = 1;
    out_report->source_c13_raw_capture_count = source_count;
    out_report->exported_c13_raw_capture_count = exported_count;
    out_report->c13_raw_capture_byte_preserved_count = preserved;
    out_report->c13_raw_capture_byte_mismatch_count = mismatches;
    out_report->source_c13_raw_capture_byte_count =
        (uint32_t)source_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT;
    out_report->exported_c13_raw_capture_byte_count =
        (uint32_t)exported_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT;
    out_report->source_c13_raw_capture_fingerprint =
        dm1_original_save_hash_bytes(&source_rows[0][0],
            (size_t)source_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT);
    out_report->exported_c13_raw_capture_fingerprint =
        dm1_original_save_hash_bytes(&exported_rows[0][0],
            (size_t)exported_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT);
    out_report->c13_raw_capture_byte_preservation_ok =
        source_count == exported_count &&
        preserved == source_count &&
        mismatches == 0 &&
        memcmp(source_rows, exported_rows,
               (size_t)source_count * DM1_PC34_ORIGINAL_EVENT_BYTE_COUNT) == 0;
    return 1;
}

static void dm1_original_save_c13_roundtrip_emission_receipt(
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint32_t fingerprint = 2166136261u;

    if (!out_report || out_report->source_c13_event_count <= 0) {
        return;
    }
    out_report->c13_roundtrip_emission_receipt_available = 1;
    out_report->c13_roundtrip_emission_valid =
        out_report->c13_event_byte_preservation_ok &&
        out_report->header_part_shape_receipt_available &&
        out_report->header_identity_preservation_ok &&
        out_report->part_byte_count_preservation_ok &&
        out_report->m516_champion_record_receipt_available &&
        out_report->m516_champion_record_byte_preservation_ok &&
        out_report->party_info_byte_receipt_available &&
        out_report->party_info_byte_preservation_ok &&
        out_report->c3_event_layout_receipt_available &&
        out_report->c3_event_byte_preservation_ok &&
        out_report->c4_timeline_layout_receipt_available &&
        out_report->c4_timeline_byte_preservation_ok &&
        out_report->dungeon_tail_byte_receipt_available &&
        out_report->dungeon_tail_byte_preservation_ok;
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, (uint32_t)out_report->source_c13_event_count);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, out_report->source_c3_event_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, out_report->source_c4_timeline_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, out_report->source_m516_champion_record_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, out_report->source_party_info_fingerprint);
    fingerprint = dm1_original_save_corpus_hash_step(
        fingerprint, out_report->source_dungeon_tail_fingerprint);
    out_report->c13_roundtrip_emission_fingerprint =
        fingerprint ? fingerprint : 1u;
}

static int original_pc34_part_payload_span(const uint8_t *bytes,
                                           size_t size,
                                           int target_part,
                                           size_t *out_offset,
                                           size_t *out_size)
{
    size_t cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int part;

    if (out_offset) *out_offset = 0u;
    if (out_size) *out_size = 0u;
    if (!bytes || !out_offset || !out_size || target_part < 0 ||
        target_part >= SAVEGAME_PC34_PART_COUNT || cursor > size) {
        return 0;
    }
    for (part = 0; part < SAVEGAME_PC34_PART_COUNT; ++part) {
        uint16_t byte_count;

        if (cursor + 2u > size) {
            return 0;
        }
        byte_count = read_u16_le(bytes + cursor);
        cursor += 2u;
        if ((size_t)byte_count > size - cursor) {
            return 0;
        }
        if (part == target_part) {
            *out_offset = cursor;
            *out_size = byte_count;
            return 1;
        }
        cursor += byte_count;
    }
    return 0;
}

/* This identifies the immutable corpus input that supplied C13. It keeps the
 * stored C3 range, not a reconstructed decoded event, so corpus admission can
 * reject a later file substitution before F0433 output is trusted. */
static void dm1_original_save_c13_roundtrip_input_identity_receipt(
    const uint8_t *bytes,
    size_t size,
    const DM1OriginalSavePC34HandoffReport *source_report,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    size_t c3_offset;
    size_t c3_size;
    int event_index;
    int c13_count = 0;

    if (!bytes || !source_report || !out_report ||
        size == 0u || size > UINT32_MAX ||
        source_report->decoded_event_count < 0 ||
        source_report->decoded_event_count > DM1_EVENT_MAX_COUNT) {
        return;
    }
    for (event_index = 0; event_index < source_report->decoded_event_count;
         ++event_index) {
        if (source_report->events[event_index].type ==
            DM1_EVENT_VI_ALTAR_REBIRTH) {
            ++c13_count;
        }
    }
    if (c13_count == 0 ||
        !original_pc34_part_payload_span(bytes, size,
                                         SAVEGAME_PC34_PART_EVENTS,
                                         &c3_offset, &c3_size) ||
        c3_offset > UINT32_MAX || c3_size > UINT32_MAX ||
        c3_size != source_report->part_byte_counts[SAVEGAME_PC34_PART_EVENTS]) {
        return;
    }
    out_report->c13_roundtrip_input_byte_count = (uint32_t)size;
    out_report->c13_roundtrip_input_hash =
        dm1_original_save_hash_bytes(bytes, size);
    out_report->c13_roundtrip_input_c3_byte_offset = (uint32_t)c3_offset;
    out_report->c13_roundtrip_input_c3_byte_count = (uint32_t)c3_size;
    out_report->c13_roundtrip_input_c3_fingerprint =
        dm1_original_save_hash_bytes(bytes + c3_offset, c3_size);
    out_report->c13_roundtrip_input_identity_receipt_available =
        out_report->c13_roundtrip_input_hash != 0u &&
        out_report->c13_roundtrip_input_c3_fingerprint != 0u;
}

/* ReDMCSB PROJEXPL.C F0213/F0224 creates C25/C24 with B.Location and
 * C.Slot. F0433/F0435 retain the enclosing EVENT; this receipt deliberately
 * retains only the four source-owned C15 union bytes. */
static void dm1_original_save_explosion_union_slot_receipt_bytes(
    const struct DM1_Event_V1 *event,
    uint8_t out_bytes[4])
{
    out_bytes[0] = event->b_mapX;
    out_bytes[1] = event->b_mapY;
    out_bytes[2] = event->c_cell;
    out_bytes[3] = event->c_effect;
}

static void dm1_original_save_sort_explosion_union_receipt_rows(
    uint8_t rows[DM1_EVENT_MAX_COUNT][4],
    int count)
{
    int i;

    for (i = 1; i < count; ++i) {
        uint8_t row[4];
        int j = i;

        memcpy(row, rows[i], sizeof(row));
        while (j > 0 && memcmp(rows[j - 1], row, sizeof(row)) > 0) {
            memcpy(rows[j], rows[j - 1], sizeof(row));
            --j;
        }
        memcpy(rows[j], row, sizeof(row));
    }
}

static void fill_roundtrip_core_report(
    const DM1OriginalSavePC34HandoffReport *source_report,
    const DM1OriginalSavePC34HandoffReport *export_report,
    const struct GameWorld_Compat *reloaded_world,
    const struct DM1_EventQueue_V1 *reloaded_queue,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    if (!out_report) {
        return;
    }
    memset(out_report, 0, sizeof(*out_report));
    if (source_report) {
        out_report->source_champion_count =
            source_report->imported_champion_count;
        out_report->source_map_index = source_report->imported_map_index;
        out_report->source_map_x = source_report->imported_map_x;
        out_report->source_map_y = source_report->imported_map_y;
        out_report->source_direction = source_report->imported_direction;
        out_report->source_game_time = source_report->original_game_time;
        out_report->source_event_count = source_report->original_event_count;
        out_report->source_active_group_count =
            source_report->original_current_active_group_count;
        out_report->source_dungeon_tail_present =
            source_report->dungeon_tail_present;
        out_report->source_dungeon_tail_byte_count =
            source_report->dungeon_tail_byte_count;
        out_report->source_dungeon_tail_checksum =
            source_report->dungeon_tail_actual_checksum;
    }
    if (export_report) {
        out_report->exported_champion_count =
            export_report->imported_champion_count;
        out_report->exported_map_index = export_report->imported_map_index;
        out_report->exported_map_x = export_report->imported_map_x;
        out_report->exported_map_y = export_report->imported_map_y;
        out_report->exported_direction = export_report->imported_direction;
        out_report->exported_game_time = export_report->original_game_time;
        out_report->exported_event_count = export_report->original_event_count;
        out_report->exported_active_group_count =
            export_report->original_current_active_group_count;
        out_report->exported_dungeon_tail_present =
            export_report->dungeon_tail_present;
        out_report->exported_dungeon_tail_byte_count =
            export_report->dungeon_tail_byte_count;
        out_report->exported_dungeon_tail_checksum =
            export_report->dungeon_tail_actual_checksum;
    }
    if (reloaded_world) {
        out_report->reloaded_champion_count =
            reloaded_world->party.championCount;
        out_report->reloaded_map_index = reloaded_world->party.mapIndex;
        out_report->reloaded_map_x = reloaded_world->party.mapX;
        out_report->reloaded_map_y = reloaded_world->party.mapY;
        out_report->reloaded_direction = reloaded_world->party.direction;
        out_report->reloaded_game_time = reloaded_world->gameTick;
        out_report->reloaded_active_group_count =
            reloaded_world->creatureAICount;
    }
    if (reloaded_queue) {
        out_report->reloaded_event_count = reloaded_queue->eventCount;
    }
    if (source_report && export_report) {
        out_report->source_external_portrait_byte_count =
            source_report->external_portrait_byte_count;
        out_report->source_external_portrait_fingerprint =
            source_report->external_portrait_fingerprint;
        out_report->exported_external_portrait_byte_count =
            export_report->external_portrait_byte_count;
        out_report->exported_external_portrait_fingerprint =
            export_report->external_portrait_fingerprint;
        out_report->external_portrait_byte_receipt_available =
            source_report->external_portrait_byte_count ==
                SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT &&
            export_report->external_portrait_byte_count ==
                SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT &&
            source_report->external_portrait_fingerprint != 0u &&
            export_report->external_portrait_fingerprint != 0u;
    }

    /* ReDMCSB LOADSAVE.C F0433:1641-1682 serializes the live dungeon
     * immediately after the five save parts, and F0435 restores it before
     * exposing runtime state. Do not certify an original-save round trip
     * solely from PARTY/EVENT facts when the dungeon tail was dropped. */
    if (export_report) {
        out_report->reloaded_dungeon_tail_present =
            export_report->dungeon_tail_present;
        out_report->reloaded_dungeon_tail_byte_count =
            export_report->dungeon_tail_byte_count;
        out_report->reloaded_dungeon_tail_checksum =
            export_report->dungeon_tail_actual_checksum;
    }
    out_report->dungeon_tail_matches =
        out_report->source_dungeon_tail_present ==
            out_report->exported_dungeon_tail_present &&
        out_report->source_dungeon_tail_present ==
            out_report->reloaded_dungeon_tail_present &&
        out_report->source_dungeon_tail_byte_count ==
            out_report->exported_dungeon_tail_byte_count &&
        out_report->source_dungeon_tail_byte_count ==
            out_report->reloaded_dungeon_tail_byte_count &&
        out_report->source_dungeon_tail_checksum ==
            out_report->exported_dungeon_tail_checksum &&
        out_report->source_dungeon_tail_checksum ==
            out_report->reloaded_dungeon_tail_checksum;

    out_report->core_state_matches =
        out_report->source_champion_count ==
            out_report->exported_champion_count &&
        out_report->source_champion_count ==
            out_report->reloaded_champion_count &&
        out_report->source_map_index == out_report->exported_map_index &&
        out_report->source_map_index == out_report->reloaded_map_index &&
        out_report->source_map_x == out_report->exported_map_x &&
        out_report->source_map_x == out_report->reloaded_map_x &&
        out_report->source_map_y == out_report->exported_map_y &&
        out_report->source_map_y == out_report->reloaded_map_y &&
        out_report->source_direction == out_report->exported_direction &&
        out_report->source_direction == out_report->reloaded_direction &&
        out_report->source_game_time == out_report->exported_game_time &&
        out_report->source_game_time == out_report->reloaded_game_time &&
        out_report->source_event_count == out_report->exported_event_count &&
        out_report->source_event_count == out_report->reloaded_event_count &&
        out_report->source_active_group_count ==
            out_report->exported_active_group_count &&
        out_report->source_active_group_count ==
            out_report->reloaded_active_group_count &&
        out_report->dungeon_tail_matches;
}

int dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
    const uint8_t *bytes,
    size_t size,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    DM1OriginalSavePC34HandoffReport import_report;
    DM1OriginalSavePC34HandoffReport export_report;
    struct GameWorld_Compat reloaded_world;
    struct DM1_EventQueue_V1 reloaded_queue;
    int result;

    if (out_report) {
        memset(out_report, 0, sizeof(*out_report));
    }
    if (!bytes || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }

    result = dm1_v1_original_save_pc34_roundtrip_world_bytes(
        bytes, size, game_id, out_bytes, out_capacity, out_size,
        &import_report, &export_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    memset(&reloaded_world, 0, sizeof(reloaded_world));
    memset(&reloaded_queue, 0, sizeof(reloaded_queue));
    result = dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
        out_bytes, *out_size, &reloaded_world, &reloaded_queue,
        &export_report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        F0883_WORLD_Free_Compat(&reloaded_world);
        return result;
    }

    fill_roundtrip_core_report(&import_report, &export_report,
                               &reloaded_world, &reloaded_queue,
                               out_report);
    if (out_report && out_report->external_portrait_byte_receipt_available) {
        out_report->external_portrait_byte_preservation_ok =
            dm1_original_save_external_portraits_match(
                bytes, size, &import_report, out_bytes, *out_size,
                &export_report);
    }
    if (out_report && !dm1_original_save_inactive_champion_records_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report)) {
        out_report->inactive_champion_record_byte_receipt_available = 0;
    }
    if (out_report) {
        (void)dm1_original_save_header_part_shape_match(
            &import_report, &export_report, out_report);
        (void)dm1_original_save_m516_champion_records_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report);
        (void)dm1_original_save_c13_champion_records_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report);
        (void)dm1_original_save_party_info_bytes_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report);
        (void)dm1_original_save_c3_event_bytes_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report);
        (void)dm1_original_save_c4_timeline_bytes_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report);
        (void)dm1_original_save_dungeon_tail_bytes_match(
            bytes, size, &import_report, out_bytes, *out_size,
            &export_report, out_report);
        (void)dm1_original_save_c13_event_emission_bytes_match(
            &import_report, &export_report, out_report);
        (void)dm1_original_save_c13_raw_capture_bytes_match(
            &import_report, &export_report, out_report);
        dm1_original_save_c13_roundtrip_emission_receipt(out_report);
        dm1_original_save_c13_roundtrip_input_identity_receipt(
            bytes, size, &import_report, out_report);
    }
    F0883_WORLD_Free_Compat(&reloaded_world);
    if (out_report && !out_report->core_state_matches) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
    }
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_roundtrip_world_file(
    const char *path,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34HandoffReport *import_report,
    DM1OriginalSavePC34HandoffReport *verify_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (!dm1_original_save_file_opens_for_read(path)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    /* Product-facing file round trips accept only external PC34 envelopes.
     * F0433 verification output carries Firestaff's manifest and must never
     * re-enter the original-save corpus/product import route as evidence. */
    if (!dm1_original_save_corpus_external_pc34_file(path, NULL)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }
    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    /* ReDMCSB LOADSAVE.C F0435 reads original PC34 bytes from disk before
     * materializing runtime GLOBAL_DATA/ACTIVE_GROUP/PARTY/EVENT state.
     * This corpus-facing wrapper keeps Firestaff's file edge on the same
     * bounded import-export verification path as the byte helper. */
    result = dm1_v1_original_save_pc34_roundtrip_world_bytes(
        bytes, size, game_id, out_bytes, out_capacity, out_size,
        import_report, verify_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_roundtrip_world_reload_file(
    const char *path,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34RoundtripReport *out_report)
{
    uint8_t *bytes;
    size_t size;
    int result;

    if (!path || !out_bytes || !out_size) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    if (!dm1_original_save_file_opens_for_read(path)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }
    if (!dm1_original_save_corpus_external_pc34_file(path, NULL)) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34;
    }
    result = read_original_pc34_file_bytes(path, &bytes, &size);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        return result;
    }

    result = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        bytes, size, game_id, out_bytes, out_capacity, out_size, out_report);
    free(bytes);
    return result;
}

int dm1_v1_original_save_pc34_roundtrip_corpus_root(
    const char *root,
    DM1OriginalSavePC34CorpusRoundtripReport *out_report)
{
    DM1OriginalSaveCorpusManifest corpus;
    DM1OriginalSavePC34CorpusRoundtripReport report;
    uint8_t *exported_bytes;
    int i;

    if (!root || !root[0] || !out_report) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    memset(&report, 0, sizeof(report));
    report.first_failure_result = DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
    report.provenance_fingerprint = 2166136261u;
    memset(&corpus, 0, sizeof(corpus));
    if (!dm1_v1_original_save_classify_corpus_root(root, &corpus)) {
        report.discovery_root_error = 1;
        *out_report = report;
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    report.scan_succeeded = 1;
    report.scanned_file_count = corpus.scanned_file_count;
    report.rejected_count = corpus.rejected_count;
    report.truncated_count = corpus.truncated_count;
    for (i = 0; i < corpus.present_count &&
                i < (int)DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP; ++i) {
        dm1_original_save_corpus_record_discovery(
            &report, &corpus.results[i], corpus.paths[i]);
    }
    exported_bytes = (uint8_t *)malloc(SAVEGAME_PC34_MAX_FILE_SIZE);
    if (!exported_bytes) {
        *out_report = report;
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
    }

    /* ReDMCSB LOADSAVE.C F0435 reads only a valid header plus the five
     * checksum-protected parts. F0433 subsequently serializes the live
     * state. Keep corpus proof in memory so validation cannot create or
     * replace a sibling DMSAVE.DAT. */
    for (i = 0; i < corpus.present_count &&
                i < (int)DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP; ++i) {
        DM1OriginalSavePC34RoundtripReport roundtrip;
        DM1OriginalSavePC34CorpusReceipt *receipt;
        uint8_t *source_bytes = NULL;
        size_t source_size = 0u;
        size_t exported_size = 0u;
        int result;
        int firestaff_manifest = 0;

        if (!corpus.results[i].pc34_loader_part_envelope_candidate) {
            continue;
        }
        /* ReDMCSB LOADSAVE.C F0435 chooses a PC34 save from the decoded
         * header and its five part envelopes. Keep a path-independent record
         * of precisely those classifier facts, so moving the corpus cannot
         * change its provenance receipt. */
        report.provenance_fingerprint =
            dm1_original_save_corpus_fingerprint_mix(
                report.provenance_fingerprint,
                corpus.results[i].game_id);
        report.provenance_fingerprint =
            dm1_original_save_corpus_fingerprint_mix(
                report.provenance_fingerprint,
                (uint32_t)corpus.results[i].size_bytes);
        report.provenance_fingerprint =
            dm1_original_save_corpus_fingerprint_mix(
                report.provenance_fingerprint,
                (uint32_t)(corpus.results[i].size_bytes >> 32));
        report.provenance_fingerprint =
            dm1_original_save_corpus_fingerprint_mix(
                report.provenance_fingerprint,
                corpus.results[i].prefix_checksum32);
        report.provenance_fingerprint =
            dm1_original_save_corpus_fingerprint_mix(
                report.provenance_fingerprint,
                corpus.results[i].save_part_loader_envelope_payload_bytes);
        if (!dm1_original_save_corpus_external_pc34_file(
                corpus.paths[i], &firestaff_manifest)) {
            if (firestaff_manifest) {
                ++report.firestaff_manifest_rejected_count;
            } else {
                ++report.nonoriginal_envelope_rejected_count;
            }
            if (report.first_failure_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                report.first_failure_result =
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            continue;
        }
        ++report.pc34_candidate_count;
        if (!report.first_pc34_path[0]) {
            snprintf(report.first_pc34_path, sizeof(report.first_pc34_path),
                     "%s", corpus.paths[i]);
        }
        if (report.receipt_count >=
            (int)DM1_ORIGINAL_SAVE_PC34_CORPUS_RECEIPT_CAP) {
            continue;
        }
        receipt = &report.receipts[report.receipt_count++];
        memset(receipt, 0, sizeof(*receipt));
        snprintf(receipt->path, sizeof(receipt->path), "%s", corpus.paths[i]);
        receipt->classified_loader_envelope = 1;
        receipt->external_original = 1;
        receipt->game_id = corpus.results[i].game_id;
        receipt->source_f7057_envelope_end_offset =
            (uint32_t)SAVEGAME_PC34_DM_SAVE_HEADER_SIZE +
            corpus.results[i].save_part_loader_envelope_payload_bytes;
        receipt->source_f7057_trailing_byte_count =
            (uint32_t)corpus.results[i].size_bytes -
            receipt->source_f7057_envelope_end_offset;
        result = read_original_pc34_file_bytes(
            corpus.paths[i], &source_bytes, &source_size);
        if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
            receipt->source_handoff_result = result;
            if (report.first_failure_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                report.first_failure_result = result;
            }
            continue;
        }
        receipt->source_byte_count = (uint32_t)source_size;
        receipt->source_hash = dm1_original_save_hash_bytes(
            source_bytes, source_size);
        if (!dm1_original_save_corpus_admit_discovered_bytes(
                source_bytes, source_size, &corpus.results[i], receipt)) {
            receipt->source_handoff_result =
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            if (report.first_failure_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                report.first_failure_result = receipt->source_handoff_result;
            }
            free(source_bytes);
            source_bytes = NULL;
            continue;
        }
        dm1_original_save_corpus_receipt_source_handoff(
            source_bytes, source_size, receipt);
        dm1_original_save_corpus_receipt_runtime_stage(
            source_bytes, source_size, receipt);
        ++report.runtime_stage_attempted_count;
        if (receipt->source_runtime_stage_committed) {
            ++report.runtime_stage_succeeded_count;
        } else if (receipt->source_runtime_stage_result ==
                   DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
            ++report.runtime_stage_unavailable_count;
        } else {
            ++report.runtime_stage_failed_count;
        }
        if (receipt->source_runtime_adopt_attempted) {
            ++report.runtime_adopt_attempted_count;
            if (receipt->source_runtime_adopted &&
                receipt->source_runtime_adopt_result ==
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                ++report.runtime_adopt_succeeded_count;
            } else {
                ++report.runtime_adopt_failed_count;
            }
        }
        memset(&roundtrip, 0, sizeof(roundtrip));
        ++report.roundtrip_attempted_count;
        receipt->roundtrip_attempted = 1;
        result = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
            source_bytes, source_size,
            corpus.results[i].game_id,
            exported_bytes,
            SAVEGAME_PC34_MAX_FILE_SIZE,
            &exported_size,
            &roundtrip);
        receipt->roundtrip_result = result;
        receipt->core_state_matches = roundtrip.core_state_matches;
        receipt->exported_byte_count = (uint32_t)exported_size;
        receipt->exported_hash = exported_size > 0u
            ? dm1_original_save_hash_bytes(exported_bytes, exported_size)
            : 0u;
        receipt->source_c13_event_count = roundtrip.source_c13_event_count;
        receipt->exported_c13_event_count = roundtrip.exported_c13_event_count;
        receipt->c13_byte_preservation_ok =
            roundtrip.c13_event_byte_preservation_ok;
        receipt->c13_roundtrip_emission_receipt_available =
            roundtrip.c13_roundtrip_emission_receipt_available;
        receipt->c13_roundtrip_emission_valid =
            roundtrip.c13_roundtrip_emission_valid;
        receipt->c13_roundtrip_emission_fingerprint =
            roundtrip.c13_roundtrip_emission_fingerprint;
        receipt->c13_roundtrip_input_byte_count =
            roundtrip.c13_roundtrip_input_byte_count;
        receipt->c13_roundtrip_input_hash =
            roundtrip.c13_roundtrip_input_hash;
        receipt->c13_roundtrip_input_c3_byte_offset =
            roundtrip.c13_roundtrip_input_c3_byte_offset;
        receipt->c13_roundtrip_input_c3_byte_count =
            roundtrip.c13_roundtrip_input_c3_byte_count;
        receipt->c13_roundtrip_input_c3_fingerprint =
            roundtrip.c13_roundtrip_input_c3_fingerprint;
        receipt->header_part_shape_receipt_available =
            roundtrip.header_part_shape_receipt_available;
        receipt->header_identity_preservation_ok =
            roundtrip.header_identity_preservation_ok;
        receipt->part_byte_count_preservation_ok =
            roundtrip.part_byte_count_preservation_ok;
        memcpy(receipt->source_part_byte_counts,
               roundtrip.source_part_byte_counts,
               sizeof(receipt->source_part_byte_counts));
        memcpy(receipt->exported_part_byte_counts,
               roundtrip.exported_part_byte_counts,
               sizeof(receipt->exported_part_byte_counts));
        receipt->c3_event_layout_receipt_available =
            roundtrip.c3_event_layout_receipt_available;
        receipt->source_c3_event_record_count =
            roundtrip.source_c3_event_record_count;
        receipt->source_c3_event_byte_count =
            roundtrip.source_c3_event_byte_count;
        receipt->source_c3_event_fingerprint =
            roundtrip.source_c3_event_fingerprint;
        receipt->exported_c3_event_record_count =
            roundtrip.exported_c3_event_record_count;
        receipt->exported_c3_event_byte_count =
            roundtrip.exported_c3_event_byte_count;
        receipt->exported_c3_event_fingerprint =
            roundtrip.exported_c3_event_fingerprint;
        receipt->c3_event_byte_preservation_ok =
            roundtrip.c3_event_byte_preservation_ok;
        receipt->c13_raw_capture_receipt_available =
            roundtrip.c13_raw_capture_receipt_available;
        receipt->source_c13_raw_capture_count =
            roundtrip.source_c13_raw_capture_count;
        receipt->exported_c13_raw_capture_count =
            roundtrip.exported_c13_raw_capture_count;
        receipt->c13_raw_capture_byte_preserved_count =
            roundtrip.c13_raw_capture_byte_preserved_count;
        receipt->c13_raw_capture_byte_mismatch_count =
            roundtrip.c13_raw_capture_byte_mismatch_count;
        receipt->c13_raw_capture_byte_preservation_ok =
            roundtrip.c13_raw_capture_byte_preservation_ok;
        receipt->source_c13_raw_capture_byte_count =
            roundtrip.source_c13_raw_capture_byte_count;
        receipt->source_c13_raw_capture_fingerprint =
            roundtrip.source_c13_raw_capture_fingerprint;
        receipt->exported_c13_raw_capture_byte_count =
            roundtrip.exported_c13_raw_capture_byte_count;
        receipt->exported_c13_raw_capture_fingerprint =
            roundtrip.exported_c13_raw_capture_fingerprint;
        receipt->c4_timeline_layout_receipt_available =
            roundtrip.c4_timeline_layout_receipt_available;
        receipt->source_c4_timeline_index_count =
            roundtrip.source_c4_timeline_index_count;
        receipt->source_c4_timeline_byte_count =
            roundtrip.source_c4_timeline_byte_count;
        receipt->source_c4_timeline_fingerprint =
            roundtrip.source_c4_timeline_fingerprint;
        receipt->exported_c4_timeline_index_count =
            roundtrip.exported_c4_timeline_index_count;
        receipt->exported_c4_timeline_byte_count =
            roundtrip.exported_c4_timeline_byte_count;
        receipt->exported_c4_timeline_fingerprint =
            roundtrip.exported_c4_timeline_fingerprint;
        receipt->c4_timeline_byte_preservation_ok =
            roundtrip.c4_timeline_byte_preservation_ok;
        receipt->dungeon_tail_byte_receipt_available =
            roundtrip.dungeon_tail_byte_receipt_available;
        receipt->dungeon_tail_byte_preservation_ok =
            roundtrip.dungeon_tail_byte_preservation_ok;
        if ((!dm1_original_save_c13_corpus_admit_roundtrip_input(receipt) ||
             !dm1_original_save_c13_corpus_admit_raw_capture(receipt) ||
             !dm1_original_save_c13_corpus_bind_runtime_handoff(receipt) ||
             !dm1_original_save_c13_publish_active_runtime_state(receipt) ||
             !dm1_original_save_c13_consume_active_runtime_state(receipt) ||
             !dm1_original_save_c13_handoff_consumption_to_visible_runtime(
                 receipt) ||
             !dm1_original_save_c13_visible_runtime_lifecycle(receipt) ||
             !dm1_original_save_c13_visible_runtime_m11_handoff(receipt) ||
             !dm1_original_save_c13_visible_runtime_m11_lifecycle(receipt) ||
             !dm1_original_save_c13_build_runtime_frame(receipt) ||
             !dm1_original_save_c13_runtime_frame_lifecycle(receipt) ||
             !dm1_original_save_c13_runtime_frame_m11_bridge(receipt) ||
             !dm1_original_save_c13_discovered_capture_to_m11_runtime(
                 receipt)) &&
            result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
            result = DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            receipt->roundtrip_result = result;
        }
        free(source_bytes);
        source_bytes = NULL;
        receipt->roundtrip_receipts_committed =
            result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
            roundtrip.core_state_matches &&
            dm1_original_save_corpus_receipt_has_core_roundtrip_evidence(
                receipt);
        if (result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
            roundtrip.core_state_matches) {
            ++report.roundtrip_succeeded_count;
            ++report.core_state_match_count;
            if (!report.first_roundtrip_path[0]) {
                snprintf(report.first_roundtrip_path,
                         sizeof(report.first_roundtrip_path), "%s",
                         corpus.paths[i]);
            }
        } else {
            ++report.roundtrip_failed_count;
            if (result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                result = DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT;
            }
            if (report.first_failure_result ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                report.first_failure_result = result;
            }
        }
    }
    free(exported_bytes);
    if (report.roundtrip_succeeded_count == 0) {
        report.roundtrip_hash = 0u;
    }
    report.provenance_fingerprint = report.roundtrip_hash;
    *out_report = report;
    return DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK;
}

int dm1_v1_original_save_pc34_roundtrip_configured_corpus(
    DM1OriginalSavePC34CorpusRoundtripReport *out_report)
{
    const char *root;
    const char *home;
    char default_root[DM1_ORIGINAL_SAVE_PATH_MAX];
    int default_root_length;

    if (!out_report) {
        return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT;
    }
    root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    if (!root || !root[0]) {
        root = getenv("FIRESTAFF_DM1_DATA_DIR");
    }
    if (!root || !root[0]) {
        root = getenv("FIRESTAFF_DATA_DIR");
    }
    if (!root || !root[0]) {
        home = getenv("HOME");
        default_root_length = home && home[0]
            ? snprintf(default_root, sizeof(default_root),
                       "%s/.firestaff/data/dm1", home)
            : -1;
        if (default_root_length > 0 &&
            (size_t)default_root_length < sizeof(default_root)) {
            root = default_root;
        } else {
            memset(out_report, 0, sizeof(*out_report));
            out_report->discovery_root_error = 1;
            out_report->first_failure_result =
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
            return DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE;
        }
    }
    return dm1_v1_original_save_pc34_roundtrip_corpus_root(root, out_report);
}

const char *dm1_v1_original_save_pc34_handoff_result_name(int result)
{
    switch (result) {
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK: return "OK";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT: return "argument";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34: return "not-pc34";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT: return "import";
    case DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE: return "file";
    default: return "unknown";
    }
}

const char *dm1_v1_original_save_pc34_handoff_source_evidence(void)
{
    return "ReDMCSB DEFS.H:468-480 DM_SAVE_HEADER and 503-521 constants; "
           "SAVEHEAD.C F0429/F0430 header obfuscation; "
           "LOADSAVE.C F0435 PC save load path; "
           "LOADSAVE.C F0435 dungeon tail read path and F0421 checksum; "
           "DEFS.H:394-418 THING type/index layout; "
           "DEFS.H:574-587 ACTIVE_GROUP; "
           "DEFS.H:880-920 EVENT and timeline save arrays; "
           "DEFS.H:661-705 CHAMPION_EXCLUDING_PORTRAIT; "
           "READWRIT.C F0417/F0418/F0419 save-part checksum and obfuscation; "
           "READWRIT.C F0421 staged dungeon-tail running checksum";
}
