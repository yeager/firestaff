/* Source-owned File_header world materialisation for DM2 New Game. */

#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_data_tables_pc34_compat.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_skproject_core.h"

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

    if (!owner || !owner->transaction.possessions.valid ||
        !owner->transaction.possessions.incomplete_game_load ||
        owner->transaction.possessions.source_item_count < 0 ||
        owner->transaction.possessions.placed_item_count < 0 ||
        owner->transaction.possessions.unplaced_item_count != 0 ||
        owner->transaction.possessions.source_item_count !=
            owner->transaction.possessions.placed_item_count ||
        owner->transaction.possessions.placed_item_count >
            DM2_V1_BOOT_MAX_NEW_GAME_POSSESSIONS) {
        return 0;
    }
    for (i = 0; i < owner->transaction.possessions.placed_item_count; ++i) {
        const DM2_V1_BootNewGamePossession *possession =
            &owner->transaction.possessions.possessions[i];
        int16_t source_next;
        int j;

        if (possession->source_object_id == 0u ||
            possession->source_object_id == 0xfffeu ||
            possession->source_object_id == 0xffffu ||
            possession->record_type !=
                (uint8_t)((possession->source_object_id >> 10) & 15u) ||
            possession->equipped_record_id !=
                (uint16_t)(possession->source_object_id & 0x3fffu) ||
            dm2_v1_record_pool_address(&owner->record_pools,
                (int16_t)possession->source_object_id) == NULL ||
            !dm2_v1_record_pool_next_link(&owner->record_pools,
                (int16_t)possession->source_object_id, &source_next) ||
            (uint16_t)source_next != possession->source_next_object_id) {
            return 0;
        }
        /* DM2_SELECT_CHAMPION walks each mirror tile chain once.  Two
         * private inventory slots must never claim the same File_header
         * record merely because its orientation-free handle fits both.
         * Source: SKProject SKULLWIN/c_hero.cpp::DM2_SELECT_CHAMPION
         * (1139-1157), ::DM2_ADD_ITEM_TO_PLAYER (2188-2244). */
        for (j = 0; j < i; ++j) {
            if (owner->transaction.possessions.possessions[j]
                    .source_object_id == possession->source_object_id) {
                return 0;
            }
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

/* DM2_LOAD_NEW_DUNGEON clears party.heros_in_party, stores -1 in
 * savegamewpc.w_00 and resets savegamel1 before it reads File_header data.
 * Keep those exact values under the same private owner that later clones the
 * authenticated dungeon. No call here opens media, writes a save, or
 * manufactures an empty runtime party. */
static int dm2_v1_game_load_owner_materialize_new_dungeon_reset(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadNewDungeonResetReceipt candidate;

    if (!owner || !owner->transaction.valid ||
        !owner->transaction.incomplete_game_load ||
        !owner->dungeon.raw_data || owner->dungeon.raw_size <= 0 ||
        !owner->record_pools.valid || !owner->record_pools.record_graph_complete) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.party_count = 0;
    candidate.leader_hand_record = DM2_V1_RECORD_HANDLE_NULL;
    candidate.save_stream_bytes_consumed = 0u;
    candidate.receipt_hash = 0x4c4e4452u; /* "LNDR" */
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, owner->transaction.transaction_hash);
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, (uint16_t)candidate.party_count);
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, (uint16_t)candidate.leader_hand_record);
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, candidate.save_stream_bytes_consumed);
    candidate.valid = candidate.receipt_hash != 0u;
    if (!candidate.valid) return 0;
    owner->load_new_dungeon_reset = candidate;
    return 1;
}

static int dm2_v1_game_load_owner_materialize_dyn4(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_BootChampionSelectionCensus census;
    DM2_V1_GdatDyn4SoundState sound_state;
    uint32_t hash = 2166136261u;
    int i;

    if (!owner || !owner->asset_loader || !owner->transaction.dyn4_roster.valid ||
        !owner->transaction.dyn4_roster.incomplete_champion_activation ||
        owner->transaction.dyn4_roster.selector_count <= 0 ||
        owner->transaction.dyn4_roster.selector_count >
            DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES) return 0;
    memset(&census, 0, sizeof(census));
    if (!dm2_v1_boot_champion_selection_census(
            owner->boot_profile, &census) || !census.valid ||
        census.candidate_count != owner->transaction.dyn4_roster.selector_count) {
        return 0;
    }
    dm2_v1_gdat_dyn4_sound_state_init(&sound_state);
    for (i = 0; i < owner->transaction.dyn4_roster.selector_count; ++i) {
        const DM2_V1_GdatDyn4SelectionReceipt *source =
            &owner->transaction.dyn4_roster.selections[i];
        DM2_V1_GdatDyn4SelectionReceipt recounted;
        DM2_V1_GdatDyn4MaterializedSelection *selection =
            &owner->dyn4_selections[i];
        const uint32_t resource_id = census.candidates[i].mirror.dynamic_load_id;
        if (!source->valid || source->rejected_raw_count != 0u ||
            source->raw_loadable_entry_count == 0u ||
            resource_id == 0u ||
            !dm2_v1_gdat_dyn4_selection_receipt(owner->asset_loader,
                resource_id, &recounted) ||
            recounted.receipt_hash != source->receipt_hash ||
            !dm2_v1_gdat_dyn4_materialize_selection(owner->asset_loader,
                resource_id, &sound_state, selection) || !selection->valid ||
            selection->receipt_hash == 0u || selection->block_count == 0u) {
            return 0;
        }
        hash = dm2_v1_game_load_owner_hash_step(hash, resource_id);
        hash = dm2_v1_game_load_owner_hash_step(hash, selection->receipt_hash);
        owner->dyn4_selector_ids[i] = resource_id;
    }
    owner->dyn4_selector_count = (uint16_t)owner->transaction.dyn4_roster.selector_count;
    owner->dyn4_materialized_hash = hash;
    owner->dyn4_materialized = hash != 0u;
    return owner->dyn4_materialized;
}

