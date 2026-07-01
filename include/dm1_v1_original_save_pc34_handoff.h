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

typedef struct {
    DM1OriginalSaveClassifyResult classify;
    int importer_result;
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
    int dungeon_tail_present;
    uint32_t dungeon_tail_byte_count;
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

const char *dm1_v1_original_save_pc34_handoff_result_name(int result);
const char *dm1_v1_original_save_pc34_handoff_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_ORIGINAL_SAVE_PC34_HANDOFF_H */
