#ifndef FIRESTAFF_DM2_V1_GAME_LOAD_WORLD_OWNER_H
#define FIRESTAFF_DM2_V1_GAME_LOAD_WORLD_OWNER_H

/*
 * dm2_v1_game_load_world_owner.h — owned File_header world for New Game.
 *
 * SKProject loads the dungeon structure before DM2_GAME_LOAD mutates the
 * DYN, champion and timer state.  This owner is that first durable boundary:
 * it copies only the hash-admitted File_header image and its exact DB pools
 * to RAM.  It is intentionally not a playable session.
 *
 * Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON (616-640),
 *         ::DM2_GAME_LOAD (1415-1546),
 *         SKULLWIN/c_record.cpp::DM2_GET_ADDRESS_OF_RECORD (28-57).
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_timer_queue_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* `prepared` means all source bytes have one RAM owner. `committed` is
     * deliberately zero until the later source-ordered DYN/hero/timer
     * transaction can publish every owner atomically. */
    int prepared;
    int committed;
    int current_map;
    uint32_t source_transaction_hash;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet record_pools;
    /* c_savegame.cpp::DM2_READ_DUNGEON_STRUCTURE computes these capacities
     * before it allocates the original 12-byte c_tim array and index heap.
     * They are allocation limits, not invented queued timers. */
    uint16_t record_capacities[DM2_V1_RECORD_POOL_COUNT];
    DM2_V1_TimerEntry *timer_entries;
    int16_t *timer_indices;
    DM2_V1_TimerQueue timer_queue;
    uint16_t timer_capacity;
    int fresh_game_mode;
    const DM2_V1_AssetLoader *asset_loader;
    int dyn4_materialized;
    uint16_t dyn4_selector_count;
    uint32_t dyn4_materialized_hash;
    uint16_t validated_map_count;
    uint32_t validated_world_hash;
    DM2_V1_GdatDyn4MaterializedSelection dyn4_selections[
        DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES];
    DM2_V1_BootNewGameTransactionReceipt transaction;
    /* This remains zeroed until the source-ordered champion step.  It is
     * never installed in M11 or the runtime party state. */
    int champion_selection_materialized;
    DM2_V1_SksaveItemBonusReceipt champion_item_bonus;
    DM2_V1_Party selected_party;
} DM2_V1_GameLoadWorldOwner;

typedef struct {
    int valid;
    uint16_t candidate_count;
    uint16_t control_bit2_clear_count;
    uint16_t activation_count;
    uint16_t queued_timer_count;
    uint32_t receipt_hash;
} DM2_V1_GameLoadActuatorGeneratorReceipt;

typedef struct {
    int valid;
    int actuator_invoked;
    int message_queued;
    int requeued;
    int active_flag_cleared;
    int16_t record_link;
    uint8_t action;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t target_direction;
} DM2_V1_GameLoadTickGeneratorReceipt;

/* Result of consuming a private 0x04 message produced by the tick-generator
 * continuation above.  Tile class 3 is the only complete source case that
 * needs no further owner: c_tim_proc.cpp falls through without a handler.
 * Every mutating class remains blocked until one source-owned FLOOR/WALL
 * chain, including its follow-up timers, can be committed as a unit. */
typedef struct {
    int valid;
    int source_noop;
    int blocked_incomplete_chain;
    int map;
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    uint8_t action;
    uint8_t tile_class;
    uint8_t raw_tile;
} DM2_V1_GameLoadActuateReceipt;

/* Build a source-owned, pre-selection world from one currently mounted,
 * hash-verified PC File_header/GDAT pair and exact mirror clicks.  It rejects
 * partial record graphs and never modifies profile-owned raw media. */
int dm2_v1_game_load_world_owner_init_new_game(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootProfile *profile,
    const DM2_V1_BootNewGamePartySelection *selections,
    int selection_count);

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner);
int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner);

/* Source port of c_tim_proc.cpp::DM2_PROCESS_ACTUATOR_TICK_GENERATOR for
 * the prepared fresh-game owner. It changes only owned DB3 bytes and its
 * owned c_tim heap; an enqueue failure restores both before returning 0. */
int dm2_v1_game_load_world_owner_process_actuator_tick_generators(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadActuatorGeneratorReceipt *out_receipt);

/* Materialize the click-ordered c_hero and possession result that was
 * authenticated while the owner was built.  This is deliberately after the
 * fresh-game actuator-generator phase, as in c_savegame.cpp::DM2_GAME_LOAD.
 * It only transfers real source records into the private c_party image; it
 * neither removes map-chain records nor publishes a live session. */
int dm2_v1_game_load_world_owner_materialize_champion_selection(
    DM2_V1_GameLoadWorldOwner *owner);

/* Source port of c_tim_proc.cpp::DM2_CONTINUE_TICK_GENERATOR and its
 * DM2_INVOKE_ACTUATOR/DM2_INVOKE_MESSAGE tail. `timer` must be a popped 0x56
 * entry from this owner's c_tim heap. This schedules only private 0x04/0x56
 * entries and never dispatches their side effects into M11. */
int dm2_v1_game_load_world_owner_continue_tick_generator(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadTickGeneratorReceipt *out_receipt);

/* Consume one private 0x04 message after it was popped from this owner's
 * queue.  This ports DM2_PROCEED_TIMERS' tile-class selection exactly, but
 * performs a successful action only for source class 3 (the original no-op).
 * Classes 0, 1, 2, 4, 5 and 6 are rejected without mutation rather than
 * applying an incomplete wall/floor/door/pit/teleporter/trick-wall fragment.
 */
int dm2_v1_game_load_world_owner_dispatch_actuate_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadActuateReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