static uint32_t dm2_v1_game_load_owner_hash_bytes(const uint8_t *bytes,
                                                   size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || byte_count == 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int dm2_v1_game_load_owner_dyn4_has_raw(
    const DM2_V1_GameLoadWorldOwner *owner, uint16_t raw_index)
{
    uint16_t selector;

    if (!owner || !owner->dyn4_materialized) return 0;
    for (selector = 0u; selector < owner->dyn4_selector_count; ++selector) {
        const DM2_V1_GdatDyn4MaterializedSelection *selection =
            &owner->dyn4_selections[selector];
        uint16_t block;
        if (!selection->valid || !selection->raw_indices) return 0;
        for (block = 0u; block < selection->block_count; ++block) {
            if (selection->raw_indices[block] == raw_index) return 1;
        }
    }
    return 0;
}

static int dm2_v1_game_load_owner_dyn4_matches_selector(
    const DM2_V1_GdatEntry *entry, uint32_t selector)
{
    const uint8_t category = (uint8_t)(selector >> 24);
    const uint8_t index = (uint8_t)(selector >> 16);
    const uint8_t type = (uint8_t)(selector >> 8);
    const uint8_t field = (uint8_t)selector;

    if (!entry) return 0;
    return (category == 0xffu || entry->cls1 == category) &&
           (index == 0xffu || entry->cls2 == index) &&
           (type == 0xffu || entry->cls3 == type) &&
           (field == 0xffu || entry->cls4 == field);
}

static void dm2_v1_game_load_owner_sound_free(
    DM2_V1_GameLoadSoundOwner *sound)
{
    if (!sound) return;
    free(sound->queue_entries);
    free(sound->sample_bindings);
    memset(sound, 0, sizeof(*sound));
}

/* Private c_dballoc/c_sound/c_gdatfile hand-off.  The capacity is the real
 * DM2_dballoc_3e74_24b8 census (292 rows in the admitted PC corpus), so it
 * must not be squeezed into Firestaff's old 64-entry gameplay queue.  Only
 * raw blocks already copied by the source DYN4 selectors are bound.  No PCM
 * header conversion, mixer state, global runtime binding or cue is created.
 *
 * Source: SKProject SKULLWIN/c_gdatfile.cpp::
 * DM2_dballoc_3e74_24b8 (273-333), DM2_LOAD_DYN4 (1294-1510),
 * DM2_482b_0684 (932-975); c_sound.cpp::DM2_SOUND9 (650-662).
 */
static int dm2_v1_game_load_owner_materialize_sound(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadSoundOwner candidate;
    const DM2_V1_AssetLoader *loader;
    uint16_t selector;

    if (!owner || !(loader = owner->asset_loader) || !owner->dyn4_materialized ||
        owner->dyn4_selector_count == 0u) return 0;
    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_dballoc_3e74_24b8_receipt(loader, &candidate.allocation) ||
        !candidate.allocation.accepted ||
        candidate.allocation.sound_entry_count == 0u ||
        candidate.allocation.unique_raw_index_count == 0u) {
        return 0;
    }
    candidate.queue_capacity = candidate.allocation.sound_entry_count;
    candidate.sample_capacity = candidate.allocation.unique_raw_index_count;
    candidate.queue_entries = calloc(candidate.queue_capacity,
                                     sizeof(*candidate.queue_entries));
    candidate.sample_bindings = calloc(candidate.sample_capacity,
                                       sizeof(*candidate.sample_bindings));
    if (!candidate.queue_entries || !candidate.sample_bindings) goto fail;

    /* This is the second DM2_LOAD_DYN4 descriptor pass.  It calls SOUND9 in
     * GDAT table order and permits only rows whose raw block was marked and
     * materialised by one of the real selectors. */
    for (selector = 0u; selector < owner->dyn4_selector_count; ++selector) {
        uint16_t ordinal;
        const uint32_t selector_id = owner->dyn4_selector_ids[selector];
        if (selector_id == 0u) goto fail;
        for (ordinal = 0u; ordinal < loader->entry_count; ++ordinal) {
            const DM2_V1_GdatEntry *entry = &loader->entries[ordinal];
            uint16_t raw_index;
            uint16_t existing;

            if (entry->cls3 != DM2_GDAT_ENTRY_TYPE_SOUND ||
                !dm2_v1_game_load_owner_dyn4_matches_selector(entry,
                                                               selector_id) ||
                (entry->data_index & 0x8000u) != 0u) {
                continue;
            }
            raw_index = entry->data_index;
            if (!dm2_v1_game_load_owner_dyn4_has_raw(owner, raw_index)) {
                continue;
            }
            for (existing = 0u; existing < candidate.queue_entry_count;
                 ++existing) {
                const DM2_V1_SoundSsoundEntry *prior =
                    &candidate.queue_entries[existing];
                if (prior->b_02 == (int8_t)entry->cls1 &&
                    prior->b_03 == (int8_t)entry->cls2 &&
                    prior->b_04 == (int8_t)entry->cls4) {
                    break;
                }
            }
            if (existing != candidate.queue_entry_count) continue;
            if (candidate.queue_entry_count >= candidate.queue_capacity)
                goto fail;
            candidate.queue_entries[candidate.queue_entry_count].b_02 =
                (int8_t)entry->cls1;
            candidate.queue_entries[candidate.queue_entry_count].b_03 =
                (int8_t)entry->cls2;
            candidate.queue_entries[candidate.queue_entry_count].b_04 =
                (int8_t)entry->cls4;
            candidate.queue_entries[candidate.queue_entry_count].w_00 = -1;
            candidate.queue_entries[candidate.queue_entry_count].w_05 = -1;
            ++candidate.queue_entry_count;
        }
    }
    if (candidate.queue_entry_count == 0u) goto fail;

    /* DM2_482b_0684: resolve each SOUND9 row through the same GDAT table,
     * then share or allocate a source sample slot by its raw index. */
    for (selector = 0u; selector < candidate.queue_entry_count; ++selector) {
        DM2_V1_SoundSsoundEntry *entry = &candidate.queue_entries[selector];
        DM2_V1_GdatSoundEntryReceipt source;
        uint16_t binding;
        const uint8_t *raw;
        size_t raw_size = 0u;

        memset(&source, 0, sizeof(source));
        if (!dm2_v1_gdat_sound_entry_receipt(loader, (uint8_t)entry->b_02,
                (uint8_t)entry->b_03, (uint8_t)entry->b_04, 0, 0,
                &source) || !source.accepted ||
            !dm2_v1_game_load_owner_dyn4_has_raw(owner, source.raw_index)) {
            goto fail;
        }
        for (binding = 0u; binding < candidate.sample_binding_count;
             ++binding) {
            if (candidate.sample_bindings[binding].raw_index == source.raw_index)
                break;
        }
        if (binding == candidate.sample_binding_count) {
            if (binding >= candidate.sample_capacity ||
                !(raw = dm2_v1_load_gdat_raw_data(loader, source.raw_index,
                                                   &raw_size)) ||
                raw_size != source.raw_length || raw_size > UINT16_MAX) {
                goto fail;
            }
            candidate.sample_bindings[binding].raw_index = source.raw_index;
            candidate.sample_bindings[binding].raw_length = (uint16_t)raw_size;
            candidate.sample_bindings[binding].source_payload_hash =
                dm2_v1_game_load_owner_hash_bytes(raw, raw_size);
            if (candidate.sample_bindings[binding].source_payload_hash == 0u)
                goto fail;
            candidate.materialized_raw_hash = dm2_v1_game_load_owner_hash_step(
                candidate.materialized_raw_hash ? candidate.materialized_raw_hash :
                    2166136261u, source.raw_index);
            candidate.materialized_raw_hash = dm2_v1_game_load_owner_hash_step(
                candidate.materialized_raw_hash,
                candidate.sample_bindings[binding].source_payload_hash);
            ++candidate.sample_binding_count;
        }
        entry->w_05 = (int16_t)source.raw_index;
        entry->w_00 = (int16_t)binding;
    }
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        0x474c534fu, candidate.allocation.receipt_hash); /* "GLSO" */
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, candidate.queue_entry_count);
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, candidate.sample_binding_count);
    candidate.receipt_hash = dm2_v1_game_load_owner_hash_step(
        candidate.receipt_hash, candidate.materialized_raw_hash);
    candidate.valid = candidate.receipt_hash != 0u;
    if (!candidate.valid) goto fail;
    owner->sound_owner = candidate;
    return 1;

fail:
    dm2_v1_game_load_owner_sound_free(&candidate);
    return 0;
}

static int dm2_v1_game_load_owner_validate_world_maps(
    DM2_V1_GameLoadWorldOwner *owner)
{
    uint32_t hash = 2166136261u;
    int total_records = 0;
    int map;

    if (!owner || !owner->transaction.world_interactions.valid ||
        !owner->transaction.world_interactions.incomplete_world ||
        owner->dungeon.level_count != owner->transaction.world_interactions.map_count)
        return 0;
    for (map = 0; map < owner->dungeon.level_count; ++map) {
        DM2_V1_FileHeaderRuntimeMapReceipt receipt;
        memset(&receipt, 0, sizeof(receipt));
        if (!dm2_v1_dungeon_validate_file_header_runtime_map(
                &owner->dungeon, map, &receipt) || !receipt.committed ||
            !receipt.incomplete_world || receipt.map != map ||
            receipt.record_count < 0 || total_records > INT_MAX - receipt.record_count) {
            return 0;
        }
        total_records += receipt.record_count;
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)map);
        hash = dm2_v1_game_load_owner_hash_step(hash, receipt.map_data_hash);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)receipt.record_count);
    }
    if (total_records != owner->transaction.world_interactions.total_records)
        return 0;
    owner->validated_map_count = (uint16_t)owner->dungeon.level_count;
    owner->validated_world_hash = hash;
    return hash != 0u;
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

