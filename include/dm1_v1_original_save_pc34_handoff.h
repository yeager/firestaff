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

/* ReDMCSB READWRIT.C F0421. Read one source span into destination and add
 * its unsigned bytes to the caller-owned 16-bit running checksum. The cursor
 * and checksum are unchanged when the complete source span is unavailable. */
int dm1_v1_original_save_pc34_f0421_read_bytes_with_checksum(
    const uint8_t* source,
    size_t source_size,
    size_t* io_cursor,
    uint8_t* destination,
    size_t byte_count,
    uint16_t* io_running_checksum);

/* ReDMCSB READWRIT.C F0422. Write one complete dungeon-tail source span and
 * add its unsigned bytes to the caller-owned 16-bit running checksum. The
 * cursor and checksum are unchanged when the complete destination span is
 * unavailable. F0433 owns the enclosing tail and its final checksum word. */
int dm1_v1_original_save_pc34_f0422_write_bytes_with_checksum(
    uint8_t* destination,
    size_t destination_size,
    size_t* io_cursor,
    const uint8_t* source,
    size_t byte_count,
    uint16_t* io_running_checksum);

#define DM1_ORIGINAL_SAVE_PC34_HANDOFF_ACTIVE_GROUP_REPORT_CAP \
    GAMEWORLD_CREATURE_AI_CAPACITY
#define DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT 4
#define SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT \
    (CHAMPION_MAX_PARTY * CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT)
#define DM1_F0259_MOVE_TIMER_AUX4_PC34 0
#define DM1_ORIGINAL_SAVE_PC34_C3_RECEIPT_BYTE_CAP \
    (DM1_EVENT_MAX_COUNT * GAMEWORLD_PC34_ORIGINAL_C3_EVENT_BYTE_COUNT)
