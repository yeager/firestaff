/* Source-owned File_header world materialisation for DM2 New Game. */

#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_data_tables_pc34_compat.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static int dm2_v1_game_load_owner_prepare_timer_capacity(
    DM2_V1_GameLoadWorldOwner *owner)
{
    int type;
    int capacity = 50; /* c_savegame.cpp:395, fresh GAME_LOAD. */

    if (!owner) return 0;
    for (type = 0; type < DM2_V1_RECORD_POOL_COUNT; ++type) {
        const int source_count = owner->dungeon.thing_type_counts[type];
        const int hard_limit = type == 15 ? 0x300 : 0x400;
        int db_capacity;

        if (source_count < 0 || source_count > hard_limit ||
            dm2_v1_record_pool_record_size(type) == 0) {
            /* DB11..DB13 are deliberately unallocated in c_record. */
            if (source_count != 0) return 0;
            owner->record_capacities[type] = 0u;
            continue;
        }
        db_capacity = (int)dm2_v1_table_1d281c[type] + source_count;
        if (db_capacity > hard_limit) db_capacity = hard_limit;
        if (db_capacity < source_count || db_capacity > UINT16_MAX) return 0;
        owner->record_capacities[type] = (uint16_t)db_capacity;
        if (type == 4 || type >= 14) {
            if (capacity > INT16_MAX - db_capacity) return 0;
            capacity += db_capacity;
        }
    }
    if (capacity <= 0 || capacity > INT16_MAX) return 0;
    owner->timer_entries = (DM2_V1_TimerEntry *)calloc((size_t)capacity,
        sizeof(*owner->timer_entries));
    owner->timer_indices = (int16_t *)calloc((size_t)capacity,
        sizeof(*owner->timer_indices));
    if (!owner->timer_entries || !owner->timer_indices) return 0;
    dm2_v1_timer_queue_init(&owner->timer_queue, owner->timer_entries,
                            owner->timer_indices, (int16_t)capacity);
    owner->timer_capacity = (uint16_t)capacity;
    owner->fresh_game_mode = 1;
    return 1;
}

static int dm2_v1_game_load_owner_validate_possessions(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    int i;

    if (!owner) return 0;
    for (i = 0; i < owner->transaction.possessions.placed_item_count; ++i) {
        const DM2_V1_BootNewGamePossession *possession =
            &owner->transaction.possessions.possessions[i];
        if (possession->source_object_id == 0u ||
            possession->equipped_record_id !=
                (uint16_t)(possession->source_object_id & 0x3fffu) ||
            dm2_v1_record_pool_address(&owner->record_pools,
                (int16_t)possession->source_object_id) == NULL) {
            return 0;
        }
    }
    return 1;
}

static int dm2_v1_game_load_owner_validate_selected_party(
    const DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_Party *party)
{
    int hero_index;

    if (!owner || !party || party->heros_in_party <= 0 ||
        party->heros_in_party != owner->transaction.hero_count ||
        party->heros_in_party > DM2_MAX_HEROES) return 0;
    for (hero_index = 0; hero_index < party->heros_in_party; ++hero_index) {
        const DM2_V1_BootNewGameChampionAdmissionReceipt *admission =
            &owner->transaction.party.admissions[hero_index];
        int slot;
        if (!admission->valid || !admission->incomplete_game_load ||
            party->hero[hero_index].herotype !=
                (int8_t)admission->selection.revive_data.hero_type ||
            party->hero[hero_index].partypos < 0 ||
            party->hero[hero_index].partypos > 3) return 0;
        for (slot = 0; slot < DM2_NUM_ITEMS; ++slot) {
            const int16_t item = party->hero[hero_index].item[slot];
            int possession_index;
            int found = item == DM2_V1_RECORD_HANDLE_NULL;
            if (item == DM2_V1_RECORD_HANDLE_NULL) continue;
            if (!dm2_v1_record_pool_address(&owner->record_pools, item)) return 0;
            for (possession_index = 0;
                 possession_index < owner->transaction.possessions.placed_item_count;
                 ++possession_index) {
                const DM2_V1_BootNewGamePossession *possession =
                    &owner->transaction.possessions.possessions[possession_index];
                if (possession->hero_index == (uint8_t)hero_index &&
                    possession->equipped_record_id == (uint16_t)item) {
                    found = 1;
                    break;
                }
            }
            if (!found) return 0;
        }
    }
    for (hero_index = 0;
         hero_index < owner->transaction.possessions.placed_item_count;
         ++hero_index) {
        const DM2_V1_BootNewGamePossession *possession =
            &owner->transaction.possessions.possessions[hero_index];
        int slot;
        int found = 0;
        if (possession->hero_index >= (uint8_t)party->heros_in_party ||
            !dm2_v1_record_pool_address(&owner->record_pools,
                (int16_t)possession->equipped_record_id)) return 0;
        for (slot = 0; slot < DM2_NUM_ITEMS; ++slot) {
            if (party->hero[possession->hero_index].item[slot] ==
                (int16_t)possession->equipped_record_id) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }
    return 1;
}

static uint32_t dm2_v1_game_load_owner_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static int dm2_v1_game_load_owner_tick_multiplier(uint8_t subtype)
{
    switch (subtype) {
    case 0x1e: return 1;
    case 0x33: return 8;
    case 0x34: return 16;
    case 0x35: return 32;
    case 0x36: return 64;
    case 0x37: return 128;
    default: return 0;
    }
}

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner)
{
    if (!owner) return;
    free(owner->timer_indices);
    free(owner->timer_entries);
    dm2_v1_record_pool_set_free(&owner->record_pools);
    dm2_v1_dungeon_free(&owner->dungeon);
    memset(owner, 0, sizeof(*owner));
}