/* Resolve exactly one DM2_GET_TELEPORTER_DETAIL probe against the private
 * File_header world.  c_querydb first uses the origin record to select its
 * destination-map span, then verifies the destination square.  Keeping that
 * selection here avoids treating a filename, a guessed map, or a renderer
 * coordinate as game data.
 *
 * Source: SKProject SKWINSPX/src/v5/skgdtqdb.cpp::DM2_GET_TELEPORTER_DETAIL
 * (3113-3165), skmove.cpp::DM2_move_2fcf_0b8b (947-1015).
 */
static int dm2_v1_game_load_owner_get_teleporter_detail(
    const DM2_V1_GameLoadWorldOwner *owner,
    int map,
    int x,
    int y,
    DM2_V1_SkprojectTeleporterDetail *out_detail)
{
    const uint8_t *origin_tiles;
    const uint8_t *destination_tiles;
    const uint8_t *origin_record;
    DM2_V1_SkprojectQuery0cee0897Receipt origin_receipt;
    DM2_V1_SkprojectGetTeleporterDetailReceipt detail_receipt;
    uint16_t first_link;
    uint16_t word4;
    uint8_t source_detail;
    uint8_t destination_map;
    int16_t origin_width;
    int16_t origin_height;
    int16_t destination_width;
    int16_t destination_height;

    if (out_detail) memset(out_detail, 0, sizeof(*out_detail));
    if (!owner || !out_detail || map < 0 || map >= owner->dungeon.level_count)
        return 0;
    origin_tiles = dm2_v1_dungeon_level_tile_data(&owner->dungeon, map,
                                                   &origin_width, &origin_height);
    if (!origin_tiles || x < 0 || y < 0 || x >= origin_width || y >= origin_height)
        return 0;
    memset(&origin_receipt, 0, sizeof(origin_receipt));
    if (!dm2_v1_skproject_query_0cee_0897(
            (int16_t)x, (int16_t)y, origin_tiles, origin_width, origin_height,
            &owner->record_pools, &first_link, &source_detail,
            &origin_receipt) || !origin_receipt.valid || source_detail == 0u ||
        !(origin_record = dm2_v1_record_pool_address(&owner->record_pools,
                                                      (int16_t)first_link))) {
        return 0;
    }
    word4 = (uint16_t)origin_record[4] | ((uint16_t)origin_record[5] << 8);
    destination_map = (uint8_t)(word4 >> 8);
    if (destination_map >= (uint8_t)owner->dungeon.level_count)
        return 0;
    destination_tiles = dm2_v1_dungeon_level_tile_data(
        &owner->dungeon, (int)destination_map, &destination_width,
        &destination_height);
    if (!destination_tiles) return 0;
    memset(&detail_receipt, 0, sizeof(detail_receipt));
    return dm2_v1_skproject_get_teleporter_detail(
        (int16_t)x, (int16_t)y, origin_tiles, origin_width, origin_height,
        &owner->record_pools, (uint8_t)map, destination_tiles,
        destination_width, destination_height, out_detail, &detail_receipt) &&
        detail_receipt.valid && detail_receipt.dest_map == destination_map;
}