#define DM1_ORIGINAL_SAVE_PC34_C4_RECEIPT_BYTE_CAP \
    (DM1_EVENT_MAX_COUNT * GAMEWORLD_PC34_ORIGINAL_C4_HEAP_ENTRY_BYTE_COUNT)

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
    /* Source-owned C2 party metadata. F0435 imports these fixed M516
     * records before it restores the C3/C4 timeline, so runtime adoption
     * must retain the selected members, leader, and inventory slots. */
    uint32_t source_party_champion_metadata_fingerprint;
    uint32_t source_party_champion_state_fingerprint;
    int c13_party_runtime_receipt_valid;
    int c13_party_runtime_champion_count;
    int c13_party_runtime_active_champion_index;
    uint32_t c13_party_runtime_metadata_fingerprint;
    uint32_t c13_party_runtime_state_fingerprint;
    uint32_t c13_party_runtime_timeline_fingerprint;
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
    /* Authenticated F0433 C3/C4 plaintext from the validated part buffers.
     * The world boundary publishes this only after full F0435 success. */
    int c3_c4_receipt_valid;
    uint32_t c3_raw_event_byte_count;
    uint32_t c4_raw_heap_byte_count;
    uint32_t c3_raw_event_fingerprint;
    uint32_t c4_raw_heap_fingerprint;
    uint32_t c3_c4_runtime_event_count;
    uint32_t timeline_runtime_fingerprint;
    uint8_t c3_raw_event_bytes[DM1_ORIGINAL_SAVE_PC34_C3_RECEIPT_BYTE_CAP];
    uint8_t c4_raw_heap_bytes[DM1_ORIGINAL_SAVE_PC34_C4_RECEIPT_BYTE_CAP];
    int event_decode_truncated_count;
    /* C4 TIMELINE is a one-index-per-live-EVENT heap in the F0238/F0433
     * path. Retain the first duplicate relation so rejected external saves
     * have stable source provenance without publishing a candidate world. */
    int timeline_duplicate_first_slot;
    int timeline_duplicate_slot;
    int timeline_duplicate_event_index;
    int timeline_invalid_slot;
    int timeline_invalid_event_index;
    int timeline_invalid_event_is_none;
    /* Every non-NONE C3 EVENT emitted by F0433 must appear once in the C4
     * heap written by F0238. Retain the first missing row for rollback
     * provenance before runtime materialization can drop it. */
    int timeline_orphan_active_event_index;
    int timeline_orphan_active_event_type;
    /* ReDMCSB TIMELINE.C F0234 expects C4 to remain a binary heap. Keep
     * the first parent/child violation so a checksum-valid reordered C4
     * payload cannot resume later events ahead of earlier ones. */
    int timeline_heap_invalid_parent_slot;
    int timeline_heap_invalid_child_slot;
    int timeline_heap_invalid_parent_event_index;
    int timeline_heap_invalid_child_event_index;
    int first_unused_event_index_points_to_active;
    int first_unused_event_index_event_type;
    uint32_t external_portrait_byte_count;
    uint32_t external_portrait_byte_offset;
    uint32_t external_portrait_fingerprint;
    uint32_t pc34_party_part_byte_offset;
    uint16_t pc34_party_part_key;
    int external_portrait_payload_count;
    int external_portrait_imported_count;
    int dungeon_tail_present;
    uint32_t dungeon_tail_byte_count;
    uint16_t dungeon_tail_expected_checksum;
    uint16_t dungeon_tail_actual_checksum;
    int dungeon_tail_checksum_ok;
    int dungeon_tail_map_count;
    int dungeon_tail_column_count;
    int dungeon_tail_column_table_valid;
    uint32_t dungeon_tail_column_terminal_sft_count;
    uint32_t dungeon_tail_column_table_fingerprint;
    int dungeon_tail_square_first_thing_count;
    uint32_t dungeon_tail_square_first_thing_fingerprint;
    int dungeon_tail_text_data_word_count;
    uint32_t dungeon_tail_thing_data_byte_count;
    uint32_t dungeon_tail_raw_map_data_byte_count;
    int dungeon_tail_text_string_count;
    uint32_t dungeon_tail_fingerprint;
    int dungeon_tail_runtime_imported;
    /* F0435/F0433 adoption receipt. These values are published only after
     * the loaded dungeon's G0280/SquareFirstThings arrays and the restored
     * C3/C4 runtime timeline still match the authenticated tail. */
    int dungeon_tail_runtime_receipt_valid;
    uint32_t dungeon_tail_runtime_column_table_fingerprint;
    uint32_t dungeon_tail_runtime_square_first_thing_fingerprint;
    uint32_t dungeon_tail_runtime_timeline_fingerprint;
    int imported_event73_count_thieves_eye;
    int imported_magical_light_amount;
    int imported_event79_count_footprints;
    int imported_event71_count_invisibility;
    int imported_party_shield_defense;
    int imported_party_fire_shield_defense;
    int imported_party_spell_shield_defense;
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
    int core_state_matches;
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
    int c3_event_layout_receipt_available;
    uint32_t source_c3_event_record_count;
    uint32_t source_c3_event_byte_count;
    uint32_t source_c3_event_fingerprint;
    uint32_t exported_c3_event_record_count;
    uint32_t exported_c3_event_byte_count;
    uint32_t exported_c3_event_fingerprint;
    int c3_event_byte_preservation_ok;
    int c4_timeline_layout_receipt_available;
    uint32_t source_c4_timeline_index_count;
    uint32_t source_c4_timeline_byte_count;
    uint32_t source_c4_timeline_fingerprint;
    uint32_t exported_c4_timeline_index_count;
    uint32_t exported_c4_timeline_byte_count;
    uint32_t exported_c4_timeline_fingerprint;
    int c4_timeline_byte_preservation_ok;
    int source_dungeon_tail_present;
    uint32_t source_dungeon_tail_byte_count;
    uint16_t source_dungeon_tail_checksum;
    uint32_t source_dungeon_tail_fingerprint;
    int exported_dungeon_tail_present;
    uint32_t exported_dungeon_tail_byte_count;
    uint16_t exported_dungeon_tail_checksum;
    uint32_t exported_dungeon_tail_fingerprint;
    int reloaded_dungeon_tail_present;
    uint32_t reloaded_dungeon_tail_byte_count;
    uint16_t reloaded_dungeon_tail_checksum;
    int dungeon_tail_matches;
    int dungeon_tail_byte_receipt_available;
    int dungeon_tail_byte_preservation_ok;
    uint32_t source_party_info_byte_count;
    uint32_t source_party_info_fingerprint;
    uint32_t exported_party_info_byte_count;
    uint32_t exported_party_info_fingerprint;
    int party_info_byte_receipt_available;
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
    int source_c13_champion_record_reference_count;
    int c13_champion_record_byte_receipt_available;
    int c13_champion_record_byte_preserved_count;
    int c13_champion_record_byte_mismatch_count;
    int c13_champion_record_byte_preservation_ok;
    int source_c13_event_count;
    int exported_c13_event_count;
    int c13_event_byte_preservation_ok;
    /* F0433 emission receipt for source-owned C13 rows. It commits only
     * when C2, C3, C4, and the optional source tail retain raw bytes. */
    int c13_roundtrip_emission_receipt_available;
    int c13_roundtrip_emission_valid;
    uint32_t c13_roundtrip_emission_fingerprint;
    /* Raw PC34 input identity for a C13-bearing round trip. The span is the
     * stored C3 payload, before F0417 deobfuscation. */
    int c13_roundtrip_input_identity_receipt_available;
    uint32_t c13_roundtrip_input_byte_count;
    uint32_t c13_roundtrip_input_hash;
    uint32_t c13_roundtrip_input_c3_byte_offset;
    uint32_t c13_roundtrip_input_c3_byte_count;
    uint32_t c13_roundtrip_input_c3_fingerprint;
} DM1OriginalSavePC34RoundtripReport;

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
    uint16_t associated_thing;
} DM1OriginalSavePC34ProjectileEventPlan;

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

