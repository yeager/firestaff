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
#include "dm2_v1_sound.h"
#include "dm2_v1_timer_queue_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t raw_index;
    uint16_t raw_length;
    uint32_t source_payload_hash;
} DM2_V1_GameLoadSoundSampleBinding;

/* Immutable transition receipt for the actual mirror clicks.  The final hero
 * count alone cannot prove that an eventual input owner preserved the click
 * order.  This remains private until complete GAME_LOAD ownership exists.
 * Source: SKProject c_hero.cpp::DM2_SELECT_CHAMPION (1119-1168),
 *         ::DM2_SELECT_CHAMPION_LEADER (2325-2354). */
typedef struct {
    int valid;
    uint8_t click_count;
    uint8_t leader_select_count;
    int16_t initial_event_hero_index;
    int16_t final_event_hero_index;
    int16_t next_champion_number_after_click[DM2_MAX_HEROES];
    uint16_t mirror_object_id[DM2_MAX_HEROES];
    int8_t party_position[DM2_MAX_HEROES];
    uint32_t transition_hash;
} DM2_V1_GameLoadChampionSelectionReceipt;

/* The original start path first sizes xsndptr2 from every type-2 GDAT row,
 * then DM2_LOAD_DYN4 adds only marked triples and DM2_482b_0684 binds their
 * already materialised raw samples.  This is a durable private equivalent.
 * It deliberately has no mixer, PCM conversion or process-global binding. */
typedef struct {
    int valid;
    DM2_V1_DballocSoundCensusReceipt allocation;
    DM2_V1_SoundSsoundEntry *queue_entries;
    uint16_t queue_capacity;
    uint16_t queue_entry_count;
    DM2_V1_GameLoadSoundSampleBinding *sample_bindings;
    uint16_t sample_capacity;
    uint16_t sample_binding_count;
    uint32_t materialized_raw_hash;
    uint32_t receipt_hash;
} DM2_V1_GameLoadSoundOwner;

/* Private result of the three writes in DM2_LOAD_NEW_DUNGEON immediately
 * before DM2_READ_DUNGEON_STRUCTURE(1). The selected medium itself remains
 * owned by BootProfile; this is deliberately not a host file-open shim.
 * Source: SKProject SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON
 * (616-640). */
typedef struct {
    int valid;
    int16_t party_count;
    int16_t leader_hand_record;
    uint32_t save_stream_bytes_consumed;
    uint32_t receipt_hash;
} DM2_V1_GameLoadNewDungeonResetReceipt;

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
    /* The private LOAD_NEW_DUNGEON reset is an explicit predecessor of every
     * selected champion. It records source values only and cannot publish a
     * party or a save stream to M11. */
    DM2_V1_GameLoadNewDungeonResetReceipt load_new_dungeon_reset;
    int actuator_generators_processed;
    /* DM2_move_2fcf_0b8b / CHANGE_CURRENT_MAP_TO source state.  This stays
     * private until the later all-owner session commit. */
    int source_map_context_materialized;
    int source_party_map;
    uint8_t source_party_x;
    uint8_t source_party_y;
    uint8_t source_party_direction;
    /* Private result of skmove.cpp::DM2_move_2fcf_0b8b.  The original
     * probes the party square and its four neighbours for a File_header
     * teleporter after GAME_LOAD.  These are map-context bytes only; no
     * viewport, party placement or live map is published from them. */
    int source_staircase_flag;
    int16_t source_teleporter_map;
    int16_t source_display_x;
    int16_t source_display_y;
    uint8_t source_party_absdir;
    int source_display_pose_valid;
    int16_t source_last_moved_record;
    /* Borrowed only while this private owner exists; it is the same
     * hash-admitted profile that owns asset_loader. */
    const DM2_V1_BootProfile *boot_profile;
    const DM2_V1_AssetLoader *asset_loader;
    int dyn4_materialized;
    uint16_t dyn4_selector_count;
    uint32_t dyn4_selector_ids[
        DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES];
    uint32_t dyn4_materialized_hash;
    uint16_t validated_map_count;
    uint32_t validated_world_hash;
    DM2_V1_GdatDyn4MaterializedSelection dyn4_selections[
        DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES];
    DM2_V1_GameLoadSoundOwner sound_owner;
    DM2_V1_BootNewGameTransactionReceipt transaction;
    /* This remains zeroed until the source-ordered champion step.  It is
     * never installed in M11 or the runtime party state. */
    int champion_selection_materialized;
    /* Private equivalents of ddat.v1e0288 and eventqueue.event_heroidx.
     * SELECT_CHAMPION writes v1e0288 after every real mirror click, and only
     * the first click calls SELECT_CHAMPION_LEADER(0).  They are kept here
     * because the UI/event queue is not yet a session owner. */
    int16_t source_next_champion_number;
    int16_t source_event_hero_index;
    DM2_V1_GameLoadChampionSelectionReceipt champion_selection_receipt;
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
 * PIT/TELEPORTER may enter the owned DB2-only FLOOR subset only for a close
 * transition. Opening invokes DM2_ADVANCE_TILES_TIME, whose party/creature
 * moves remain outside this owner. Every other mutating class remains blocked
 * until its complete source chain and follow-up timers share one owner. */
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
    /* Only the DB2-only FLOOR atom below can commit.  The count and hash
     * describe mutable bytes in this private owner, never a hint payload or
     * a live HUD side effect. */
    int text_records_seen;
    int text_records_toggled;
    int blocked_non_text_chain;
    int blocked_hint_delivery;
    int blocked_unowned_tile_advance;
    int pit_tele_tile_mutated;
    uint8_t tile_state_before;
    uint8_t tile_state_after;
    uint32_t private_text_visibility_hash;
} DM2_V1_GameLoadActuateReceipt;