static int dm2_v1_game_load_owner_materialize_move_2fcf_0b8b(
    DM2_V1_GameLoadWorldOwner *owner)
{
    static const int8_t direction_x[4] = { 0, 1, 0, -1 };
    static const int8_t direction_y[4] = { -1, 0, 1, 0 };
    DM2_V1_SkprojectTeleporterDetail detail;
    const int map = owner->source_party_map;
    const int x = owner->source_party_x;
    const int y = owner->source_party_y;
    const int direction = owner->source_party_direction;
    int probe_direction;

    if (!owner || map < 0 || map >= owner->dungeon.level_count ||
        direction < 0 || direction > 3) return 0;

    /* DM2_move_2fcf_0b8b begins with v1e027c=-1 and treats a failed direct
     * probe exactly like the four failed adjacent probes. */
    owner->source_staircase_flag = 0;
    owner->source_teleporter_map = -1;
    owner->source_display_x = 0;
    owner->source_display_y = 0;
    owner->source_party_absdir = 0;
    owner->source_display_pose_valid = 0;
    if (dm2_v1_game_load_owner_get_teleporter_detail(owner, map, x, y,
                                                      &detail)) {
        owner->source_staircase_flag = 1;
        owner->source_teleporter_map = detail.b_04;
        owner->source_display_x = detail.b_02;
        owner->source_display_y = detail.b_03;
        owner->source_party_absdir = (uint8_t)((detail.b_01 - detail.b_00 +
                                                 direction) & 3);
        owner->source_display_pose_valid = 1;
    } else {
        for (probe_direction = 0; probe_direction < 4; ++probe_direction) {
            const int probe_x = x + direction_x[probe_direction];
            const int probe_y = y + direction_y[probe_direction];
            int display_direction;
            if (!dm2_v1_game_load_owner_get_teleporter_detail(
                    owner, map, probe_x, probe_y, &detail)) {
                continue;
            }
            /* table1d27fc/table1d2804 and the direction arithmetic are
             * copied from skmove.cpp lines 975-1003. */
            display_direction = (probe_direction + detail.b_01 + 6 -
                                 detail.b_00 + 2) & 3;
            owner->source_staircase_flag = 1;
            owner->source_teleporter_map = detail.b_04;
            owner->source_display_x = (int16_t)(detail.b_02 +
                                                 direction_x[display_direction]);
            owner->source_display_y = (int16_t)(detail.b_03 +
                                                 direction_y[display_direction]);
            owner->source_party_absdir = (uint8_t)((detail.b_04 + direction) & 3);
            owner->source_display_pose_valid = 1;
            break;
        }
    }
    /* v1d3248 is reset after either source path; this is the private analogue
     * only, not an M11 held-item or movement record. */
    owner->source_last_moved_record = -1;
    return 1;
}

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner)
{
    int i;
    if (!owner) return;
    for (i = 0; i < DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES; ++i) {
        dm2_v1_gdat_dyn4_materialized_selection_free(&owner->dyn4_selections[i]);
    }
    dm2_v1_game_load_owner_sound_free(&owner->sound_owner);
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
    candidate.boot_profile = profile;
    candidate.asset_loader = dm2_v1_boot_asset_loader(profile);
    candidate.source_transaction_hash = candidate.transaction.transaction_hash;
    if (!candidate.asset_loader || !candidate.asset_loader->loaded ||
        candidate.source_transaction_hash == 0u ||
        !dm2_v1_game_load_owner_validate_possessions(&candidate) ||
        !dm2_v1_game_load_owner_validate_world_maps(&candidate) ||
        !dm2_v1_game_load_owner_materialize_new_dungeon_reset(&candidate)) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    if (!dm2_v1_game_load_owner_materialize_dyn4(&candidate)) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    if (!dm2_v1_game_load_owner_materialize_sound(&candidate)) {
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
    DM2_V1_GameLoadChampionSelectionReceipt selection_receipt;
    int hero_index;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        owner->champion_selection_materialized || !owner->fresh_game_mode ||
        !owner->source_map_context_materialized || owner->committed ||
        !owner->load_new_dungeon_reset.valid ||
        owner->load_new_dungeon_reset.party_count != 0 ||
        owner->load_new_dungeon_reset.leader_hand_record !=
            DM2_V1_RECORD_HANDLE_NULL ||
        owner->load_new_dungeon_reset.save_stream_bytes_consumed != 0u) return 0;

    /* Mirror events call SELECT_CHAMPION after DM2_GAME_LOAD has returned.
     * This private transaction materialises their already authenticated click
     * order and RNG against GDAT and map links; it does not claim to run the
     * UI event loop or install an M11 party.  Source: SKProject skhero.cpp
     * DM2_SELECT_CHAMPION (1119-1128), DM2_SELECT_CHAMPION_LEADER (2327-2354).
     */
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
    if (candidate.heros_in_party <= 0 ||
        candidate.heros_in_party > DM2_MAX_HEROES ||
        candidate.hero[0].curHP == 0) {
        return 0;
    }

    /* Preserve the individual SELECT_CHAMPION transitions, rather than
     * collapsing their click order into the final party count.  These bytes
     * are all already authenticated by BootNewGamePartyReceipt: no UI event
     * is fabricated here.  The original first click alone selects leader 0;
     * subsequent clicks advance ddat.v1e0288 but leave event_heroidx intact.
     * Source: SKProject SKULLWIN/c_hero.cpp::DM2_SELECT_CHAMPION
     * (1119-1168), ::DM2_SELECT_CHAMPION_LEADER (2325-2354). */
    memset(&selection_receipt, 0, sizeof(selection_receipt));
    selection_receipt.click_count = (uint8_t)candidate.heros_in_party;
    selection_receipt.leader_select_count = 1u;
    selection_receipt.initial_event_hero_index = DM2_HERO_NONE;
    selection_receipt.final_event_hero_index = 0;
    selection_receipt.transition_hash = 0x43534c54u; /* "CSLT" */
    selection_receipt.transition_hash = dm2_v1_game_load_owner_hash_step(
        selection_receipt.transition_hash,
        (uint32_t)(uint16_t)selection_receipt.initial_event_hero_index);
    for (hero_index = 0; hero_index < candidate.heros_in_party; ++hero_index) {
        const DM2_V1_BootNewGameChampionAdmissionReceipt *admission =
            &owner->transaction.party.admissions[hero_index];
        uint16_t mirror_object_id;
        int previous;

        if (!admission->valid || !admission->incomplete_game_load ||
            admission->selection.mirror.object_id == 0u ||
            candidate.hero[hero_index].herotype !=
                (int8_t)admission->selection.revive_data.hero_type ||
            candidate.hero[hero_index].partypos < 0 ||
            candidate.hero[hero_index].partypos > 3) {
            return 0;
        }
        mirror_object_id = admission->selection.mirror.object_id;
        for (previous = 0; previous < hero_index; ++previous) {
            if (selection_receipt.mirror_object_id[previous] == mirror_object_id)
                return 0;
        }
        selection_receipt.next_champion_number_after_click[hero_index] =
            (int16_t)(hero_index + 1);
        selection_receipt.mirror_object_id[hero_index] = mirror_object_id;
        selection_receipt.party_position[hero_index] =
            candidate.hero[hero_index].partypos;
        selection_receipt.transition_hash = dm2_v1_game_load_owner_hash_step(
            selection_receipt.transition_hash, mirror_object_id);
        selection_receipt.transition_hash = dm2_v1_game_load_owner_hash_step(
            selection_receipt.transition_hash,
            (uint32_t)(hero_index + 1));
        selection_receipt.transition_hash = dm2_v1_game_load_owner_hash_step(
            selection_receipt.transition_hash,
            (uint32_t)(uint8_t)candidate.hero[hero_index].partypos);
    }
    selection_receipt.transition_hash = dm2_v1_game_load_owner_hash_step(
        selection_receipt.transition_hash,
        (uint32_t)(uint16_t)selection_receipt.final_event_hero_index);
    if (selection_receipt.transition_hash == 0u) return 0;
    selection_receipt.valid = 1;
    /* The source invokes SELECT_CHAMPION_LEADER only for RG5W == 0, after
     * setting v1e0288 to one.  Replaying it with the final party count would
     * incorrectly set bit 0x1400 on hero 0. */
    dm2_v1_party_select_champion_leader(&candidate, 0, DM2_HERO_NONE, 1);
    if (candidate.curactevhero != 0) {
        return 0;
    }
    /* DM2_GAME_LOAD has already run DM2_move_2fcf_0b8b before a mirror click.
     * Preserve that private party-facing result when c_hero materializes;
     * it does not expose an M11 party or a viewport. */
    candidate.absdir = owner->source_party_absdir;
    owner->selected_party = candidate;
    owner->source_next_champion_number = candidate.heros_in_party;
    owner->source_event_hero_index = 0;
    owner->champion_selection_receipt = selection_receipt;
    owner->champion_selection_materialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_source_map_context(
    DM2_V1_GameLoadWorldOwner *owner)
{
    const int map = owner ? owner->transaction.entrance.map : -1;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->fresh_game_mode || !owner->actuator_generators_processed ||
        owner->source_map_context_materialized || owner->committed ||
        map < 0 || map >= owner->dungeon.level_count ||
        !owner->dungeon.initial_party_pose_valid ||
        owner->dungeon.initial_party_x != owner->transaction.entrance.x ||
        owner->dungeon.initial_party_y != owner->transaction.entrance.y ||
        owner->dungeon.initial_party_dir != owner->transaction.entrance.direction) {
        return 0;
    }

    /* SKProject sksvgame.cpp::DM2_GAME_LOAD (1561-1565) calls
     * DM2_move_2fcf_0b8b only after PROCESS_ACTUATOR_TICK_GENERATOR.
     * DUNGEON_STRUCTURE established v1e0266=0 for New Game and header w8
     * supplies the three pose fields.  These are source bytes, not defaults.
     * Run its teleporter probes against the owned File_header pools as well;
     * their display pose remains private because this owner exposes no view. */
    owner->current_map = map;
    owner->source_party_map = map;
    owner->source_party_x = (uint8_t)owner->dungeon.initial_party_x;
    owner->source_party_y = (uint8_t)owner->dungeon.initial_party_y;
    owner->source_party_direction = (uint8_t)owner->dungeon.initial_party_dir;
    if (!dm2_v1_game_load_owner_materialize_move_2fcf_0b8b(owner)) return 0;
    owner->source_map_context_materialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    return owner != NULL && owner->prepared && !owner->committed &&
        owner->dungeon.raw_data != NULL && owner->dungeon.raw_size > 0 &&
        owner->record_pools.valid && owner->record_pools.record_graph_complete &&
        owner->dyn4_materialized && owner->dyn4_selector_count > 0u &&
        owner->sound_owner.valid && owner->sound_owner.queue_entries != NULL &&
        owner->sound_owner.sample_bindings != NULL &&
        owner->sound_owner.queue_entry_count > 0u &&
        owner->sound_owner.sample_binding_count > 0u &&
        owner->validated_map_count == (uint16_t)owner->dungeon.level_count &&
        owner->validated_world_hash != 0u &&
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
        !owner->fresh_game_mode || owner->actuator_generators_processed ||
        owner->committed) return 0;
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
    if (!receipt.valid) goto fail;
    owner->actuator_generators_processed = 1;
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

/*
 * Source-complete WALL/FLOOR_MECHA atom for PUSH_BUTTON_SWITCH.
 *
 * skevent.cpp::ACTUATE_WALL_MECHA and ::ACTUATE_FLOOR_MECHA walk each record
 * in source order, and their 0x46 arm calls PUSH_BUTTON_SWITCH. That arm reads only the actuator
 * payload and the *first* target-tile record (GET_ADDRESS_OF_TILE_RECORD),
 * then changes Door::w2 bit 13.  It neither moves a record nor needs a
 * party, CAII slot, sound delivery or a follow-up timer.  Thus it is safe to
 * retain inside the cloned File_header owner only when the entire originating
 * chain consists of real DB3 0x46 records and every target is a real direct
 * DB0 door.  All targets are validated before the first write so a broken
 * later target cannot leave a partial source dispatch behind.
 *
 * Source: SKProject SKULLWIN/skevent.cpp::ACTUATE_FLOOR_MECHA (2080-2145),
 * ::PUSH_BUTTON_SWITCH (2010-2028); SKWIN/c_map.cpp::
 * GET_ADDRESS_OF_TILE_RECORD; SKWIN/DME.h::Door.
 *
 * Return 1 for a committed atom, 0 when this is not an all-push-button
 * chain, and -1 for a candidate chain whose complete owner is contradictory.
 */