int dm1_v1_original_save_pc34_handoff_vi_altar_rebirth_event_plan(
    const struct DM1_Event_V1 *src,
    int source_event_index,
    DM1OriginalSavePC34ViAltarRebirthEventPlan *out_plan);

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
    /* Set only after the complete F0435 -> F0433 -> F0435 transaction
     * succeeds. Failed rows can retain raw diagnostic evidence below, but
     * must never present it as a committed C13/C24/C25 corpus receipt. */
    int roundtrip_receipts_committed;
    /* A corpus-only staging receipt for the original source bytes. It records
     * which F0435 validation boundary rejected a candidate without exposing a
     * partially imported runtime world or promoting optional-event evidence. */
    int source_handoff_result;
    int source_importer_result;
    int source_part_checksum_ok_count;
    /* A second, no-fallback F0435 receipt stages the exact source snapshot
     * through the candidate runtime world. It commits only when that source
     * carries and materializes its own dungeon; tail-less saves are never
     * paired with a host-created or borrowed start dungeon for corpus proof. */
    int source_runtime_stage_attempted;
    int source_runtime_stage_result;
    int source_runtime_stage_committed;
    int source_runtime_stage_owns_dungeon;
    int source_runtime_stage_event_count;
    int source_runtime_stage_c13_event_count;
    /* A C13 count alone cannot prove that F0435 retained its source-owned
     * Location/Cell/Effect/Priority state.  These fields are set only when
     * every raw C13 row is found in the tail-backed runtime timeline. */
    int source_runtime_stage_c13_admitted_count;
    int source_runtime_stage_c13_admission_ok;
    uint32_t source_runtime_stage_c13_fingerprint;
    int source_runtime_stage_c13_party_receipt_valid;
    uint32_t source_runtime_stage_party_metadata_fingerprint;
    uint32_t source_runtime_stage_party_state_fingerprint;
    int source_runtime_stage_timeline_count;
    /* A tail-backed source must also cross the final candidate-to-live
     * ownership transfer with no start-world argument. */
    int source_runtime_adopt_attempted;
    int source_runtime_adopt_result;
    int source_runtime_adopted;
    int source_runtime_adopt_owns_dungeon;
    int source_runtime_adopt_event_count;
    int source_runtime_adopt_c13_admitted_count;
    int source_runtime_adopt_c13_admission_ok;
    uint32_t source_runtime_adopt_c13_fingerprint;
    int source_runtime_adopt_c13_party_receipt_valid;
    uint32_t source_runtime_adopt_party_metadata_fingerprint;
    uint32_t source_runtime_adopt_party_state_fingerprint;
    int source_runtime_adopt_timeline_count;
    /* The F0238 queue is a separate runtime owner from world.timeline.
     * Corpus adoption must move that validated C3/C4 queue with the world,
     * rather than merely observing its pre-adoption count. */
    int source_runtime_adopt_queue_committed;
    int source_runtime_adopt_queue_event_count;
    int source_runtime_adopt_queue_first_unused_index;
    uint32_t game_id;
    uint32_t source_byte_count;
    uint32_t source_hash;
    uint32_t source_f7057_envelope_end_offset;
    uint32_t source_f7057_trailing_byte_count;
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
    int c13_roundtrip_emission_receipt_available;
    int c13_roundtrip_emission_valid;
    uint32_t c13_roundtrip_emission_fingerprint;
    int c13_roundtrip_input_admission_available;
    int c13_roundtrip_input_admission_valid;
    uint32_t c13_roundtrip_input_byte_count;
    uint32_t c13_roundtrip_input_hash;
    uint32_t c13_roundtrip_input_c3_byte_offset;
    uint32_t c13_roundtrip_input_c3_byte_count;
    uint32_t c13_roundtrip_input_c3_fingerprint;
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
    int c3_event_layout_receipt_available;
    uint32_t source_c3_event_record_count;
    uint32_t source_c3_event_byte_count;
    uint32_t source_c3_event_fingerprint;
    uint32_t exported_c3_event_record_count;
    uint32_t exported_c3_event_byte_count;
    uint32_t exported_c3_event_fingerprint;
    int c3_event_byte_preservation_ok;
    int c4_timeline_layout_receipt_available;
    uint32_t source_c4_timeline_index_count;
    uint32_t source_c4_timeline_byte_count;
    uint32_t source_c4_timeline_fingerprint;
    uint32_t exported_c4_timeline_index_count;
    uint32_t exported_c4_timeline_byte_count;
    uint32_t exported_c4_timeline_fingerprint;
    int c4_timeline_byte_preservation_ok;
    int source_c13_champion_record_reference_count;
    int c13_champion_record_byte_receipt_available;
    int c13_champion_record_byte_preserved_count;
    int c13_champion_record_byte_mismatch_count;
    int c13_champion_record_byte_preservation_ok;
    uint32_t source_party_info_byte_count;
    uint32_t source_party_info_fingerprint;
    uint32_t exported_party_info_byte_count;
    uint32_t exported_party_info_fingerprint;
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
    uint32_t f7057_envelope_end_offset;
    uint32_t f7057_trailing_byte_count;
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
    int rejected_count;
    int truncated_count;
    int pc34_candidate_count;
    int roundtrip_attempted_count;
    int roundtrip_succeeded_count;
    int core_state_match_count;
    int roundtrip_failed_count;
    int runtime_stage_attempted_count;
    int runtime_stage_succeeded_count;
    int runtime_stage_unavailable_count;
    int runtime_stage_failed_count;
    int runtime_adopt_attempted_count;
    int runtime_adopt_succeeded_count;
    int runtime_adopt_failed_count;
    /* Stable FNV-1a fingerprint of successful transient F0435->F0433
     * exports. Zero means no original save completed the round trip. */
    uint32_t roundtrip_hash;
    int firestaff_manifest_rejected_count;
    int nonoriginal_envelope_rejected_count;
    int first_failure_result;
    uint32_t provenance_fingerprint;
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

