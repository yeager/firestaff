#ifndef DM1_V1_ORIGINAL_SAVE_PC34_HANDOFF_H
#define DM1_V1_ORIGINAL_SAVE_PC34_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_original_save_classifier.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_savegame_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK = 0,
    DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_ARGUMENT = -1,
    DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_NOT_PC34 = -2,
    DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_IMPORT = -3,
    DM1_ORIGINAL_SAVE_PC34_HANDOFF_ERR_FILE = -4
} DM1OriginalSavePC34HandoffResult;

#define DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP \
    GAMEWORLD_CREATURE_AI_CAPACITY

typedef struct {
    int group_thing_index;
    int directions;
    int cells;
    int last_move_time;
    int delay_fleeing_from_target;
    int target_map_x;
    int target_map_y;
    int prior_map_x;
    int prior_map_y;
    int home_map_x;
    int home_map_y;
    int aspect[4];
} DM1OriginalSavePC34ActiveGroupRecord;

/* ReDMCSB DEFS.H EVENT.C.Projectile is a packed 16-bit motion record. This
 * plan is the narrow source-bound bridge for a saved C48/C49 event; callers
 * must still validate the target map before publishing a runtime projectile. */
typedef struct {
    int valid;
    int source_event_type;
    int source_event_index;
    int projectile_index;
    int projectile_category;
    int projectile_subtype;
    int map_index;
    int map_x;
    int map_y;
    int cell;
    int direction;
    int step_energy;
    int first_move_grace;
    int kinetic_energy;
    int attack;
    unsigned short associated_thing;
} DM1OriginalSavePC34ProjectileEventPlan;

/* ReDMCSB TIMELINE.C F0255 owns all of C13's union fields: B.Location,
 * C.A.Cell, C.A.Effect, and A.A.Priority. This plan preserves that exact
 * source surface for a later atomic bones/explosion/rebirth runtime commit. */
typedef struct {
    int valid;
    int source_event_index;
    int champion_index;
    int map_index;
    int map_x;
    int map_y;
    int cell;
    int step;
    uint32_t fire_at_tick;
} DM1OriginalSavePC34ViAltarRebirthEventPlan;

typedef struct {
    DM1OriginalSaveClassifyResult classify;
    int importer_result;
    int resumed_from_backup;
    int backup_promoted_to_primary;
    uint16_t part_expected_checksums[5];
    uint16_t part_actual_checksums[5];
    uint32_t part_byte_counts[5];
    int part_checksum_ok_count;
    int imported_champion_block_count;
    int imported_champion_slot_count;
    int imported_skill_level_count;
    int imported_champion_count;
    int imported_map_index;
    int imported_map_x;
    int imported_map_y;
    int imported_direction;
    int imported_active_champion_index;
    uint32_t original_game_time;
    int original_current_active_group_count;
    int original_maximum_active_group_count;
    int decoded_active_group_count;
    int reported_active_group_count;
    int active_group_decode_truncated_count;
    int active_group_runtime_imported_count;
    int active_group_runtime_truncated_count;
    int active_group_runtime_resolved_count;
    int active_group_runtime_unresolved_count;
    int original_event_count;
    int original_first_unused_event_index;
    int original_event_maximum_count;
    int decoded_event_count;
    int decoded_timeline_index_count;
    int event_decode_truncated_count;
    uint32_t external_portrait_byte_count;
    uint32_t external_portrait_byte_offset;
    uint32_t external_portrait_fingerprint;
    uint32_t pc34_party_part_byte_offset;
    uint16_t pc34_party_part_key;
    int external_portrait_payload_count;
    int external_portrait_imported_count;
    int dungeon_tail_present;
    uint32_t dungeon_tail_byte_count;
    uint32_t dungeon_tail_fingerprint;
    uint16_t dungeon_tail_expected_checksum;
    uint16_t dungeon_tail_actual_checksum;
    int dungeon_tail_checksum_ok;
    int dungeon_tail_map_count;
    int dungeon_tail_column_count;
    int dungeon_tail_square_first_thing_count;
    int dungeon_tail_text_data_word_count;
    uint32_t dungeon_tail_thing_data_byte_count;
    uint32_t dungeon_tail_raw_map_data_byte_count;
    int dungeon_tail_runtime_imported;
    DM1OriginalSavePC34ActiveGroupRecord
        active_groups[DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP];
    struct DM1_Event_V1 events[DM1_EVENT_MAX_COUNT];
    uint16_t timeline_indices[DM1_EVENT_MAX_COUNT];
} DM1OriginalSavePC34HandoffReport;

