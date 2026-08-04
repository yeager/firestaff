#ifndef DM2_V1_SAVEGAME_PC34_COMPAT_H
#define DM2_V1_SAVEGAME_PC34_COMPAT_H

/*
 * dm2_v1_savegame_pc34_compat.h — DM2 save/load game unified module.
 *
 * Source: skproject c_savegame.cpp (29 functions, 2288 lines).
 *
 * This is a convenience aggregation header for the entire DM2 save/load
 * subsystem.  All logic is implemented across granular sub-modules:
 *
 *   save_load.h                  — SuppressReader/Writer, SKLOAD_READ,
 *                                  SKSAVE_WRITE, SELECT_LOAD_GAME,
 *                                  GAME_LOAD top-level flow
 *   save_suppress_masks.h        — Bit-level suppress masks for
 *                                  savegame_buffer, hero, save state, timer
 *   save_record_masks.h          — Record type sizes and diff masks
 *                                  (table1d64db, table_recordsizes)
 *   save_timers.h                — Timer record accessors, sort, rearrange,
 *                                  materialize, weather owner receipt
 *   save_compact_timerlist.h     — COMPACT_TIMERLIST pre-save cleanup
 *   save_dungeon_data.h          — READ_DUNGEON_STRUCTURE helpers
 *                                  (tile suppress size, teleporter forward ref)
 *   save_read_record_checkcode.h — READ_RECORD_CHECKCODE recursive reader
 *   save_write_record_checkcode.h— WRITE_RECORD_CHECKCODE recursive writer
 *   save_write_possession_indices.h — WRITE_POSSESSION_INDICES
 *   save_load_extra_dungeon_data.h  — READ_SKSAVE_DUNGEON
 *   save_store_extra_dungeon_data.h — STORE_EXTRA_DUNGEON_DATA
 *   save_post_load_global_effects.h — PROCEED_GLOBAL_EFFECT_TIMERS
 *   save_post_load_timer_rebuild.h  — 3a15_020f timer/hero index rebuild
 *   save_orchestrator.h          — GAME_SAVE_MENU orchestration flow
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Sub-module includes — the full DM2 save/load API
 * ======================================================================== */

#include "dm2_v1_save_load.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"
#include "dm2_v1_save_record_masks_pc34_compat.h"
#include "dm2_v1_save_timers_pc34_compat.h"
#include "dm2_v1_save_compact_timerlist_pc34_compat.h"
#include "dm2_v1_save_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_write_possession_indices_pc34_compat.h"
#include "dm2_v1_save_load_extra_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_store_extra_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_post_load_global_effects_pc34_compat.h"
#include "dm2_v1_save_post_load_timer_rebuild_pc34_compat.h"
#include "dm2_v1_save_orchestrator_pc34_compat.h"

/* ========================================================================
 * Savegame buffer layout — s_savegamebuffer (0x3c bytes)
 *
 * Offset  Size  Field                  skproject name
 * ------  ----  ---------------------  ------------------
 * 0x00    i32   game_tick              s33_00.l_00
 * 0x04    ui32  random_seed            s33_00.ul_04
 * 0x08    i16   heros_in_party         s33_00.w_08
 * 0x0A    i16   party_x                s33_00.w_0a
 * 0x0C    i16   party_y                s33_00.w_0c
 * 0x0E    i16   party_dir              s33_00.w_0e
 * 0x10    i16   party_map_unused       s33_00.w_10
 * 0x12    i16   active_hero            s33_00.w_12
 * 0x14    i16   num_timers             s33_00.w_14
 * 0x16    i32   field_v1d26a4          s33_00.l_16
 * 0x1A    i32   field_v1e01a0          s33_00.l_1a
 * 0x1E    i16   field_v1e026e          s33_00.w_1e
 * 0x20    i16   field_v1e025e          s33_00.w_20
 * 0x22    i16   field_v1e0274          s33_00.w_22
 * 0x24    i32   field_l_24             (not confirmed)
 * 0x28    i16   packed_v1d26a0_a2      s33_00.w_28
 * 0x2A    i32   field_v1e147f          s33_00.l_2a
 * 0x2E    i8    field_v1e1480          s33_00.b_2e
 * 0x2F    i8    field_v1e1483          s33_00.b_2f
 * 0x30    i8    field_v1e1482          s33_00.b_30
 * 0x31    i8    field_v1e147e          s33_00.b_31
 * 0x32    i8    field_v1e147d          s33_00.b_32
 * 0x33    i8    field_v1e1484          s33_00.b_33
 * 0x34    i16   field_v1e1474          s33_00.w_34
 * 0x36    i8    field_v1e147b          s33_00.b_36
 * 0x37    i8    field_v1e1478          s33_00.b_37
 * 0x38    i32   field_v1e1434          s33_00.l_38
 * ======================================================================== */

