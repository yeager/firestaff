#ifndef FIRESTAFF_CSB_V1_F0806_ENTRANCE_LOOP_RUNTIME_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0806_ENTRANCE_LOOP_RUNTIME_HANDOFF_PC34_COMPAT_H

#include "csb_v1_f0439_f0441_f0442_startend_entrance_boundaries_pc34_compat.h"
#include "csb_v1_f0797_startend_entrance_micro_dungeon_pc34_compat.h"
#include "csb_v1_f0807_entrance_animation_step_runtime_coupling_pc34_compat.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0806_MODE_LOAD_SAVED_GAME_PC34 = 0,
    CSB_V1_F0806_MODE_LOAD_DUNGEON_PC34 = 1,
    CSB_V1_F0806_MODE_WAITING_ON_ENTRANCE_PC34 = 99,
    CSB_V1_F0806_COMMAND_ENTRANCE_DRAW_CREDITS_PC34 = 202,
    CSB_V1_F0806_COMMAND_ENTRANCE_ENTER_DUNGEON_PC34 = 200,
    CSB_V1_F0806_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON_PC34 = 201
};

typedef enum CSB_V1_F0806_EntranceLoopDecision_PC34 {
    CSB_V1_F0806_ENTRANCE_LOOP_DECISION_NONE_PC34 = 0,
    CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_SAVED_GAME_PC34 = 1,
    CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_DUNGEON_PC34 = 2,
    CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_BONUS_DUNGEON_PC34 = 3
} CSB_V1_F0806_EntranceLoopDecision_PC34;

typedef struct CSB_V1_F0806_EntranceLoopFacts_PC34 {
    int valid;
    int entrance_assets_bound;
    int entrance_input_tables_bound;
    int entrance_music_started;
    int draw_entrance_each_outer_loop;
    int pointer_shown_before_wait;
    int input_discarded_before_wait;
    int waiting_mode_set_before_queue;
    int command_queue_processed_until_exit;
    int credits_command_loops_before_exit;
    int final_new_game_mode;
    int final_command;
    int bonus_dungeon_selected;
    int post_selection_switch_sound_played;
    int post_selection_delay_ticks;
    int temporary_entrance_memory_released;
    int no_synthetic_input;
    int no_synthetic_graphics_bytes;
    int no_fallback_visuals;
    int no_legacy_entrance_wrapper;
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 draw_entrance;
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 draw_credits;
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 micro_dungeon;
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 door_animation_step;
} CSB_V1_F0806_EntranceLoopFacts_PC34;

typedef struct CSB_V1_F0806_EntranceLoopReceipt_PC34 {
    int valid;
    CSB_V1_F0806_EntranceLoopDecision_PC34 decision;
    int final_new_game_mode;
    int final_command;
    int bonus_dungeon_selected;
    int entrance_draw_consumed;
    int credits_route_consumed;
    int micro_dungeon_consumed;
    int door_animation_consumed;
    int waiting_loop_source_locked;
    int command_queue_source_locked;
    int temporary_entrance_memory_released;
    int no_synthetic_input;
    int no_synthetic_graphics_bytes;
    int no_fallback_visuals;
    int no_legacy_entrance_wrapper;
    int opening_material_consumed;
    uint32_t source_tick;
    uint32_t session_generation;
    uint32_t opening_host_surface_hash;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
    const char *source_evidence;
} CSB_V1_F0806_EntranceLoopReceipt_PC34;

void csb_v1_f0806_entrance_loop_receipt_init_pc34(
    CSB_V1_F0806_EntranceLoopReceipt_PC34 *receipt);

int F0806_F0806_ENTRANCE_int(
    const CSB_V1_F0806_EntranceLoopFacts_PC34 *facts,
    CSB_V1_F0806_EntranceLoopReceipt_PC34 *out_receipt);

/* Bind F0806's final entrance-to-runtime handoff to the resident C004/C002/
 * C003 host raster. The last F0807 door step must be from the same verified
 * package session and source tick; this rejects a cached entrance page or a
 * later-session handoff even when the individual facts still look valid. */
int csb_v1_f0806_entrance_loop_runtime_handoff_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *opening_host,
    const CSB_V1_F0806_EntranceLoopFacts_PC34 *facts,
    CSB_V1_F0806_EntranceLoopReceipt_PC34 *out_receipt);

const char *csb_v1_f0806_entrance_loop_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0806_ENTRANCE_LOOP_RUNTIME_HANDOFF_PC34_COMPAT_H */