int dm2_v1_game_load_world_owner_init_new_game(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootProfile *profile,
    const DM2_V1_BootNewGamePartySelection *selections,
    int selection_count)
{
    const DM2_V1_DungeonData *source;
    DM2_V1_GameLoadWorldOwner candidate;

    if (!owner) return 0;
    /* Constructor callers pass a fresh/zeroed owner.  Do not inspect a
     * possibly uninitialised caller buffer here; reinitialising an existing
     * owner is explicitly free-then-init so no source RAM owner leaks. */
    memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    if (!profile || !profile->assets_verified || !profile->dungeon_data ||
        !selections || selection_count <= 0 ||
        selection_count > DM2_MAX_HEROES ||
        !dm2_v1_boot_new_game_transaction_receipt(
            profile, selections, selection_count, &candidate.transaction) ||
        !candidate.transaction.valid ||
        !candidate.transaction.incomplete_game_load) {
        return 0;
    }
    source = (const DM2_V1_DungeonData *)profile->dungeon_data;
    if (!source->raw_data || source->raw_size <= 0 ||
        !source->record_graph_complete ||
        candidate.transaction.entrance.map < 0 ||
        candidate.transaction.entrance.map >= source->level_count ||
        dm2_v1_dungeon_load(&candidate.dungeon, source->raw_data,
                            source->raw_size) != 0 ||
        !candidate.dungeon.record_graph_complete ||
        !candidate.dungeon.initial_party_pose_valid ||
        candidate.dungeon.initial_party_x != candidate.transaction.entrance.x ||
        candidate.dungeon.initial_party_y != candidate.transaction.entrance.y ||
        candidate.dungeon.initial_party_dir != candidate.transaction.entrance.direction ||
        !dm2_v1_record_pool_set_init_from_dungeon(&candidate.record_pools,
                                                   &candidate.dungeon) ||
        !candidate.record_pools.valid ||
        !candidate.record_pools.record_graph_complete ||
        !dm2_v1_game_load_owner_prepare_timer_capacity(&candidate)) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }

    candidate.current_map = candidate.transaction.entrance.map;
    candidate.asset_loader = dm2_v1_boot_asset_loader(profile);
    candidate.source_transaction_hash = candidate.transaction.transaction_hash;
    if (!candidate.asset_loader || !candidate.asset_loader->loaded ||
        candidate.source_transaction_hash == 0u ||
        !dm2_v1_game_load_owner_validate_possessions(&candidate)) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    candidate.prepared = 1;
    candidate.committed = 0;
    *owner = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_champion_selection(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_Party candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        owner->champion_selection_materialized || !owner->fresh_game_mode ||
        owner->committed) return 0;

    /* SKProject c_savegame.cpp::DM2_GAME_LOAD reaches SELECT_CHAMPION only
     * after DM2_PROCESS_ACTUATOR_TICK_GENERATOR. The transaction has replayed
     * c_hero.cpp's click order and RNG against authenticated GDAT and map
     * links. This creates a private c_party owner, never an M11 party. */
    candidate = owner->transaction.possessions.projected_party;
    if (!dm2_v1_game_load_owner_validate_possessions(owner) ||
        !dm2_v1_game_load_owner_validate_selected_party(owner, &candidate) ||
        !dm2_v1_new_game_apply_source_item_bonuses(&candidate,
            &owner->record_pools, owner->asset_loader,
            &owner->champion_item_bonus) ||
        !owner->champion_item_bonus.valid ||
        owner->champion_item_bonus.blocked) {
        return 0;
    }
    owner->selected_party = candidate;
    owner->champion_selection_materialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    return owner != NULL && owner->prepared && !owner->committed &&
        owner->dungeon.raw_data != NULL && owner->dungeon.raw_size > 0 &&
        owner->record_pools.valid && owner->record_pools.record_graph_complete &&
        owner->source_transaction_hash != 0u;
}