/* ReDMCSB DUNGEON.C F0145/F0147 unpack the two-bit ACTIVE_GROUP cell and
 * direction lanes. Lane 0 is the first creature lane consumed by GROUP.C
 * active-group runtime state; lanes 1..3 preserve the remaining packed
 * creature slots for original-save/runtime handoff. */
int F0145_DUNGEON_GetGroupCells(int packedCells, int creatureLane);
int F0147_DUNGEON_GetGroupDirections(int packedDirections, int creatureLane);
int dm1_v1_original_save_pc34_unpack_active_group_lanes(
    int packedCells,
    int packedDirections,
    int outCells[DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT],
    int outDirections[DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT]);

int dm1_v1_original_save_pc34_handoff_apply_event_queue(
    const DM1OriginalSavePC34HandoffReport *report,
    struct DM1_EventQueue_V1 *queue);

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
 * live runtime consumes HoC state. This file entry point admits only an
 * external PC34 envelope: a Firestaff-manifest-bearing F0433 export cannot
 * re-enter the launcher/runtime resume route. These helpers stage all parsed
 * state in a candidate world and commit it only after the final
 * dungeon/timeline handoff succeeds. On failure, world, event_queue, and
 * out_report retain their prior values. A tail-less original save borrows
 * start_world's already materialized dungeon; out_world may not alias
 * start_world. */