static int dm2_v1_game_load_owner_dispatch_push_button_floor(
    DM2_V1_GameLoadWorldOwner *owner, int map, int x, int y,
    uint8_t action, DM2_V1_GameLoadActuateReceipt *receipt)
{
    int chain_limit = 0;
    int16_t link;
    int16_t *targets = NULL;
    uint8_t *next_bits = NULL;
    int count = 0;
    uint32_t hash = 0x50425357u; /* "PBSW" */
    int result = 0;

    if (!owner || !receipt || !owner->source_map_context_materialized ||
        action > 2u) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon,
                                                    map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    targets = (int16_t *)calloc((size_t)chain_limit, sizeof(*targets));
    next_bits = (uint8_t *)calloc((size_t)chain_limit, sizeof(*next_bits));
    if (!targets || !next_bits) goto done;

    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *actuator;
        const uint8_t *door;
        int16_t next;
        int16_t target;
        int target_x, target_y;
        uint8_t old_bit;
        int prior;

        if (link == DM2_V1_RECORD_HANDLE_NULL || count >= chain_limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(actuator = dm2_v1_record_pool_address(&owner->record_pools,
                                                     link)) ||
            dm2_actu_type(actuator) != DM2_ACTU_PUSH_BUTTON_SWITCH ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, link, &next)) {
            result = count == 0 ? 0 : -1;
            goto done;
        }
        target_x = (int)dm2_actu_xcoord(actuator);
        target_y = (int)dm2_actu_ycoord(actuator);
        if (target_x < 0 || target_y < 0 ||
            target_x >= owner->dungeon.level_widths[map] ||
            target_y >= owner->dungeon.level_heights[map]) {
            result = -1;
            goto done;
        }
        target = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon,
                                                           map, target_x, target_y);
        if (target == DM2_V1_RECORD_HANDLE_NULL ||
            target == DM2_V1_RECORD_HANDLE_END ||
            dm2_v1_record_handle_pool(target) != DM2_DB_DOOR ||
            !(door = dm2_v1_record_pool_address(&owner->record_pools, target)) ||
            !dm2_v1_record_pool_address_mut(&owner->record_pools, target)) {
            result = -1;
            goto done;
        }
        old_bit = dm2_door_bit13(door);
        /* Source-order duplicate targets observe the preceding 0x46 write. */
        for (prior = 0; prior < count; ++prior) {
            if (targets[prior] == target) old_bit = next_bits[prior];
        }
        targets[count] = target;
        next_bits[count] = action == DM2_ACTMSG_TOGGLE ?
            (uint8_t)!old_bit : (action == DM2_ACTMSG_OPEN_SET ? 1u : 0u);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)target);
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                (uint32_t)next_bits[count]);
        ++count;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    for (int i = 0; i < count; ++i) {
        uint8_t *door = dm2_v1_record_pool_address_mut(&owner->record_pools,
                                                        targets[i]);
        if (!door) { result = -1; goto done; }
        if (dm2_door_bit13(door) != next_bits[i]) {
            dm2_door_set_bit13(door, next_bits[i]);
            ++receipt->push_button_doors_mutated;
        }
    }
    receipt->push_button_actuators_seen = count;
    receipt->private_push_button_hash = hash;
    receipt->valid = 1;
    result = 1;

done:
    free(next_bits);
    free(targets);
    return result;
}

/*
 * Private source-complete WALL_MECHA atom for CROSS_MAP.
 *
 * c_tim_proc.cpp::DM2_ACTUATE_WALL_MECHA dispatches a direction-matching
 * type 0x16 record by queueing an ACTUATE (0x04) message on the map encoded
 * in Actuator::Data.  Its target coordinates and direction come directly
 * from the same DB3 record.  No party, creature, item, palette or sound
 * state participates in this particular arm.  We therefore retain it only
 * when every *matching* DB3 record in the real tile chain is CROSS_MAP and
 * every resulting target is within the owned File_header world.  Other
 * direction records are source-ignored; every other matching actuator is an
 * unowned WALL_MECHA family and blocks before the first timer is queued.
 *
 * This is deliberately stricter than the source walk about non-DB3 chain
 * members.  Refusing an otherwise valid route is safe; skipping a text,
 * teleporter or object callback would not be.  Queue rollback makes this a
 * single private mutation.
 *
 * Source: SKProject SKULLWIN/c_tim_proc.cpp::DM2_ACTUATE_WALL_MECHA
 * (1923+), SKWINSPX/src/v4/skevent.cpp::CROSS_MAP (1920-1933),
 * c_tim_proc.cpp::DM2_INVOKE_MESSAGE (4332-4365).
 */
static int dm2_v1_game_load_owner_dispatch_cross_map_wall(
    DM2_V1_GameLoadWorldOwner *owner, int map, int x, int y,
    uint8_t direction, uint8_t action, DM2_V1_GameLoadActuateReceipt *receipt)
{
    int chain_limit = 0;
    int16_t link;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    int count = 0;
    int steps = 0;
    uint32_t hash = 0x43524d50u; /* "CRMP" */
    int result = 0;

    if (!owner || !receipt || direction > 3u || action > 2u ||
        owner->timer_capacity == 0u || !owner->timer_entries ||
        !owner->timer_indices) {
        return -1;
    }
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count) {
            return -1;
        }
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL) {
        return 0;
    }
    messages = (DM2_V1_TimerEntry *)calloc((size_t)chain_limit,
                                            sizeof(*messages));
    if (!messages) return -1;

    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint16_t data;
        int target_map;
        int target_x;
        int target_y;
        uint8_t target_direction;
        DM2_V1_TimerEntry *message;

        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= chain_limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&owner->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, link, &next)) {
            result = -1;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        if (dm2_actu_type(record) != DM2_ACTU_CROSS_MAP) {
            result = -1;
            goto done;
        }
        data = dm2_actu_data(record);
        target_map = (int)(data & 0x003fu);
        target_x = (int)dm2_actu_xcoord(record);
        target_y = (int)dm2_actu_ycoord(record);
        /* CROSS_MAP stores its target facing in Data, unlike the ordinary
         * generic actuator-message path that uses Actuator::Direction.
         * SKWINSPX/v4/skevent.cpp::CROSS_MAP (1920-1933). */
        target_direction = (uint8_t)((data >> 6) & 3u);
        if (target_map < 0 || target_map >= owner->dungeon.level_count ||
            target_x < 0 || target_y < 0 ||
            target_x >= owner->dungeon.level_widths[target_map] ||
            target_y >= owner->dungeon.level_heights[target_map] ||
            target_direction > 3u) {
            result = -1;
            goto done;
        }
        message = &messages[count];
        dm2_v1_timer_entry_init(message);
        dm2_v1_timer_set_mticks(message, (int16_t)target_map,
                                owner->timer_queue.gametick);
        message->ttype = 0x04u;
        message->actor = action == 0u ? 1u : (action == 1u ? 3u : 2u);
        message->xA = (int8_t)target_x;
        message->yA = (int8_t)target_y;
        message->wvalueB = (int16_t)((uint16_t)target_direction |
                                      ((uint16_t)action << 8));
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_map);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_x);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_y);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_direction);
        ++count;
        ++steps;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)owner->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)owner->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) {
        result = -1;
        goto done;
    }
    memcpy(timer_backup, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*index_backup));
    queue_backup = owner->timer_queue;
    for (int i = 0; i < count; ++i) {
        if (dm2_v1_timer_queue(&owner->timer_queue, &messages[i]) < 0) {
            memcpy(owner->timer_entries, timer_backup,
                   (size_t)owner->timer_capacity * sizeof(*timer_backup));
            memcpy(owner->timer_indices, index_backup,
                   (size_t)owner->timer_capacity * sizeof(*index_backup));
            owner->timer_queue = queue_backup;
            result = -1;
            goto done;
        }
    }
    receipt->cross_map_actuators_seen = count;
    receipt->cross_map_messages_queued = count;
    receipt->private_cross_map_hash = hash;
    receipt->valid = 1;
    result = 1;