int dm2_v1_game_load_world_owner_process_actuator_tick_generators(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadActuatorGeneratorReceipt *out_receipt)
{
    DM2_V1_GameLoadActuatorGeneratorReceipt receipt;
    DM2_V1_RecordPool *db3;
    uint8_t *db3_backup = NULL;
    uint8_t *db3_extension_backup = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    int max_chain_steps = 0;
    int map;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->fresh_game_mode || owner->committed) return 0;
    db3 = &owner->record_pools.pools[3];
    if (!db3->bytes || db3->record_size != 8 || db3->record_count <= 0 ||
        (db3->extension_count > 0 && !db3->extension_bytes) ||
        owner->timer_capacity == 0u || !owner->timer_entries ||
        !owner->timer_indices) return 0;

    /* c_tim_proc.cpp:4396-4510 mutates DB3 and queues c_tim records as one
     * GAME_LOAD phase. Retain exact owner-state snapshots so a bounded host
     * allocation failure cannot leave a half-started original world. */
    db3_backup = (uint8_t *)malloc((size_t)db3->record_count * 8u);
    if (db3->extension_count > 0) {
        db3_extension_backup = (uint8_t *)malloc(
            (size_t)db3->extension_count * 8u);
    }
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)owner->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)owner->timer_capacity * sizeof(*index_backup));
    if (!db3_backup || (db3->extension_count > 0 && !db3_extension_backup) ||
        !timer_backup || !index_backup) goto fail;
    memcpy(db3_backup, db3->bytes, (size_t)db3->record_count * 8u);
    if (db3_extension_backup) {
        memcpy(db3_extension_backup, db3->extension_bytes,
               (size_t)db3->extension_count * 8u);
    }
    memcpy(timer_backup, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*index_backup));
    queue_backup = owner->timer_queue;

    for (map = 0; map < DM2_V1_RECORD_POOL_COUNT; ++map) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[map];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            max_chain_steps > INT_MAX - pool->record_count ||
            max_chain_steps + pool->record_count >
                INT_MAX - pool->extension_count) goto fail;
        max_chain_steps += pool->record_count + pool->extension_count;
    }
    if (max_chain_steps <= 0) goto fail;

    receipt.receipt_hash = 0x41475447u; /* "AGTG" */
    for (map = 0; map < owner->dungeon.level_count; ++map) {
        int x;
        const int width = owner->dungeon.level_widths[map];
        const int height = owner->dungeon.level_heights[map];
        if (width <= 0 || height <= 0) goto fail;
        for (x = 0; x < width; ++x) {
            int y;
            for (y = 0; y < height; ++y) {
                const int raw_link = dm2_v1_dungeon_get_first_thing(
                    &owner->dungeon, map, x, y);
                int16_t link;
                int steps = 0;
                /* The File_header accessor uses -1 for byte-squares without
                 * the tile-record flag. c_map walks only the corresponding
                 * ground-stack roots, so there is no chain to follow here. */
                if (raw_link < 0) continue;
                if ((uint16_t)raw_link == DM2_THING_NULL_MARKER) goto fail;
                link = (int16_t)(uint16_t)raw_link;
                /* Dungeon roots carry 0xfffe, while c_record returns the
                 * same terminal link as signed int16_t -2. Keep the walk in
                 * source link representation so it cannot chase -2 as DB15.
                 * SKProject c_record.cpp::DM2_GET_NEXT_RECORD_LINK. */
                while (link != DM2_V1_RECORD_HANDLE_END) {
                    int16_t next;
                    const int pool = dm2_v1_record_handle_pool(link);
                    if (++steps > max_chain_steps ||
                        !dm2_v1_record_pool_next_link(&owner->record_pools,
                            link, &next)) goto fail;
                    if (pool == 3) {
                        uint8_t *record = dm2_v1_record_pool_address_mut(
                            &owner->record_pools, link);
                        uint16_t attributes;
                        uint8_t subtype;
                        int multiplier;
                        if (!record) goto fail;
                        attributes = (uint16_t)record[2] |
                            ((uint16_t)record[3] << 8);
                        subtype = (uint8_t)(attributes & 0x7fu);
                        multiplier = dm2_v1_game_load_owner_tick_multiplier(subtype);
                        if (multiplier != 0) {
                            ++receipt.candidate_count;
                            receipt.receipt_hash = dm2_v1_game_load_owner_hash_step(
                                receipt.receipt_hash, (uint32_t)map);
                            receipt.receipt_hash = dm2_v1_game_load_owner_hash_step(
                                receipt.receipt_hash, (uint16_t)link);
                            receipt.receipt_hash = dm2_v1_game_load_owner_hash_step(
                                receipt.receipt_hash, attributes);
                            if ((record[4] & 0x04u) == 0u) {
                                record[4] &= (uint8_t)~0x01u;
                                ++receipt.control_bit2_clear_count;
                            } else {
                                const uint16_t period = attributes >> 7;
                                ++receipt.activation_count;
                                owner->current_map = map;
                                if (period != 0u) {
                                    DM2_V1_TimerEntry timer;
                                    const uint32_t cadence =
                                        (uint32_t)period * (uint32_t)multiplier;
                                    if (cadence == 0u) goto fail;
                                    dm2_v1_timer_entry_init(&timer);
                                    dm2_v1_timer_set_mticks(&timer, (int16_t)map,
                                        owner->timer_queue.gametick +
                                        owner->timer_queue.gametick % cadence);
                                    timer.ttype = 0x56u;
                                    timer.actor = 0u;
                                    timer.xA = (int8_t)((uint16_t)link & 0xffu);
                                    timer.yA = (int8_t)(((uint16_t)link >> 8) & 0xffu);
                                    timer.wvalueB = (int16_t)(uint8_t)multiplier;
                                    if (dm2_v1_timer_queue(&owner->timer_queue,
                                        &timer) < 0) goto fail;
                                    record[4] |= 0x01u;
                                    ++receipt.queued_timer_count;
                                }
                            }
                        }
                    }
                    if (next == DM2_V1_RECORD_HANDLE_NULL) goto fail;
                    link = next;
                }
            }
        }
    }
    receipt.valid = receipt.receipt_hash != 0u;
    free(index_backup);
    free(timer_backup);
    free(db3_extension_backup);
    free(db3_backup);
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;