/* Private result of one real 0x01 DM2_STEP_DOOR timer.  A step remains
 * private until the GAME_LOAD transaction owns party damage, creature
 * collision and the audible GDAT/SND delivery path.  Therefore this atom
 * accepts only an authenticated DB0-first door square without the party or a
 * DB4 creature in its local chain.  It still preserves the original timer's
 * DB0 handle and source direction, and requeues the next real 0x01 record
 * only while an animation frame remains. */
typedef struct {
    int valid;
    int source_noop_destroyed;
    int door_state_mutated;
    int requeued;
    int blocked_party_collision;
    int blocked_creature_collision;
    int blocked_incomplete_chain;
    int map;
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    int16_t door_record_link;
    uint8_t state_before;
    uint8_t state_after;
    uint32_t private_animation_hash;
} DM2_V1_GameLoadDoorStepReceipt;

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

/* Source-ordered New Game equivalent of the DM2_move_2fcf_0b8b call at the
 * tail of DM2_GAME_LOAD.  It is legal only after the full generator scan and
 * derives map, position and direction from authenticated DUNGEON.DAT header
 * fields.  It does not publish a party, viewport or M11 map. */
int dm2_v1_game_load_world_owner_materialize_source_map_context(
    DM2_V1_GameLoadWorldOwner *owner);

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
 * performs a successful action only for source class 3 (the original no-op)
 * and the fully-owned DB2-only FLOOR subset.  The latter implements the
 * exact Text::TextVisibility update from ACTUATE_FLOOR_MECHA, but rejects a
 * mixed record chain and a newly-visible party-square hint until the message
 * delivery owner exists. Classes 2 and 5 can also close their source tile
 * before that same DB2 atom; their opening path stays blocked because it
 * requires DM2_ADVANCE_TILES_TIME. Classes 0, 4 and 6 remain rejected without
 * mutation rather than applying an incomplete wall/door/trick-wall fragment.
 */
int dm2_v1_game_load_world_owner_dispatch_actuate_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadActuateReceipt *out_receipt);

/* Consume one private type-0x01 DM2_STEP_DOOR timer.  The timer must carry
 * the direct DB0 door handle in wvalueB, an original direction in actor, and
 * its actual File_header coordinate/map.  No coordinate-only door is ever
 * accepted.  Source: SKProject SKULLWIN/c_tim_proc.cpp::DM2_STEP_DOOR
 * (line 127+) and ::DM2_ACTUATE_DOOR (line 3744). */
int dm2_v1_game_load_world_owner_process_door_step_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadDoorStepReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