done:
    free(index_backup);
    free(timer_backup);
    free(messages);
    return result;
}

int dm2_v1_game_load_world_owner_dispatch_actuate_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadActuateReceipt *out_receipt)
{
    DM2_V1_GameLoadActuateReceipt receipt;
    const int map = timer ? dm2_v1_timer_get_map(timer) : -1;
    const int x = timer ? (int)(uint8_t)timer->xA : -1;
    const int y = timer ? (int)(uint8_t)timer->yA : -1;
    const uint16_t payload = timer ? (uint16_t)timer->wvalueB : 0u;
    const uint8_t direction = (uint8_t)(payload & 0xffu);
    const uint8_t action = (uint8_t)(payload >> 8);
    int raw_tile;
    int deferred_pit_tele_close = 0;
    uint16_t deferred_tile_raw = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) || !timer ||
        timer->ttype != 0x04u || map < 0 || map >= owner->dungeon.level_count ||
        direction > 3u || action > 2u ||
        (action == 0u && timer->actor != 1u) ||
        (action == 1u && timer->actor != 3u) ||
        (action == 2u && timer->actor != 2u)) {
        return 0;
    }
    raw_tile = dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y);
    if (raw_tile < 0) return 0;

    receipt.map = map;
    receipt.x = (uint8_t)x;
    receipt.y = (uint8_t)y;
    receipt.direction = direction;
    receipt.action = action;
    receipt.raw_tile = (uint8_t)raw_tile;
    receipt.tile_class = (uint8_t)((unsigned int)raw_tile >> 5);

    /* SKProject sktimprc.cpp::DM2_PROCEED_TIMERS (4283-4327): type 3
     * deliberately has no case body.  It is the only 0x04 outcome that can
     * be completed without synthesising a FLOOR/WALL callback graph. */
    if (receipt.tile_class == 3u) {
        receipt.valid = 1;
        receipt.source_noop = 1;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }

    if (receipt.tile_class == 0u) {
        const int cross_map = dm2_v1_game_load_owner_dispatch_cross_map_wall(
            owner, map, x, y, direction, action, &receipt);
        if (cross_map > 0) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (cross_map < 0) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
    }

    if (receipt.tile_class == 0u || receipt.tile_class == 1u) {
        const int push_button = dm2_v1_game_load_owner_dispatch_push_button_floor(
            owner, map, x, y, action, &receipt);
        if (push_button > 0) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (push_button < 0) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
    }

    /* sktimprc.cpp::DM2_ACTUATE_PITFALL (3708-3742) and
     * ::DM2_ACTUATE_TELEPORTER (3833-3873) change bit 3, then both enter
     * DM2_ACTUATE_FLOOR_MECHA.  Their open path calls
     * DM2_ADVANCE_TILES_TIME, which can move the party marker and creatures.
     * That movement has no complete owner at GAME_LOAD, so it must fail
     * before either the tile or a DB2 text record is touched.  Closing does
     * not call ADVANCE_TILES_TIME and can therefore share the already-owned
     * all-DB2 FLOOR atom below. */
    if (receipt.tile_class == 2u || receipt.tile_class == 5u) {
        const uint8_t current_open = (uint8_t)(((unsigned int)raw_tile >> 3) & 1u);
        uint8_t requested_action = action;

        receipt.tile_state_before = current_open;
        if (requested_action == 2u) requested_action = current_open;
        if (requested_action == 0u) {
            receipt.blocked_unowned_tile_advance = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        /* Requested action 1 is the exact source close operation.  It is
         * deliberately retained even when bit 3 was already clear: source
         * still dispatches FLOOR_MECHA in that case. */
        deferred_tile_raw = (uint16_t)((unsigned int)raw_tile & ~0x08u);
        deferred_pit_tele_close = 1;
        receipt.tile_state_after = 0u;
    }

    if (receipt.tile_class == 1u || deferred_pit_tele_close) {
        const int16_t root = (int16_t)dm2_v1_dungeon_get_first_thing(
            &owner->dungeon, map, x, y);
        int16_t link = root;
        int16_t *links = NULL;
        uint8_t *visibility = NULL;
        int link_count = 0;
        int max_links;
        uint32_t hash = 2166136261u;
        int success = 0;

        /* c_tim_proc.cpp::DM2_ACTUATE_FLOOR_MECHA (3009-3180) is a single
         * ordered DB0..DB3 walk.  This private atom deliberately accepts
         * only an all-DB2 chain.  A non-text record has source behaviour in
         * another subsystem, so skipping it here would incorrectly promote
         * a partial FLOOR dispatch. */
        if (!owner->source_map_context_materialized ||
            root == DM2_V1_RECORD_HANDLE_END ||
            root == DM2_V1_RECORD_HANDLE_NULL ||
            owner->record_pools.pools[2].record_size != 4 ||
            owner->record_pools.pools[2].record_count < 0 ||
            owner->record_pools.pools[2].extension_count < 0) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        max_links = owner->record_pools.pools[2].record_count +
                    owner->record_pools.pools[2].extension_count;
        if (max_links <= 0) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        links = (int16_t *)calloc((size_t)max_links, sizeof(*links));
        visibility = (uint8_t *)calloc((size_t)max_links, sizeof(*visibility));
        if (!links || !visibility) goto floor_done;

        while (link != DM2_V1_RECORD_HANDLE_END) {
            const uint8_t *record;
            uint16_t w2;
            uint16_t text_index;
            uint8_t mode;
            uint8_t ext_usage;
            uint8_t complex_usage;
            uint8_t old_visibility;
            uint8_t new_visibility;
            int16_t next;

            if (link == DM2_V1_RECORD_HANDLE_NULL || link_count >= max_links ||
                dm2_v1_record_handle_pool(link) != 2) {
                receipt.blocked_non_text_chain = 1;
                goto floor_done;
            }
            record = dm2_v1_record_pool_address(&owner->record_pools, link);
            if (!record || !dm2_v1_record_pool_next_link(
                    &owner->record_pools, link, &next)) {
                receipt.blocked_incomplete_chain = 1;
                goto floor_done;
            }
            w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
            mode = (uint8_t)((w2 >> 1) & 3u);
            text_index = (uint16_t)((w2 >> 3) & 0x1fffu);
            ext_usage = (uint8_t)((text_index >> 8) & 0x1fu);
            complex_usage = (uint8_t)((text_index >> 9) & 0x0fu);
            old_visibility = (uint8_t)(w2 & 1u);
            if (mode != 0u &&
                !(mode == 1u && ext_usage == 5u) &&
                !(mode == 2u && complex_usage == 2u)) {
                /* The original merely continues for this DB2 form.  It is
                 * nevertheless a wholly-owned text record, so retain its
                 * source link and leave its visibility untouched. */
                new_visibility = old_visibility;
            } else {
                new_visibility = action == 2u ? (uint8_t)!old_visibility :
                    (action == 0u ? 1u : 0u);
                /* The source calls QUERY_MESSAGE_TEXT/DISPLAY_HINT_TEXT when
                 * a mode-zero text becomes visible at the party square.  No
                 * partial owner may silently suppress that player-visible
                 * effect, so reject before altering any private record. */
                if (mode == 0u && old_visibility == 0u &&
                    new_visibility != 0u && map == owner->source_party_map &&
                    x == (int)owner->source_party_x &&
                    y == (int)owner->source_party_y) {
                    receipt.blocked_hint_delivery = 1;
                    goto floor_done;
                }
            }
            links[link_count] = link;
            visibility[link_count] = new_visibility;
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
            hash = dm2_v1_game_load_owner_hash_step(hash, w2);
            hash = dm2_v1_game_load_owner_hash_step(hash, new_visibility);
            ++link_count;
            link = next;
        }
        if (link_count == 0) {
            receipt.blocked_incomplete_chain = 1;
            goto floor_done;
        }
        /* Resolve every mutable address before the first visibility bit is
         * written.  The owner cannot publish half a DB2 chain if an address
         * invariant breaks between source validation and commit. */
        for (int i = 0; i < link_count; ++i) {
            if (!dm2_v1_record_pool_address_mut(&owner->record_pools,
                                                links[i])) {
                receipt.blocked_incomplete_chain = 1;
                goto floor_done;
            }
        }
        /* The source changes the PIT/TELE tile before FLOOR_MECHA.  All DB2
         * links have now been authenticated and made mutable, so this is the
         * first possible mutation in this private transaction. */
        if (deferred_pit_tele_close &&
            dm2_v1_dungeon_set_tile_raw(&owner->dungeon, map, x, y,
                                        deferred_tile_raw) != 0) {
            receipt.blocked_incomplete_chain = 1;
            goto floor_done;
        }
        for (int i = 0; i < link_count; ++i) {
            uint8_t *record = dm2_v1_record_pool_address_mut(
                &owner->record_pools, links[i]);
            uint16_t w2;
            w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
            if ((uint8_t)(w2 & 1u) != visibility[i]) {
                w2 = (uint16_t)((w2 & ~1u) | visibility[i]);
                record[2] = (uint8_t)w2;
                record[3] = (uint8_t)(w2 >> 8);
                ++receipt.text_records_toggled;
            }
        }
        receipt.text_records_seen = link_count;
        receipt.pit_tele_tile_mutated = deferred_pit_tele_close &&
            (((unsigned int)raw_tile & 0x08u) != 0u);
        receipt.private_text_visibility_hash = hash;
        if (deferred_pit_tele_close)
            receipt.private_text_visibility_hash = dm2_v1_game_load_owner_hash_step(
                receipt.private_text_visibility_hash, deferred_tile_raw);
        receipt.valid = hash != 0u;
        success = receipt.valid;

floor_done:
        free(visibility);
        free(links);
        if (success) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        receipt.blocked_incomplete_chain = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    /* PIT/TELE must always fall into FLOOR_MECHA; DOOR requeues a type-1
     * c_tim; TRICKWALL consults party/CAII; WALL walks complete DB chains.
     * None may be mutated as a coordinate-only approximation. */
    receipt.blocked_incomplete_chain = 1;
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_world_owner_process_door_step_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadDoorStepReceipt *out_receipt)
{
    DM2_V1_GameLoadDoorStepReceipt receipt;
    const int map = timer ? dm2_v1_timer_get_map(timer) : -1;
    const int x = timer ? (int)(uint8_t)timer->xA : -1;
    const int y = timer ? (int)(uint8_t)timer->yA : -1;
    const int16_t link = timer ? timer->wvalueB : DM2_V1_RECORD_HANDLE_NULL;
    const int direction = timer ? (int)timer->actor : -1;
    const uint8_t *door;
    int raw_tile;
    int16_t chain_link;
    int chain_limit = 0;
    int chain_count = 0;
    int db;
    int old_state;
    int new_state;
    int16_t queued_slot = -1;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) || !timer ||
        timer->ttype != 0x01u || map < 0 ||
        map >= owner->dungeon.level_count || direction < 0 || direction > 1 ||
        link == DM2_V1_RECORD_HANDLE_NULL || link == DM2_V1_RECORD_HANDLE_END ||
        dm2_v1_record_handle_pool(link) != 0) {
        return 0;
    }
    raw_tile = dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y);
    if (raw_tile < 0 || ((unsigned int)raw_tile >> 5) != 4u ||
        dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y) != link ||
        !(door = dm2_v1_record_pool_address(&owner->record_pools, link))) {
        return 0;
    }
    /* All source state accessed below is local to the cloned File_header
     * owner.  Validate the complete tile chain first: a creature caught by a
     * closing door needs CAII/damage/timer ownership, so it must stop this
     * atom before either its map byte or queue is touched. */
    for (db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count) {
            return 0;
        }
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return 0;
    chain_link = link;
    while (chain_link != DM2_V1_RECORD_HANDLE_END) {
        int16_t next;
        if (chain_link == DM2_V1_RECORD_HANDLE_NULL ||
            chain_count >= chain_limit ||
            !dm2_v1_record_pool_address(&owner->record_pools, chain_link) ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, chain_link,
                                           &next)) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        if (dm2_v1_record_handle_pool(chain_link) == 4) {
            receipt.blocked_creature_collision = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        ++chain_count;
        chain_link = next;
    }
    if (map == owner->source_party_map && x == (int)owner->source_party_x &&
        y == (int)owner->source_party_y) {
        receipt.blocked_party_collision = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.map = map;
    receipt.x = (uint8_t)x;
    receipt.y = (uint8_t)y;
    receipt.direction = (uint8_t)direction;
    receipt.door_record_link = link;
    old_state = dm2_door_get_state((uint16_t)raw_tile);
    receipt.state_before = (uint8_t)old_state;
    if (old_state == DM2_DOOR_STATE_DESTROYED) {
        receipt.state_after = (uint8_t)old_state;
        receipt.source_noop_destroyed = 1;
        receipt.private_animation_hash = dm2_v1_game_load_owner_hash_step(
            0x44535450u, (uint16_t)link); /* "DSTP" */
        receipt.valid = receipt.private_animation_hash != 0u;
        if (out_receipt) *out_receipt = receipt;
        return receipt.valid;
    }
    new_state = dm2_door_apply_toggle_step(old_state, direction);
    if (new_state < DM2_DOOR_STATE_OPEN || new_state > DM2_DOOR_STATE_CLOSED)
        return 0;
    receipt.state_after = (uint8_t)new_state;
    if (new_state != old_state &&
        !dm2_v1_record_pool_address_mut(&owner->record_pools, link)) {
        receipt.blocked_incomplete_chain = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if ((direction == 0 && new_state != DM2_DOOR_STATE_OPEN) ||
        (direction == 1 && new_state != DM2_DOOR_STATE_CLOSED)) {
        DM2_V1_TimerEntry next = *timer;
        next.l_00 += 1; /* DM2_STEP_DOOR queues its next source tick. */
        if ((queued_slot = dm2_v1_timer_queue(&owner->timer_queue, &next)) < 0)
            return 0;
        receipt.requeued = 1;
    }
    if (new_state != old_state && dm2_v1_dungeon_set_tile_raw(
            &owner->dungeon, map, x, y,
            dm2_door_set_state((uint16_t)raw_tile, new_state)) != 0) {
        if (queued_slot >= 0) dm2_v1_timer_delete(&owner->timer_queue, queued_slot);
        return 0;
    }
    receipt.door_state_mutated = new_state != old_state;
    receipt.private_animation_hash = dm2_v1_game_load_owner_hash_step(
        0x44535450u, (uint16_t)link); /* "DSTP" */
    receipt.private_animation_hash = dm2_v1_game_load_owner_hash_step(
        receipt.private_animation_hash, (uint32_t)old_state);
    receipt.private_animation_hash = dm2_v1_game_load_owner_hash_step(
        receipt.private_animation_hash, (uint32_t)new_state);
    receipt.private_animation_hash = dm2_v1_game_load_owner_hash_step(
        receipt.private_animation_hash, (uint32_t)timer->l_00);
    receipt.valid = receipt.private_animation_hash != 0u;
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

typedef struct {
    uint8_t *dungeon_raw;
    uint8_t *pool_bytes[DM2_V1_RECORD_POOL_COUNT];
    uint8_t *pool_extension_bytes[DM2_V1_RECORD_POOL_COUNT];
    DM2_V1_TimerEntry *timer_entries;
    int16_t *timer_indices;
    DM2_V1_TimerQueue timer_queue;
    int current_map;
} DM2_V1_GameLoadTimerSnapshot;

/* The timer heap is only a small part of a source timer step: 0x04 can alter
 * a File_header tile and DB0/DB2, while 0x56 changes DB3 before it queues its
 * successors.  Keep all mutable source-owned bytes in the outer transaction
 * so a later unowned consumer cannot consume the old timer or leak a partial
 * change. */
static void dm2_v1_game_load_owner_timer_snapshot_free(
    DM2_V1_GameLoadTimerSnapshot *snapshot)
{
    int db;
    if (!snapshot) return;
    for (db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        free(snapshot->pool_extension_bytes[db]);
        free(snapshot->pool_bytes[db]);
    }
    free(snapshot->timer_indices);
    free(snapshot->timer_entries);
    free(snapshot->dungeon_raw);
    memset(snapshot, 0, sizeof(*snapshot));
}

static int dm2_v1_game_load_owner_timer_snapshot_take(
    const DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadTimerSnapshot *snapshot)
{
    int db;

    if (!owner || !snapshot || !owner->dungeon.raw_data ||
        owner->dungeon.raw_size == 0u || owner->timer_capacity == 0u ||
        !owner->timer_entries || !owner->timer_indices) return 0;
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->dungeon_raw = (uint8_t *)malloc(owner->dungeon.raw_size);
    snapshot->timer_entries = (DM2_V1_TimerEntry *)malloc(
        (size_t)owner->timer_capacity * sizeof(*snapshot->timer_entries));
    snapshot->timer_indices = (int16_t *)malloc(
        (size_t)owner->timer_capacity * sizeof(*snapshot->timer_indices));
    if (!snapshot->dungeon_raw || !snapshot->timer_entries ||
        !snapshot->timer_indices) goto fail;
    memcpy(snapshot->dungeon_raw, owner->dungeon.raw_data, owner->dungeon.raw_size);
    memcpy(snapshot->timer_entries, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*snapshot->timer_entries));
    memcpy(snapshot->timer_indices, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*snapshot->timer_indices));
    snapshot->timer_queue = owner->timer_queue;
    snapshot->current_map = owner->current_map;
    for (db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        size_t main_size;
        size_t extension_size;
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            ((pool->record_count != 0 || pool->extension_count != 0) &&
             pool->record_size <= 0) ||
            (pool->record_size > 0 &&
             ((size_t)pool->record_count > SIZE_MAX / (size_t)pool->record_size ||
              (size_t)pool->extension_count > SIZE_MAX / (size_t)pool->record_size))) {
            goto fail;
        }
        main_size = (size_t)pool->record_count * (size_t)pool->record_size;
        extension_size = (size_t)pool->extension_count * (size_t)pool->record_size;
        if ((main_size != 0u && !pool->bytes) ||
            (extension_size != 0u && !pool->extension_bytes)) goto fail;
        if (main_size != 0u) {
            snapshot->pool_bytes[db] = (uint8_t *)malloc(main_size);
            if (!snapshot->pool_bytes[db]) goto fail;
            memcpy(snapshot->pool_bytes[db], pool->bytes, main_size);
        }
        if (extension_size != 0u) {
            snapshot->pool_extension_bytes[db] = (uint8_t *)malloc(extension_size);
            if (!snapshot->pool_extension_bytes[db]) goto fail;
            memcpy(snapshot->pool_extension_bytes[db], pool->extension_bytes,
                   extension_size);
        }
    }
    return 1;