int dm1_v1_original_save_pc34_handoff_materialize_runtime_from_file(
    const char *path,
    const struct GameWorld_Compat *start_world,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report);

/* The immutable-byte counterpart is used by import/browser paths.  It keeps
 * the same F0435 candidate-world transaction as the file route, including
 * borrowing start_world's original DUNGEON.DAT backing for a tail-less save. */
int dm1_v1_original_save_pc34_handoff_materialize_runtime_from_bytes(
    const uint8_t *bytes,
    size_t size,
    const struct GameWorld_Compat *start_world,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *event_queue,
    DM1OriginalSavePC34HandoffReport *out_report);

int dm1_v1_original_save_pc34_handoff_adopt_runtime_world(
    struct GameWorld_Compat *runtime_world,
    struct GameWorld_Compat *loaded_world);

/* Transfer a fully staged F0435 runtime pair. The world and F0238 queue are
 * committed together only after both source objects have been validated; the
 * consumed candidate pair is cleared so it cannot be published twice. */
int dm1_v1_original_save_pc34_handoff_adopt_runtime_state(
    struct GameWorld_Compat *runtime_world,
    struct DM1_EventQueue_V1 *runtime_queue,
    struct GameWorld_Compat *loaded_world,
    struct DM1_EventQueue_V1 *loaded_queue);

/* Complete the ReDMCSB LOADSAVE.C F0435 resume boundary for already-read
 * PC34 bytes. The source is staged against the live DM1 dungeon, then the
 * candidate world and F0238 event queue are adopted atomically. On failure
 * neither live owner nor out_report is changed. This is deliberately an
 * import/runtime operation only; F0433 export remains a separate explicit
 * caller decision. */
int dm1_v1_original_save_pc34_handoff_resume_runtime_from_bytes(
    const uint8_t *bytes,
    size_t size,
    struct GameWorld_Compat *runtime_world,
    struct DM1_EventQueue_V1 *runtime_queue,
    DM1OriginalSavePC34HandoffReport *out_report);

/* ReDMCSB READWRIT.C F0420: prefix an even-byte save part with its original
 * 16-bit size, obfuscate it through F0417 exactly once, and return the
 * source checksum. The caller retains transaction ownership of the complete
 * F0433 save envelope; malformed spans are rejected before any write. */
int dm1_v1_original_save_pc34_write_part_f0420(
    uint8_t *dst,
    size_t dst_capacity,
    const uint8_t *plain,
    size_t byte_count,
    uint16_t key,
    uint16_t *out_checksum);

/* ReDMCSB READWRIT.C F0419: consume one F0420 length-prefixed save part and
 * apply F0417 exactly once to recover its source bytes and checksum. A bad
 * prefix or span is rejected before body access. The cursor advances across a
 * complete part even when its source checksum differs, matching the reader's
 * diagnostic contract; F0435 retains ownership of whole-save publication. */
int dm1_v1_original_save_pc34_read_part_f0419(
    const uint8_t *bytes,
    size_t size,
    size_t *cursor,
    uint16_t key,
    uint16_t expected_checksum,
    uint8_t *out_plain,
    size_t out_capacity,
    size_t *out_size,
    uint16_t *out_actual_checksum);

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

/* Verify every classifier-qualified PC34 save below `root` through the
 * F0435 -> F0433 -> F0435 transient-memory round trip. Header-only,
 * rejected, and non-PC34 files are reported by the classifier but are never
 * handed to the importer. A successful scan returns OK even when a candidate
 * fails, so the caller can inspect the complete corpus receipt. */
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

int dm1_v1_original_save_pc34_handoff_projectile_event_plan( const struct DM1_Event_V1 *src, int source_index, const struct DungeonThings_Compat *things, DM1OriginalSavePC34ProjectileEventPlan *out_plan);

#endif /* DM1_V1_ORIGINAL_SAVE_PC34_HANDOFF_H */