typedef struct {
    int champion_count;
    int map_index;
    int map_x;
    int map_y;
    int direction;
    int active_champion_index;
    int current_active_group_count;
    int maximum_active_group_count;
    int event_count;
    int event_maximum_count;
    uint32_t game_time;
    uint32_t game_id;
} DM1OriginalSavePC34FixtureSpec;

typedef struct {
    int source_champion_count;
    int exported_champion_count;
    int reloaded_champion_count;
    int source_map_index;
    int exported_map_index;
    int reloaded_map_index;
    int source_map_x;
    int exported_map_x;
    int reloaded_map_x;
    int source_map_y;
    int exported_map_y;
    int reloaded_map_y;
    int source_direction;
    int exported_direction;
    int reloaded_direction;
    uint32_t source_game_time;
    uint32_t exported_game_time;
    uint32_t reloaded_game_time;
    int source_event_count;
    int exported_event_count;
    int reloaded_event_count;
    int source_active_group_count;
    int exported_active_group_count;
    int reloaded_active_group_count;
    /* F0433/F0435 source-owned DM_SAVE_HEADER identity and the five raw
     * uint16 length prefixes. Noise/Keys/Checksums are intentionally not
     * compared: F0433 regenerates them, while AdditionalData is Firestaff's
     * manifest slot rather than an imported external-corpus mirror. */
    int header_part_shape_receipt_available;
    uint16_t source_header_format_id;
    uint16_t exported_header_format_id;
    uint16_t source_header_platform;
    uint16_t exported_header_platform;
    uint16_t source_header_dungeon_id;
    uint16_t exported_header_dungeon_id;
    uint32_t source_header_game_id;
    uint32_t exported_header_game_id;
    int header_identity_preservation_ok;
    uint32_t source_part_byte_counts[5];
    uint32_t exported_part_byte_counts[5];
    int part_byte_count_preservation_ok;
    /* ReDMCSB DEFS.H EVENT is the ten-byte on-disk record that F0433
     * writes and F0435 reads.  C13 owns every B/C union byte, unlike
     * several timer types with source-uninitialised union arms. */
    int c13_byte_receipt_available;
    int source_c13_event_count;
    int exported_c13_event_count;
    int c13_byte_preserved_count;
    int c13_byte_mismatch_count;
    int c13_byte_preservation_ok;
    /* Canonical ten-byte C13 EVENT rows, sorted only for this receipt: F0651
     * can reorder EVENT storage, but must not change any F0433 payload byte. */
    uint32_t source_c13_event_byte_count;
    uint32_t source_c13_event_fingerprint;
    uint32_t exported_c13_event_byte_count;
    uint32_t exported_c13_event_fingerprint;
    int c13_timeline_byte_receipt_available;
    int source_c13_timeline_reference_count;
    int exported_c13_timeline_reference_count;
    int c13_timeline_byte_preserved_count;
    int c13_timeline_byte_mismatch_count;
    int c13_timeline_byte_preservation_ok;
    /* Raw little-endian C4 index bytes, retained in original timeline order. */
    uint32_t source_c13_timeline_reference_byte_count;
    uint32_t source_c13_timeline_reference_fingerprint;
    uint32_t exported_c13_timeline_reference_byte_count;
    uint32_t exported_c13_timeline_reference_fingerprint;
    /* PROJEXPL.C F0213 owns C25's B.Location and C.Slot union bytes. Keep
     * their four-byte record apart from generic EVENT fields. */
    int c25_union_slot_byte_receipt_available;
    int source_c25_event_count;
    int exported_c25_event_count;
    int c25_union_slot_byte_preserved_count;
    int c25_union_slot_byte_mismatch_count;
    int c25_union_slot_byte_preservation_ok;
    uint32_t source_c25_union_slot_byte_count;
    uint32_t source_c25_union_slot_fingerprint;
    uint32_t exported_c25_union_slot_byte_count;
    uint32_t exported_c25_union_slot_fingerprint;
    /* PROJEXPL.C F0224 owns C24's B.Location and C.Slot union bytes for a
     * linked fluxcage; retain no host explosion-index interpretation. */
    int c24_union_slot_byte_receipt_available;
    int source_c24_event_count;
    int exported_c24_event_count;
    int c24_union_slot_byte_preserved_count;
    int c24_union_slot_byte_mismatch_count;
    int c24_union_slot_byte_preservation_ok;
    uint32_t source_c24_union_slot_byte_count;
    uint32_t source_c24_union_slot_fingerprint;
    uint32_t exported_c24_union_slot_byte_count;
    uint32_t exported_c24_union_slot_fingerprint;
    int c4_timeline_layout_receipt_available;
    uint32_t source_c4_timeline_index_count;
    uint32_t source_c4_timeline_byte_count;
    uint32_t source_c4_timeline_fingerprint;
    uint32_t exported_c4_timeline_index_count;
    uint32_t exported_c4_timeline_byte_count;
    uint32_t exported_c4_timeline_fingerprint;
    int c4_timeline_byte_preservation_ok;
    /* CLIKVIEW.C F0374 writes C13.Priority from the dropped bones'
     * ChargeCount, which names an active M516_CHAMPIONS slot. Keep that
     * C13 -> C2 relation as an independent raw-record receipt. */
    int c13_champion_record_byte_receipt_available;
    int source_c13_champion_record_reference_count;
    int c13_champion_record_byte_preserved_count;
    int c13_champion_record_byte_mismatch_count;
    int c13_champion_record_byte_preservation_ok;
    int party_info_byte_receipt_available;
    uint32_t source_party_info_byte_count;
    uint32_t exported_party_info_byte_count;
    int party_info_byte_preservation_ok;
    int external_portrait_byte_receipt_available;
    uint32_t source_external_portrait_byte_count;
    uint32_t source_external_portrait_fingerprint;
    uint32_t exported_external_portrait_byte_count;
    uint32_t exported_external_portrait_fingerprint;
    int external_portrait_byte_preservation_ok;
    int inactive_champion_record_byte_receipt_available;
    int inactive_champion_record_count;
    int inactive_champion_record_byte_preserved_count;
    int inactive_champion_record_byte_preservation_ok;
    int m516_champion_record_receipt_available;
    uint32_t source_m516_champion_record_count;
    uint32_t source_m516_champion_record_byte_count;
    uint32_t source_m516_champion_record_fingerprint;
    uint32_t exported_m516_champion_record_count;
    uint32_t exported_m516_champion_record_byte_count;
    uint32_t exported_m516_champion_record_fingerprint;
    int m516_champion_record_byte_preservation_ok;
    /* ReDMCSB F0433 appends the optional saved dungeon after portraits.
     * The corpus receipt compares that raw original tail only when the
     * external source actually carries one; an absent tail is also explicit. */
    int dungeon_tail_byte_receipt_available;
    uint32_t source_dungeon_tail_byte_count;
    uint32_t source_dungeon_tail_fingerprint;
    uint32_t exported_dungeon_tail_byte_count;
    uint32_t exported_dungeon_tail_fingerprint;
    int dungeon_tail_byte_preservation_ok;
    int core_state_matches;
} DM1OriginalSavePC34RoundtripReport;