fail:
    dm2_v1_game_load_owner_timer_snapshot_free(snapshot);
    return 0;
}

static void dm2_v1_game_load_owner_timer_snapshot_restore(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_GameLoadTimerSnapshot *snapshot)
{
    int db;
    if (!owner || !snapshot || !snapshot->dungeon_raw ||
        !snapshot->timer_entries || !snapshot->timer_indices) return;
    memcpy(owner->dungeon.raw_data, snapshot->dungeon_raw, owner->dungeon.raw_size);
    memcpy(owner->timer_entries, snapshot->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*snapshot->timer_entries));
    memcpy(owner->timer_indices, snapshot->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*snapshot->timer_indices));
    owner->timer_queue = snapshot->timer_queue;
    owner->current_map = snapshot->current_map;
    for (db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        const size_t main_size = (size_t)pool->record_count * (size_t)pool->record_size;
        const size_t extension_size = (size_t)pool->extension_count *
            (size_t)pool->record_size;
        if (main_size != 0u && snapshot->pool_bytes[db])
            memcpy(pool->bytes, snapshot->pool_bytes[db], main_size);
        if (extension_size != 0u && snapshot->pool_extension_bytes[db])
            memcpy(pool->extension_bytes, snapshot->pool_extension_bytes[db],
                   extension_size);
    }
}