#define DM2_SAVEGAME_BUFFER_SIZE  0x3C

/* Savegame header (s_hex30) — 0x2A bytes written before dungeon structure. */
#define DM2_SAVEGAME_HEADER_SIZE  0x2A

/* Dungeon structure header (s_sgwords) — 0x2C bytes. */
#define DM2_SAVEGAME_DUNGEON_HEADER_SIZE  0x2C

/* Maximum record types in the save format. */
#define DM2_SAVEGAME_MAX_RECORD_TYPES  16

/* Record type codes used in save/load. */
#define DM2_RECORD_TYPE_DOOR       0x00
#define DM2_RECORD_TYPE_TELEPORTER 0x01
#define DM2_RECORD_TYPE_TEXT       0x02
#define DM2_RECORD_TYPE_ACTUATOR   0x03
#define DM2_RECORD_TYPE_CREATURE   0x04
#define DM2_RECORD_TYPE_WEAPON     0x05
#define DM2_RECORD_TYPE_CLOTHING   0x06
#define DM2_RECORD_TYPE_SCROLL     0x07
#define DM2_RECORD_TYPE_POTION     0x08
#define DM2_RECORD_TYPE_CONTAINER  0x09
#define DM2_RECORD_TYPE_MISC_A     0x0A
#define DM2_RECORD_TYPE_MISC_B     0x0B
#define DM2_RECORD_TYPE_MISC_C     0x0C
#define DM2_RECORD_TYPE_MISC_D     0x0D
#define DM2_RECORD_TYPE_MISSILE    0x0E
#define DM2_RECORD_TYPE_CLOUD      0x0F

/* Tile types from dungeon structure (top 3 bits of tile byte). */
#define DM2_TILE_WALL       0
#define DM2_TILE_FLOOR      1
#define DM2_TILE_PIT        2
#define DM2_TILE_STAIRS     3
#define DM2_TILE_DOOR       4
#define DM2_TILE_TELEPORTER 5
#define DM2_TILE_TRICK_WALL 6
#define DM2_TILE_SPECIAL    7

/* Suppress sizes per tile type (from DM2_save_tile_suppress_size). */
#define DM2_TILE_SUPPRESS_WALL       0
#define DM2_TILE_SUPPRESS_FLOOR      0
#define DM2_TILE_SUPPRESS_PIT        8
#define DM2_TILE_SUPPRESS_STAIRS     0
#define DM2_TILE_SUPPRESS_DOOR       7
#define DM2_TILE_SUPPRESS_TELEPORTER 8  /* or 0 for forward-ref */
#define DM2_TILE_SUPPRESS_TRICK      4
#define DM2_TILE_SUPPRESS_SPECIAL    0

/* Actuator sub-types that carry a 9-bit suppress field (from WRITE/READ). */
#define DM2_ACTUATOR_TYPE_27  0x27
#define DM2_ACTUATOR_TYPE_1B  0x1B
#define DM2_ACTUATOR_TYPE_1D  0x1D
#define DM2_ACTUATOR_TYPE_41  0x41
#define DM2_ACTUATOR_TYPE_2C  0x2C
#define DM2_ACTUATOR_TYPE_32  0x32
#define DM2_ACTUATOR_TYPE_30  0x30
#define DM2_ACTUATOR_TYPE_2D  0x2D

/* Source parity evidence. */
const char *dm2_v1_savegame_source_evidence(void);

/* Module count: total number of sub-modules aggregated here. */
int dm2_v1_savegame_submodule_count(void);

/* Verify all sub-modules have consistent source evidence strings. */
bool dm2_v1_savegame_all_evidence_present(void);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVEGAME_PC34_COMPAT_H */