#define DM1_ORIGINAL_SAVE_PC34_CORPUS_RECEIPT_CAP \
    DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP

/* One classifier-qualified corpus row. Its source and transient-export
 * hashes bind a round trip to an external PC34 file without retaining or
 * promoting unowned save bytes. */
typedef struct {
    int classified_loader_envelope;
    int external_original;
    int firestaff_manifest;
    int roundtrip_attempted;
    int roundtrip_result;
    int core_state_matches;
    uint32_t game_id;
    uint32_t source_byte_count;
    uint32_t source_hash;
    uint32_t exported_byte_count;
    uint32_t exported_hash;
    int source_c13_event_count;
    int exported_c13_event_count;
    int c13_byte_preserved_count;
    int c13_byte_mismatch_count;
    int c13_byte_preservation_ok;
    uint32_t source_c13_event_byte_count;
    uint32_t source_c13_event_fingerprint;
    uint32_t exported_c13_event_byte_count;
    uint32_t exported_c13_event_fingerprint;
    int header_part_shape_receipt_available;
    uint16_t source_header_format_id;
    uint16_t exported_header_format_id;
    uint16_t source_header_platform;
    uint16_t exported_header_platform;
    uint16_t source_header_dungeon_id;
    uint16_t exported_header_dungeon_id;
    uint32_t source_header_game_id;
    uint32_t exported_header_game_id;
    int header_identity_preservation_ok;
    uint32_t source_part_byte_counts[5];
    uint32_t exported_part_byte_counts[5];
    int part_byte_count_preservation_ok;
    int source_c13_timeline_reference_count;
    int exported_c13_timeline_reference_count;
    int c13_timeline_byte_preserved_count;
    int c13_timeline_byte_mismatch_count;
    int c13_timeline_byte_preservation_ok;
    uint32_t source_c13_timeline_reference_byte_count;
    uint32_t source_c13_timeline_reference_fingerprint;
    uint32_t exported_c13_timeline_reference_byte_count;
    uint32_t exported_c13_timeline_reference_fingerprint;
    int c25_union_slot_byte_receipt_available;
    int source_c25_event_count;
    int exported_c25_event_count;
    int c25_union_slot_byte_preserved_count;
    int c25_union_slot_byte_mismatch_count;
    int c25_union_slot_byte_preservation_ok;
    uint32_t source_c25_union_slot_byte_count;
    uint32_t source_c25_union_slot_fingerprint;
    uint32_t exported_c25_union_slot_byte_count;
    uint32_t exported_c25_union_slot_fingerprint;
    int c24_union_slot_byte_receipt_available;
    int source_c24_event_count;
    int exported_c24_event_count;
    int c24_union_slot_byte_preserved_count;
    int c24_union_slot_byte_mismatch_count;
    int c24_union_slot_byte_preservation_ok;
    uint32_t source_c24_union_slot_byte_count;
    uint32_t source_c24_union_slot_fingerprint;
    uint32_t exported_c24_union_slot_byte_count;
    uint32_t exported_c24_union_slot_fingerprint;
    int c4_timeline_layout_receipt_available;
    uint32_t source_c4_timeline_index_count;
    uint32_t source_c4_timeline_byte_count;
    uint32_t source_c4_timeline_fingerprint;
    uint32_t exported_c4_timeline_index_count;
    uint32_t exported_c4_timeline_byte_count;
    uint32_t exported_c4_timeline_fingerprint;
    int c4_timeline_byte_preservation_ok;
    int source_c13_champion_record_reference_count;
    int c13_champion_record_byte_preserved_count;
    int c13_champion_record_byte_mismatch_count;
    int c13_champion_record_byte_preservation_ok;
    uint32_t source_party_info_byte_count;
    uint32_t exported_party_info_byte_count;
    int party_info_byte_preservation_ok;
    uint32_t source_external_portrait_fingerprint;
    uint32_t exported_external_portrait_fingerprint;
    int external_portrait_byte_preservation_ok;
    int inactive_champion_record_count;
    int inactive_champion_record_byte_preserved_count;
    int inactive_champion_record_byte_preservation_ok;
    int m516_champion_record_receipt_available;
    uint32_t source_m516_champion_record_count;
    uint32_t source_m516_champion_record_byte_count;
    uint32_t source_m516_champion_record_fingerprint;
    uint32_t exported_m516_champion_record_count;
    uint32_t exported_m516_champion_record_byte_count;
    uint32_t exported_m516_champion_record_fingerprint;
    int m516_champion_record_byte_preservation_ok;
    int dungeon_tail_byte_receipt_available;
    uint32_t source_dungeon_tail_byte_count;
    uint32_t source_dungeon_tail_fingerprint;
    uint32_t exported_dungeon_tail_byte_count;
    uint32_t exported_dungeon_tail_fingerprint;
    int dungeon_tail_byte_preservation_ok;
    char path[DM1_ORIGINAL_SAVE_PATH_MAX];
} DM1OriginalSavePC34CorpusReceipt;