int dm2_v1_game_load_world_owner_process_next_due_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadTimerProcessReceipt *out_receipt)
{
    DM2_V1_GameLoadTimerProcessReceipt receipt;
    DM2_V1_GameLoadTimerSnapshot snapshot;
    DM2_V1_TimerEntry timer;
    int accepted = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    memset(&snapshot, 0, sizeof(snapshot));
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        owner->timer_queue.num_timers <= 0 ||
        !dm2_v1_game_load_owner_timer_snapshot_take(owner, &snapshot)) {
        return 0;
    }
    /* DM2_PROCEED_TIMERS asks the heap whether the head is due before it
     * deletes it. Snapshot predates this call because the heap may sift. */
    if (!dm2_v1_timer_is_due(&owner->timer_queue)) {
        dm2_v1_game_load_owner_timer_snapshot_restore(owner, &snapshot);
        dm2_v1_game_load_owner_timer_snapshot_free(&snapshot);
        return 0;
    }
    dm2_v1_timer_get_and_delete_next(&owner->timer_queue, &timer);
    receipt.timer_dequeued = 1;
    receipt.timer_type = timer.ttype;
    receipt.timer_map = dm2_v1_timer_get_map(&timer);
    if (receipt.timer_map < 0 || receipt.timer_map >= owner->dungeon.level_count)
        goto blocked;
    owner->current_map = receipt.timer_map;
    switch (timer.ttype) {
        case 0x56u:
            accepted = dm2_v1_game_load_world_owner_continue_tick_generator(
                owner, &timer, &receipt.tick_generator);
            break;
        case 0x04u:
            accepted = dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                owner, &timer, &receipt.actuate);
            break;
        case 0x01u:
            accepted = dm2_v1_game_load_world_owner_process_door_step_timer(
                owner, &timer, &receipt.door_step);
            break;
        default:
            break;
    }
    if (!accepted) goto blocked;
    receipt.valid = 1;
    dm2_v1_game_load_owner_timer_snapshot_free(&snapshot);
    if (out_receipt) *out_receipt = receipt;
    return 1;

blocked:
    dm2_v1_game_load_owner_timer_snapshot_restore(owner, &snapshot);
    receipt.valid = 0;
    receipt.timer_dequeued = 0;
    receipt.blocked_unowned_timer = 1;
    dm2_v1_game_load_owner_timer_snapshot_free(&snapshot);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}