fail:
    if (db3_backup) memcpy(db3->bytes, db3_backup,
        (size_t)db3->record_count * 8u);
    if (db3_extension_backup) memcpy(db3->extension_bytes,
        db3_extension_backup, (size_t)db3->extension_count * 8u);
    if (timer_backup) memcpy(owner->timer_entries, timer_backup,
        (size_t)owner->timer_capacity * sizeof(*timer_backup));
    if (index_backup) memcpy(owner->timer_indices, index_backup,
        (size_t)owner->timer_capacity * sizeof(*index_backup));
    if (timer_backup && index_backup) owner->timer_queue = queue_backup;
    free(index_backup);
    free(timer_backup);
    free(db3_extension_backup);
    free(db3_backup);
    return 0;
}

int dm2_v1_game_load_world_owner_continue_tick_generator(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadTickGeneratorReceipt *out_receipt)
{
    DM2_V1_GameLoadTickGeneratorReceipt receipt;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    uint8_t *record;
    uint8_t record_byte4;
    const int previous_map = owner ? owner->current_map : 0;
    const int map = timer ? dm2_v1_timer_get_map(timer) : -1;
    const int16_t record_link = timer ? (int16_t)((uint16_t)(uint8_t)timer->xA |
        ((uint16_t)(uint8_t)timer->yA << 8)) : DM2_V1_RECORD_HANDLE_NULL;
    uint16_t w2;
    uint16_t w4;
    uint16_t w6;
    int control_bit2;
    int alternating;
    int remaining;
    int should_invoke;
    uint8_t action;
    uint8_t alternating_toggle = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) || !timer ||
        timer->ttype != 0x56u || map < 0 || map >= owner->dungeon.level_count ||
        dm2_v1_record_handle_pool(record_link) != 3 ||
        owner->timer_capacity == 0u || !owner->timer_entries ||
        !owner->timer_indices) {
        return 0;
    }
    record = dm2_v1_record_pool_address_mut(&owner->record_pools, record_link);
    if (!record) return 0;
    record_byte4 = record[4];

    /* c_tim_proc.cpp:995-1066 schedules an ACTUATE message before it places
     * the next 0x56 instance in c_tim. Snapshot the complete private heap so
     * a capacity failure cannot publish only one half of that source step. */
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)owner->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)owner->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto fail;
    memcpy(timer_backup, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*index_backup));
    queue_backup = owner->timer_queue;
    owner->current_map = map; /* PROCEED_TIMERS changes map before dispatch. */

    w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
    w6 = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
    control_bit2 = (w4 & 0x0004u) != 0u;
    alternating = (w4 & 0x0018u) == 0x0018u;
    if (alternating) {
        alternating_toggle = (uint8_t)(((uint16_t)timer->wvalueB >> 8) & 1u) ^ 1u;
        action = alternating_toggle == 0u ? 1u : 0u;
        remaining = control_bit2 || alternating_toggle != 0u;
        should_invoke = 1;
    } else {
        action = (uint8_t)((w4 >> 3) & 3u);
        remaining = control_bit2;
        should_invoke = control_bit2;
    }

    receipt.record_link = record_link;
    receipt.action = action;
    receipt.target_x = (uint8_t)(w6 & 0x001fu);
    receipt.target_y = (uint8_t)((w6 >> 5) & 0x001fu);
    receipt.target_direction = (uint8_t)((w6 >> 10) & 3u);
    if (should_invoke) {
        DM2_V1_TimerEntry message;
        dm2_v1_timer_entry_init(&message);
        dm2_v1_timer_set_mticks(&message, (int16_t)map,
            owner->timer_queue.gametick + ((w4 >> 7) & 0x007fu));
        message.ttype = 0x04u;
        if (action == 0u) message.actor = 1u;
        else if (action == 1u) message.actor = 3u;
        else if (action == 2u) message.actor = 2u;
        message.xA = (int8_t)receipt.target_x;
        message.yA = (int8_t)receipt.target_y;
        message.wvalueB = (int16_t)((uint16_t)receipt.target_direction |
                                     ((uint16_t)action << 8));
        if (dm2_v1_timer_queue(&owner->timer_queue, &message) < 0) goto fail;
        receipt.actuator_invoked = 1;
        receipt.message_queued = 1;
    }

    if (remaining) {
        const int multiplier = dm2_v1_game_load_owner_tick_multiplier(
            (uint8_t)(w2 & 0x007fu));
        const uint16_t period = w2 >> 7;
        DM2_V1_TimerEntry continuation;
        if (multiplier == 0 || period == 0u ||
            (uint8_t)timer->wvalueB != (uint8_t)multiplier) goto fail;
        continuation = *timer;
        if (alternating) {
            continuation.wvalueB = (int16_t)((uint16_t)(uint8_t)timer->wvalueB |
                ((uint16_t)alternating_toggle << 8));
        }
        continuation.l_00 += (int32_t)((uint32_t)period * (uint32_t)multiplier);
        if (dm2_v1_timer_queue(&owner->timer_queue, &continuation) < 0) goto fail;
        receipt.requeued = 1;
    } else {
        record[4] &= (uint8_t)~0x01u;
        receipt.active_flag_cleared = 1;
    }

    receipt.valid = 1;
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

fail:
    if (timer_backup) memcpy(owner->timer_entries, timer_backup,
        (size_t)owner->timer_capacity * sizeof(*timer_backup));
    if (index_backup) memcpy(owner->timer_indices, index_backup,
        (size_t)owner->timer_capacity * sizeof(*index_backup));
    if (timer_backup && index_backup) owner->timer_queue = queue_backup;
    if (record) record[4] = record_byte4;
    owner->current_map = previous_map;
    free(index_backup);
    free(timer_backup);
    return 0;
}