/* Discovery is deliberately separate from import receipts. It records every
 * regular file inspected below the explicitly supplied PC34 corpus root,
 * including header-only and rejected neighbours, without searching game-data
 * roots or handing any non-PC34 file to F0435. */
typedef struct {
    uint32_t source_byte_count;
    uint32_t header_prefix_fingerprint;
    int shape;
    int readiness;
    uint16_t save_format_id;
    uint16_t save_platform;
    uint16_t save_dungeon_id;
    uint32_t save_game_id;
    int pc34_version_platform_identity_ok;
    int pc34_importer_candidate;
    int pc34_loader_part_envelope_candidate;
    int external_original;
    int roundtrip_eligible;
    int result;
    char reason[96];
    char path[DM1_ORIGINAL_SAVE_PATH_MAX];
} DM1OriginalSavePC34CorpusDiscoveryReceipt;

/* Corpus proof is deliberately separate from the header-only classifier.
 * It never writes an export beside a user save: each eligible file is
 * imported, exported into transient memory, and reloaded from that buffer. */
typedef struct {
    int scan_succeeded;
    int discovery_root_error;
    int scanned_file_count;
    int discovery_file_count;
    int discovery_pc34_header_count;
    int discovery_pc34_version_platform_identity_count;
    int discovery_pc34_version_platform_rejected_count;
    int discovery_loader_envelope_count;
    int discovery_rejected_count;
    int discovery_truncated_count;
    int pc34_candidate_count;
    int roundtrip_attempted_count;
    int roundtrip_succeeded_count;
    int core_state_match_count;
    int roundtrip_failed_count;
    /* Files below the corpus root that were deliberately not eligible for
     * F0435 import, including truncated/non-PC34 payloads. */
    int rejected_count;
    /* Stable FNV-1a fingerprint of successful transient F0435->F0433
     * exports. Zero means no original save completed the round trip. */
    uint32_t roundtrip_hash;
    int firestaff_manifest_rejected_count;
    int nonoriginal_envelope_rejected_count;
    int first_failure_result;
    int discovery_receipt_count;
    DM1OriginalSavePC34CorpusDiscoveryReceipt
        discovery_receipts[DM1_ORIGINAL_SAVE_PC34_CORPUS_RECEIPT_CAP];
    int receipt_count;
    DM1OriginalSavePC34CorpusReceipt
        receipts[DM1_ORIGINAL_SAVE_PC34_CORPUS_RECEIPT_CAP];
    char first_pc34_path[DM1_ORIGINAL_SAVE_PATH_MAX];
    char first_roundtrip_path[DM1_ORIGINAL_SAVE_PATH_MAX];
} DM1OriginalSavePC34CorpusRoundtripReport;

/* Transient HoC state is not part of ReDMCSB's save parts.  Firestaff may
 * persist it beside a quicksave, but it must be re-materialized only after
 * F0435 has restored PARTY and the dungeon. */
typedef struct {
    int candidate_mirror_ordinal;
    int candidate_party_index;
    int candidate_panel_active;
    int inventory_panel_active;
} DM1OriginalSavePC34HoCResumeState;

/* Classify `bytes` as a ReDMCSB DM1 PC 3.4 save header, then hand
 * the same byte buffer through a bounded ReDMCSB PC save-part
 * reader for GLOBAL_DATA and optional timeline handoff.
 *
 * Source-lock:
 *   ReDMCSB DEFS.H C5_FORMAT_DM_AMIGA_36_PC_CSB_AMIGA_PC98_X68000_FM_TOWNS,
 *   C9_PLATFORM_PC, C10_DUNGEON_DM; LOADSAVE.C F0435 accepts that
 *   PC envelope before reading GLOBAL_DATA and save parts.
 *
 * Scope:
 *   This is a bounded runtime-data handoff. It does not scan paths,
 *   persist bytes, write a save back, or claim real original-save
 *   round-trip compatibility.
 */
int dm1_v1_original_save_pc34_handoff_bytes(
    const uint8_t *bytes,
    size_t size,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report);

int dm1_v1_original_save_pc34_handoff_file(
    const char *path,
    struct SaveGame_Compat *out_state,
    DM1OriginalSavePC34HandoffReport *out_report);

int dm1_v1_original_save_pc34_handoff_apply_active_groups(
    DM1OriginalSavePC34HandoffReport *report,
    struct GameWorld_Compat *world);

/* ReDMCSB LOADSAVE.C F0435 reads EVENTS/TIMELINE before F0651 exposes the
 * resumed timeline. This validates every source index and commits a fully
 * staged queue in one assignment; malformed reports leave `queue` unchanged.
 */
int dm1_v1_original_save_pc34_handoff_apply_event_queue(
    const DM1OriginalSavePC34HandoffReport *report,
    struct DM1_EventQueue_V1 *queue);

int dm1_v1_original_save_pc34_handoff_projectile_event_plan(
    const struct DM1_Event_V1 *event,
    int source_event_index,
    const struct DungeonThings_Compat *things,
    DM1OriginalSavePC34ProjectileEventPlan *out_plan);

/* ReDMCSB CLIKVIEW.C F0374 writes C13 with Effect=2, while TIMELINE.C F0255
 * consumes steps 2, 1, and 0. Reject all other values and invalid champion or
 * cell ownership rather than treating a saved C13 as a generic square event. */
int dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
    const struct DM1_Event_V1 *src,
    int source_event_index,
    DM1OriginalSavePC34ViAltarRebirthEventPlan *out_plan);

int dm1_v1_original_save_pc34_handoff_load_world_from_file(
    const char *path,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report);

int dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report);

/* ReDMCSB LOADSAVE.C F0435 restores save parts and the dungeon before the
 * live runtime consumes HoC state. These helpers stage all parsed state in
 * a candidate world and commit it only after the final dungeon/timeline
 * handoff succeeds. On failure, world, event_queue, and out_report retain
 * their prior values. A tail-less original save borrows start_world's
 * already materialized dungeon; out_world may not alias start_world. */
int dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
    const char *path,
    const struct GameWorld_Compat *start_world,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report);

int dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
    struct GameWorld_Compat *runtime_world,
    struct GameWorld_Compat *loaded_world);

void dm1_v1_original_save_pc34_handoff_normalize_hoc_resume_state(
    const struct GameWorld_Compat *world,
    DM1OriginalSavePC34HoCResumeState *state);

/* Import a ReDMCSB PC34-shaped save into a bounded DM1 runtime world,
 * immediately export that world back through the PC34 exporter, and
 * validate the exported bytes through the same handoff reader.
 *
 * This is a verification/export bridge, not an in-place byte copier:
 * header noise and Firestaff-only manifest bytes may differ, while
 * GLOBAL_DATA/ACTIVE_GROUP/PARTY/EVENT/TIMELINE plus any materialized
 * dungeon tail must remain handoff-readable.
 *
 * Source-lock:
 *   ReDMCSB LOADSAVE.C F0435 load order and F0433 save-part order;
 *   READWRIT.C F0417/F0419/F0420 checksum/obfuscation; F0433 dungeon
 *   tail write and F0435 dungeon tail read.
 */
int dm1_v1_original_save_pc34_roundtrip_world_bytes(
    const uint8_t *bytes,
    size_t size,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34HandoffReport *import_report,
    DM1OriginalSavePC34HandoffReport *verify_report);

/* Full bounded original-save runtime round-trip:
 *   original PC34 bytes -> Firestaff world -> PC34 export -> Firestaff world.
 *
 * `out_report` records the source, exported-byte, and reloaded-world core
 * runtime state and sets core_state_matches only when party pose, game time,
 * event count, and active-group count survive both handoff edges.
 */
int dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
    const uint8_t *bytes,
    size_t size,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34RoundtripReport *out_report);

int dm1_v1_original_save_pc34_roundtrip_world_file(
    const char *path,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34HandoffReport *import_report,
    DM1OriginalSavePC34HandoffReport *verify_report);

int dm1_v1_original_save_pc34_roundtrip_world_reload_file(
    const char *path,
    uint32_t game_id,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size,
    DM1OriginalSavePC34RoundtripReport *out_report);

/* Verify every external, classifier-qualified PC34 save below `root` through
 * the F0435 -> F0433 -> F0435 transient-memory round trip. Firestaff's own
 * versioned PC34 manifest is rejected as corpus provenance, and a CSBWin
 * GAMEBLOCK1 cannot qualify without ReDMCSB's five length/key/checksum save
 * parts. Header-only, rejected, and non-PC34 files are never handed to the
 * importer. A successful scan returns OK even when a candidate fails, so the
 * caller can inspect the complete corpus receipt. */
int dm1_v1_original_save_pc34_roundtrip_corpus_root(
    const char *root,
    DM1OriginalSavePC34CorpusRoundtripReport *out_report);

/* Builds a bounded ReDMCSB PC34-shaped original-save byte stream for
 * importer/export handoff verification. This is not a full user save
 * exporter: it writes a deterministic GLOBAL_DATA/ACTIVE_GROUP/PARTY/
 * EVENT/TIMELINE envelope that the real classifier and handoff reader
 * must accept.
 *
 * Source-lock:
 *   ReDMCSB LOADSAVE.C F0433 save-part order and header write;
 *   SAVEHEAD.C F0430 header checksum/obfuscation; READWRIT.C F0417
 *   part checksum/obfuscation.
 */
int dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
    const DM1OriginalSavePC34FixtureSpec *spec,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size);

const char *dm1_v1_original_save_pc34_handoff_result_name(int result);
const char *dm1_v1_original_save_pc34_handoff_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_ORIGINAL_SAVE_PC34_HANDOFF_H */
