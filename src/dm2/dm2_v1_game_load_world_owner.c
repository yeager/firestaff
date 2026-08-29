/* Source-owned File_header world materialisation for DM2 New Game. */

#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_creature_animation_gdat.h"
#include "dm2_v1_fmtowns_graphics_dat.h"
#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_data_tables_pc34_compat.h"
#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_creature_something_pc34_compat.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_creature_ops_pc34_compat.h"
#include "dm2_v1_delete_creature_full_pc34_compat.h"
#include "dm2_v1_cloud_pc34_compat.h"
#include "dm2_v1_hero_ops_pc34_compat.h"
#include "dm2_v1_skproject_core.h"
#include "dm2_v1_sound_queue_pc34_compat.h"
#include "dm2_v1_think_creature_pc34_compat.h"
#include "dm2_v1_timer_ops_pc34_compat.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

enum {
    DM2_V1_GAME_LOAD_WORLD_OWNER_LIFECYCLE_TAG = 0x474c574fu, /* "GLWO" */
    DM2_V1_GAME_LOAD_RUNTIME_CANDIDATE_LIFECYCLE_TAG = 0x47534c43u /* "GSLC" */
};

/* Constructors deliberately accept caller-owned storage that might not have
 * been initialized yet. Inspect its first four object-representation bytes
 * through memcpy rather than reading an indeterminate uint32_t lvalue. Both
 * owner structs keep lifecycle_tag as their first field for this purpose. */
static int dm2_v1_game_load_world_owner_is_initialized(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    uint32_t tag = 0u;

    if (!owner) return 0;
    memcpy(&tag, owner, sizeof(tag));
    return tag == DM2_V1_GAME_LOAD_WORLD_OWNER_LIFECYCLE_TAG;
}

static int dm2_v1_game_load_runtime_candidate_is_initialized(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate)
{
    uint32_t tag = 0u;

    if (!candidate) return 0;
    memcpy(&tag, candidate, sizeof(tag));
    return tag == DM2_V1_GAME_LOAD_RUNTIME_CANDIDATE_LIFECYCLE_TAG;
}

static uint32_t dm2_v1_game_load_owner_hash_step(uint32_t hash,
                                                  uint32_t value);

/* These are c_dm2data's explicit fresh-game writes, not inferred values.
 * Keep this separate from c_move execution: DM2_PERFORM_MOVE also requires
 * c_moverec, timer, actuator and creature dispatch ownership. */
static void dm2_v1_game_load_runtime_candidate_init_movement_state(
    DM2_V1_GameLoadMovementState *movement)
{
    if (!movement) return;
    memset(movement, 0, sizeof(*movement));
    movement->pending_creature = DM2_V1_RECORD_HANDLE_END;
    movement->valid = 1;
}

/* DM2_INIT establishes the zeroed c_moverec register block. The final
 * DM2_move_2fcf_0b8b map change supplies v1d3248 later, so retain both the
 * forced selector and its selected result rather than mislabelling either as
 * a record handle. */
static void dm2_v1_game_load_moverec_state_init(
    DM2_V1_GameLoadMoverecState *moverec, int current_map)
{
    if (!moverec || current_map < 0 || current_map > INT16_MAX) return;
    memset(moverec, 0, sizeof(*moverec));
    moverec->v1d3248_before_final_change = -1;
    moverec->v1d3248 = (int16_t)current_map;
    moverec->valid = 1;
}

static uint16_t dm2_v1_game_load_owner_read_u16le(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint16_t dm2_v1_game_load_owner_read_record_u16(
    const DM2_V1_GameLoadWorldOwner *owner, const uint8_t *bytes)
{
    if (owner && owner->dungeon.words_big_endian)
        return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
    return dm2_v1_game_load_owner_read_u16le(bytes);
}

static void dm2_v1_game_load_owner_write_u16le(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static void dm2_v1_game_load_owner_write_record_u16(
    const DM2_V1_GameLoadWorldOwner *owner, uint8_t *bytes, uint16_t value)
{
    if (owner && owner->dungeon.words_big_endian) {
        bytes[0] = (uint8_t)(value >> 8);
        bytes[1] = (uint8_t)value;
    } else {
        dm2_v1_game_load_owner_write_u16le(bytes, value);
    }
}

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

static int dm2_v1_game_load_owner_materialize_caii_capacity(
    DM2_V1_GameLoadWorldOwner *owner)
{
    const DM2_V1_RecordPool *db4;
    DM2_V1_GameLoadCaiiCapacityReceipt candidate;
    uint32_t hash = 0x43414949u; /* "CAII" */
    int index;
    int nonstatic_count = 0;

    if (!owner || !owner->asset_loader || !owner->asset_loader->loaded ||
        owner->caii_source.valid || owner->caii_capacity.valid ||
        !owner->record_pools.valid) return 0;
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 6 || db4->record_count <= 0 ||
        db4->record_count > UINT16_MAX ||
        !dm2_v1_caii_source_owner_init(&owner->caii_source,
                                       owner->asset_loader, &owner->dungeon)) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    for (index = 0; index < db4->record_count; ++index) {
        const uint8_t *record = db4->bytes +
            (size_t)index * (size_t)db4->record_size;
        const DM2_AIDefinition *ai = NULL;
        const uint16_t record_word = (uint16_t)record[0] |
            ((uint16_t)record[1] << 8);

        if (record_word == 0xffffu) continue;
        if (!dm2_v1_caii_source_owner_ai_spec_def(&owner->caii_source,
                                                   record[4], &ai) || !ai) {
            dm2_v1_caii_source_owner_free(&owner->caii_source);
            return 0;
        }
        /* DM2_1c9a_3c30 counts only AIDefinition rows with flag bit 0
         * clear. This is a capacity calculation, not a creature activation. */
        if ((ai->w0AIFlags & 1u) == 0u) ++nonstatic_count;
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)index);
        hash = dm2_v1_game_load_owner_hash_step(hash, record[4]);
        hash = dm2_v1_game_load_owner_hash_step(hash, ai->w0AIFlags);
    }
    if (nonstatic_count > INT_MAX - 0x64) {
        dm2_v1_caii_source_owner_free(&owner->caii_source);
        return 0;
    }
    candidate.db4_record_count = (uint16_t)db4->record_count;
    candidate.nonstatic_creature_count = (uint16_t)nonstatic_count;
    candidate.source_capacity = (uint16_t)((nonstatic_count + 0x64 <
        db4->record_count) ? nonstatic_count + 0x64 : db4->record_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.db4_record_count);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                            candidate.nonstatic_creature_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.source_capacity);
    hash = dm2_v1_game_load_owner_hash_step(hash, owner->caii_source.source_hash);
    if (hash == 0u || candidate.source_capacity == 0u) {
        dm2_v1_caii_source_owner_free(&owner->caii_source);
        return 0;
    }
    candidate.source_hash = hash;
    candidate.valid = 1;
    owner->caii_capacity = candidate;
    return 1;
}

/* Static creatures take the deterministic GAF branch in
 * DM2_GET_CREATURE_ANIMATION_FRAME: find command 0x11 in RAW8/0xfb, count
 * RAW7/0xfc rows through the terminator, then encode the record's packed
 * coordinate. Dynamic creatures take 4FCC and may consume c_random, so they
 * are intentionally not approximated here.
 * Source: SKProject SKULLWIN/c_creature.cpp:3217-3278. */
static int dm2_v1_game_load_owner_static_caii_animation_frame(
    const DM2_V1_AssetLoader *loader, uint8_t creature_type,
    uint16_t packed_position, uint16_t *out_frame)
{
    const uint8_t *attribution;
    const uint8_t *info;
    size_t attribution_size = 0u;
    size_t info_size = 0u;
    size_t row;
    uint16_t base;
    uint32_t count = 0u;

    if (!loader || !out_frame) return 0;
    *out_frame = 0xffffu;
    if (loader->gdat_version == DM2_FMTOWNS_GDAT_VERSION) {
        return dm2_v1_creature_animation_gdat_static_frame_fmtowns(
            loader, creature_type, packed_position, out_frame);
    }
    attribution = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_CREATURES, creature_type, DM2_GDAT_ENTRY_TYPE_RAW8,
        DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, &attribution_size);
    if (!attribution || attribution_size < 4u) return 0;
    for (row = 0u; row * 4u + 3u < attribution_size; ++row) {
        const uint16_t command = (uint16_t)attribution[row * 4u] |
            ((uint16_t)attribution[row * 4u + 1u] << 8);
        if (command == 0xffffu || command == 0x0011u) break;
    }
    if (row * 4u + 3u >= attribution_size) return 0;
    if ((uint16_t)attribution[row * 4u] != 0x0011u) {
        /* The source's terminator is a legitimate no-animation result for
         * static objects without a 0x11 attribution row. */
        return 1;
    }
    base = (uint16_t)attribution[row * 4u + 2u] |
        ((uint16_t)attribution[row * 4u + 3u] << 8);
    info = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_CREATURES, creature_type, DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, &info_size);
    if (!info || info_size < 4u) return 0;
    for (;;) {
        const size_t index = (size_t)base + count;
        if (index * 4u + 1u >= info_size || count >= 0x3fffu) return 0;
        ++count;
        if ((info[index * 4u + 1u] & 0xf0u) == 0u) break;
    }
    *out_frame = (uint16_t)(count | (packed_position == 0u ? 0x9000u :
        (((uint32_t)packed_position & 0x3fu) << 6) | 0x8000u));
    return 1;
}

static int dm2_v1_game_load_owner_materialize_caii_map_candidates(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadCaiiMapCandidate *candidates;
    DM2_V1_GameLoadCaiiMapReceipt receipt;
    const DM2_V1_RecordPool *db4;
    uint32_t hash = 0x43414d50u; /* "CAMP" */
    int capacity;
    int count = 0;
    int map;

    if (!owner || !owner->caii_source.valid || !owner->caii_capacity.valid ||
        owner->caii_map_receipt.valid || owner->caii_map_candidates ||
        !owner->record_pools.valid || owner->dungeon.level_count <= 0) {
        return 0;
    }
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 14 || db4->record_count <= 0 ||
        db4->record_count > UINT16_MAX) {
        return 0;
    }
    capacity = db4->record_count;
    candidates = (DM2_V1_GameLoadCaiiMapCandidate *)calloc((size_t)capacity,
                                                             sizeof(*candidates));
    if (!candidates) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (map = 0; map < owner->dungeon.level_count; ++map) {
        int x;
        const int width = owner->dungeon.level_widths[map];
        const int height = owner->dungeon.level_heights[map];
        if (width <= 0 || height <= 0) {
            goto fail;
        }
        for (x = 0; x < width; ++x) {
            int y;
            for (y = 0; y < height; ++y) {
                const int raw = dm2_v1_dungeon_get_tile_raw(&owner->dungeon,
                                                             map, x, y);
                int16_t link;
                int chain_count = 0;
                int16_t next;
                if (raw < 0) {
                    goto fail;
                }
                /* c_1c9a.cpp:9916 tests this exact ground-stack flag before
                 * walking the tile record chain. */
                if (((uint16_t)raw & 0x10u) == 0u) continue;
                link = (int16_t)dm2_v1_dungeon_get_first_thing(
                    &owner->dungeon, map, x, y);
                /* Authenticated Mac File_header ground stacks use
                 * OBJECT_NULL for some object-bearing squares. They are
                 * empty chains, not malformed CAII roots. */
                if (link == DM2_V1_RECORD_HANDLE_NULL)
                    continue;
                while (link != DM2_V1_RECORD_HANDLE_END) {
                    uint8_t *record = NULL;
                    const DM2_AIDefinition *ai = NULL;
                    if (link == DM2_V1_RECORD_HANDLE_NULL ||
                        chain_count++ >= capacity) goto fail;
                    if (owner->dungeon.source_words_big_endian) {
                        const uint8_t *link_record =
                            dm2_v1_record_pool_address(&owner->record_pools, link);
                        if (!link_record) goto fail;
                        next = (int16_t)(((uint16_t)link_record[0] << 8) |
                                         (uint16_t)link_record[1]);
                    } else if (!dm2_v1_record_pool_next_link(
                                   &owner->record_pools, link, &next)) {
                        goto fail;
                    }
                    if (dm2_v1_record_handle_pool(link) == 4) {
                        DM2_V1_GameLoadCaiiMapCandidate *candidate;
                        int duplicate = 0;
                        int prior;
                        for (prior = 0; prior < count; ++prior) {
                            if (candidates[prior].record_handle == link) {
                                duplicate = 1;
                                break;
                            }
                        }
                        if (duplicate) {
                            link = next;
                            continue;
                        }
                        if (count >= capacity ||
                            !(record = dm2_v1_record_pool_address_mut(
                                &owner->record_pools, link)) ||
                            !dm2_v1_caii_source_owner_ai_spec_def(
                                &owner->caii_source, record[4], &ai) || !ai) {
                            goto fail;
                        }
                        for (prior = 0; prior < count; ++prior) {
                            if (candidates[prior].record_handle == link) {
                                duplicate = 1;
                                break;
                            }
                        }
                        if (duplicate) {
                            link = next;
                            continue;
                        }
                        candidate = &candidates[count++];
                        candidate->map = (int16_t)map;
                        candidate->x = (uint8_t)x;
                        candidate->y = (uint8_t)y;
                        candidate->record_handle = link;
                        candidate->creature_type = record[4];
                        candidate->static_ai = (uint8_t)((ai->w0AIFlags & 1u) != 0u);
                        candidate->record_word_a = owner->dungeon.source_words_big_endian
                            ? (uint16_t)(((uint16_t)record[10] << 8) | record[11])
                            : (uint16_t)record[10] | ((uint16_t)record[11] << 8);
                        candidate->packed_position = owner->dungeon.source_words_big_endian
                            ? (uint16_t)(((uint16_t)record[12] << 8) | record[13])
                            : (uint16_t)record[12] | ((uint16_t)record[13] << 8);
                        candidate->static_animation_frame = 0xffffu;
                        if (candidate->static_ai) {
                            if (!dm2_v1_game_load_owner_static_caii_animation_frame(
                                owner->asset_loader, record[4],
                                candidate->packed_position,
                                &candidate->static_animation_frame)) {
                                /* FM Towns has authentic static creature
                                 * records outside the 42-entry type-06
                                 * animation roster.  The native GAF path
                                 * returns no frame for those records; that
                                 * is a valid no-animation state, not a
                                 * reason to reject the whole dungeon. */
                                if (owner->asset_loader->gdat_version !=
                                    DM2_FMTOWNS_GDAT_VERSION &&
                                    !owner->dungeon.words_big_endian) {
                                    goto fail;
                                }
                                candidate->static_animation_frame = 0xffffu;
                            }
                            ++receipt.static_candidate_count;
                        } else {
                            ++receipt.dynamic_candidate_count;
                        }
                        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                                 (uint16_t)map);
                        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                                 (uint16_t)x);
                        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                                 (uint16_t)y);
                        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                                 (uint16_t)link);
                        hash = dm2_v1_game_load_owner_hash_step(hash, record[4]);
                        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                                 ai->w0AIFlags);
                        hash = dm2_v1_game_load_owner_hash_step(hash,
                            candidate->static_animation_frame);
                    }
                    link = next;
                }
            }
        }
    }
    receipt.map_count = (uint16_t)owner->dungeon.level_count;
    receipt.candidate_count = (uint16_t)count;
    if (receipt.candidate_count == 0u ||
            receipt.candidate_count != receipt.static_candidate_count +
            receipt.dynamic_candidate_count) goto fail;
    hash = dm2_v1_game_load_owner_hash_step(hash, receipt.map_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, receipt.candidate_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, receipt.static_candidate_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, receipt.dynamic_candidate_count);
    if (hash == 0u) {
        goto fail;
    }
    receipt.source_hash = hash;
    receipt.valid = 1;
    owner->caii_map_candidates = candidates;
    owner->caii_map_receipt = receipt;
    return 1;
fail:
    free(candidates);
    return 0;
}

/* Reserve exactly the source-sized c_creature array before the later
 * RESET_CAII transaction.  Resetting DB4 byte@5 and assigning candidates
 * must include both static 09db and dynamic 0a48 paths, so doing either here
 * would invent a partial GAME_LOAD state.
 * Source: SKProject SKULLWIN/startend.cpp::DM2_RESET_CAII (1033-1070),
 * c_random.cpp::c_randomdata::init (7-13). */
static int dm2_v1_game_load_owner_materialize_caii_storage(
    DM2_V1_GameLoadWorldOwner *owner)
{
    if (!owner || !owner->caii_capacity.valid ||
        owner->caii_capacity.source_capacity == 0u ||
        owner->caii_slots.valid || owner->caii_rng_initialized) {
        return 0;
    }
    dm2_v1_caii_array_init(&owner->caii_slots,
                           (int)owner->caii_capacity.source_capacity);
    if (!owner->caii_slots.valid ||
        owner->caii_slots.capacity !=
            (int)owner->caii_capacity.source_capacity ||
        owner->caii_slots.alloc_count != 0) {
        dm2_v1_caii_array_free(&owner->caii_slots);
        return 0;
    }
    dm2_v1_drops_rng_init(&owner->caii_rng);
    owner->caii_rng_initialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_static_caii(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_RecordPool *db4;
    uint8_t *snapshot;
    size_t db4_bytes;
    uint32_t hash = 0x43535441u; /* "CSTA" */
    int index;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->caii_source.valid || !owner->caii_map_receipt.valid ||
        !owner->caii_map_candidates || !owner->caii_slots.valid ||
        !owner->caii_rng_initialized || owner->caii_static_animation.valid ||
        owner->caii_map_receipt.static_candidate_count == 0u) {
        return 0;
    }
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 14 || db4->record_count <= 0 ||
        db4->record_count > UINT16_MAX ||
        (size_t)db4->record_count > SIZE_MAX / (size_t)db4->record_size) {
        return 0;
    }
    db4_bytes = (size_t)db4->record_count * (size_t)db4->record_size;
    snapshot = (uint8_t *)malloc(db4_bytes);
    if (!snapshot) return 0;
    memcpy(snapshot, db4->bytes, db4_bytes);

    /* RESET_CAII clears the owner byte on every DB4 record before
     * FILL_ORPHAN_CAII begins its map traversal.  This is source state, not
     * a replacement creature allocation. */
    for (index = 0; index < db4->record_count; ++index)
        db4->bytes[(size_t)index * (size_t)db4->record_size + 5u] = 0xffu;

    for (index = 0; index < owner->caii_map_receipt.candidate_count; ++index) {
        const DM2_V1_GameLoadCaiiMapCandidate *candidate =
            &owner->caii_map_candidates[index];
        const DM2_AIDefinition *ai = NULL;
        uint8_t *record;
        const uint8_t *animation = NULL;
        uint16_t adjacent_base;
        int16_t animation_word;
        uint16_t old_word;
        uint16_t merged_word;
        DM2_V1_CreatureAnimFrameReceipt frame_receipt;

        if (!candidate->static_ai) continue;
        record = dm2_v1_record_pool_address_mut(&owner->record_pools,
                                                candidate->record_handle);
        if (!record || record[4] != candidate->creature_type ||
            !dm2_v1_caii_source_owner_ai_spec_def(&owner->caii_source,
                                                   record[4], &ai) ||
            !ai || (ai->w0AIFlags & 1u) == 0u) {
            goto rollback;
        }
        old_word = owner->dungeon.source_words_big_endian
            ? (uint16_t)(((uint16_t)record[10] << 8) | record[11])
            : (uint16_t)record[10] | ((uint16_t)record[11] << 8);
        if (old_word != candidate->record_word_a) goto rollback;
        adjacent_base = owner->dungeon.source_words_big_endian
            ? (uint16_t)(((uint16_t)record[8] << 8) | record[9])
            : (uint16_t)record[8] | ((uint16_t)record[9] << 8);
        animation_word = (int16_t)old_word;
        memset(&frame_receipt, 0, sizeof(frame_receipt));
        if (owner->dungeon.words_big_endian) {
            /* Macintosh DB4 is authenticated as a big-endian source, but
             * its static GAF attribution is not present in the retained
             * Graphics.dat resource.  Keep the original no-animation word
             * and leave the record untouched. */
            continue;
        } else if (owner->asset_loader->gdat_version == DM2_FMTOWNS_GDAT_VERSION) {
            /* HME-242 has no per-creature FB/FC owner.  The type-06 pass
             * already resolved the authenticated shared sequence, while
             * 0xffff is the native no-animation result for static records
             * outside that roster. */
            if (candidate->static_animation_frame == 0xffffu) continue;
            animation_word = (int16_t)candidate->static_animation_frame;
        } else {
            if (candidate->static_animation_frame == 0xffffu)
                continue;
            if (dm2_v1_creature_get_animation_frame_with_ai_spec(
                    owner->asset_loader, &owner->caii_rng, ai, record[4], 0x11,
                    &adjacent_base, &animation_word, &animation,
                    candidate->packed_position, &frame_receipt) != 1 ||
                !frame_receipt.valid || !frame_receipt.static_path ||
                animation != NULL || (uint16_t)animation_word !=
                    candidate->static_animation_frame) {
                goto rollback;
            }
        }
        /* c_1c9a.cpp:9944-9957: 09db writes the animation word, then the
         * caller preserves bits 0x0060 from the pre-animation record and
         * restores the 0x8001 sentinel shape when applicable. */
        merged_word = (uint16_t)animation_word;
        merged_word |= (uint16_t)((old_word ^ merged_word) & 0x0060u);
        if ((old_word & 0x803fu) == 0x8001u)
            merged_word = (uint16_t)((merged_word & 0x7fc0u) | 0x8001u);
        dm2_v1_game_load_owner_write_record_u16(owner, record + 10u,
                                                merged_word);
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                 (uint16_t)candidate->map);
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                 candidate->record_handle);
        hash = dm2_v1_game_load_owner_hash_step(hash, old_word);
        hash = dm2_v1_game_load_owner_hash_step(hash, merged_word);
    }
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)db4->record_count);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, owner->caii_map_receipt.static_candidate_count);
    if (hash == 0u || owner->caii_rng.random != 0u) goto rollback;
    owner->caii_static_animation.valid = 1;
    owner->caii_static_animation.db4_slot_reset_count =
        (uint16_t)db4->record_count;
    owner->caii_static_animation.static_animation_count =
        owner->caii_map_receipt.static_candidate_count;
    owner->caii_static_animation.source_hash = hash;
    free(snapshot);
    return 1;

rollback:
    memcpy(db4->bytes, snapshot, db4_bytes);
    free(snapshot);
    return 0;
}

int dm2_v1_game_load_world_owner_schedule_caii_think(
    DM2_V1_GameLoadWorldOwner *owner, int16_t record_handle, int map,
    int x, int y, DM2_V1_GameLoadCaiiThinkReceipt *out_receipt)
{
    DM2_V1_GameLoadCaiiThinkReceipt receipt;
    DM2_V1_RecordPool *db4;
    uint8_t *record = NULL;
    uint8_t *slot = NULL;
    uint8_t slot_backup[DM2_V1_CAII_SLOT_SIZE];
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup = {0};
    DM2_V1_DropRng rng_backup = {0};
    int16_t prior_timer;
    int16_t queued_timer;
    int slot_index;
    DM2_V1_TimerEntry timer;

    memset(&receipt, 0, sizeof(receipt));
    receipt.timer_slot = -1;
    receipt.caii_slot = -1;
    if (out_receipt) *out_receipt = receipt;
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->caii_slots.valid || !owner->caii_rng_initialized ||
        !owner->timer_entries || !owner->timer_indices ||
        !owner->caii_map_candidates ||
        owner->timer_capacity == 0u || map < 0 || map > 0xff ||
        x < 0 || x > 0x1f || y < 0 || y > 0x1f ||
        dm2_v1_record_handle_pool(record_handle) != 4) return 0;

    db4 = &owner->record_pools.pools[4];
    record = dm2_v1_record_pool_address_mut(&owner->record_pools, record_handle);
    if (!db4->bytes || db4->record_size < 14 || !record || record[5] == 0xffu)
        return 0;
    slot_index = (int)record[5];
    if (slot_index < 0 || slot_index >= owner->caii_slots.capacity ||
        !(slot = owner->caii_slots.slots +
            (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE) ||
        (int16_t)dm2_v1_game_load_owner_read_u16le(slot) < 0 ||
        (dm2_v1_game_load_owner_read_u16le(slot) & 0x03ffu) !=
            ((uint16_t)record_handle & 0x03ffu)) return 0;

    /* 0cf7 operates on the creature actually residing at the supplied cell.
     * The future all-map caller already has the source traversal candidate;
     * retain that guard here so no coordinate-only timer can be fabricated. */
    {
        int found = 0;
        int i;
        for (i = 0; i < owner->caii_map_receipt.candidate_count; ++i) {
            const DM2_V1_GameLoadCaiiMapCandidate *candidate =
                &owner->caii_map_candidates[i];
            if (candidate->record_handle == record_handle && candidate->map == map &&
                candidate->x == (uint8_t)x && candidate->y == (uint8_t)y) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }

    timer_backup = (DM2_V1_TimerEntry *)malloc((size_t)owner->timer_capacity *
                                                sizeof(*timer_backup));
    index_backup = (int16_t *)malloc((size_t)owner->timer_capacity *
                                     sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto rollback;
    memcpy(slot_backup, slot, sizeof(slot_backup));
    memcpy(timer_backup, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*index_backup));
    queue_backup = owner->timer_queue;
    rng_backup = owner->caii_rng;

    /* SKProject c_1c9a.cpp::DM2_1c9a_0cf7 (5695-5728): cancel the old
     * pending think timer, then queue tick+1 with the DB4 group-dependent
     * 0x21/0x22 type and save the actual c_tim slot at c_creature word@2. */
    prior_timer = (int16_t)dm2_v1_game_load_owner_read_u16le(slot + 2u);
    if (prior_timer >= 0) {
        if (prior_timer >= owner->timer_capacity ||
            owner->timer_entries[prior_timer].ttype == 0u) goto rollback;
        dm2_v1_timer_delete(&owner->timer_queue, prior_timer);
        receipt.replaced_pending_timer = 1;
    }
    dm2_v1_timer_entry_init(&timer);
    dm2_v1_timer_set_mticks(&timer, (int16_t)map,
                             owner->timer_queue.gametick + 1);
    timer.ttype = dm2_v1_game_load_owner_read_u16le(record + 8u) != 0xffffu
        ? 0x22u : 0x21u;
    timer.actor = record[4];
    timer.xA = (int8_t)x;
    timer.yA = (int8_t)y;
    queued_timer = dm2_v1_timer_queue(&owner->timer_queue, &timer);
    if (queued_timer < 0) goto rollback;
    dm2_v1_game_load_owner_write_u16le(slot + 2u, (uint16_t)queued_timer);
    if (owner->caii_rng.random != rng_backup.random) goto rollback;

    receipt.valid = 1;
    receipt.record_handle = record_handle;
    receipt.caii_slot = (int16_t)slot_index;
    receipt.timer_slot = queued_timer;
    receipt.timer_type = timer.ttype;
    receipt.creature_type = record[4];
    receipt.map = (int16_t)map;
    receipt.x = (uint8_t)x;
    receipt.y = (uint8_t)y;
    receipt.due_tick = (uint32_t)dm2_v1_timer_get_ticks(&timer);
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

rollback:
    if (timer_backup && index_backup) {
        memcpy(owner->timer_entries, timer_backup,
               (size_t)owner->timer_capacity * sizeof(*timer_backup));
        memcpy(owner->timer_indices, index_backup,
               (size_t)owner->timer_capacity * sizeof(*index_backup));
        owner->timer_queue = queue_backup;
        memcpy(slot, slot_backup, sizeof(slot_backup));
        owner->caii_rng = rng_backup;
    }
    free(index_backup);
    free(timer_backup);
    return 0;
}

int dm2_v1_game_load_world_owner_materialize_dynamic_caii(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadCaiiDynamicReceipt *out_receipt)
{
    DM2_V1_GameLoadCaiiDynamicReceipt receipt;
    DM2_V1_RecordPool *db4;
    uint8_t *db4_snapshot = NULL;
    uint8_t *slots_snapshot = NULL;
    DM2_V1_TimerEntry *timers_snapshot = NULL;
    int16_t *indices_snapshot = NULL;
    DM2_V1_TimerQueue queue_snapshot = {0};
    DM2_V1_DropRng rng_snapshot = {0};
    DM2_V1_SoundQueueState sound_state;
    DM2_V1_SoundSfx positional_snapshot[DM2_V1_SOUND_POSITIONAL_CAP];
    DM2_V1_SoundSfx immediate_snapshot[DM2_V1_SOUND_IMMEDIATE_CAP];
    DM2_V1_SoundDelayedSlot delayed_snapshot[DM2_V1_SOUND_DELAYED_SLOT_COUNT];
    int32_t sample_slots_snapshot[DM2_V1_SOUND_SAMPLE_SLOT_COUNT];
    uint16_t positional_count_snapshot = 0u;
    uint16_t immediate_count_snapshot = 0u;
    size_t db4_bytes;
    uint32_t hash = 0x4344494eu; /* "CDIN" */
    int index;

    memset(&receipt, 0, sizeof(receipt));
    receipt.failed_record_handle = -1;
    receipt.failed_map = -1;
    if (out_receipt) *out_receipt = receipt;
    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->caii_static_animation.valid || !owner->caii_slots.valid ||
        !owner->caii_rng_initialized || !owner->caii_map_candidates ||
        !owner->timer_entries || !owner->timer_indices ||
        owner->caii_dynamic.valid) {
        return 0;
    }

    receipt.valid = 1;

    /* RESET_CAII precedes this all-map walk, so static materialisation has
     * already reset every DB4 owner byte.  The dynamic half must now either
     * allocate every source candidate with its 0cf7/0a48 consequences or
     * restore every mutable owner.  A per-creature partial success would not
     * be a GAME_LOAD state. Source: startend.cpp::DM2_RESET_CAII (1111-1145),
     * SK1C9A.cpp::DM2_FILL_ORPHAN_CAII (10000-10031). */
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 16 || db4->record_count <= 0 ||
        !owner->caii_local_context_receipt.valid ||
        !owner->caii_local_contexts || !owner->sound_owner.spatial_context_valid ||
        owner->caii_local_context_receipt.context_count !=
            owner->caii_map_receipt.dynamic_candidate_count) {
        return 0;
    }
    db4_bytes = (size_t)db4->record_count * (size_t)db4->record_size;
    db4_snapshot = malloc(db4_bytes);
    slots_snapshot = malloc((size_t)owner->caii_slots.capacity *
                            DM2_V1_CAII_SLOT_SIZE);
    timers_snapshot = malloc((size_t)owner->timer_capacity *
                             sizeof(*timers_snapshot));
    indices_snapshot = malloc((size_t)owner->timer_capacity *
                              sizeof(*indices_snapshot));
    if (!db4_snapshot || !slots_snapshot || !timers_snapshot ||
        !indices_snapshot) goto rollback;
    memcpy(db4_snapshot, db4->bytes, db4_bytes);
    memcpy(slots_snapshot, owner->caii_slots.slots,
           (size_t)owner->caii_slots.capacity * DM2_V1_CAII_SLOT_SIZE);
    memcpy(timers_snapshot, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*timers_snapshot));
    memcpy(indices_snapshot, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*indices_snapshot));
    queue_snapshot = owner->timer_queue;
    rng_snapshot = owner->caii_rng;
    positional_count_snapshot = owner->sound_owner.positional_count;
    immediate_count_snapshot = owner->sound_owner.immediate_count;
    memcpy(positional_snapshot, owner->sound_owner.positional,
           sizeof(positional_snapshot));
    memcpy(immediate_snapshot, owner->sound_owner.immediate,
           sizeof(immediate_snapshot));
    memcpy(delayed_snapshot, owner->sound_owner.delayed,
           sizeof(delayed_snapshot));
    memcpy(sample_slots_snapshot, owner->sound_owner.sample_slots,
           sizeof(sample_slots_snapshot));

    dm2_v1_sound_queue_state_init(&sound_state, 0u);
    if (!dm2_v1_sound_queue_bind_entries(&sound_state,
                                         owner->sound_owner.queue_entries,
                                         owner->sound_owner.queue_entry_count,
                                         owner->sound_owner.queue_capacity)) {
        goto rollback;
    }
    sound_state.positional_count = owner->sound_owner.positional_count;
    sound_state.immediate_count = owner->sound_owner.immediate_count;
    sound_state.sound_enabled = owner->sound_owner.sound_enabled;
    sound_state.master_sfx_volume = owner->sound_owner.master_sfx_volume;
    memcpy(sound_state.positional, owner->sound_owner.positional,
           sizeof(sound_state.positional));
    memcpy(sound_state.immediate, owner->sound_owner.immediate,
           sizeof(sound_state.immediate));
    memcpy(sound_state.delayed, owner->sound_owner.delayed,
           sizeof(sound_state.delayed));
    memcpy(sound_state.sample_slots, owner->sound_owner.sample_slots,
           sizeof(sound_state.sample_slots));

    for (index = 0; index < owner->caii_map_receipt.candidate_count; ++index) {
        const DM2_V1_GameLoadCaiiMapCandidate *candidate =
            &owner->caii_map_candidates[index];
        const DM2_AIDefinition *ai = NULL;
        uint8_t *record;
        uint8_t *slot = NULL;
        int slot_index;
        int16_t adj[2];
        const uint8_t *animation = NULL;
        DM2_V1_GameLoadCaiiThinkReceipt think;
        DM2_V1_CreatureSomethingReceipt something;


        if (candidate->static_ai) continue;
        record = dm2_v1_record_pool_address_mut(&owner->record_pools,
                                                candidate->record_handle);
        if (!record) {
            goto rollback;
        }
        /* FILL_ORPHAN_CAII visits map roots, but the same DB4 object may be
         * reachable from more than one authenticated ground stack. Once its
         * owner byte is assigned, the source skips that duplicate rather
         * than allocating a second CAII slot. */
        if (record[5] != 0xffu) continue;
        if (record[4] != candidate->creature_type ||
            !dm2_v1_caii_source_owner_ai_spec_def(&owner->caii_source,
                                                   record[4], &ai) || !ai ||
            (ai->w0AIFlags & 1u) != 0u) {
            goto rollback;
        }
        ++receipt.dynamic_candidate_count;
        for (slot_index = 0; slot_index < owner->caii_slots.capacity;
             ++slot_index) {
            slot = owner->caii_slots.slots +
                (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE;
            if ((int16_t)dm2_v1_game_load_owner_read_u16le(slot) < 0) break;
        }
        if (slot_index >= owner->caii_slots.capacity) {
            goto rollback;
        }
        memset(slot, 0, DM2_V1_CAII_SLOT_SIZE);
        dm2_v1_game_load_owner_write_u16le(slot,
            (uint16_t)candidate->record_handle & 0x03ffu);
        dm2_v1_game_load_owner_write_u16le(slot + 2u, 0xffffu);
        slot[4] = (uint8_t)(owner->timer_queue.gametick - 0x7f);
        slot[6] = (uint8_t)((owner->timer_queue.gametick >> 2) - 1);
        dm2_v1_game_load_owner_write_u16le(slot + 12u,
            (uint16_t)((candidate->x & 0x1fu) |
                       ((candidate->y & 0x1fu) << 5) |
                       ((candidate->map & 0x3fu) << 10)));
        /* SK1C9A.cpp::DM2_14cd_0802, called through
         * PREPARE_LOCAL_CREATURE_VAR before 0cf7. */
        slot[0x12] = 0xffu;
        slot[0x16] = 0xffu;
        slot[0x17] = 0xffu;
        slot[0x1a] = 0xffu;
        record[5] = (uint8_t)slot_index;
        ++owner->caii_slots.alloc_count;
        ++receipt.allocated_slot_count;
        memset(&think, 0, sizeof(think));
        if (!dm2_v1_game_load_world_owner_schedule_caii_think(
            owner, candidate->record_handle, candidate->map,
                candidate->x, candidate->y, &think) || !think.valid) {
            goto rollback;
        }
        ++receipt.think_timer_count;
        slot[0x1a] = dm2_v1_game_load_owner_read_u16le(record + 8u) != 0xffffu
            ? 0u : 0x11u;
        /* DM2_ALLOC_CAII_TO_CREATURE sets the dynamic creature's 0x8000
         * state and clears 0x4000 immediately before 0a48. */
        record[11] = (uint8_t)((record[11] | 0x80u) & ~0x40u);
        adj[0] = (int16_t)dm2_v1_game_load_owner_read_u16le(slot + 8u);
        adj[1] = (int16_t)dm2_v1_game_load_owner_read_u16le(slot + 10u);
        memset(&something, 0, sizeof(something));
        {
            int something_result =
                dm2_v1_creature_something_1c9a_0a48_with_ai_spec(
            &owner->record_pools, &owner->caii_slots, owner->asset_loader,
            ai, &owner->caii_rng, candidate->record_handle, adj,
            &animation, candidate->map, owner->preselection_entrance.map,
            candidate->record_word_a & 0x003fu, 0, 0,
            candidate->x, candidate->y,
                (unsigned long)owner->timer_queue.gametick, &something);
            if (something_result < 0 || !something.valid) {
                receipt.failed_record_handle = candidate->record_handle;
                receipt.failed_creature_type = candidate->creature_type;
                receipt.failed_map = candidate->map;
                receipt.failed_x = candidate->x;
                receipt.failed_y = candidate->y;
                receipt.failed_0a48_result = something_result;
                goto rollback;
            }
        }
        dm2_v1_game_load_owner_write_u16le(slot + 8u, (uint16_t)adj[0]);
        dm2_v1_game_load_owner_write_u16le(slot + 10u, (uint16_t)adj[1]);
        if (something.noise_would_queue && !owner->dungeon.words_big_endian) {
            DM2_V1_SoundQueueEnv env;
            DM2_V1_SoundQueueReceipt noise;
            memset(&env, 0, sizeof(env));
            memset(&noise, 0, sizeof(noise));
            env.current_map = candidate->map;
            env.gate_map_a = owner->sound_owner.spatial_audible_map;
            env.gate_map_b = owner->sound_owner.spatial_alternate_map;
            env.facing = owner->source_party_direction;
            env.party_x = owner->source_party_x;
            env.party_y = owner->source_party_y;
            env.gametick = owner->timer_queue.gametick;
            if (!dm2_v1_sound_queue_noise_gen1(
                    &sound_state, 0x0f, (int8_t)record[4],
                    (int8_t)something.noise_index, 0x46, 0x80,
                    candidate->x, candidate->y, 1, &env, &noise)) {
                /* A non-audible map gate is the source's normal no-op.
                 * Missing occlusion, class triple or sample ownership is
                 * not; preserve the whole GAME_LOAD candidate instead. */
                if (!noise.rejected_map_gate) goto rollback;
                ++receipt.noise_map_gate_count;
            } else {
                ++receipt.noise_queue_count;
            }
        }
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                 (uint16_t)candidate->map);
        hash = dm2_v1_game_load_owner_hash_step(hash, candidate->record_handle);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)slot_index);
        hash = dm2_v1_game_load_owner_hash_step(hash, think.timer_type);
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                 (uint16_t)something.delta);
    }
    if (receipt.dynamic_candidate_count !=
        owner->caii_map_receipt.dynamic_candidate_count) {
        goto rollback;
    }
    owner->sound_owner.positional_count = sound_state.positional_count;
    owner->sound_owner.immediate_count = sound_state.immediate_count;
    memcpy(owner->sound_owner.positional, sound_state.positional,
           sizeof(owner->sound_owner.positional));
    memcpy(owner->sound_owner.immediate, sound_state.immediate,
           sizeof(owner->sound_owner.immediate));
    memcpy(owner->sound_owner.delayed, sound_state.delayed,
           sizeof(owner->sound_owner.delayed));
    memcpy(owner->sound_owner.sample_slots, sound_state.sample_slots,
           sizeof(owner->sound_owner.sample_slots));
    if (owner->caii_slots.alloc_count !=
        (int)receipt.dynamic_candidate_count ||
        receipt.allocated_slot_count != receipt.dynamic_candidate_count ||
        receipt.think_timer_count != receipt.dynamic_candidate_count ||
        hash == 0u) {
        goto rollback;
    }
    receipt.source_hash = dm2_v1_game_load_owner_hash_step(
        hash, receipt.noise_queue_count);
    if (receipt.source_hash == 0u) goto rollback;
    receipt.valid = 1;
    owner->caii_dynamic = receipt;
    if (out_receipt) *out_receipt = receipt;
    free(indices_snapshot);
    free(timers_snapshot);
    free(slots_snapshot);
    free(db4_snapshot);
    return 1;

rollback:
    if (db4_snapshot) memcpy(db4->bytes, db4_snapshot, db4_bytes);
    if (slots_snapshot) memcpy(owner->caii_slots.slots, slots_snapshot,
                               (size_t)owner->caii_slots.capacity *
                               DM2_V1_CAII_SLOT_SIZE);
    if (timers_snapshot) memcpy(owner->timer_entries, timers_snapshot,
                                (size_t)owner->timer_capacity *
                                sizeof(*timers_snapshot));
    if (indices_snapshot) memcpy(owner->timer_indices, indices_snapshot,
                                 (size_t)owner->timer_capacity *
                                 sizeof(*indices_snapshot));
    if (db4_snapshot && slots_snapshot && timers_snapshot && indices_snapshot) {
        owner->timer_queue = queue_snapshot;
        owner->caii_rng = rng_snapshot;
        owner->sound_owner.positional_count = positional_count_snapshot;
        owner->sound_owner.immediate_count = immediate_count_snapshot;
        memcpy(owner->sound_owner.positional, positional_snapshot,
               sizeof(positional_snapshot));
        memcpy(owner->sound_owner.immediate, immediate_snapshot,
               sizeof(immediate_snapshot));
        memcpy(owner->sound_owner.delayed, delayed_snapshot,
               sizeof(delayed_snapshot));
        memcpy(owner->sound_owner.sample_slots, sample_slots_snapshot,
               sizeof(sample_slots_snapshot));
    }
    receipt.blocked_unowned_0a48 = 1;
    if (out_receipt) *out_receipt = receipt;
    free(indices_snapshot);
    free(timers_snapshot);
    free(slots_snapshot);
    free(db4_snapshot);
    return 0;
}

int dm2_v1_game_load_world_owner_materialize_caii_local_context(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadCaiiLocalContext *contexts;
    DM2_V1_GameLoadCaiiLocalContextReceipt receipt;
    const int home_map = owner ? owner->preselection_entrance.map : -1;
    uint32_t hash = 0x30413438u; /* "0A48" */
    int index;
    int count = 0;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->caii_source.valid || !owner->caii_map_receipt.valid ||
        !owner->caii_map_candidates || !owner->sound_owner.valid ||
        !owner->sound_owner.runtime_queue_initialized ||
        owner->caii_local_contexts || owner->caii_local_context_receipt.valid ||
        home_map < 0 || home_map >= owner->dungeon.level_count ||
        owner->caii_map_receipt.dynamic_candidate_count == 0u) return 0;
    contexts = calloc(owner->caii_map_receipt.dynamic_candidate_count,
                      sizeof(*contexts));
    if (!contexts) return 0;
    memset(&receipt, 0, sizeof(receipt));

    for (index = 0; index < owner->caii_map_receipt.candidate_count; ++index) {
        const DM2_V1_GameLoadCaiiMapCandidate *candidate =
            &owner->caii_map_candidates[index];
        const DM2_AIDefinition *ai = NULL;
        const uint8_t *record;
        DM2_V1_GameLoadCaiiLocalContext *context;

        if (candidate->static_ai) continue;
        if (count >= owner->caii_map_receipt.dynamic_candidate_count ||
            !(record = dm2_v1_record_pool_address(&owner->record_pools,
                                                   candidate->record_handle)) ||
            record[4] != candidate->creature_type || record[5] != 0xffu ||
            !dm2_v1_caii_source_owner_ai_spec_def(&owner->caii_source,
                                                   record[4], &ai) || !ai ||
            (ai->w0AIFlags & 1u) != 0u) {
            free(contexts);
            return 0;
        }
        context = &contexts[count++];
        context->record_handle = candidate->record_handle;
        context->map = candidate->map;
        context->x = candidate->x;
        context->y = candidate->y;
        context->creature_type = record[4];
        context->ai_flags = ai->w0AIFlags;
        context->record_word_a = owner->dungeon.source_words_big_endian
            ? (uint16_t)(((uint16_t)record[10] << 8) | record[11])
            : dm2_v1_game_load_owner_read_u16le(record + 10u);
        context->packed_position = owner->dungeon.source_words_big_endian
            ? (uint16_t)(((uint16_t)record[12] << 8) | record[13])
            : dm2_v1_game_load_owner_read_u16le(record + 12u);
        context->initial_timer_type =
            (owner->dungeon.source_words_big_endian
                ? (uint16_t)(((uint16_t)record[8] << 8) | record[9])
                : dm2_v1_game_load_owner_read_u16le(record + 8u)) == 0xffffu ?
                0x21u : 0x22u;
        context->home_map = (int16_t)home_map;
        /* DM2_query_1c9a_02c3 identifies the two-word c_creature sequence
         * pair used by 0a48. Dynamic candidates obtain that pair only after
         * original slot allocation has zeroed it, so this context owns the
         * identity and source initial values but no fabricated slot. */
        context->adj_owner_offset = 8u;
        context->adj_base_before = 0u;
        context->adj_frame_before = 0u;
        /* c_1c9a.cpp:5539-5561 forms QUEUE_NOISE_GEN1 only after the real
         * GAF row yields byte@0. Retain a pending requirement, never a
         * request with a sentinel animation index. */
        context->noise_request_pending = 1;
        hash = dm2_v1_game_load_owner_hash_step(hash,
            (uint16_t)context->record_handle);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)context->map);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->x);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->y);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->creature_type);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->ai_flags);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->record_word_a);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->packed_position);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->initial_timer_type);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)context->home_map);
        hash = dm2_v1_game_load_owner_hash_step(hash, context->adj_owner_offset);
        context->source_hash = hash;
        context->valid = 1;
        ++receipt.noise_request_pending_count;
    }
    if (count != owner->caii_map_receipt.dynamic_candidate_count || hash == 0u)
        goto fail;
    receipt.context_count = (uint16_t)count;
    receipt.source_hash = dm2_v1_game_load_owner_hash_step(hash,
                                                            receipt.context_count);
    if (receipt.source_hash == 0u) goto fail;
    receipt.valid = 1;
    owner->caii_local_contexts = contexts;
    owner->caii_local_context_receipt = receipt;
    return 1;
fail:
    free(contexts);
    return 0;
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

    if (!owner ||
        (!owner->source_preselection_ready &&
         (!owner->transaction.valid || !owner->transaction.incomplete_game_load)) ||
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
        candidate.receipt_hash, owner->source_preselection_ready
            ? owner->preselection_hash : owner->transaction.transaction_hash);
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
    const DM2_V1_BootChampionDyn4RosterReceipt *roster;
    DM2_V1_BootChampionSelectionCensus census;
    DM2_V1_GdatDyn4SoundState sound_state;
    uint32_t hash = 2166136261u;
    int i;

    if (!owner || !owner->asset_loader) return 0;
    roster = owner->source_preselection_ready
        ? &owner->preselection_dyn4_roster : &owner->transaction.dyn4_roster;
    if (!roster->valid || !roster->incomplete_champion_activation ||
        roster->selector_count <= 0 || roster->selector_count >
            DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES) return 0;
    memset(&census, 0, sizeof(census));
    if (!dm2_v1_boot_champion_selection_census(
            owner->boot_profile, &census) || !census.valid ||
        census.candidate_count != roster->selector_count) {
        return 0;
    }
    dm2_v1_gdat_dyn4_sound_state_init(&sound_state);
    for (i = 0; i < roster->selector_count; ++i) {
        const DM2_V1_GdatDyn4SelectionReceipt *source =
            &roster->selections[i];
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
    owner->dyn4_selector_count = (uint16_t)roster->selector_count;
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

static int dm2_v1_game_load_owner_dyn4_has_raw_selection(
    const DM2_V1_GdatDyn4MaterializedSelection *selections,
    uint16_t selection_count, uint16_t raw_index)
{
    uint16_t selector;

    if (!selections || selection_count == 0u) return 0;
    for (selector = 0u; selector < selection_count; ++selector) {
        const DM2_V1_GdatDyn4MaterializedSelection *selection =
            &selections[selector];
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

void dm2_v1_game_load_sound_owner_free(
    DM2_V1_GameLoadSoundOwner *sound)
{
    if (!sound) return;
    free(sound->queue_entries);
    free(sound->sample_bindings);
    memset(sound, 0, sizeof(*sound));
}

uint16_t dm2_v1_game_load_sound_owner_query_entry(
    const DM2_V1_GameLoadSoundOwner *owner,
    int8_t cls1, int8_t cls2, int8_t cls3)
{
    uint16_t index;

    if (!owner || !owner->valid || !owner->runtime_queue_initialized ||
        !owner->queue_entries || !owner->sample_bindings ||
        owner->queue_entry_count == 0u ||
        owner->queue_entry_count > owner->queue_capacity ||
        owner->sample_binding_count == 0u ||
        owner->sample_binding_count > owner->sample_capacity) {
        return 0u;
    }
    /* c_sound.cpp::DM2_QUERY_SND_ENTRY_INDEX is a 1-based linear scan.
     * Reject an entry whose 482b binding no longer points to the admitted
     * raw block rather than returning a stale source index. */
    for (index = 0u; index < owner->queue_entry_count; ++index) {
        const DM2_V1_SoundSsoundEntry *entry = &owner->queue_entries[index];
        uint16_t binding;

        if (entry->b_02 != cls1 || entry->b_03 != cls2 ||
            entry->b_04 != cls3) {
            continue;
        }
        if (entry->w_00 < 0 || entry->w_05 < 0 ||
            (uint16_t)entry->w_00 >= owner->sample_binding_count) {
            return 0u;
        }
        binding = (uint16_t)entry->w_00;
        if (owner->sample_bindings[binding].raw_index !=
                (uint16_t)entry->w_05 ||
            owner->sample_bindings[binding].raw_length == 0u ||
            owner->sample_bindings[binding].source_payload_hash == 0u) {
            return 0u;
        }
        return (uint16_t)(index + 1u);
    }
    return 0u;
}

int dm2_v1_game_load_sound_owner_queue_noise_gen2(
    DM2_V1_GameLoadSoundOwner *owner,
    int8_t cls1, int8_t cls2, int8_t cls3, int8_t cls_alt,
    int16_t x, int16_t y, int16_t ecxw, int16_t volume, int16_t delay_mode,
    const DM2_V1_SoundQueueEnv *env,
    DM2_V1_GameLoadSoundNoiseReceipt *out_receipt)
{
    DM2_V1_GameLoadSoundNoiseReceipt receipt;
    DM2_V1_SoundQueueReceipt noise;
    DM2_V1_SoundQueueState state;
    DM2_V1_SoundSfx positional[DM2_V1_SOUND_POSITIONAL_CAP];
    DM2_V1_SoundSfx immediate[DM2_V1_SOUND_IMMEDIATE_CAP];
    DM2_V1_SoundDelayedSlot delayed[DM2_V1_SOUND_DELAYED_SLOT_COUNT];
    int32_t sample_slots[DM2_V1_SOUND_SAMPLE_SLOT_COUNT];
    uint16_t positional_count;
    uint16_t immediate_count;
    int queued;

    memset(&receipt, 0, sizeof(receipt));
    memset(&noise, 0, sizeof(noise));
    if (out_receipt) *out_receipt = receipt;
    if (!owner || !owner->valid || !owner->runtime_queue_initialized) {
        receipt.blocked_owner = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!owner->queue_entries || owner->queue_entry_count == 0u ||
        owner->queue_entry_count > owner->queue_capacity ||
        !owner->sample_bindings || owner->sample_binding_count == 0u ||
        owner->sample_binding_count > owner->sample_capacity) {
        receipt.blocked_queue = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    /* Adapt only the fixed c_sfx state.  GAME_LOAD's authenticated dynamic
     * xsndptr2 span remains bound as the compatibility state's backing. */
    dm2_v1_sound_queue_state_init(&state, 0u);
    if (!dm2_v1_sound_queue_bind_entries(&state, owner->queue_entries,
                                         owner->queue_entry_count,
                                         owner->queue_capacity)) {
        receipt.blocked_queue = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    positional_count = owner->positional_count;
    immediate_count = owner->immediate_count;
    memcpy(positional, owner->positional, sizeof(positional));
    memcpy(immediate, owner->immediate, sizeof(immediate));
    memcpy(delayed, owner->delayed, sizeof(delayed));
    memcpy(sample_slots, owner->sample_slots,
           sizeof(sample_slots));
    state.positional_count = positional_count;
    state.immediate_count = immediate_count;
    state.sound_enabled = owner->sound_enabled;
    state.master_sfx_volume = owner->master_sfx_volume;
    memcpy(state.positional, positional, sizeof(positional));
    memcpy(state.immediate, immediate, sizeof(immediate));
    memcpy(state.delayed, delayed, sizeof(delayed));
    memcpy(state.sample_slots, sample_slots, sizeof(sample_slots));

    queued = dm2_v1_sound_queue_noise_gen2(
        &state, cls1, cls2, cls3, cls_alt, x, y, ecxw, volume, delay_mode,
        env, &noise);
    if (!queued) {
        receipt.blocked_queue = noise.rejected_positional_full ||
            noise.rejected_immediate_full || noise.rejected_delayed_full ||
            noise.rejected_map_gate || noise.rejected_facing_unproven ||
            noise.rejected_occlusion_unavailable ||
            noise.rejected_occlusion_blocked;
        receipt.blocked_sample = noise.rejected_query_miss ||
            noise.rejected_sample_unresolved;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    owner->positional_count = state.positional_count;
    owner->immediate_count = state.immediate_count;
    memcpy(owner->positional, state.positional, sizeof(owner->positional));
    memcpy(owner->immediate, state.immediate, sizeof(owner->immediate));
    memcpy(owner->delayed, state.delayed, sizeof(owner->delayed));
    memcpy(owner->sample_slots, state.sample_slots,
           sizeof(owner->sample_slots));
    receipt.valid = noise.valid;
    receipt.queued_positional = noise.queued_positional;
    receipt.queue_index = noise.queue_index;
    receipt.sample_id = noise.sample_id;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        owner->receipt_hash,
        (uint32_t)(uint16_t)cls1 ^ ((uint32_t)(uint16_t)cls2 << 8) ^
        ((uint32_t)(uint16_t)cls3 << 16) ^ ((uint32_t)(uint16_t)cls_alt << 24));
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        receipt.transaction_hash,
        (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        receipt.transaction_hash,
        (uint32_t)(uint16_t)ecxw ^ ((uint32_t)(uint16_t)volume << 16) ^
        (uint32_t)(uint16_t)delay_mode ^ noise.queue_index);
    if (receipt.transaction_hash == 0u) receipt.valid = 0;
    if (!receipt.valid) {
        owner->positional_count = positional_count;
        owner->immediate_count = immediate_count;
        memcpy(owner->positional, positional, sizeof(positional));
        memcpy(owner->immediate, immediate, sizeof(immediate));
        memcpy(owner->delayed, delayed, sizeof(delayed));
        memcpy(owner->sample_slots, sample_slots,
               sizeof(sample_slots));
    }
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

static void dm2_v1_game_load_owner_light_visibility_free(
    DM2_V1_GameLoadLightVisibilityOwner *visibility)
{
    if (!visibility) return;
    free(visibility->primary_cells);
    free(visibility->secondary_cells);
    memset(visibility, 0, sizeof(*visibility));
}

/* c_sound::init owns the fixed sfx tables, while c_dballoc owns the dynamic
 * xsndptr2 capacity.  Preserve both shapes: allocating a 64-entry substitute
 * for xsndptr2 would silently discard hash-admitted SOUND9 rows.
 * Source: SKProject SKULLWIN/c_sound.cpp::c_sound::init (32-63),
 * c_gdatfile.cpp::DM2_dballoc_3e74_24b8 (273-333). */
static int dm2_v1_game_load_owner_sound_init_runtime_state(
    DM2_V1_GameLoadSoundOwner *sound)
{
    uint16_t slot;

    if (!sound || !sound->queue_entries || sound->queue_capacity == 0u ||
        sound->queue_entry_count == 0u || sound->runtime_queue_initialized ||
        sound->positional_count != 0u || sound->immediate_count != 0u) {
        return 0;
    }
    for (slot = 0u; slot < DM2_V1_SOUND_SAMPLE_SLOT_COUNT; ++slot) {
        sound->sample_slots[slot] = -1;
    }
    sound->sound_enabled = 1;
    sound->master_sfx_volume = 7;
    sound->runtime_queue_initialized = 1;
    return 1;
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
    if (!owner) return 0;
    return dm2_v1_game_load_sound_owner_materialize_from_dyn4(
        owner->asset_loader, owner->dyn4_selector_count,
        owner->dyn4_selector_ids, owner->dyn4_selections,
        &owner->sound_owner);
}

int dm2_v1_game_load_sound_owner_materialize_from_dyn4(
    const DM2_V1_AssetLoader *loader, uint16_t selector_count,
    const uint32_t *selector_ids,
    const DM2_V1_GdatDyn4MaterializedSelection *selections,
    DM2_V1_GameLoadSoundOwner *out_owner)
{
    DM2_V1_GameLoadSoundOwner candidate;
    const int fmtowns = loader &&
        loader->gdat_version == DM2_FMTOWNS_GDAT_VERSION;
    uint16_t selector;

    if (!loader || !out_owner || !selector_ids || !selections ||
        !loader->loaded || selector_count == 0u) return 0;
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
    for (selector = 0u; selector < selector_count; ++selector) {
        uint16_t ordinal;
        const uint32_t selector_id = selector_ids[selector];
        if (selector_id == 0u) goto fail;
        for (ordinal = 0u; ordinal < loader->entry_count; ++ordinal) {
            const DM2_V1_GdatEntry *entry = &loader->entries[ordinal];
            uint16_t raw_index;
            uint16_t existing;

            if (entry->cls3 != DM2_GDAT_ENTRY_TYPE_SOUND ||
                (!fmtowns && !dm2_v1_game_load_owner_dyn4_matches_selector(entry,
                                                               selector_id)) ||
                (!fmtowns && (entry->data_index & 0x8000u) != 0u)) {
                continue;
            }
            raw_index = fmtowns ? (uint16_t)(entry->data_index & 0x7fffu) :
                                  entry->data_index;
            if (!fmtowns && !dm2_v1_game_load_owner_dyn4_has_raw_selection(
                    selections, selector_count, raw_index)) {
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
            (!fmtowns && !dm2_v1_game_load_owner_dyn4_has_raw_selection(
                selections, selector_count, source.raw_index))) {
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
    if (!candidate.valid ||
        !dm2_v1_game_load_owner_sound_init_runtime_state(&candidate)) {
        goto fail;
    }
    *out_owner = candidate;
    return 1;

fail:
    dm2_v1_game_load_sound_owner_free(&candidate);
    return 0;
}

static int dm2_v1_game_load_owner_validate_world_maps(
    DM2_V1_GameLoadWorldOwner *owner)
{
    const DM2_V1_FileHeaderWorldInteractionReceipt *world;
    uint32_t hash = 2166136261u;
    int total_records = 0;
    int map;

    if (!owner) return 0;
    world = owner->source_preselection_ready
        ? &owner->preselection_world_interactions
        : &owner->transaction.world_interactions;
    if (!world->valid || !world->incomplete_world ||
        owner->dungeon.level_count != world->map_count) return 0;
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
    /* The Macintosh File_header census intentionally omits maps whose
     * generic object chains have no decoded optional projection.  The
     * runtime-map receipts still validate those source records, so compare
     * the aggregate only for little-endian profiles where the census is
     * complete. */
    if (!owner->dungeon.words_big_endian && total_records != world->total_records) {
        return 0;
    }
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
    owner->source_teleporter_probe_direction = -1;
    owner->source_teleporter_source_direction = 0;
    owner->source_teleporter_destination_direction = 0;
    owner->source_display_pose_valid = 0;
    if (dm2_v1_game_load_owner_get_teleporter_detail(owner, map, x, y,
                                                      &detail)) {
        owner->source_staircase_flag = 1;
        owner->source_teleporter_map = detail.b_04;
        owner->source_display_x = detail.b_02;
        owner->source_display_y = detail.b_03;
        owner->source_teleporter_source_direction = detail.b_00;
        owner->source_teleporter_destination_direction = detail.b_01;
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
            owner->source_teleporter_probe_direction =
                (int8_t)probe_direction;
            owner->source_teleporter_source_direction = detail.b_00;
            owner->source_teleporter_destination_direction = detail.b_01;
            owner->source_party_absdir = (uint8_t)((detail.b_01 + 6 -
                                                     detail.b_00 + direction) & 3);
            owner->source_display_pose_valid = 1;
            break;
        }
    }
    /* DM2_move_2fcf_0b8b forces v1d3248=-1, then its final
     * CHANGE_CURRENT_MAP_TO restores the selected map. This is c_map state,
     * not a moved-record ObjectID. */
    dm2_v1_game_load_moverec_state_init(&owner->source_moverec, map);
    return 1;
}

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner)
{
    int i;
    if (!owner) return;
    if (!dm2_v1_game_load_world_owner_is_initialized(owner)) {
        /* Public constructors accept unknown caller storage.  Do not ever
         * interpret arbitrary bytes as owned allocation pointers. */
        memset(owner, 0, sizeof(*owner));
        return;
    }
    for (i = 0; i < DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES; ++i) {
        dm2_v1_gdat_dyn4_materialized_selection_free(&owner->dyn4_selections[i]);
    }
    dm2_v1_gdat_scene_m11_command_plan_free(&owner->preselection_scene_plan);
    dm2_v1_game_load_owner_light_visibility_free(
        &owner->preselection_light_visibility);
    dm2_v1_game_load_sound_owner_free(&owner->sound_owner);
    dm2_v1_caii_array_free(&owner->caii_slots);
    dm2_v1_caii_source_owner_free(&owner->caii_source);
    free(owner->caii_map_candidates);
    free(owner->caii_local_contexts);
    free(owner->timer_indices);
    free(owner->timer_entries);
    dm2_v1_record_pool_set_free(&owner->record_pools);
    dm2_v1_dungeon_free(&owner->dungeon);
    memset(owner, 0, sizeof(*owner));
}

static void dm2_v1_game_load_runtime_candidate_sound_free(
    DM2_V1_GameLoadSoundOwner *sound)
{
    dm2_v1_game_load_sound_owner_free(sound);
}

/* c_dballoc allocates the source reserve after the static File_header pools
 * have been copied.  DB14/DB15 are therefore allowed to have zero static
 * records while still owning a source-sized runtime span. */
static int dm2_v1_game_load_runtime_candidate_materialize_dynamic_pools(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    const DM2_V1_GameLoadWorldOwner *source)
{
    for (int type = 14; type <= 15; ++type) {
        DM2_V1_RecordPool *pool = &candidate->record_pools.pools[type];
        uint16_t capacity = source->record_capacities[type];
        uint8_t *bytes;
        size_t old_bytes;
        size_t new_bytes;

        if (capacity == 0u || pool->record_size <= 0 ||
            pool->record_count < 0 || pool->record_count > (int)capacity ||
            pool->extension_count != 0) return 0;
        if (pool->record_count == (int)capacity) continue;
        old_bytes = (size_t)pool->record_count * (size_t)pool->record_size;
        new_bytes = (size_t)capacity * (size_t)pool->record_size;
        bytes = calloc(1u, new_bytes);
        if (!bytes) return 0;
        if (old_bytes > 0u && !pool->bytes) {
            free(bytes);
            return 0;
        }
        if (old_bytes > 0u) memcpy(bytes, pool->bytes, old_bytes);
        for (int index = pool->record_count; index < (int)capacity; ++index) {
            bytes[(size_t)index * (size_t)pool->record_size] = 0xffu;
            bytes[(size_t)index * (size_t)pool->record_size + 1u] = 0xffu;
        }
        free(pool->bytes);
        pool->bytes = bytes;
        pool->record_count = (int)capacity;
    }
    return 1;
}

static int dm2_v1_game_load_runtime_candidate_sound_clone(
    DM2_V1_GameLoadSoundOwner *out, const DM2_V1_GameLoadSoundOwner *source)
{
    return dm2_v1_game_load_sound_owner_clone(out, source);
}

static int dm2_v1_game_load_runtime_candidate_sksave_map_clone(
    DM2_V1_SksaveMapOwner *out, const DM2_V1_SksaveMapOwner *source)
{
    DM2_V1_SksaveMapOwner candidate;

    if (!out || !source || !source->valid || !source->dungeon ||
        source->column_index_count == 0u || !source->column_indices ||
        source->ground_stack_count == 0u || !source->ground_stack_links ||
        source->map_tiles_size == 0u || !source->map_tiles) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate = *source;
    candidate.column_indices = NULL;
    candidate.ground_stack_links = NULL;
    candidate.map_tiles = NULL;
    candidate.column_indices = malloc(source->column_index_count *
                                      sizeof(*candidate.column_indices));
    candidate.ground_stack_links = malloc(source->ground_stack_count *
                                          sizeof(*candidate.ground_stack_links));
    candidate.map_tiles = malloc(source->map_tiles_size);
    if (!candidate.column_indices || !candidate.ground_stack_links ||
        !candidate.map_tiles) {
        dm2_v1_sksave_map_owner_free(&candidate);
        return 0;
    }
    memcpy(candidate.column_indices, source->column_indices,
           source->column_index_count * sizeof(*candidate.column_indices));
    memcpy(candidate.ground_stack_links, source->ground_stack_links,
           source->ground_stack_count * sizeof(*candidate.ground_stack_links));
    memcpy(candidate.map_tiles, source->map_tiles, source->map_tiles_size);
    dm2_v1_sksave_map_owner_free(out);
    *out = candidate;
    return 1;
}

int dm2_v1_game_load_sound_owner_clone(
    DM2_V1_GameLoadSoundOwner *out, const DM2_V1_GameLoadSoundOwner *source)
{
    DM2_V1_GameLoadSoundOwner candidate;

    if (!out || !source || !source->valid ||
        !source->runtime_queue_initialized ||
        (source->queue_entry_count > source->queue_capacity) ||
        (source->sample_binding_count > source->sample_capacity) ||
        (source->queue_capacity != 0u && !source->queue_entries) ||
        (source->sample_capacity != 0u && !source->sample_bindings)) return 0;
    candidate = *source;
    candidate.queue_entries = NULL;
    candidate.sample_bindings = NULL;
    if (source->queue_capacity != 0u) {
        candidate.queue_entries = calloc(source->queue_capacity,
                                         sizeof(*candidate.queue_entries));
        if (!candidate.queue_entries) goto fail;
        memcpy(candidate.queue_entries, source->queue_entries,
               (size_t)source->queue_capacity * sizeof(*candidate.queue_entries));
    }
    if (source->sample_capacity != 0u) {
        candidate.sample_bindings = calloc(source->sample_capacity,
                                           sizeof(*candidate.sample_bindings));
        if (!candidate.sample_bindings) goto fail;
        memcpy(candidate.sample_bindings, source->sample_bindings,
               (size_t)source->sample_capacity * sizeof(*candidate.sample_bindings));
    }
    *out = candidate;
    return 1;
fail:
    dm2_v1_game_load_sound_owner_free(&candidate);
    return 0;
}

void dm2_v1_game_load_runtime_session_candidate_free(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate)
{
    if (!candidate) return;
    if (!dm2_v1_game_load_runtime_candidate_is_initialized(candidate)) {
        memset(candidate, 0, sizeof(*candidate));
        return;
    }
    dm2_v1_game_load_runtime_candidate_sound_free(&candidate->sound_owner);
    dm2_v1_sksave_map_owner_free(&candidate->sksave_map_owner);
    candidate->sksave_map_owner_valid = 0;
    dm2_v1_caii_array_free(&candidate->caii_slots);
    dm2_v1_caii_source_owner_free(&candidate->caii_source);
    free(candidate->timer_indices);
    free(candidate->timer_entries);
    free(candidate->local_dyn_map_scan.records);
    dm2_v1_record_pool_set_free(&candidate->record_pools);
    dm2_v1_dungeon_free(&candidate->dungeon);
    memset(candidate, 0, sizeof(*candidate));
}

int dm2_v1_game_load_runtime_session_candidate_is_valid(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate)
{
    if (!dm2_v1_game_load_runtime_candidate_is_initialized(candidate) ||
        !candidate->valid || candidate->candidate_hash == 0u ||
        candidate->source_transaction_hash == 0u ||
        !candidate->dungeon.raw_data || candidate->dungeon.raw_size <= 0 ||
        !candidate->dungeon.record_graph_complete ||
        !candidate->record_pools.valid ||
        !candidate->record_pools.record_graph_complete ||
        !candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u ||
        candidate->timer_queue.entries != candidate->timer_entries ||
        candidate->timer_queue.indices != candidate->timer_indices ||
        candidate->timer_queue.max_timers !=
            (int16_t)candidate->timer_capacity ||
        !candidate->caii_slots.valid || candidate->caii_slots.capacity <= 0 ||
        !candidate->caii_slots.slots || !candidate->caii_source.valid ||
        !candidate->asset_loader || !candidate->asset_loader->loaded ||
        !candidate->sound_owner.valid ||
        !candidate->sound_owner.runtime_queue_initialized ||
        !candidate->map_context.valid || candidate->party.heros_in_party <= 0 ||
        candidate->party.heros_in_party > DM2_MAX_HEROES ||
        candidate->source_event_hero_index < 0 ||
        candidate->source_event_hero_index >= candidate->party.heros_in_party ||
        candidate->event_queue.entries != 0 ||
        candidate->event_queue.idx != 0 || candidate->event_queue.out_idx != 0 ||
        candidate->event_queue.fetch_busy ||
        candidate->event_queue.singleevent_available ||
        candidate->event_queue.button0x2) {
        return 0;
    }
    return 1;
}

typedef struct {
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate;
    DM2_V1_SkprojectCreatureAISpec ai_spec;
    int owner_failed;
} DM2_V1_GameLoadSpatialQueryContext;

static int32_t dm2_v1_game_load_spatial_creature_at(int16_t x, int16_t y,
                                                     void *user)
{
    DM2_V1_GameLoadSpatialQueryContext *context = user;
    int16_t handle;

    if (!context || !context->candidate || x < 0 || y < 0 ||
        x >= context->candidate->map_context.width ||
        y >= context->candidate->map_context.height) {
        return -1;
    }
    handle = dm2_v1_get_creature_at(&context->candidate->record_pools,
                                    &context->candidate->dungeon,
                                    context->candidate->current_map, x, y);
    return handle == DM2_V1_RECORD_HANDLE_NULL ? -1 : (int32_t)handle;
}

static const uint8_t *dm2_v1_game_load_spatial_record(uint16_t handle,
                                                       uint16_t *out_size,
                                                       void *user)
{
    DM2_V1_GameLoadSpatialQueryContext *context = user;
    const DM2_V1_RecordPool *pool;
    const uint8_t *record;
    int db;

    if (out_size) *out_size = 0u;
    if (!context || !context->candidate ||
        dm2_v1_record_handle_pool((int16_t)handle) != 4) {
        return NULL;
    }
    db = dm2_v1_record_handle_pool((int16_t)handle);
    pool = &context->candidate->record_pools.pools[db];
    if (!pool->bytes || pool->record_size < 16) return NULL;
    record = dm2_v1_record_pool_address(&context->candidate->record_pools,
                                        (int16_t)handle);
    if (!record) return NULL;
    if (out_size) *out_size = (uint16_t)pool->record_size;
    return record;
}

static const DM2_V1_SkprojectCreatureAISpec *
dm2_v1_game_load_spatial_ai_spec(uint8_t creature_type, void *user)
{
    DM2_V1_GameLoadSpatialQueryContext *context = user;
    const DM2_AIDefinition *source = NULL;

    if (!context || !context->candidate ||
        !dm2_v1_caii_source_owner_ai_spec_def(&context->candidate->caii_source,
                                               creature_type, &source) ||
        !source) {
        if (context) context->owner_failed = 1;
        return NULL;
    }
    context->ai_spec.word30 = source->w30;
    context->ai_spec.word32 = source->w32;
    context->ai_spec.word34 = (uint16_t)source->b34 |
                               ((uint16_t)source->b35 << 8);
    return &context->ai_spec;
}

static uint16_t dm2_v1_game_load_spatial_pos5x5(const uint8_t *record,
                                                 uint16_t record_size,
                                                 uint8_t rotation_param,
                                                 void *user)
{
    DM2_V1_GameLoadSpatialQueryContext *context = user;
    const DM2_AIDefinition *ai = NULL;
    const uint8_t *gdat;
    uint8_t *cursor;
    size_t gdat_size = 0u;
    uint16_t addend;
    uint16_t timer_word;
    uint8_t pos = 0x0cu;

    if (!context || !context->candidate || !record || record_size < 16u ||
        !context->candidate->asset_loader ||
        !context->candidate->asset_loader->loaded ||
        !dm2_v1_caii_source_owner_ai_spec_def(&context->candidate->caii_source,
                                               record[4], &ai) || !ai) {
        if (context) context->owner_failed = 1;
        return 0x0cu;
    }
    if ((ai->w0AIFlags & 1u) != 0u) {
        cursor = (uint8_t *)record + 8u;
    } else {
        uint8_t slot_index = record[5];
        uint8_t *slot;
        uint16_t record_index;

        if (slot_index == 0xffu || !context->candidate->caii_slots.valid ||
            (int)slot_index >= context->candidate->caii_slots.capacity) {
            context->owner_failed = 1;
            return 0x0cu;
        }
        slot = context->candidate->caii_slots.slots +
            (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE;
        record_index = dm2_v1_game_load_owner_read_u16le(slot);
        if (record_index != ((uint16_t)(record -
                context->candidate->record_pools.pools[4].bytes) /
                (uint16_t)context->candidate->record_pools.pools[4].record_size &
                DM2_V1_RECORD_INDEX_MASK)) {
            context->owner_failed = 1;
            return 0x0cu;
        }
        cursor = slot + 8u;
    }
    gdat = dm2_v1_asset_load_typed_sized(
        context->candidate->asset_loader, DM2_GDAT_CATEGORY_CREATURES,
        record[4], DM2_GDAT_ENTRY_TYPE_RAW7,
        DM2_GDAT_CREATURE_ANIM_FRAME_SEQUENCE, &gdat_size);
    /* QUERY_CREATURE_5x5_POS returns the source's centre position 0x0c when
     * an otherwise admitted GDAT row is absent. Missing media ownership is
     * different and was rejected above. */
    if (!gdat) return 0x0cu;
    addend = dm2_v1_game_load_owner_read_u16le(cursor);
    timer_word = dm2_v1_game_load_owner_read_u16le(cursor + 2u);
    if (!dm2_v1_skproject_query_creature_5x5_pos(
            record, rotation_param,
            &(DM2_V1_SkprojectCreatureAISpec){
                ai->w30, ai->w32,
                (uint16_t)ai->b34 | ((uint16_t)ai->b35 << 8)},
            addend, &timer_word, gdat,
            (uint32_t)gdat_size, &pos, NULL)) {
        context->owner_failed = 1;
        return 0x0cu;
    }
    dm2_v1_game_load_owner_write_u16le(cursor + 2u, timer_word);
    return pos;
}

static int dm2_v1_game_load_spatial_query_098d(int16_t x, int16_t y,
                                                int16_t value, int16_t *out_x,
                                                int16_t *out_y, void *user)
{
    (void)user;
    return dm2_v1_skproject_098d_000f(x, y, value, out_x, out_y, NULL);
}

int dm2_v1_game_load_runtime_session_candidate_query_nearest_creature(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int16_t *io_x,
    int16_t *io_y, uint16_t direction, uint32_t *out_handle,
    DM2_V1_GameLoadSpatialQueryReceipt *out_receipt)
{
    DM2_V1_GameLoadSpatialQueryReceipt receipt;
    DM2_V1_GameLoadSpatialQueryContext context;
    int result;

    if (out_handle) *out_handle = 0xffffu;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    memset(&context, 0, sizeof(context));
    if (!candidate || !dm2_v1_game_load_runtime_candidate_is_initialized(candidate) ||
        !candidate->valid || !io_x || !io_y || !out_handle) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.input_x = *io_x;
    receipt.input_y = *io_y;
    receipt.direction = direction;
    if (!candidate->asset_loader || !candidate->asset_loader->loaded ||
        !candidate->caii_source.valid || !candidate->caii_slots.valid ||
        !candidate->record_pools.valid || !candidate->map_context.valid) {
        receipt.blocked_missing_owner = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (*io_x < 0 || *io_y < 0 || *io_x >= candidate->map_context.width ||
        *io_y >= candidate->map_context.height ||
        (direction != 0xffu && direction >= 4u)) {
        receipt.blocked_out_of_bounds = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    context.candidate = candidate;
    result = dm2_v1_skproject_query_1c9a_03cf(
        io_x, io_y, direction, dm2_v1_game_load_spatial_creature_at,
        dm2_v1_game_load_spatial_record, dm2_v1_game_load_spatial_ai_spec,
        dm2_v1_game_load_spatial_pos5x5, dm2_v1_game_load_spatial_query_098d,
        &context, dm2_v1_table_1d2752,
        (uint16_t)(sizeof(dm2_v1_table_1d2752) /
                   sizeof(dm2_v1_table_1d2752[0])),
        dm2_v1_table_1d62b0,
        (uint16_t)(sizeof(dm2_v1_table_1d62b0) /
                   sizeof(dm2_v1_table_1d62b0[0])),
        dm2_v1_table_1d62d0,
        (uint16_t)(sizeof(dm2_v1_table_1d62d0) /
                   sizeof(dm2_v1_table_1d62d0[0])),
        dm2_v1_table_1d62e0,
        (uint16_t)(sizeof(dm2_v1_table_1d62e0) /
                   sizeof(dm2_v1_table_1d62e0[0])),
        dm2_v1_table_1d62e8,
        (uint16_t)(sizeof(dm2_v1_table_1d62e8) /
                   sizeof(dm2_v1_table_1d62e8[0])), out_handle,
        &receipt.source);
    if (context.owner_failed) {
        *out_handle = 0xffffu;
        receipt.blocked_missing_owner = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.result_handle = *out_handle;
    receipt.valid = receipt.source.valid;
    if (out_receipt) *out_receipt = receipt;
    /* The source returns OBJECT_END_MARKER when its five-cell scan has no
     * qualifying creature. That is a valid completed query, not an owner
     * failure. The caller distinguishes it through result_handle. */
    (void)result;
    return 1;
}

/* SKULLWIN/c_move.cpp::DM2_12b4_0953.  Keep it next to the private
 * c_querydb callbacks because both branches must resolve the same cloned DB4
 * record, CAII cursor and hash-admitted dtRaw7/0xfd row. */
static int dm2_v1_game_load_runtime_candidate_creature_offset(
    DM2_V1_GameLoadSpatialQueryContext *context, int16_t handle,
    uint8_t *out_offset)
{
    const DM2_V1_RecordPool *pool;
    const uint8_t *record;
    uint16_t orientation;
    uint8_t relative_direction;
    uint16_t position;

    if (!context || !context->candidate || !out_offset ||
        dm2_v1_record_handle_pool(handle) != 4) return 0;
    pool = &context->candidate->record_pools.pools[4];
    record = dm2_v1_record_pool_address(&context->candidate->record_pools,
                                        handle);
    if (!record || pool->record_size < 16) return 0;
    orientation = dm2_v1_game_load_owner_read_u16le(record + 14u);
    relative_direction = (uint8_t)((context->candidate->source_party_direction +
        ((orientation >> 14) & 3u)) & 3u);
    if (relative_direction != 1u && relative_direction != 3u) {
        *out_offset = 0u;
        return 1;
    }
    position = dm2_v1_game_load_spatial_pos5x5(record,
                                                (uint16_t)pool->record_size,
                                                relative_direction, context);
    if (context->owner_failed) return 0;
    *out_offset = (uint8_t)((position % 5u) != 2u);
    return 1;
}

int dm2_v1_game_load_runtime_session_candidate_classify_move(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, uint8_t move_command,
    int16_t source_x, int16_t source_y, int16_t target_x, int16_t target_y,
    DM2_V1_GameLoadMoveClassificationReceipt *out_receipt)
{
    DM2_V1_GameLoadMoveClassificationReceipt receipt;
    DM2_V1_GameLoadSpatialQueryContext context;
    const DM2_AIDefinition *ai;
    const uint8_t *record;
    int source_raw;
    int target_raw;
    int16_t direct;
    int16_t scan_x;
    int16_t scan_y;
    uint32_t spatial_handle = 0xffffu;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    memset(&context, 0, sizeof(context));
    receipt.direct_creature = DM2_V1_RECORD_HANDLE_NULL;
    receipt.spatial_creature = DM2_V1_RECORD_HANDLE_NULL;
    receipt.move_command = move_command;
    receipt.source_x = source_x;
    receipt.source_y = source_y;
    receipt.target_x = target_x;
    receipt.target_y = target_y;
    if (!candidate || !dm2_v1_game_load_runtime_candidate_is_initialized(candidate) ||
        !candidate->valid || !candidate->map_context.valid) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->asset_loader || !candidate->asset_loader->loaded ||
        !candidate->record_pools.valid || !candidate->caii_source.valid ||
        !candidate->caii_slots.valid || candidate->current_map < 0 ||
        candidate->current_map >= candidate->dungeon.level_count) {
        receipt.blocked_missing_owner = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (source_x < 0 || source_y < 0 || target_x < 0 || target_y < 0 ||
        source_x >= candidate->map_context.width ||
        source_y >= candidate->map_context.height ||
        target_x >= candidate->map_context.width ||
        target_y >= candidate->map_context.height) {
        receipt.blocked_out_of_bounds = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    source_raw = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon,
                                              candidate->current_map,
                                              source_x, source_y);
    target_raw = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon,
                                              candidate->current_map,
                                              target_x, target_y);
    if (source_raw < 0 || target_raw < 0) {
        receipt.blocked_out_of_bounds = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.source_tile = (uint8_t)source_raw;
    receipt.target_tile = (uint8_t)target_raw;
    /* DM2_12b4_0881: source wall/pit type 3 plus literal move 2 is the
     * stair-back special case, before the target classification. */
    if ((receipt.source_tile >> 5) == 3u && move_command == 2u) {
        receipt.classification = 1u;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if ((receipt.target_tile >> 5) == 3u) {
        receipt.classification = 2u;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if (dm2_v1_skproject_is_tile_blocked(receipt.target_tile,
                                         &receipt.tile_blocked)) {
        receipt.classification = 3u;
        receipt.valid = 1;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    direct = dm2_v1_get_creature_at(&candidate->record_pools,
                                    &candidate->dungeon,
                                    candidate->current_map, target_x,
                                    target_y);
    receipt.direct_creature = direct;
    if (direct != DM2_V1_RECORD_HANDLE_NULL) {
        record = dm2_v1_record_pool_address(&candidate->record_pools, direct);
        if (!record || dm2_v1_record_handle_pool(direct) != 4 ||
            !dm2_v1_caii_source_owner_ai_spec_def(&candidate->caii_source,
                                                   record[4], &ai) || !ai) {
            receipt.blocked_missing_owner = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        if ((ai->w0AIFlags & 0x8000u) == 0u) {
            context.candidate = candidate;
            if (!dm2_v1_game_load_runtime_candidate_creature_offset(
                    &context, direct, &receipt.direct_creature_offset)) {
                receipt.blocked_missing_owner = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
            receipt.classification = receipt.direct_creature_offset ? 5u : 4u;
            receipt.valid = 1;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
    }
    scan_x = target_x;
    scan_y = target_y;
    if (!dm2_v1_game_load_runtime_session_candidate_query_nearest_creature(
            candidate, &scan_x, &scan_y, 0xffu, &spatial_handle,
            &receipt.spatial)) {
        receipt.blocked_missing_owner = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.spatial_creature = (int16_t)spatial_handle;
    if (spatial_handle == 0xffffu) {
        receipt.classification = 6u;
    } else {
        record = dm2_v1_record_pool_address(&candidate->record_pools,
                                            (int16_t)spatial_handle);
        if (!record || dm2_v1_record_handle_pool((int16_t)spatial_handle) != 4 ||
            !dm2_v1_caii_source_owner_ai_spec_def(&candidate->caii_source,
                                                   record[4], &ai) || !ai) {
            receipt.blocked_missing_owner = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.classification = (ai->w0AIFlags & 0x8000u) == 0u ? 5u : 6u;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_game_load_runtime_session_candidate_census_moverec_square(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t x, int16_t y, DM2_V1_GameLoadMoverecSquareReceipt *out_receipt)
{
    DM2_V1_GameLoadMoverecSquareReceipt receipt;
    int16_t link;
    int max_links = 0;
    int pool;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.map = -1;
    receipt.x = x;
    receipt.y = y;
    receipt.first_record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.chain_hash = 2166136261u;
    if (!candidate || !dm2_v1_game_load_runtime_candidate_is_initialized(candidate) ||
        !candidate->valid || !candidate->map_context.valid ||
        !candidate->record_pools.valid ||
        !candidate->record_pools.record_graph_complete ||
        candidate->current_map < 0 ||
        candidate->current_map >= candidate->dungeon.level_count) {
        receipt.blocked_invalid_candidate = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.map = (int16_t)candidate->current_map;
    if (x < 0 || y < 0 || x >= candidate->map_context.width ||
        y >= candidate->map_context.height) {
        receipt.blocked_out_of_bounds = 1;
        *out_receipt = receipt;
        return 0;
    }
    {
        const int tile = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon,
                                                      candidate->current_map,
                                                      x, y);
        if (tile < 0) {
            receipt.blocked_out_of_bounds = 1;
            *out_receipt = receipt;
            return 0;
        }
        receipt.tile = (uint8_t)tile;
    }
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon,
                                                    candidate->current_map,
                                                    x, y);
    if (link == DM2_V1_RECORD_HANDLE_NULL) {
        receipt.blocked_missing_record = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.first_record = link;
    for (pool = 0; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        const DM2_V1_RecordPool *source_pool =
            &candidate->record_pools.pools[pool];
        if (source_pool->record_count < 0 || source_pool->extension_count < 0 ||
            max_links > INT_MAX - source_pool->record_count ||
            max_links + source_pool->record_count > INT_MAX -
                source_pool->extension_count) {
            receipt.blocked_invalid_candidate = 1;
            *out_receipt = receipt;
            return 0;
        }
        max_links += source_pool->record_count + source_pool->extension_count;
    }
    if (max_links <= 0 || max_links > UINT16_MAX) {
        receipt.blocked_missing_record = 1;
        *out_receipt = receipt;
        return 0;
    }
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        const DM2_V1_RecordPool *source_pool;
        int16_t next;
        int type;
        int index;

        if (link == DM2_V1_RECORD_HANDLE_NULL ||
            receipt.record_count >= (uint16_t)max_links) {
            receipt.blocked_cycle = 1;
            *out_receipt = receipt;
            return 0;
        }
        type = dm2_v1_record_handle_pool(link);
        index = dm2_v1_record_handle_index(link);
        if (type < 0 || type >= DM2_V1_RECORD_POOL_COUNT || index < 0) {
            receipt.blocked_missing_record = 1;
            *out_receipt = receipt;
            return 0;
        }
        source_pool = &candidate->record_pools.pools[type];
        record = dm2_v1_record_pool_address(&candidate->record_pools, link);
        if (!record || source_pool->record_size <= 0) {
            receipt.blocked_missing_record = 1;
            *out_receipt = receipt;
            return 0;
        }
        ++receipt.record_type_count[type];
        ++receipt.record_count;
        receipt.chain_hash = dm2_v1_game_load_owner_hash_step(receipt.chain_hash,
                                                               (uint16_t)link);
        for (int byte = 0; byte < source_pool->record_size; ++byte) {
            receipt.chain_hash = dm2_v1_game_load_owner_hash_step(
                receipt.chain_hash, record[byte]);
        }
        if (!dm2_v1_record_pool_next_link(&candidate->record_pools, link,
                                          &next)) {
            receipt.blocked_missing_record = 1;
            *out_receipt = receipt;
            return 0;
        }
        link = next;
    }
    receipt.chain_hash = dm2_v1_game_load_owner_hash_step(
        receipt.chain_hash, (uint16_t)DM2_V1_RECORD_HANDLE_END);
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

static int dm2_v1_game_load_record_chain_is_bounded(
    const DM2_V1_RecordPoolSet *set, int16_t head, int16_t needle,
    int *out_contains)
{
    int max_links = 1;
    int16_t cursor = head;

    if (out_contains) *out_contains = 0;
    if (!set || !set->valid) return 0;
    for (int pool = 0; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        max_links += set->pools[pool].record_count;
        max_links += set->pools[pool].extension_count;
    }
    if (cursor == DM2_V1_RECORD_HANDLE_END ||
        cursor == DM2_V1_RECORD_HANDLE_NULL) return 1;
    for (int step = 0; step < max_links; ++step) {
        int16_t next;
        if (cursor == needle && out_contains) *out_contains = 1;
        if (!dm2_v1_record_pool_next_link(set, cursor, &next)) return 0;
        if (next == DM2_V1_RECORD_HANDLE_END) return 1;
        if (next == DM2_V1_RECORD_HANDLE_NULL) return 0;
        cursor = next;
    }
    return 0;
}

/* Cross-map MOVE_RECORD_TO needs both representations of c_map to agree.
 * The pool mirror is the mutable runtime owner while the raw dungeon image
 * is the source save/load owner; accepting either one alone would make a
 * later map traversal observe a different chain. */
static int dm2_v1_game_load_record_chain_mirrors_complete(
    const DM2_V1_RecordPoolSet *set, const DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int16_t needle, int *out_contains,
    int *out_has_actuator)
{
    int budget = 1;
    int16_t pool_cursor;
    int16_t raw_cursor;
    if (out_contains) *out_contains = 0;
    if (out_has_actuator) *out_has_actuator = 0;
    if (!set || !set->valid || !dungeon || !dungeon->raw_data ||
        map < 0 || map >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map])
        return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        if (set->pools[db].record_count < 0 ||
            set->pools[db].extension_count < 0 ||
            budget > INT_MAX - set->pools[db].record_count -
                set->pools[db].extension_count)
            return 0;
        budget += set->pools[db].record_count +
                  set->pools[db].extension_count;
    }
    pool_cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, map, x, y);
    raw_cursor = pool_cursor;
    for (int step = 0; step < budget; ++step) {
        int raw_type = -1;
        int raw_size = 0;
        const uint8_t *pool_record;
        const uint8_t *raw_record;
        int16_t pool_next;
        int16_t raw_next;
        if (pool_cursor != raw_cursor ||
            pool_cursor == DM2_V1_RECORD_HANDLE_NULL ||
            raw_cursor == DM2_V1_RECORD_HANDLE_NULL)
            return 0;
        if (pool_cursor == DM2_V1_RECORD_HANDLE_END)
            return 1;
        pool_record = dm2_v1_record_pool_address(set, pool_cursor);
        raw_record = (const uint8_t *)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)raw_cursor, &raw_type, NULL, &raw_size);
        if (!pool_record || !raw_record || raw_size < 2 ||
            !dm2_v1_record_pool_next_link(set, pool_cursor, &pool_next))
            return 0;
        raw_next = (int16_t)((uint16_t)raw_record[0] |
                             ((uint16_t)raw_record[1] << 8));
        if (pool_next != raw_next)
            return 0;
        if (pool_cursor == needle && out_contains) *out_contains = 1;
        if (dm2_v1_record_handle_pool(pool_cursor) == 3 && out_has_actuator)
            *out_has_actuator = 1;
        if (pool_next == DM2_V1_RECORD_HANDLE_NULL ||
            raw_next == DM2_V1_RECORD_HANDLE_NULL)
            return 0;
        pool_cursor = pool_next;
        raw_cursor = raw_next;
    }
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_move_record_to(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record, int16_t source_x, int16_t source_y,
    int16_t destination_x, int16_t destination_y,
    DM2_V1_GameLoadRecordMoveReceipt *out_receipt)
{
    DM2_V1_GameLoadRecordMoveReceipt receipt;
    DM2_V1_RecordPoolSet pool_backup;
    uint8_t *raw_backup = NULL;
    size_t raw_size;
    int source_head;
    int destination_head;
    int16_t source_head_io;
    int16_t destination_head_io;
    int source_contains = 0;
    int destination_contains = 0;
    int source_chain_ok;
    int destination_chain_ok;
    uint32_t hash;

    memset(&receipt, 0, sizeof(receipt));
    memset(&pool_backup, 0, sizeof(pool_backup));
    receipt.map = -1;
    receipt.record = record;
    receipt.source_x = source_x;
    receipt.source_y = source_y;
    receipt.destination_x = destination_x;
    receipt.destination_y = destination_y;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(
            candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.map = candidate->current_map;
    if (record == DM2_V1_RECORD_HANDLE_NULL ||
        record == DM2_V1_RECORD_HANDLE_END || source_x < 0 || source_y < 0 ||
        destination_x < 0 || destination_y < 0 ||
        source_x >= candidate->map_context.width ||
        source_y >= candidate->map_context.height ||
        destination_x >= candidate->map_context.width ||
        destination_y >= candidate->map_context.height ||
        (source_x == destination_x && source_y == destination_y)) {
        receipt.blocked_map_or_square = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    source_head = dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, candidate->current_map, source_x, source_y);
    destination_head = dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, candidate->current_map, destination_x,
        destination_y);
    if (source_head < 0 || destination_head < 0) {
        receipt.blocked_source_chain = source_head < 0;
        receipt.blocked_destination_chain = destination_head < 0;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.source_head_before = (uint16_t)source_head;
    receipt.destination_head_before = (uint16_t)destination_head;
    source_chain_ok = dm2_v1_game_load_record_chain_is_bounded(
            &candidate->record_pools, (int16_t)source_head, record,
            &source_contains);
    destination_chain_ok = dm2_v1_game_load_record_chain_is_bounded(
            &candidate->record_pools, (int16_t)destination_head, record,
            &destination_contains);
    if (!source_chain_ok || !destination_chain_ok || !source_contains) {
        receipt.blocked_source_chain = !source_chain_ok || !source_contains;
        receipt.blocked_destination_chain = !destination_chain_ok;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (destination_contains) {
        receipt.blocked_destination_chain = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!dm2_v1_record_pool_set_clone(&pool_backup,
                                      &candidate->record_pools)) {
        receipt.blocked_source_chain = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    raw_size = (size_t)candidate->dungeon.raw_size;
    raw_backup = malloc(raw_size);
    if (!raw_backup) {
        dm2_v1_record_pool_set_free(&pool_backup);
        receipt.blocked_source_chain = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    memcpy(raw_backup, candidate->dungeon.raw_data, raw_size);
    source_head_io = (int16_t)source_head;
    destination_head_io = (int16_t)destination_head;
    if (!dm2_v1_record_pool_cut_from_tile(
            &candidate->record_pools, &candidate->dungeon,
            candidate->current_map, source_x, source_y, record)) {
        receipt.blocked_source_chain = 1;
        goto rollback;
    }
    source_head_io = (int16_t)dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, candidate->current_map, source_x, source_y);
    if (!dm2_v1_record_pool_append_to_list(
            &candidate->record_pools, &destination_head_io, record) ||
        dm2_v1_dungeon_set_first_thing(
            &candidate->dungeon, candidate->current_map, destination_x,
            destination_y, (uint16_t)destination_head_io) != 0) {
        receipt.blocked_destination_chain = 1;
        goto rollback;
    }
    receipt.source_head_after = (uint16_t)source_head_io;
    receipt.destination_head_after = (uint16_t)destination_head_io;
    hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash, (uint16_t)record);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                             (uint16_t)source_head_io);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                             (uint16_t)destination_head_io);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                             (uint16_t)source_x << 8 |
                                             (uint16_t)source_y);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                             (uint16_t)destination_x << 8 |
                                             (uint16_t)destination_y);
    receipt.transaction_hash = hash;
    receipt.valid = 1;
    receipt.mutated = 1;
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

rollback:
    memcpy(candidate->dungeon.raw_data, raw_backup, raw_size);
    dm2_v1_record_pool_set_free(&candidate->record_pools);
    candidate->record_pools = pool_backup;
    memset(&pool_backup, 0, sizeof(pool_backup));
    receipt.rolled_back = 1;
    free(raw_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

/* c_map.cpp::DM2_CHANGE_CURRENT_MAP_TO has no party movement side effect.
 * Keep that narrow source operation private over the already-cloned world:
 * mapdat.map/v1e03c0/v1e03f4 become raw offsets in map_context, while
 * v1e0280/v1e027e/v1e0282/v1e0264 remain the separately retained display
 * pose.  A caller can therefore prepare a later moverec/light transaction
 * without mutating the File_header source owner or manufacturing gameplay. */
static int dm2_v1_game_load_runtime_candidate_change_current_map(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int new_map,
    int force_reload)
{
    DM2_V1_SkprojectChangeCurrentMapReceipt context;
    int previous_map;
    int use_alternate;

    if (!candidate || !candidate->dungeon.raw_data ||
        !candidate->record_pools.valid || new_map < 0 ||
        new_map >= candidate->dungeon.level_count) {
        return 0;
    }
    if (!force_reload && new_map == candidate->current_map) return 1;
    previous_map = force_reload ? -1 : candidate->current_map;
    memset(&context, 0, sizeof(context));
    if (!dm2_v1_skproject_change_current_map_to(
            &candidate->dungeon, previous_map, new_map,
            candidate->source_party_x, candidate->source_party_y,
            candidate->source_party_map, candidate->source_party_direction,
            &context) || !context.valid || context.unchanged ||
        context.map_descriptor_offset < 0 || context.raw_tile_map_offset < 0 ||
        context.column_index_offset < 0 || context.first_thing_base_offset < 0) {
        return 0;
    }
    use_alternate = candidate->source_staircase_flag != 0 &&
                    candidate->source_teleporter_map == new_map;
    /* c_map's alternate display pose is meaningful only for the preceding
     * DM2_move_2fcf_0b8b staircase/teleporter context.  Do not turn a
     * malformed private candidate into a normal-pose fallback. */
    if (use_alternate && !candidate->source_display_pose_valid) return 0;
    candidate->map_context = context;
    candidate->current_map = new_map;
    candidate->moverec.v1d3248 = (int16_t)new_map;
    candidate->source_runtime_display_uses_alternate = use_alternate;
    if (use_alternate) {
        candidate->source_display_map = candidate->source_teleporter_map;
        candidate->source_runtime_display_x = candidate->source_display_x;
        candidate->source_runtime_display_y = candidate->source_display_y;
        candidate->source_runtime_display_direction = candidate->source_party_absdir;
    } else {
        candidate->source_display_map = candidate->source_party_map;
        candidate->source_runtime_display_x = candidate->source_party_x;
        candidate->source_runtime_display_y = candidate->source_party_y;
        candidate->source_runtime_display_direction = candidate->source_party_direction;
    }
    return 1;
}

int dm2_v1_game_load_runtime_session_candidate_dispatch_moverec(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record, int16_t x, int16_t y, int32_t kind, int32_t flags,
    DM2_V1_GameLoadMoverecDispatchReceipt *out_receipt)
{
    DM2_V1_GameLoadMoverecDispatchReceipt receipt;
    uint8_t *record_bytes;

    memset(&receipt, 0, sizeof(receipt));
    receipt.map = candidate ? candidate->current_map : -1;
    receipt.record = record;
    receipt.x = x;
    receipt.y = y;
    receipt.kind = kind;
    receipt.flags = flags;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (candidate->current_map < 0 || candidate->current_map >= candidate->dungeon.level_count ||
        x < 0 || y < 0 || x >= candidate->map_context.width ||
        y >= candidate->map_context.height) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (record == (int16_t)-1) {
        /* c_moverec.cpp keeps 0xffff as the party/self sentinel; it has no
         * RecordPool address but is still a valid dispatcher input. */
        receipt.db_type = 0xffu;
    } else {
        record_bytes = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                                       (uint16_t)record);
        if (!record_bytes) {
            receipt.blocked_record = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.db_type = (uint8_t)(((uint16_t)record >> 10) & 0x0fu);
    }
    /* Source c_moverec.cpp:1147 enters the post-move dispatcher here. The
     * dispatcher itself remains callback-owned; admitting this call shape is
     * therefore the only safe candidate-side effect at this boundary. */
    receipt.dispatched = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record ^ ((uint32_t)(uint16_t)x << 16) ^
        (uint32_t)(uint16_t)y ^ (uint32_t)kind ^ (uint32_t)flags);
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

int dm2_v1_game_load_runtime_session_candidate_activate_moverec_caii(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record, int16_t x, int16_t y,
    DM2_V1_GameLoadMoverecCaiiReceipt *out_receipt)
{
    DM2_V1_GameLoadMoverecCaiiReceipt receipt;
    DM2_V1_RecordPool *db4;
    uint8_t *record_bytes;
    uint8_t *slot = NULL;
    uint8_t *record_backup = NULL;
    uint8_t *slot_backup = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup = {0};
    int caii_alloc_count_backup = 0;
    size_t record_size;
    uint16_t timer_word;
    DM2_V1_TimerEntry *timer;
    DM2_V1_TimerEntry new_timer;
    uint16_t ai_flags = 0u;
    int slot_index = -1;
    int queued_timer;
    int i;

    memset(&receipt, 0, sizeof(receipt));
    receipt.map = candidate ? candidate->current_map : -1;
    receipt.record = record;
    receipt.x = x;
    receipt.y = y;
    receipt.caii_slot = -1;
    receipt.timer_slot = -1;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (candidate->current_map < 0 || candidate->current_map >= candidate->dungeon.level_count ||
        x < 0 || y < 0 || x >= candidate->map_context.width ||
        y >= candidate->map_context.height) {
        receipt.blocked_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (dm2_v1_record_handle_pool(record) != 4) {
        receipt.blocked_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    record_bytes = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                                   (uint16_t)record);
    if (!record_bytes) {
        receipt.blocked_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (record_bytes[5] == 0xffu) {
        /* c_moverec.cpp:980-984: flags bit0 CLEAR admits the source
         * allocator. Unknown AI provenance remains fail-closed. */
        {
            const DM2_AIDefinition *definition = NULL;
            if (!dm2_v1_caii_source_owner_ai_spec_def(
                    &candidate->caii_source, (int)record_bytes[4],
                    &definition) || !definition) {
                receipt.blocked_ai_flags = 1;
                if (out_receipt) *out_receipt = receipt;
                return 0;
            }
            ai_flags = definition->w0AIFlags;
            receipt.ai_flags_known = 1;
        }
        receipt.ai_bit0_clear = (ai_flags & 1u) == 0u ? 1 : 0;
        if (!receipt.ai_bit0_clear) {
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        if (!candidate->caii_slots.valid || !candidate->caii_slots.slots) {
            receipt.blocked_no_slot = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        for (i = 0; i < candidate->caii_slots.capacity; ++i) {
            const uint8_t *candidate_slot = candidate->caii_slots.slots +
                (size_t)i * DM2_V1_CAII_SLOT_SIZE;
            if ((int16_t)dm2_v1_game_load_owner_read_u16le(candidate_slot) < 0) {
                slot_index = i;
                break;
            }
        }
        if (slot_index < 0) {
            receipt.blocked_no_slot = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        db4 = &candidate->record_pools.pools[4];
        record_size = db4->record_size;
        if (!db4->bytes || record_size < 16u ||
            candidate->timer_capacity == 0u || !candidate->timer_entries ||
            !candidate->timer_indices) {
            receipt.blocked_unbound_allocation = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        record_backup = (uint8_t *)malloc(record_size);
        slot_backup = (uint8_t *)malloc(DM2_V1_CAII_SLOT_SIZE);
        timer_backup = (DM2_V1_TimerEntry *)malloc(
            (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        index_backup = (int16_t *)malloc(
            (size_t)candidate->timer_capacity * sizeof(*index_backup));
        if (!record_backup || !slot_backup || !timer_backup || !index_backup) {
            receipt.blocked_unbound_allocation = 1;
            goto allocation_rollback;
        }
        memcpy(record_backup, record_bytes, record_size);
        slot = candidate->caii_slots.slots +
               (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE;
        memcpy(slot_backup, slot, DM2_V1_CAII_SLOT_SIZE);
        memcpy(timer_backup, candidate->timer_entries,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(index_backup, candidate->timer_indices,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        queue_backup = candidate->timer_queue;
        caii_alloc_count_backup = candidate->caii_slots.alloc_count;

        /* c_1c9a.cpp:5809-5866, source order. */
        memset(slot, 0, DM2_V1_CAII_SLOT_SIZE);
        dm2_v1_game_load_owner_write_u16le(slot,
                                           (uint16_t)record & 0x03ffu);
        dm2_v1_game_load_owner_write_u16le(slot + 2u, 0xffffu);
        slot[0x1a] = 0xffu;
        slot[6] = (uint8_t)((candidate->timer_queue.gametick >> 2) - 1u);
        slot[4] = (uint8_t)(candidate->timer_queue.gametick - 0x7fu);
        dm2_v1_game_load_owner_write_u16le(
            slot + 0x0cu, (uint16_t)((x & 0x1f) | ((y & 0x1f) << 5) |
                                     ((candidate->current_map & 0x3f) << 10)));
        slot[0x16] = 0xffu;
        slot[0x17] = 0xffu;
        slot[7] = 0u;
        record_bytes[5] = (uint8_t)slot_index;
        record_bytes[0x0f] |= 0x04u;
        candidate->caii_slots.alloc_count++;

        dm2_v1_timer_entry_init(&new_timer);
        timer = &new_timer;
        dm2_v1_timer_set_mticks(timer, (int16_t)candidate->current_map,
                                candidate->timer_queue.gametick + 1);
        timer->ttype = dm2_v1_game_load_owner_read_u16le(record_bytes + 8u) !=
                       0xffffu ? 0x22u : 0x21u;
        timer->actor = record_bytes[4];
        timer->xA = (int8_t)x;
        timer->yA = (int8_t)y;
        queued_timer = dm2_v1_timer_queue(&candidate->timer_queue, &new_timer);
        if (queued_timer < 0) goto allocation_rollback;
        dm2_v1_game_load_owner_write_u16le(slot + 2u,
                                           (uint16_t)queued_timer);
        slot[0x1a] = dm2_v1_game_load_owner_read_u16le(record_bytes + 8u) !=
                     0xffffu ? 0x00u : 0x11u;
        receipt.caii_slot = (int16_t)slot_index;
        receipt.timer_slot = (int16_t)queued_timer;
        receipt.timer_type = timer->ttype;
        receipt.due_tick = (uint32_t)dm2_v1_timer_get_ticks(timer);
        receipt.allocation_performed = 1;
        receipt.timer_scheduled = 1;
        /* The newly allocated CAII slot now owns the queued c_tim slot just
         * as surely as the existing-slot branch below updates one in place.
         * c_moverec's caller uses this bit to admit the whole transaction;
         * leaving it clear falsely rejects the source-valid no-slot path. */
        receipt.timer_updated = 1;
        receipt.valid = 1;
        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
            candidate->source_transaction_hash ^ candidate->candidate_hash,
            (uint32_t)(uint16_t)record ^ ((uint32_t)(uint16_t)x << 16) ^
            (uint32_t)(uint16_t)y ^ (uint32_t)queued_timer);
        free(index_backup);
        free(timer_backup);
        free(slot_backup);
        free(record_backup);
        if (out_receipt) *out_receipt = receipt;
        return 1;

allocation_rollback:
        if (record_backup && slot_backup && timer_backup && index_backup) {
            memcpy(record_bytes, record_backup, record_size);
            memcpy(slot, slot_backup, DM2_V1_CAII_SLOT_SIZE);
            memcpy(candidate->timer_entries, timer_backup,
                   (size_t)candidate->timer_capacity * sizeof(*timer_backup));
            memcpy(candidate->timer_indices, index_backup,
                   (size_t)candidate->timer_capacity * sizeof(*index_backup));
            candidate->timer_queue = queue_backup;
            candidate->caii_slots.alloc_count = caii_alloc_count_backup;
        }
        free(index_backup);
        free(timer_backup);
        free(slot_backup);
        free(record_backup);
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.caii_slot = (int16_t)record_bytes[5];
    if (!candidate->caii_slots.valid ||
        receipt.caii_slot < 0 || receipt.caii_slot >= candidate->caii_slots.capacity) {
        receipt.blocked_no_slot = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    slot = candidate->caii_slots.slots +
           (size_t)receipt.caii_slot * DM2_V1_CAII_SLOT_SIZE;
    timer_word = dm2_v1_game_load_owner_read_u16le(slot + 2u);
    if (timer_word == 0xffffu || timer_word >= (uint16_t)candidate->timer_capacity ||
        !candidate->timer_entries || candidate->timer_entries[timer_word].ttype == 0u) {
        receipt.blocked_no_pending_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    timer = &candidate->timer_entries[timer_word];
    /* Source setxyA + setmticks: mutate the fixed c_tim slot in place and
     * leave the heap ordering/index array untouched. */
    timer->xA = (int8_t)x;
    timer->yA = (int8_t)y;
    dm2_v1_timer_set_mticks(timer, (int16_t)candidate->current_map,
                            candidate->timer_queue.gametick);
    receipt.timer_slot = (int16_t)timer_word;
    receipt.timer_updated = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record ^ ((uint32_t)(uint16_t)x << 16) ^
        (uint32_t)(uint16_t)y ^ (uint32_t)timer_word);
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_game_load_runtime_session_candidate_detach_caii_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record,
    DM2_V1_GameLoadCandidateCaiiTimerDetachReceipt *out_receipt)
{
    DM2_V1_GameLoadCandidateCaiiTimerDetachReceipt receipt;
    uint8_t *record_bytes;
    uint8_t *slot;
    uint16_t timer_word;
    uint16_t stored_record;
    DM2_V1_TimerEntry *timer;

    memset(&receipt, 0, sizeof(receipt));
    receipt.record = record;
    receipt.caii_slot = -1;
    receipt.timer_slot = -1;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (dm2_v1_record_handle_pool(record) != 4 ||
        !(record_bytes = dm2_v1_record_pool_address_mut(
              &candidate->record_pools, (uint16_t)record)) ||
        record_bytes[5] == 0xffu) {
        receipt.blocked_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.caii_slot = (int16_t)record_bytes[5];
    if (!candidate->caii_slots.valid || !candidate->caii_slots.slots ||
        receipt.caii_slot < 0 ||
        receipt.caii_slot >= candidate->caii_slots.capacity) {
        receipt.blocked_caii = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    slot = candidate->caii_slots.slots +
           (size_t)receipt.caii_slot * DM2_V1_CAII_SLOT_SIZE;
    stored_record = dm2_v1_game_load_owner_read_u16le(slot) &
                    DM2_V1_RECORD_INDEX_MASK;
    if (stored_record != ((uint16_t)record & DM2_V1_RECORD_INDEX_MASK)) {
        receipt.blocked_caii = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    timer_word = dm2_v1_game_load_owner_read_u16le(slot + 2u);
    if (timer_word >= (uint16_t)candidate->timer_capacity ||
        !candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_queue.num_timers <= 0 ||
        dm2_v1_timer_get_heap_index(&candidate->timer_queue,
                                    (int16_t)timer_word) < 0 ||
        candidate->timer_entries[timer_word].ttype == 0u) {
        receipt.blocked_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    timer = &candidate->timer_entries[timer_word];
    if (timer->actor != record_bytes[4]) {
        receipt.blocked_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.timer_slot = (int16_t)timer_word;
    receipt.timer_type = timer->ttype;
    /* Candidate CAII slot+2 is a c_tim array index, not a source timer
     * ticket.  Delete through the candidate heap owner and clear only that
     * back-reference; the DB4 and CAII slot remain owned for the eventual
     * full delete transaction. */
    dm2_v1_timer_delete(&candidate->timer_queue, receipt.timer_slot);
    dm2_v1_game_load_owner_write_u16le(slot + 2u, 0xffffu);
    receipt.detached = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record ^
        ((uint32_t)(uint16_t)receipt.caii_slot << 16) ^
        (uint32_t)(uint16_t)receipt.timer_slot);
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_moverec_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadMoverecTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadMoverecTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_GameLoadSoundOwner sound_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t *dungeon_backup = NULL;
    uint8_t *caii_backup = NULL;
    size_t caii_bytes = 0u;
    int caii_alloc_count_backup = 0;
    int old_map = -1;
    uint8_t old_party_x = 0u;
    uint8_t old_party_y = 0u;
    int16_t old_display_map = -1;
    int16_t old_runtime_display_x = 0;
    int16_t old_runtime_display_y = 0;
    uint8_t old_runtime_display_direction = 0u;
    int old_runtime_display_uses_alternate = 0;
    int16_t timer_slot;
    int map;
    int x;
    int y;
    int source_x = -1;
    int source_y = -1;
    int source_head;
    int destination_head;
    int contains = 0;
    int found = 0;
    int party_move = 0;
    int cross_map_move = 0;
    int16_t record_handle;
    uint8_t *record_bytes;
    DM2_V1_GameLoadRecordMoveReceipt move_receipt;
    DM2_V1_GameLoadMoverecCaiiReceipt caii_receipt;
    DM2_V1_SoundQueueState sound_state;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_SoundQueueReceipt sound_receipt;

    memset(&receipt, 0, sizeof(receipt));
    memset(&pool_backup, 0, sizeof(pool_backup));
    memset(&sound_backup, 0, sizeof(sound_backup));
    memset(&move_receipt, 0, sizeof(move_receipt));
    memset(&caii_receipt, 0, sizeof(caii_receipt));
    memset(&sound_state, 0, sizeof(sound_state));
    memset(&sound_env, 0, sizeof(sound_env));
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    receipt.game_tick = game_tick;
    receipt.map = -1;
    receipt.source_map = -1;
    receipt.record = DM2_V1_RECORD_HANDLE_NULL;

    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        goto moverec_fail;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0 ||
        !candidate->caii_slots.valid || !candidate->caii_slots.slots ||
        !candidate->sound_owner.valid ||
        !candidate->sound_owner.runtime_queue_initialized ||
        !candidate->sound_owner.spatial_context_valid) {
        receipt.blocked_no_due_timer = 1;
        goto moverec_fail;
    }
    old_map = candidate->current_map;
    old_party_x = candidate->source_party_x;
    old_party_y = candidate->source_party_y;
    old_display_map = candidate->source_display_map;
    old_runtime_display_x = candidate->source_runtime_display_x;
    old_runtime_display_y = candidate->source_runtime_display_y;
    old_runtime_display_direction = candidate->source_runtime_display_direction;
    old_runtime_display_uses_alternate =
        candidate->source_runtime_display_uses_alternate;
    queue_backup = candidate->timer_queue;
    caii_alloc_count_backup = candidate->caii_slots.alloc_count;
    caii_bytes = (size_t)candidate->caii_slots.capacity * DM2_V1_CAII_SLOT_SIZE;
    timer_backup = malloc((size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = malloc((size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (candidate->dungeon.raw_size == 0u || !candidate->dungeon.raw_data ||
        !timer_backup || !index_backup ||
        (caii_bytes != 0u && !(caii_backup = malloc(caii_bytes))) ||
        !dm2_v1_record_pool_set_clone(&pool_backup, &candidate->record_pools) ||
        !dm2_v1_game_load_runtime_candidate_sound_clone(
            &sound_backup, &candidate->sound_owner))
        goto moverec_rollback;
    dungeon_backup = malloc((size_t)candidate->dungeon.raw_size);
    if (!dungeon_backup) goto moverec_rollback;
    memcpy(dungeon_backup, candidate->dungeon.raw_data,
           (size_t)candidate->dungeon.raw_size);
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (caii_bytes != 0u)
        memcpy(caii_backup, candidate->caii_slots.slots, caii_bytes);

    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto moverec_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto moverec_rollback;
    }
    receipt.timer_type = candidate->timer_entries[timer_slot].ttype;
    if (receipt.timer_type != 0x3cu && receipt.timer_type != 0x3du) {
        receipt.blocked_unsupported_type = 1;
        goto moverec_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(int8_t)timer_entry.xA;
    y = (int)(int8_t)timer_entry.yA;
    receipt.map = map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    receipt.source_map = map;
    record_handle = timer_entry.wvalueB;
    receipt.record = record_handle;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        x < 0 || y < 0 || x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        (record_handle != (int16_t)0xffff &&
         dm2_v1_record_handle_pool(record_handle) != 4) ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto moverec_rollback;
    }
    if (record_handle == (int16_t)0xffff) {
        int source_raw;
        int destination_raw = dm2_v1_dungeon_get_tile_raw(
            &candidate->dungeon, map, x, y);
        int destination_tile_class = dm2_v1_dungeon_get_square_type(
            &candidate->dungeon, map, x, y);
        int destination_square_type = destination_tile_class < 0 ? -1 :
            dm2_v1_viewport_g1_tile_class_to_square_type(
                (uint8_t)destination_tile_class);
        int source_head = dm2_v1_dungeon_get_first_thing(
            &candidate->dungeon, map, candidate->source_party_x,
            candidate->source_party_y);
        int destination_head = dm2_v1_dungeon_get_first_thing(
            &candidate->dungeon, map, x, y);
        int same_party_tile = candidate->source_party_x == (uint8_t)x &&
            candidate->source_party_y == (uint8_t)y;
        if (map != candidate->current_map || map != candidate->source_party_map ||
            candidate->source_party_x >= candidate->dungeon.level_widths[map] ||
            candidate->source_party_y >= candidate->dungeon.level_heights[map] ||
            (!same_party_tile &&
             (source_head != DM2_V1_RECORD_HANDLE_END ||
              destination_head != DM2_V1_RECORD_HANDLE_END)) ||
            destination_raw < 0 || destination_square_type != DM2_SQUARE_FLOOR ||
            dm2_v1_get_creature_at(&candidate->record_pools, &candidate->dungeon,
                                   map, x, y) != DM2_V1_RECORD_HANDLE_NULL) {
            receipt.blocked_record = 1;
            goto moverec_rollback;
        }
        source_raw = dm2_v1_dungeon_get_tile_raw(
            &candidate->dungeon, map, candidate->source_party_x,
            candidate->source_party_y);
        {
            int source_tile_class = dm2_v1_dungeon_get_square_type(
                &candidate->dungeon, map, candidate->source_party_x,
                candidate->source_party_y);
            int source_square_type = source_tile_class < 0 ? -1 :
                dm2_v1_viewport_g1_tile_class_to_square_type(
                    (uint8_t)source_tile_class);
            if (source_raw < 0 || source_square_type != DM2_SQUARE_FLOOR) {
                receipt.blocked_record = 1;
                goto moverec_rollback;
            }
        }
        party_move = 1;
        receipt.source_x = (int16_t)candidate->source_party_x;
        receipt.source_y = (int16_t)candidate->source_party_y;
        candidate->source_party_x = (uint8_t)x;
        candidate->source_party_y = (uint8_t)y;
        candidate->source_display_map = (int16_t)map;
        candidate->source_runtime_display_x = (int16_t)x;
        candidate->source_runtime_display_y = (int16_t)y;
        candidate->source_runtime_display_direction =
            candidate->source_party_direction;
        candidate->source_runtime_display_uses_alternate = 0;
    } else {
        record_bytes = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, record_handle);
        if (!record_bytes || record_bytes[5] == 0xffu) {
            receipt.blocked_record = 1;
            goto moverec_rollback;
        }
    }

    /* c_moverec changes c_map before a cross-map append.  Admit only the
     * source-shaped plain-floor DB4 form here: the mutable pool mirror and
     * raw save mirror must agree link-for-link, and neither tile may carry a
     * DB3 actuator tail.  Creature level restrictions, wake/sleep and full
     * actuator consequences remain fail-closed until their owners are bound. */
    if (record_handle != (int16_t)0xffff) {
        int source_map = -1;
        int source_cell_x = -1;
        int source_cell_y = -1;
        for (int scan_map = 0; scan_map < candidate->dungeon.level_count &&
             source_map < 0; ++scan_map) {
            if (scan_map == map) continue;
            for (int scan_x = 0; scan_x < candidate->dungeon.level_widths[scan_map] &&
                 source_map < 0; ++scan_x) {
                for (int scan_y = 0;
                     scan_y < candidate->dungeon.level_heights[scan_map];
                     ++scan_y) {
                    int chain_contains = 0;
                    int has_actuator = 0;
                    if (!dm2_v1_game_load_record_chain_mirrors_complete(
                            &candidate->record_pools, &candidate->dungeon,
                            scan_map, scan_x, scan_y, record_handle,
                            &chain_contains, &has_actuator))
                        continue;
                    if (chain_contains) {
                        if (has_actuator) {
                            source_map = -2;
                            break;
                        }
                        source_map = scan_map;
                        source_cell_x = scan_x;
                        source_cell_y = scan_y;
                        break;
                    }
                }
            }
        }
        if (source_map == -2) {
            receipt.blocked_source_chain = 1;
            goto moverec_rollback;
        }
        if (source_map >= 0) {
            receipt.source_map = source_map;
            if (source_map != map) {
                int source_class = dm2_v1_dungeon_get_square_type(
                    &candidate->dungeon, source_map, source_cell_x,
                    source_cell_y);
                int destination_class = dm2_v1_dungeon_get_square_type(
                    &candidate->dungeon, map, x, y);
                int source_square = source_class < 0 ? -1 :
                    dm2_v1_viewport_g1_tile_class_to_square_type(
                        (uint8_t)source_class);
                int destination_square = destination_class < 0 ? -1 :
                    dm2_v1_viewport_g1_tile_class_to_square_type(
                        (uint8_t)destination_class);
                int destination_contains = 0;
                int destination_actuator = 0;
                DM2_V1_SkprojectCutRecordReceipt cut_receipt;
                DM2_V1_SkprojectAppendRecordReceipt append_receipt;
                int16_t source_pool_head;
                int16_t source_raw_head;
                int16_t destination_pool_head;
                int16_t destination_raw_head;
                memset(&cut_receipt, 0, sizeof(cut_receipt));
                memset(&append_receipt, 0, sizeof(append_receipt));
                source_pool_head = (int16_t)dm2_v1_dungeon_get_first_thing(
                    &candidate->dungeon, source_map, source_cell_x,
                    source_cell_y);
                source_raw_head = source_pool_head;
                destination_pool_head = (int16_t)
                    dm2_v1_dungeon_get_first_thing(
                        &candidate->dungeon, map, x, y);
                destination_raw_head = destination_pool_head;
                if ((source_square != DM2_SQUARE_FLOOR &&
                     source_square != DM2_SQUARE_FLOOR_ORNATE) ||
                    (destination_square != DM2_SQUARE_FLOOR &&
                     destination_square != DM2_SQUARE_FLOOR_ORNATE) ||
                    dm2_v1_get_creature_at(&candidate->record_pools,
                                           &candidate->dungeon, map, x, y) !=
                        DM2_V1_RECORD_HANDLE_NULL ||
                    !dm2_v1_game_load_record_chain_mirrors_complete(
                        &candidate->record_pools, &candidate->dungeon, map,
                        x, y, record_handle, &destination_contains,
                        &destination_actuator) || destination_contains ||
                    destination_actuator) {
                    receipt.blocked_source_chain = 1;
                    goto moverec_rollback;
                }
                {
                    int pool_cut_ok = dm2_v1_record_pool_cut_from_list(
                        &candidate->record_pools, &source_pool_head,
                        record_handle);
                    int raw_cut_ok = pool_cut_ok &&
                        dm2_v1_skproject_cut_record_from(
                            &candidate->dungeon, (uint16_t)record_handle,
                            (uint16_t *)&source_raw_head,
                            -1, -1, -1, &cut_receipt);
                    if (!pool_cut_ok || !raw_cut_ok || !cut_receipt.valid) {
                        receipt.blocked_source_chain = 1;
                        goto moverec_rollback;
                    }
                }
                if (dm2_v1_dungeon_set_first_thing(
                        &candidate->dungeon, source_map, source_cell_x,
                        source_cell_y, (uint16_t)source_pool_head) != 0 ||
                    dm2_v1_dungeon_set_first_thing(
                        &candidate->dungeon, source_map, source_cell_x,
                        source_cell_y, (uint16_t)source_raw_head) != 0)
                    goto moverec_rollback;
                if (!dm2_v1_record_pool_append_to_list(
                        &candidate->record_pools, &destination_pool_head,
                        record_handle) ||
                    !dm2_v1_skproject_append_record_to(
                        &candidate->dungeon, (uint16_t)record_handle,
                        (uint16_t *)&destination_raw_head, -1, -1, -1,
                        &append_receipt) ||
                    !append_receipt.valid ||
                    dm2_v1_dungeon_set_first_thing(
                        &candidate->dungeon, map, x, y,
                        (uint16_t)destination_pool_head) != 0 ||
                    dm2_v1_dungeon_set_first_thing(
                        &candidate->dungeon, map, x, y,
                        (uint16_t)destination_raw_head) != 0) {
                    receipt.blocked_destination_chain = 1;
                    goto moverec_rollback;
                }
                source_x = source_cell_x;
                source_y = source_cell_y;
                found = 1;
                cross_map_move = 1;
            }
        }
    }
    /* The source B record must already be rooted on this c_map.  Locate the
     * source square by bounded ground-chain scans; no coordinate is guessed
     * from the record payload. */
    for (int sy = 0; !party_move && !cross_map_move &&
         sy < candidate->dungeon.level_heights[map] && !found; ++sy) {
        for (int sx = 0; sx < candidate->dungeon.level_widths[map]; ++sx) {
            int chain_contains = 0;
            source_head = dm2_v1_dungeon_get_first_thing(
                &candidate->dungeon, map, sx, sy);
            if (source_head < 0) continue;
            if (!dm2_v1_game_load_record_chain_is_bounded(
                    &candidate->record_pools, (int16_t)source_head,
                    record_handle, &chain_contains))
                continue;
            if (chain_contains) {
                source_x = sx;
                source_y = sy;
                found = 1;
            }
        }
    }
    if (!party_move && (!found || (source_x == x && source_y == y))) {
        receipt.blocked_source_chain = 1;
        goto moverec_rollback;
    }
    destination_head = dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, map, x, y);
    if (!party_move && !cross_map_move && (destination_head < 0 ||
        !dm2_v1_game_load_record_chain_is_bounded(
            &candidate->record_pools, (int16_t)destination_head,
            record_handle, &contains))) {
        receipt.blocked_destination_chain = 1;
        goto moverec_rollback;
    }
    if (!party_move && !cross_map_move && contains) {
        receipt.blocked_destination_chain = 1;
        goto moverec_rollback;
    }
    if (!party_move) {
        receipt.source_x = (int16_t)source_x;
        receipt.source_y = (int16_t)source_y;
    }
    if (!party_move && !cross_map_move &&
        (!dm2_v1_game_load_runtime_session_candidate_move_record_to(
             candidate, record_handle, (int16_t)source_x, (int16_t)source_y,
             (int16_t)x, (int16_t)y, &move_receipt) || !move_receipt.valid)) {
        receipt.blocked_source_chain = move_receipt.blocked_source_chain;
        receipt.blocked_destination_chain = move_receipt.blocked_destination_chain;
        goto moverec_rollback;
    }
    if (!party_move && (!dm2_v1_game_load_runtime_session_candidate_activate_moverec_caii(
            candidate, record_handle, (int16_t)x, (int16_t)y, &caii_receipt) ||
        !caii_receipt.valid || !caii_receipt.timer_updated)) {
        receipt.blocked_caii = 1;
        goto moverec_rollback;
    }
    dm2_v1_sound_queue_state_init(&sound_state, 0u);
    if (!dm2_v1_sound_queue_bind_entries(
            &sound_state, candidate->sound_owner.queue_entries,
            candidate->sound_owner.queue_entry_count,
            candidate->sound_owner.queue_capacity)) {
        receipt.blocked_sound = 1;
        goto moverec_rollback;
    }
    sound_state.positional_count = candidate->sound_owner.positional_count;
    sound_state.immediate_count = candidate->sound_owner.immediate_count;
    sound_state.sound_enabled = candidate->sound_owner.sound_enabled;
    sound_state.master_sfx_volume = candidate->sound_owner.master_sfx_volume;
    memcpy(sound_state.positional, candidate->sound_owner.positional,
           sizeof(sound_state.positional));
    memcpy(sound_state.immediate, candidate->sound_owner.immediate,
           sizeof(sound_state.immediate));
    memcpy(sound_state.delayed, candidate->sound_owner.delayed,
           sizeof(sound_state.delayed));
    memcpy(sound_state.sample_slots, candidate->sound_owner.sample_slots,
           sizeof(sound_state.sample_slots));
    sound_env.current_map = (int16_t)map;
    sound_env.gate_map_a = candidate->sound_owner.spatial_audible_map;
    sound_env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
    sound_env.facing = candidate->source_party_direction;
    sound_env.party_x = candidate->source_party_x;
    sound_env.party_y = candidate->source_party_y;
    sound_env.gametick = (int32_t)game_tick;
    if (!party_move && !dm2_v1_sound_queue_noise_gen1(
            &sound_state, 3, 0, (int8_t)0x89, 0x61, 0x80,
            (int16_t)x, (int16_t)y, 1, &sound_env, &sound_receipt)) {
        if (!sound_receipt.rejected_map_gate) {
            receipt.blocked_sound = 1;
            goto moverec_rollback;
        }
        receipt.noise_map_gate_noop = 1;
    } else if (party_move) {
        /* c_tim's party-sentinel path has no creature movement sound
         * record.  Preserve the source no-op while keeping the sound owner
         * inside the rollback boundary. */
        receipt.noise_map_gate_noop = 1;
    } else {
        receipt.noise_queued = 1;
    }
    candidate->sound_owner.positional_count = sound_state.positional_count;
    candidate->sound_owner.immediate_count = sound_state.immediate_count;
    memcpy(candidate->sound_owner.positional, sound_state.positional,
           sizeof(candidate->sound_owner.positional));
    memcpy(candidate->sound_owner.immediate, sound_state.immediate,
           sizeof(candidate->sound_owner.immediate));
    memcpy(candidate->sound_owner.delayed, sound_state.delayed,
           sizeof(candidate->sound_owner.delayed));
    memcpy(candidate->sound_owner.sample_slots, sound_state.sample_slots,
           sizeof(candidate->sound_owner.sample_slots));
    receipt.moved = 1;
    receipt.valid = 1;
    receipt.consumed = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record_handle ^ ((uint32_t)(uint16_t)x << 16) ^
        (uint32_t)(uint16_t)y ^ ((uint32_t)receipt.timer_type << 24));
    dm2_v1_record_pool_set_free(&pool_backup);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    free(dungeon_backup); free(caii_backup);
    free(index_backup); free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

moverec_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
    }
    if (caii_backup && candidate->caii_slots.slots)
        memcpy(candidate->caii_slots.slots, caii_backup, caii_bytes);
    candidate->caii_slots.alloc_count = caii_alloc_count_backup;
    if (dungeon_backup && candidate->dungeon.raw_data)
        memcpy(candidate->dungeon.raw_data, dungeon_backup,
               (size_t)candidate->dungeon.raw_size);
    if (pool_backup.valid) {
        dm2_v1_record_pool_set_free(&candidate->record_pools);
        candidate->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (sound_backup.valid) {
        dm2_v1_game_load_runtime_candidate_sound_free(&candidate->sound_owner);
        candidate->sound_owner = sound_backup;
        memset(&sound_backup, 0, sizeof(sound_backup));
    }
    candidate->source_party_x = old_party_x;
    candidate->source_party_y = old_party_y;
    candidate->source_display_map = old_display_map;
    candidate->source_runtime_display_x = old_runtime_display_x;
    candidate->source_runtime_display_y = old_runtime_display_y;
    candidate->source_runtime_display_direction = old_runtime_display_direction;
    candidate->source_runtime_display_uses_alternate =
        old_runtime_display_uses_alternate;
    if (candidate->current_map != old_map && old_map >= 0)
        (void)dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, old_map, 0);
    goto moverec_fail;

moverec_fail:
    dm2_v1_record_pool_set_free(&pool_backup);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    free(dungeon_backup); free(caii_backup);
    free(index_backup); free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_think_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadThinkTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadThinkTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_SourceTimer source_timer;
    DM2_V1_ThinkCreatureBinding binding;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    int16_t timer_slot;
    int old_map;
    int map;
    int x;
    int y;
    int16_t creature;

    memset(&receipt, 0, sizeof(receipt));
    receipt.creature_record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto think_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto think_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto think_rollback;
    }
    receipt.timer_type = candidate->timer_entries[timer_slot].ttype;
    if (receipt.timer_type != 0x21u && receipt.timer_type != 0x22u) {
        receipt.blocked_unsupported_type = 1;
        goto think_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(int8_t)timer_entry.xA;
    y = (int)(int8_t)timer_entry.yA;
    receipt.map = map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    if (map < 0 || map >= candidate->dungeon.level_count || x < 0 || y < 0 ||
        x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto think_rollback;
    }
    receipt.map_switch = old_map != map;
    source_timer.ticks_and_map = (uint32_t)timer_entry.l_00;
    source_timer.type = timer_entry.ttype;
    source_timer.actor = timer_entry.actor;
    source_timer.value_a = (int16_t)((uint8_t)x | ((uint16_t)(uint8_t)y << 8));
    source_timer.value_b = timer_entry.wvalueB;
    source_timer.reserved = timer_entry.dummya;
    dm2_v1_think_creature_binding_init(&binding,
                                       &candidate->record_pools,
                                       &candidate->dungeon);
    if (!dm2_v1_think_creature_timer_handler(&binding, &source_timer,
                                             (uint16_t)timer_slot, NULL)) {
        receipt.blocked_map = 1;
        goto think_rollback;
    }
    creature = binding.receipt.last_record;
    receipt.creature_record = creature;
    receipt.no_creature_at_cell = binding.receipt.no_creature_at_cell != 0;
    receipt.body_unbound = binding.receipt.body_unbound != 0;
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)timer_entry.l_00 ^ ((uint32_t)timer_entry.ttype << 24) ^
        (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

think_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (candidate->current_map != old_map) {
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
        }
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

static int dm2_v1_game_load_candidate_dispatch_counter_chain(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t direction, uint8_t incoming_action,
    DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int limit = 0, count = 0, steps = 0;
    int16_t link;
    int16_t *links = NULL;
    uint16_t *new_data = NULL;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    uint32_t hash = 0x434e5452u; /* "CNTR" */
    int result = 0;

    if (!candidate || !receipt || incoming_action > 2u || map < 0 ||
        map >= candidate->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        limit += pool->record_count + pool->extension_count;
    }
    if (limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    links = (int16_t *)calloc((size_t)limit, sizeof(*links));
    new_data = (uint16_t *)calloc((size_t)limit, sizeof(*new_data));
    messages = (DM2_V1_TimerEntry *)calloc((size_t)limit, sizeof(*messages));
    if (!links || !new_data || !messages) goto done;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint16_t old_data, data_after;
        int before_active, after_active, action_to_queue = -1;
        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&candidate->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        if (dm2_actu_type(record) != DM2_ACTU_COUNTER) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        old_data = dm2_actu_data(record);
        data_after = old_data;
        before_active = old_data == 0u || (old_data & 0x0100u) != 0u;
        if (incoming_action == 1u) data_after = (uint16_t)(old_data + 1u);
        else if (incoming_action == 0u &&
                 (!dm2_actu_once_only(record) || old_data != 0u))
            data_after = (uint16_t)(old_data - 1u);
        after_active = data_after == 0u || (data_after & 0x0100u) != 0u;
        if (after_active != before_active) {
            const uint8_t configured_action = dm2_actu_action_type(record);
            if (configured_action == 3u)
                action_to_queue = dm2_actu_revert_effect(record) == after_active ? 1 : 0;
            else if (after_active) action_to_queue = configured_action;
            if (action_to_queue < 0 || action_to_queue > 2 ||
                dm2_actu_xcoord(record) >= candidate->dungeon.level_widths[map] ||
                dm2_actu_ycoord(record) >= candidate->dungeon.level_heights[map]) {
                result = -1;
                goto done;
            }
        }
        if (!dm2_v1_record_pool_address_mut(&candidate->record_pools, link)) {
            result = -1;
            goto done;
        }
        links[count] = link;
        new_data[count] = data_after;
        if (action_to_queue >= 0) {
            DM2_V1_TimerEntry *message = &messages[count];
            dm2_v1_timer_entry_init(message);
            dm2_v1_timer_set_mticks(message, (int16_t)map,
                                    candidate->timer_queue.gametick);
            message->ttype = 0x04u;
            message->actor = action_to_queue == 0 ? 1u :
                             (action_to_queue == 1 ? 3u : 2u);
            message->xA = (int8_t)dm2_actu_xcoord(record);
            message->yA = (int8_t)dm2_actu_ycoord(record);
            message->wvalueB = (int16_t)((uint16_t)dm2_actu_direction(record) |
                ((uint16_t)action_to_queue << 8));
        }
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, old_data);
        hash = dm2_v1_game_load_owner_hash_step(hash, data_after);
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                (uint32_t)(action_to_queue + 1));
        ++count;
        ++steps;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto done;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    queue_backup = candidate->timer_queue;
    for (int i = 0; i < count; ++i) {
        if (messages[i].ttype != 0u &&
            dm2_v1_timer_queue(&candidate->timer_queue, &messages[i]) < 0) {
            memcpy(candidate->timer_entries, timer_backup,
                   (size_t)candidate->timer_capacity * sizeof(*timer_backup));
            memcpy(candidate->timer_indices, index_backup,
                   (size_t)candidate->timer_capacity * sizeof(*index_backup));
            candidate->timer_queue = queue_backup;
            goto done;
        }
    }
    for (int i = 0; i < count; ++i) {
        uint8_t *record = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, links[i]);
        if (!record) goto done;
        if (dm2_actu_data(record) != new_data[i]) {
            dm2_actu_set_data(record, new_data[i]);
            ++receipt->counter_records_mutated;
        }
        if (messages[i].ttype != 0u) ++receipt->counter_messages_queued;
    }
    receipt->counter_actuators_seen = count;
    receipt->private_counter_hash = hash;
    result = 1;
done:
    free(index_backup);
    free(timer_backup);
    free(messages);
    free(new_data);
    free(links);
    return result;
}

static int dm2_v1_game_load_candidate_dispatch_door(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t action, int raw_tile,
    DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int chain_limit = 0;
    int chain_count = 0;
    int16_t link;
    int16_t door_link;
    int direction;
    uint8_t *door_record;
    uint16_t attributes_before;
    uint16_t attributes_after;
    DM2_V1_TimerEntry message;

    if (!candidate || !receipt || map < 0 || map >= candidate->dungeon.level_count ||
        ((unsigned int)raw_tile >> 5) != 4u || action > 2u)
        return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_NULL || link == DM2_V1_RECORD_HANDLE_END ||
        dm2_v1_record_handle_pool(link) != DM2_DB_DOOR ||
        !dm2_v1_record_pool_address(&candidate->record_pools, link))
        return -1;
    door_link = link;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return -1;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        int16_t next;
        if (link == DM2_V1_RECORD_HANDLE_NULL || chain_count++ >= chain_limit ||
            !dm2_v1_record_pool_address(&candidate->record_pools, link) ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, link, &next))
            return -1;
        if (dm2_v1_record_handle_pool(link) == DM2_DB_CREATURE)
            return -1;
        link = next;
    }
    door_record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                                  door_link);
    if (!door_record) return -1;
    attributes_before = (uint16_t)door_record[2] |
                        ((uint16_t)door_record[3] << 8);
    receipt->door_record_attributes_before = attributes_before;
    if (dm2_door_get_state((uint16_t)raw_tile) == DM2_DOOR_STATE_DESTROYED) {
        receipt->door_actuator_source_noop_destroyed = 1;
        receipt->door_actuator_record =
            door_link;
        receipt->valid = 1;
        receipt->consumed = 1;
        receipt->source_noop = 1;
        return 1;
    }
    direction = action == 0u ? 0 : action == 1u ? 1 :
        (dm2_door_get_state((uint16_t)raw_tile) == DM2_DOOR_STATE_OPEN ? 1 : 0);
    dm2_v1_timer_entry_init(&message);
    dm2_v1_timer_set_mticks(&message, (int16_t)map,
                             candidate->timer_queue.gametick);
    message.ttype = 0x01u;
    message.actor = (uint8_t)direction;
    message.xA = (int8_t)x;
    message.yA = (int8_t)y;
    message.wvalueB = door_link;
    if (dm2_v1_timer_queue(&candidate->timer_queue, &message) < 0)
        return -1;
    /* DME.h::Door runtime bits and skevent.cpp::ACTUATE_DOOR: preserve the
     * static low nine bits, publish the requested animation direction, and
     * mark the private next-step timer as live. */
    attributes_after = (uint16_t)((attributes_before & ~(uint16_t)0x1e00u) |
                                   ((uint16_t)direction << 9) |
                                   0x1400u);
    door_record[2] = (uint8_t)attributes_after;
    door_record[3] = (uint8_t)(attributes_after >> 8);
    receipt->door_actuator_timer_queued = 1;
    receipt->door_actuator_record = door_link;
    receipt->door_actuator_direction = (uint8_t)direction;
    receipt->door_record_attributes_after = attributes_after;
    receipt->door_record_state_mutated = attributes_after != attributes_before;
    receipt->valid = 1;
    receipt->consumed = 1;
    return 1;
}

static int dm2_v1_game_load_candidate_dispatch_floor_text(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t tile_class, uint8_t action,
    int raw_tile, DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int16_t root, link;
    int max_links, link_count = 0;
    int16_t *links = NULL;
    uint8_t *visibility = NULL;
    uint32_t hash = 2166136261u;
    uint16_t deferred_tile_raw = 0u;
    int deferred_pit_tele_close = 0;
    int success = 0;

    if (!candidate || !receipt || map < 0 || map >= candidate->dungeon.level_count)
        return -1;
    if (tile_class == 2u || tile_class == 5u) {
        uint8_t current_open = (uint8_t)(((unsigned int)raw_tile >> 3) & 1u);
        uint8_t requested_action = action;
        receipt->tile_state_before = current_open;
        if (requested_action == 2u) requested_action = current_open;
        if (requested_action == 0u) {
            receipt->blocked_unowned_tile_advance = 1;
            return -1;
        }
        deferred_tile_raw = (uint16_t)((unsigned int)raw_tile & ~0x08u);
        deferred_pit_tele_close = 1;
        receipt->tile_state_after = 0u;
    }
    root = (int16_t)dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, map, x, y);
    if (root == DM2_V1_RECORD_HANDLE_END || root == DM2_V1_RECORD_HANDLE_NULL ||
        candidate->record_pools.pools[2].record_size != 4 ||
        candidate->record_pools.pools[2].record_count < 0 ||
        candidate->record_pools.pools[2].extension_count < 0)
        return -1;
    max_links = candidate->record_pools.pools[2].record_count +
                candidate->record_pools.pools[2].extension_count;
    if (max_links <= 0) return -1;
    links = (int16_t *)calloc((size_t)max_links, sizeof(*links));
    visibility = (uint8_t *)calloc((size_t)max_links, sizeof(*visibility));
    if (!links || !visibility) goto done;
    link = root;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        uint16_t w2, text_index;
        uint8_t mode, ext_usage, complex_usage;
        uint8_t old_visibility, new_visibility;
        int16_t next;
        if (link == DM2_V1_RECORD_HANDLE_NULL || link_count >= max_links ||
            dm2_v1_record_handle_pool(link) != DM2_DB_TEXT) {
            receipt->blocked_non_text_chain = 1;
            goto done;
        }
        record = dm2_v1_record_pool_address(&candidate->record_pools, link);
        if (!record || !dm2_v1_record_pool_next_link(
                &candidate->record_pools, link, &next))
            goto done;
        w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
        mode = (uint8_t)((w2 >> 1) & 3u);
        text_index = (uint16_t)((w2 >> 3) & 0x1fffu);
        ext_usage = (uint8_t)((text_index >> 8) & 0x1fu);
        complex_usage = (uint8_t)((text_index >> 9) & 0x0fu);
        old_visibility = (uint8_t)(w2 & 1u);
        if (mode != 0u && !(mode == 1u && ext_usage == 5u) &&
            !(mode == 2u && complex_usage == 2u)) {
            new_visibility = old_visibility;
        } else {
            new_visibility = action == 2u ? (uint8_t)!old_visibility :
                (action == 0u ? 1u : 0u);
            if (mode == 0u && old_visibility == 0u && new_visibility != 0u &&
                map == candidate->source_party_map &&
                x == (int)candidate->source_party_x &&
                y == (int)candidate->source_party_y) {
                receipt->blocked_hint_delivery = 1;
                goto done;
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
    if (link_count == 0) goto done;
    for (int i = 0; i < link_count; ++i)
        if (!dm2_v1_record_pool_address_mut(&candidate->record_pools, links[i]))
            goto done;
    if (deferred_pit_tele_close && dm2_v1_dungeon_set_tile_raw(
            &candidate->dungeon, map, x, y, deferred_tile_raw) != 0)
        goto done;
    for (int i = 0; i < link_count; ++i) {
        uint8_t *record = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, links[i]);
        uint16_t w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
        if ((uint8_t)(w2 & 1u) != visibility[i]) {
            w2 = (uint16_t)((w2 & ~1u) | visibility[i]);
            record[2] = (uint8_t)w2;
            record[3] = (uint8_t)(w2 >> 8);
            ++receipt->text_records_toggled;
        }
    }
    receipt->text_records_seen = link_count;
    receipt->pit_tele_tile_mutated = deferred_pit_tele_close &&
                                     (((unsigned int)raw_tile & 0x08u) != 0u);
    receipt->private_text_visibility_hash = hash;
    if (deferred_pit_tele_close)
        receipt->private_text_visibility_hash = dm2_v1_game_load_owner_hash_step(
            receipt->private_text_visibility_hash, deferred_tile_raw);
    receipt->valid = hash != 0u;
    success = receipt->valid ? 1 : 0;
done:
    free(visibility);
    free(links);
    return success;
}

static int dm2_v1_game_load_candidate_dispatch_finite_relay_chain(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t direction, uint8_t incoming_action,
    DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int limit = 0, count = 0, message_count = 0, steps = 0;
    int16_t link;
    int16_t *links = NULL;
    uint16_t *new_data = NULL;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    uint32_t hash = 0x464e524cu; /* "FNRL" */
    int result = 0;

    if (!candidate || !receipt || incoming_action > 2u || map < 0 ||
        map >= candidate->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        limit += pool->record_count + pool->extension_count;
    }
    if (limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    links = (int16_t *)calloc((size_t)limit, sizeof(*links));
    new_data = (uint16_t *)calloc((size_t)limit, sizeof(*new_data));
    messages = (DM2_V1_TimerEntry *)calloc((size_t)limit, sizeof(*messages));
    if (!links || !new_data || !messages) goto done;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint16_t data;
        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&candidate->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        if (dm2_actu_type(record) != DM2_ACTU_FINITE_RELAY) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        data = dm2_actu_data(record);
        if (data > 400u) {
            result = -1;
            goto done;
        }
        if (data != 0u) {
            if (dm2_actu_xcoord(record) >= candidate->dungeon.level_widths[map] ||
                dm2_actu_ycoord(record) >= candidate->dungeon.level_heights[map] ||
                !dm2_v1_record_pool_address_mut(&candidate->record_pools, link)) {
                result = -1;
                goto done;
            }
            dm2_v1_timer_entry_init(&messages[message_count]);
            dm2_v1_timer_set_mticks(&messages[message_count], (int16_t)map,
                                    candidate->timer_queue.gametick);
            messages[message_count].ttype = 0x04u;
            messages[message_count].actor = incoming_action == 0u ? 1u :
                (incoming_action == 1u ? 3u : 2u);
            messages[message_count].xA = (int8_t)dm2_actu_xcoord(record);
            messages[message_count].yA = (int8_t)dm2_actu_ycoord(record);
            messages[message_count].wvalueB = (int16_t)((uint16_t)
                dm2_actu_direction(record) | ((uint16_t)incoming_action << 8));
            ++message_count;
        }
        links[count] = link;
        new_data[count] = data == 0u ? 0u : (uint16_t)(data - 1u);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, data);
        ++count;
        ++steps;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto done;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    queue_backup = candidate->timer_queue;
    for (int i = 0; i < message_count; ++i) {
        if (dm2_v1_timer_queue(&candidate->timer_queue, &messages[i]) < 0) {
            memcpy(candidate->timer_entries, timer_backup,
                   (size_t)candidate->timer_capacity * sizeof(*timer_backup));
            memcpy(candidate->timer_indices, index_backup,
                   (size_t)candidate->timer_capacity * sizeof(*index_backup));
            candidate->timer_queue = queue_backup;
            goto done;
        }
    }
    for (int i = 0; i < count; ++i) {
        uint8_t *record = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, links[i]);
        if (!record) goto done;
        if (dm2_actu_data(record) != new_data[i]) {
            dm2_actu_set_data(record, new_data[i]);
            ++receipt->finite_relay_records_mutated;
        }
        if (messages[i].ttype != 0u) ++receipt->finite_relay_messages_queued;
    }
    receipt->finite_relay_actuators_seen = count;
    receipt->private_finite_relay_hash = hash;
    receipt->valid = 1;
    result = 1;
done:
    free(index_backup);
    free(timer_backup);
    free(messages);
    free(new_data);
    free(links);
    return result;
}

static int dm2_v1_game_load_candidate_dispatch_cross_map_wall(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t direction, uint8_t action,
    DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int limit = 0, count = 0, steps = 0;
    int16_t link;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    uint32_t hash = 0x43524d50u; /* "CRMP" */
    int result = 0;

    if (!candidate || !receipt || direction > 3u || action > 2u ||
        map < 0 || map >= candidate->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        limit += pool->record_count + pool->extension_count;
    }
    if (limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    messages = (DM2_V1_TimerEntry *)calloc((size_t)limit, sizeof(*messages));
    if (!messages) return -1;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint16_t data;
        int target_map, target_x, target_y;
        uint8_t target_direction;
        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&candidate->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        if (dm2_actu_type(record) != DM2_ACTU_CROSS_MAP) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        data = dm2_actu_data(record);
        target_map = (int)(data & 0x003fu);
        target_x = (int)dm2_actu_xcoord(record);
        target_y = (int)dm2_actu_ycoord(record);
        target_direction = (uint8_t)((data >> 6) & 3u);
        if (target_map < 0 || target_map >= candidate->dungeon.level_count ||
            target_x < 0 || target_y < 0 ||
            target_x >= candidate->dungeon.level_widths[target_map] ||
            target_y >= candidate->dungeon.level_heights[target_map]) {
            result = -1;
            goto done;
        }
        dm2_v1_timer_entry_init(&messages[count]);
        dm2_v1_timer_set_mticks(&messages[count], (int16_t)target_map,
                                candidate->timer_queue.gametick);
        messages[count].ttype = 0x04u;
        messages[count].actor = action == 0u ? 1u : (action == 1u ? 3u : 2u);
        messages[count].xA = (int8_t)target_x;
        messages[count].yA = (int8_t)target_y;
        messages[count].wvalueB = (int16_t)((uint16_t)target_direction |
                                            ((uint16_t)action << 8));
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_map);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_x);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)target_y);
        hash = dm2_v1_game_load_owner_hash_step(hash, target_direction);
        ++count;
        ++steps;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto done;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    queue_backup = candidate->timer_queue;
    for (int i = 0; i < count; ++i) {
        if (dm2_v1_timer_queue(&candidate->timer_queue, &messages[i]) < 0) {
            memcpy(candidate->timer_entries, timer_backup,
                   (size_t)candidate->timer_capacity * sizeof(*timer_backup));
            memcpy(candidate->timer_indices, index_backup,
                   (size_t)candidate->timer_capacity * sizeof(*index_backup));
            candidate->timer_queue = queue_backup;
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

static int dm2_v1_game_load_candidate_dispatch_relay_chain(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t direction, uint8_t incoming_action,
    DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int limit = 0, count = 0, message_count = 0, steps = 0;
    int16_t link;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    uint32_t hash = 0x524c5931u; /* "RLY1" */
    int result = 0;

    if (!candidate || !receipt || incoming_action > 2u || map < 0 ||
        map >= candidate->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        limit += pool->record_count + pool->extension_count;
    }
    if (limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    messages = (DM2_V1_TimerEntry *)calloc((size_t)limit, sizeof(*messages));
    if (!messages) return -1;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint8_t type, action;
        uint32_t fire_tick;
        int blocked_by_source_gate = 0;
        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&candidate->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        type = dm2_actu_type(record);
        if (type != DM2_ACTU_RELAY_1 && type != DM2_ACTU_RELAY_3) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if (dm2_actu_once_only(record) != 0u &&
            (dm2_actu_revert_effect(record) != 0u || incoming_action != 0u) &&
            (dm2_actu_revert_effect(record) == 0u || incoming_action != 1u))
            blocked_by_source_gate = 1;
        action = dm2_actu_once_only(record) != 0u ?
            dm2_actu_action_type(record) : incoming_action;
        if (!blocked_by_source_gate) {
            if (action > 2u ||
                dm2_actu_xcoord(record) >= candidate->dungeon.level_widths[map] ||
                dm2_actu_ycoord(record) >= candidate->dungeon.level_heights[map]) {
                result = -1;
                goto done;
            }
            fire_tick = candidate->timer_queue.gametick +
                (type == DM2_ACTU_RELAY_3 ?
                    ((uint32_t)dm2_actu_data(record) << dm2_actu_delay(record)) :
                    (uint32_t)dm2_actu_data(record) + dm2_actu_delay(record));
            dm2_v1_timer_entry_init(&messages[message_count]);
            dm2_v1_timer_set_mticks(&messages[message_count], (int16_t)map,
                                    fire_tick);
            messages[message_count].ttype = 0x04u;
            messages[message_count].actor = action == 0u ? 1u :
                (action == 1u ? 3u : 2u);
            messages[message_count].xA = (int8_t)dm2_actu_xcoord(record);
            messages[message_count].yA = (int8_t)dm2_actu_ycoord(record);
            messages[message_count].wvalueB = (int16_t)((uint16_t)
                dm2_actu_direction(record) | ((uint16_t)action << 8));
            ++message_count;
        }
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, dm2_actu_data(record));
        hash = dm2_v1_game_load_owner_hash_step(hash, type);
        hash = dm2_v1_game_load_owner_hash_step(hash, blocked_by_source_gate);
        ++count;
        ++steps;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto done;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    queue_backup = candidate->timer_queue;
    for (int i = 0; i < message_count; ++i) {
        if (dm2_v1_timer_queue(&candidate->timer_queue, &messages[i]) < 0) {
            memcpy(candidate->timer_entries, timer_backup,
                   (size_t)candidate->timer_capacity * sizeof(*timer_backup));
            memcpy(candidate->timer_indices, index_backup,
                   (size_t)candidate->timer_capacity * sizeof(*index_backup));
            candidate->timer_queue = queue_backup;
            goto done;
        }
    }
    receipt->relay_actuators_seen = count;
    receipt->relay_messages_queued = message_count;
    receipt->private_relay_hash = hash;
    receipt->valid = 1;
    result = 1;
done:
    free(index_backup);
    free(timer_backup);
    free(messages);
    return result;
}

static int dm2_v1_game_load_candidate_dispatch_push_button_floor(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int map, int x, int y,
    uint8_t action, DM2_V1_GameLoadCandidateActuateReceipt *receipt)
{
    int limit = 0;
    int count = 0;
    int16_t link;
    int16_t *targets = NULL;
    uint8_t *next_bits = NULL;
    uint32_t hash = 0x50425357u; /* "PBSW" */

    if (!candidate || !receipt || action > 2u || map < 0 ||
        map >= candidate->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        limit += pool->record_count + pool->extension_count;
    }
    if (limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon,
                                                    map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    targets = (int16_t *)calloc((size_t)limit, sizeof(*targets));
    next_bits = (uint8_t *)calloc((size_t)limit, sizeof(*next_bits));
    if (!targets || !next_bits) goto fail;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *actuator;
        const uint8_t *door;
        int16_t next;
        int16_t target;
        int target_x, target_y;
        uint8_t old_bit;
        if (link == DM2_V1_RECORD_HANDLE_NULL || count >= limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(actuator = dm2_v1_record_pool_address(&candidate->record_pools,
                                                     link)) ||
            dm2_actu_type(actuator) != DM2_ACTU_PUSH_BUTTON_SWITCH ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, link, &next))
            goto mixed_chain;
        target_x = (int)dm2_actu_xcoord(actuator);
        target_y = (int)dm2_actu_ycoord(actuator);
        if (target_x < 0 || target_y < 0 ||
            target_x >= candidate->dungeon.level_widths[map] ||
            target_y >= candidate->dungeon.level_heights[map])
            goto fail;
        target = (int16_t)dm2_v1_dungeon_get_first_thing(
            &candidate->dungeon, map, target_x, target_y);
        if (target == DM2_V1_RECORD_HANDLE_NULL ||
            target == DM2_V1_RECORD_HANDLE_END ||
            dm2_v1_record_handle_pool(target) != DM2_DB_DOOR ||
            !(door = dm2_v1_record_pool_address(&candidate->record_pools, target)) ||
            !dm2_v1_record_pool_address_mut(&candidate->record_pools, target))
            goto fail;
        old_bit = dm2_door_bit13(door);
        for (int prior = 0; prior < count; ++prior)
            if (targets[prior] == target) old_bit = next_bits[prior];
        targets[count] = target;
        next_bits[count] = action == DM2_ACTMSG_TOGGLE ?
            (uint8_t)!old_bit : (action == DM2_ACTMSG_OPEN_SET ? 1u : 0u);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)target);
        hash = dm2_v1_game_load_owner_hash_step(hash, next_bits[count]);
        ++count;
        link = next;
    }
    if (count == 0 || hash == 0u) goto fail;
    for (int i = 0; i < count; ++i) {
        uint8_t *door = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, targets[i]);
        if (!door) goto fail;
        if (dm2_door_bit13(door) != next_bits[i]) {
            dm2_door_set_bit13(door, next_bits[i]);
            ++receipt->push_button_doors_mutated;
        }
    }
    receipt->push_button_actuators_seen = count;
    receipt->private_push_button_hash = hash;
    receipt->valid = 1;
    free(next_bits);
    free(targets);
    return 1;

mixed_chain:
    /* A chain that started as a push-button family but diverged is an
     * authenticated contradiction, not an absent candidate. */
fail:
    free(next_bits);
    free(targets);
    return count == 0 ? 0 : -1;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_actuate_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateActuateReceipt *out_receipt)
{
    DM2_V1_GameLoadCandidateActuateReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    int16_t timer_slot;
    int old_map;
    int map, x, y, raw_tile;
    uint16_t payload;

    memset(&receipt, 0, sizeof(receipt));
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto actuate_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto actuate_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto actuate_rollback;
    }
    receipt.timer_type = candidate->timer_entries[timer_slot].ttype;
    if (receipt.timer_type != 0x04u) {
        receipt.blocked_unsupported_type = 1;
        goto actuate_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(int8_t)timer_entry.xA;
    y = (int)(int8_t)timer_entry.yA;
    receipt.map = map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    payload = (uint16_t)timer_entry.wvalueB;
    receipt.direction = (uint8_t)(payload & 0xffu);
    receipt.action = (uint8_t)(payload >> 8);
    if (map < 0 || map >= candidate->dungeon.level_count || x < 0 || y < 0 ||
        x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        receipt.direction > 3u || receipt.action > 2u ||
        (receipt.action == 0u && timer_entry.actor != 1u) ||
        (receipt.action == 1u && timer_entry.actor != 3u) ||
        (receipt.action == 2u && timer_entry.actor != 2u) ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto actuate_rollback;
    }
    receipt.map_switch = old_map != map;
    raw_tile = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon, map, x, y);
    if (raw_tile < 0) {
        receipt.blocked_map = 1;
        goto actuate_rollback;
    }
    receipt.raw_tile = (uint8_t)raw_tile;
    receipt.tile_class = (uint8_t)((unsigned int)raw_tile >> 5);
    /* SKProject sktimprc.cpp::DM2_PROCEED_TIMERS: class 3 has an intentional
     * empty 0x04 branch.  The timer has already been popped and the map
     * context authenticated, so consume it as a source no-op instead of
     * falling through to the unowned-class rollback below. */
    if (receipt.tile_class == 3u) {
        receipt.valid = 1;
        receipt.consumed = 1;
        receipt.source_noop = 1;
        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
            candidate->source_transaction_hash ^ candidate->candidate_hash,
            (uint32_t)timer_entry.l_00 ^
            ((uint32_t)receipt.tile_class << 24) ^
            (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
        free(index_backup);
        free(timer_backup);
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if (receipt.tile_class != 3u) {
        if (receipt.tile_class == 0u || receipt.tile_class == 1u) {
            if (receipt.tile_class == 0u) {
                const int cross_map = dm2_v1_game_load_candidate_dispatch_cross_map_wall(
                    candidate, map, x, y, receipt.direction, receipt.action,
                    &receipt);
                if (cross_map > 0) {
                    receipt.valid = 1;
                    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                        candidate->source_transaction_hash ^ candidate->candidate_hash,
                        receipt.private_cross_map_hash ^
                        (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
                    free(index_backup);
                    free(timer_backup);
                    if (out_receipt) *out_receipt = receipt;
                    return 1;
                }
                if (cross_map < 0) {
                    receipt.blocked_unowned_class = 1;
                    goto actuate_rollback;
                }
            }
            const int push_button =
                dm2_v1_game_load_candidate_dispatch_push_button_floor(
                    candidate, map, x, y, receipt.action, &receipt);
            if (push_button > 0) {
                receipt.valid = 1;
                receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                    candidate->source_transaction_hash ^ candidate->candidate_hash,
                    receipt.private_push_button_hash ^
                    (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
                free(index_backup);
                free(timer_backup);
                if (out_receipt) *out_receipt = receipt;
                return 1;
            }
            if (push_button < 0) {
                receipt.blocked_unowned_class = 1;
                goto actuate_rollback;
            }
            {
                const int counter = dm2_v1_game_load_candidate_dispatch_counter_chain(
                    candidate, map, x, y, receipt.direction, receipt.action,
                    &receipt);
                if (counter > 0) {
                    receipt.valid = 1;
                    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                        candidate->source_transaction_hash ^ candidate->candidate_hash,
                        receipt.private_counter_hash ^
                        (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
                    free(index_backup);
                    free(timer_backup);
                    if (out_receipt) *out_receipt = receipt;
                    return 1;
                }
                if (counter < 0) {
                    receipt.blocked_unowned_class = 1;
                    goto actuate_rollback;
                }
                {
                    const int relay = dm2_v1_game_load_candidate_dispatch_relay_chain(
                        candidate, map, x, y, receipt.direction, receipt.action,
                        &receipt);
                    if (relay > 0) {
                        receipt.valid = 1;
                        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                            candidate->source_transaction_hash ^ candidate->candidate_hash,
                            receipt.private_relay_hash ^
                            (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
                        free(index_backup);
                        free(timer_backup);
                        if (out_receipt) *out_receipt = receipt;
                        return 1;
                    }
                    if (relay < 0) {
                        receipt.blocked_unowned_class = 1;
                        goto actuate_rollback;
                    }
                    {
                        const int finite_relay =
                            dm2_v1_game_load_candidate_dispatch_finite_relay_chain(
                                candidate, map, x, y, receipt.direction,
                                receipt.action, &receipt);
                        if (finite_relay > 0) {
                            receipt.valid = 1;
                            receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                                candidate->source_transaction_hash ^ candidate->candidate_hash,
                                receipt.private_finite_relay_hash ^
                                (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
                            free(index_backup);
                            free(timer_backup);
                            if (out_receipt) *out_receipt = receipt;
                            return 1;
                        }
                        if (finite_relay < 0) {
                            receipt.blocked_unowned_class = 1;
                            goto actuate_rollback;
                        }
                    }
                }
            }
        }
    }
    if (receipt.tile_class == 4u) {
        const int door = dm2_v1_game_load_candidate_dispatch_door(
            candidate, map, x, y, receipt.action, raw_tile, &receipt);
        if (door > 0) {
            receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                candidate->source_transaction_hash ^ candidate->candidate_hash,
                (uint32_t)(uint16_t)receipt.door_actuator_record ^
                ((uint32_t)receipt.door_actuator_direction << 16) ^
                ((uint32_t)(uint16_t)x << 1) ^ ((uint32_t)(uint16_t)y << 17));
            free(index_backup);
            free(timer_backup);
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        receipt.blocked_unowned_class = 1;
        goto actuate_rollback;
    }
    if (receipt.tile_class == 1u || receipt.tile_class == 2u ||
        receipt.tile_class == 5u) {
        const int floor_text = dm2_v1_game_load_candidate_dispatch_floor_text(
            candidate, map, x, y, receipt.tile_class, receipt.action,
            raw_tile, &receipt);
        if (floor_text > 0) {
            receipt.valid = 1;
            receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                candidate->source_transaction_hash ^ candidate->candidate_hash,
                receipt.private_text_visibility_hash ^
                (uint32_t)(uint16_t)x ^ ((uint32_t)(uint16_t)y << 16));
            free(index_backup);
            free(timer_backup);
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (floor_text < 0 && receipt.blocked_unowned_tile_advance == 0)
            receipt.blocked_unowned_class = 1;
        goto actuate_rollback;
    }
    receipt.blocked_unowned_class = 1;
    goto actuate_rollback;
actuate_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_tick_generator_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateTickGeneratorReceipt *out_receipt)
{
    DM2_V1_GameLoadCandidateTickGeneratorReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry message;
    DM2_V1_TimerEntry continuation;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t *record = NULL;
    uint8_t record_byte4 = 0u;
    int16_t timer_slot;
    int old_map;
    int map;
    int16_t record_link;
    uint16_t w2, w4;
    int control_bit2, alternating, remaining, should_invoke;
    uint8_t action, alternating_toggle = 0u;

    memset(&receipt, 0, sizeof(receipt));
    receipt.record_link = DM2_V1_RECORD_HANDLE_NULL;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto generator_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) goto generator_rollback;
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x56u)
        goto generator_rollback;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    record_link = (int16_t)((uint16_t)(uint8_t)timer_entry.xA |
                            ((uint16_t)(uint8_t)timer_entry.yA << 8));
    receipt.record_link = record_link;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        dm2_v1_record_handle_pool(record_link) != 3 ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0) ||
        !(record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                                   record_link)))
        goto generator_rollback;
    receipt.action = 0u;
    record_byte4 = record[4];
    w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
    control_bit2 = (w4 & 0x0004u) != 0u;
    alternating = (w4 & 0x0018u) == 0x0018u;
    if (alternating) {
        alternating_toggle = (uint8_t)(((uint16_t)timer_entry.wvalueB >> 8) & 1u) ^ 1u;
        action = alternating_toggle == 0u ? 1u : 0u;
        remaining = control_bit2 || alternating_toggle != 0u;
        should_invoke = 1;
    } else {
        action = (uint8_t)((w4 >> 3) & 3u);
        remaining = control_bit2;
        should_invoke = control_bit2;
    }
    receipt.action = action;
    receipt.target_x = dm2_actu_xcoord(record);
    receipt.target_y = dm2_actu_ycoord(record);
    receipt.target_direction = dm2_actu_direction(record);
    if (should_invoke) {
        dm2_v1_timer_entry_init(&message);
        dm2_v1_timer_set_mticks(&message, (int16_t)map,
            candidate->timer_queue.gametick + ((w4 >> 7) & 0x007fu));
        message.ttype = 0x04u;
        if (action == 0u) message.actor = 1u;
        else if (action == 1u) message.actor = 3u;
        else if (action == 2u) message.actor = 2u;
        message.xA = (int8_t)receipt.target_x;
        message.yA = (int8_t)receipt.target_y;
        message.wvalueB = (int16_t)((uint16_t)receipt.target_direction |
                                    ((uint16_t)action << 8));
        if (dm2_v1_timer_queue(&candidate->timer_queue, &message) < 0)
            goto generator_rollback;
        receipt.actuator_invoked = 1;
        receipt.message_queued = 1;
    }
    if (remaining) {
        const int multiplier = dm2_v1_game_load_owner_tick_multiplier(
            (uint8_t)(w2 & 0x007fu));
        const uint16_t period = w2 >> 7;
        if (multiplier == 0 || period == 0u ||
            (uint8_t)timer_entry.wvalueB != (uint8_t)multiplier)
            goto generator_rollback;
        continuation = timer_entry;
        if (alternating)
            continuation.wvalueB = (int16_t)((uint16_t)(uint8_t)timer_entry.wvalueB |
                                             ((uint16_t)alternating_toggle << 8));
        continuation.l_00 += (int32_t)((uint32_t)period * (uint32_t)multiplier);
        if (dm2_v1_timer_queue(&candidate->timer_queue, &continuation) < 0)
            goto generator_rollback;
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

generator_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    if (record) record[4] = record_byte4;
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_door_step_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateDoorStepReceipt *out_receipt)
{
    DM2_V1_GameLoadCandidateDoorStepReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry next_timer;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    int16_t timer_slot;
    int old_map;
    int map, x, y, raw_tile, old_state, new_state;
    int16_t link, chain_link;
    int chain_limit = 0;
    int chain_count = 0;
    int queued = 0;
    uint8_t *door_record = NULL;
    uint8_t door_record_backup[4] = { 0u, 0u, 0u, 0u };

    memset(&receipt, 0, sizeof(receipt));
    receipt.game_tick = game_tick;
    receipt.door_record_link = DM2_V1_RECORD_HANDLE_NULL;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto door_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto door_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto door_rollback;
    }
    receipt.timer_type = candidate->timer_entries[timer_slot].ttype;
    if (receipt.timer_type != 0x01u) {
        receipt.blocked_unsupported_type = 1;
        goto door_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(int8_t)timer_entry.xA;
    y = (int)(int8_t)timer_entry.yA;
    receipt.map = map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    receipt.direction = timer_entry.actor;
    link = timer_entry.wvalueB;
    receipt.door_record_link = link;
    if (map < 0 || map >= candidate->dungeon.level_count || x < 0 || y < 0 ||
        x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        receipt.direction > 1u || link == DM2_V1_RECORD_HANDLE_NULL ||
        link == DM2_V1_RECORD_HANDLE_END ||
        dm2_v1_record_handle_pool(link) != 0 ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto door_rollback;
    }
    receipt.map_switch = old_map != map;
    raw_tile = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon, map, x, y);
    if (raw_tile < 0 || ((unsigned int)raw_tile >> 5) != 4u ||
        dm2_v1_dungeon_get_first_thing(&candidate->dungeon, map, x, y) != link ||
        !(door_record = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, link))) {
        receipt.blocked_record = 1;
        goto door_rollback;
    }
    memcpy(door_record_backup, door_record, sizeof(door_record_backup));
    receipt.door_record_attributes_before =
        (uint16_t)door_record[2] | ((uint16_t)door_record[3] << 8);
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count) {
            receipt.blocked_incomplete_chain = 1;
            goto door_rollback;
        }
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) {
        receipt.blocked_incomplete_chain = 1;
        goto door_rollback;
    }
    chain_link = link;
    while (chain_link != DM2_V1_RECORD_HANDLE_END) {
        int16_t next;
        if (chain_link == DM2_V1_RECORD_HANDLE_NULL ||
            chain_count++ >= chain_limit ||
            !dm2_v1_record_pool_address(&candidate->record_pools, chain_link) ||
            !dm2_v1_record_pool_next_link(&candidate->record_pools, chain_link, &next)) {
            receipt.blocked_incomplete_chain = 1;
            goto door_rollback;
        }
        if (dm2_v1_record_handle_pool(chain_link) == 4) {
            receipt.blocked_creature_collision = 1;
            goto door_rollback;
        }
        chain_link = next;
    }
    if (map == candidate->source_party_map && x == (int)candidate->source_party_x &&
        y == (int)candidate->source_party_y) {
        receipt.blocked_party_collision = 1;
        goto door_rollback;
    }
    old_state = dm2_door_get_state((uint16_t)raw_tile);
    receipt.state_before = (uint8_t)old_state;
    if (old_state == DM2_DOOR_STATE_DESTROYED) {
        receipt.state_after = (uint8_t)old_state;
        receipt.source_noop_destroyed = 1;
    } else {
        new_state = dm2_door_apply_toggle_step(old_state, receipt.direction);
        if (new_state < DM2_DOOR_STATE_OPEN || new_state > DM2_DOOR_STATE_CLOSED)
            goto door_rollback;
        receipt.state_after = (uint8_t)new_state;
        if ((receipt.direction == 0u && new_state != DM2_DOOR_STATE_OPEN) ||
            (receipt.direction == 1u && new_state != DM2_DOOR_STATE_CLOSED)) {
            next_timer = timer_entry;
            next_timer.l_00 += 1;
            if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
                goto door_rollback;
            queued = 1;
        }
        if (new_state != old_state && dm2_v1_dungeon_set_tile_raw(
                &candidate->dungeon, map, x, y,
                dm2_door_set_state((uint16_t)raw_tile, new_state)) != 0)
            goto door_rollback;
        receipt.door_state_mutated = new_state != old_state;
        {
            uint16_t attributes = receipt.door_record_attributes_before;
            uint16_t next_attributes = (uint16_t)(
                attributes & ~(uint16_t)0x1e00u);
            next_attributes = (uint16_t)(next_attributes |
                ((uint16_t)receipt.direction << 9));
            if (queued) next_attributes = (uint16_t)(next_attributes | 0x1400u);
            door_record[2] = (uint8_t)next_attributes;
            door_record[3] = (uint8_t)(next_attributes >> 8);
            receipt.door_record_attributes_after = next_attributes;
            receipt.door_record_state_mutated =
                next_attributes != receipt.door_record_attributes_before;
        }
    }
    receipt.valid = 1;
    receipt.consumed = 1;
    receipt.requeued = queued;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)timer_entry.l_00 ^ ((uint32_t)old_state << 8) ^
        ((uint32_t)receipt.state_after << 16) ^ (uint32_t)(uint16_t)link);
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

door_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    if (door_record)
        memcpy(door_record, door_record_backup, sizeof(door_record_backup));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_process_0e_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadProcess0eTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadProcess0eTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_Party party_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t *record_backup = NULL;
    uint8_t *record;
    int16_t timer_slot;
    int old_map;
    int map;
    int record_size;
    uint16_t db_type;
    uint16_t record_handle;
    uint8_t actor;
    DM2_V1_ProcessItemBonusReceipt bonus;

    memset(&receipt, 0, sizeof(receipt));
    memset(&bonus, 0, sizeof(bonus));
    receipt.game_tick = game_tick;
    receipt.timer_type = 0x0eu;
    receipt.record = DM2_V1_RECORD_HANDLE_NULL;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        goto process_0e_fail;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        goto process_0e_fail;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    party_backup = candidate->party;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto process_0e_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto process_0e_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto process_0e_rollback;
    }
    if (candidate->timer_entries[timer_slot].ttype != 0x0eu) {
        receipt.blocked_unsupported_type = 1;
        goto process_0e_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    receipt.map = map;
    receipt.map_switch = old_map != map;
    actor = timer_entry.actor;
    db_type = (uint16_t)(uint8_t)timer_entry.xA & 0x03ffu;
    record_handle = (uint16_t)(db_type << 10);
    receipt.actor = actor;
    receipt.record_db_type = db_type;
    receipt.record = record_handle;
    receipt.value2 = (uint16_t)timer_entry.wvalueB;
    receipt.bonus_value = (int16_t)0xfffe;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        actor >= (uint8_t)candidate->party.heros_in_party ||
        actor >= DM2_MAX_HEROES || !candidate->asset_loader ||
        !candidate->asset_loader->loaded ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, map, 0)) {
        receipt.blocked_map = map < 0 || map >= candidate->dungeon.level_count;
        receipt.blocked_actor = actor >= (uint8_t)candidate->party.heros_in_party ||
                                actor >= DM2_MAX_HEROES;
        goto process_0e_rollback;
    }
    record_size = dm2_v1_record_pool_record_size((int)db_type);
    record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                             (int16_t)record_handle);
    if (record_size <= 0 || !record) {
        receipt.blocked_record = 1;
        goto process_0e_rollback;
    }
    record_backup = (uint8_t *)malloc((size_t)record_size);
    if (!record_backup) goto process_0e_rollback;
    memcpy(record_backup, record, (size_t)record_size);
    if (!dm2_v1_record_pool_set_itemtype(&candidate->record_pools,
                                         (int16_t)record_handle,
                                         receipt.value2) ||
        !dm2_v1_process_source_item_bonus_for_timer(
            &candidate->party.hero[actor], (int16_t)actor, record_handle,
            -1, (int16_t)0xfffe, &candidate->record_pools,
            candidate->asset_loader, &bonus)) {
        receipt.blocked_item_bonus = 1;
        goto process_0e_rollback;
    }
    memcpy(record, record_backup, (size_t)record_size);
    receipt.bonus_applied = 1;
    receipt.valid = 1;
    receipt.consumed = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)record_handle ^ ((uint32_t)receipt.value2 << 16) ^
        (uint32_t)(uint16_t)candidate->party.hero[actor].curMP);
    free(record_backup);
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

process_0e_rollback:
    if (record_backup && record)
        memcpy(record, record_backup, (size_t)record_size);
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->party = party_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(record_backup);
    free(index_backup);
    free(timer_backup);
process_0e_fail:
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_process_sound_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadProcessSoundTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadProcessSoundTimerReceipt receipt;
    DM2_V1_GameLoadSoundOwner sound_backup;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_SoundQueueState state;
    DM2_V1_SoundQueueEnv env;
    DM2_V1_SoundQueueReceipt snd;
    DM2_V1_SoundDelayedSlot *slot;
    int16_t timer_slot;
    uint8_t slot_index;
    int map;

    memset(&receipt, 0, sizeof(receipt));
    memset(&sound_backup, 0, sizeof(sound_backup));
    receipt.game_tick = game_tick;
    receipt.timer_type = 0x15u;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        goto process_sound_fail;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        goto process_sound_fail;
    }
    queue_backup = candidate->timer_queue;
    timer_backup = malloc((size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = malloc((size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup ||
        !dm2_v1_game_load_runtime_candidate_sound_clone(&sound_backup,
                                                        &candidate->sound_owner))
        goto process_sound_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto process_sound_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x15u) {
        receipt.blocked_unsupported_type = 1;
        goto process_sound_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    slot_index = (uint8_t)timer_entry.xA;
    receipt.slot = slot_index;
    if (slot_index >= DM2_V1_SOUND_DELAYED_SLOT_COUNT) {
        receipt.blocked_slot = 1;
        goto process_sound_rollback;
    }
    slot = &candidate->sound_owner.delayed[slot_index];
    if (slot->l_00 == 0) {
        receipt.blocked_slot = 1;
        goto process_sound_rollback;
    }
    map = (int)(int8_t)slot->barr_04[3];
    receipt.map = map;
    receipt.x = (int16_t)(int8_t)slot->barr_04[4];
    receipt.y = (int16_t)(int8_t)slot->barr_04[5];
    receipt.ecxw = slot->w_0a;
    receipt.volume = slot->w_0c;
    if (map != candidate->current_map &&
        map != candidate->sound_owner.spatial_audible_map &&
        map != candidate->sound_owner.spatial_alternate_map) {
        receipt.map_gate_noop = 1;
        slot->l_00 = 0;
    } else {
        if (!candidate->sound_owner.spatial_context_valid ||
            !candidate->sound_owner.queue_entries ||
            candidate->sound_owner.queue_entry_count == 0u ||
            !candidate->sound_owner.sample_bindings ||
            candidate->sound_owner.sample_binding_count == 0u) {
            receipt.blocked_sound = 1;
            goto process_sound_rollback;
        }
        dm2_v1_sound_queue_state_init(&state, 0u);
        if (!dm2_v1_sound_queue_bind_entries(
                &state, candidate->sound_owner.queue_entries,
                candidate->sound_owner.queue_entry_count,
                candidate->sound_owner.queue_capacity)) {
            receipt.blocked_sound = 1;
            goto process_sound_rollback;
        }
        state.positional_count = candidate->sound_owner.positional_count;
        state.immediate_count = candidate->sound_owner.immediate_count;
        state.sound_enabled = candidate->sound_owner.sound_enabled;
        state.master_sfx_volume = candidate->sound_owner.master_sfx_volume;
        memcpy(state.positional, candidate->sound_owner.positional, sizeof(state.positional));
        memcpy(state.immediate, candidate->sound_owner.immediate, sizeof(state.immediate));
        memcpy(state.delayed, candidate->sound_owner.delayed, sizeof(state.delayed));
        memcpy(state.sample_slots, candidate->sound_owner.sample_slots, sizeof(state.sample_slots));
        memset(&env, 0, sizeof(env));
        env.current_map = (int16_t)candidate->current_map;
        env.gate_map_a = candidate->sound_owner.spatial_audible_map;
        env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
        env.facing = candidate->source_party_direction;
        env.party_x = candidate->source_party_x;
        env.party_y = candidate->source_party_y;
        env.gametick = (int32_t)game_tick;
        memset(&snd, 0, sizeof(snd));
        if (!dm2_v1_sound_queue_noise_gen1(
                &state, (int8_t)slot->barr_04[0], (int8_t)slot->barr_04[1],
                (int8_t)slot->barr_04[2], slot->w_0a, slot->w_0c,
                receipt.x, receipt.y, 1, &env, &snd)) {
            receipt.blocked_sound = 1;
            goto process_sound_rollback;
        }
        candidate->sound_owner.positional_count = state.positional_count;
        candidate->sound_owner.immediate_count = state.immediate_count;
        memcpy(candidate->sound_owner.positional, state.positional, sizeof(candidate->sound_owner.positional));
        memcpy(candidate->sound_owner.immediate, state.immediate, sizeof(candidate->sound_owner.immediate));
        memcpy(candidate->sound_owner.delayed, state.delayed, sizeof(candidate->sound_owner.delayed));
        memcpy(candidate->sound_owner.sample_slots, state.sample_slots, sizeof(candidate->sound_owner.sample_slots));
        slot->l_00 = 0;
        receipt.sound_queued = 1;
    }
    receipt.valid = 1;
    receipt.consumed = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)slot_index ^ ((uint32_t)(uint16_t)receipt.ecxw << 8) ^
        ((uint32_t)(uint16_t)receipt.volume << 24) ^ (uint32_t)(uint16_t)map);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    free(index_backup); free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

process_sound_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
    }
    if (sound_backup.valid) {
        dm2_v1_game_load_runtime_candidate_sound_free(&candidate->sound_owner);
        candidate->sound_owner = sound_backup;
        memset(&sound_backup, 0, sizeof(sound_backup));
    }
    free(index_backup); free(timer_backup);
process_sound_fail:
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

static int dm2_v1_game_load_candidate_delete_ai_flags(
    void *context, int creature_type, uint16_t *out_flags)
{
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate = context;
    const DM2_AIDefinition *definition = NULL;
    if (out_flags) *out_flags = 0u;
    if (!candidate || !out_flags || creature_type < 0 ||
        !dm2_v1_caii_source_owner_ai_spec_def(
            &candidate->caii_source, creature_type, &definition) || !definition)
        return 0;
    *out_flags = definition->w0AIFlags;
    return 1;
}

static int dm2_v1_game_load_candidate_delete_gdat_word1(
    void *context, int creature_type, uint16_t *out_word)
{
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate = context;
    if (out_word) *out_word = 0u;
    if (!candidate || !candidate->asset_loader || !out_word || creature_type < 0)
        return 0;
    return dm2_v1_asset_load_word_value(
        candidate->asset_loader, DM2_GDAT_CATEGORY_CREATURES,
        creature_type, 0x01, out_word);
}

static int dm2_v1_game_load_candidate_delete_invoke(
    void *context, int map, int xa, int ya, int xb, int sel,
    int32_t tick, uint32_t *out_ticket)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = context;
    DM2_V1_TimerEntry timer;
    int16_t slot;
    uint16_t sel_uw;
    if (out_ticket) *out_ticket = 0u;
    if (!candidate || !candidate->timer_entries ||
        candidate->timer_capacity == 0u || map < 0 || map > 0xff ||
        xa < 0 || ya < 0 || xb < 0 || sel < 0 || tick < 0)
        return 0;
    memset(&timer, 0, sizeof(timer));
    dm2_v1_timer_set_mticks(&timer, (int16_t)map, tick);
    timer.ttype = 0x04u;
    sel_uw = (uint16_t)sel;
    timer.actor = sel_uw == 0u ? 1u : (sel_uw == 1u ? 3u :
                    (sel_uw == 2u ? 2u : 0u));
    timer.xA = (int8_t)(xa & 0xff);
    timer.yA = (int8_t)(ya & 0xff);
    timer.wvalueB = (int16_t)((xb & 0xff) | ((sel & 0xff) << 8));
    slot = dm2_v1_timer_queue(&candidate->timer_queue, &timer);
    if (slot < 0) return 0;
    if (out_ticket) *out_ticket = (uint32_t)(uint16_t)slot + 1u;
    return 1;
}

/* c_record.cpp:544-1074 — read-only DB4 branch of
 * RECYCLE_A_RECORD_FROM_THE_WORLD.  The source can only recycle a creature
 * after DELETE_CREATURE_RECORD owns its possession chain, CAII mode, invoke
 * message and tile-rooted cut.  Keep this census separate from allocation:
 * it explains a full DB4 pool without claiming that a candidate is safe to
 * delete. */
static void dm2_v1_game_load_candidate_recycler_census(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint16_t *out_examined, int *out_found,
    int16_t *out_record, int16_t *out_map,
    int16_t *out_x, int16_t *out_y,
    int16_t *out_caii_slot, int *out_caii_match,
    uint16_t *out_timer_slot, int *out_pending_timer,
    int16_t *out_possession_root, uint16_t *out_ai_flags,
    int *out_ai_flags_known, uint16_t *out_gdat_word1,
    int *out_gdat_word1_known, int *out_drop_slots_loaded,
    int *out_delete_inputs_ready)
{
    uint16_t examined = 0u;
    int found = 0;
    int16_t found_record = DM2_V1_RECORD_HANDLE_NULL;
    int16_t found_map = -1;
    int16_t found_x = -1;
    int16_t found_y = -1;
    int16_t found_caii_slot = -1;
    int found_caii_match = 0;
    uint16_t found_timer_slot = 0xffffu;
    int found_pending_timer = 0;
    int16_t found_possession_root = DM2_V1_RECORD_HANDLE_END;
    uint16_t found_ai_flags = 0u;
    int found_ai_flags_known = 0;
    uint16_t found_gdat_word1 = 0u;
    int found_gdat_word1_known = 0;
    int found_drop_slots_loaded = 0;
    int found_delete_inputs_ready = 0;
    int chain_corrupt = 0;
    int map;

    if (out_examined) *out_examined = 0u;
    if (out_found) *out_found = 0;
    if (out_record) *out_record = DM2_V1_RECORD_HANDLE_NULL;
    if (out_map) *out_map = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (out_caii_slot) *out_caii_slot = -1;
    if (out_caii_match) *out_caii_match = 0;
    if (out_timer_slot) *out_timer_slot = 0xffffu;
    if (out_pending_timer) *out_pending_timer = 0;
    if (out_possession_root) *out_possession_root = DM2_V1_RECORD_HANDLE_END;
    if (out_ai_flags) *out_ai_flags = 0u;
    if (out_ai_flags_known) *out_ai_flags_known = 0;
    if (out_gdat_word1) *out_gdat_word1 = 0u;
    if (out_gdat_word1_known) *out_gdat_word1_known = 0;
    if (out_drop_slots_loaded) *out_drop_slots_loaded = 0;
    if (out_delete_inputs_ready) *out_delete_inputs_ready = 0;
    if (!candidate || !candidate->record_pools.valid ||
        !candidate->dungeon.raw_data) return;
    for (map = 0; map < candidate->dungeon.level_count; ++map) {
        int x;
        if (map == candidate->current_map || map == candidate->source_party_map)
            continue;
        for (x = 0; x < candidate->dungeon.level_widths[map]; ++x) {
            int y;
            for (y = 0; y < candidate->dungeon.level_heights[map]; ++y) {
                int16_t cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
                    &candidate->dungeon, map, x, y);
                size_t budget = 0u;
                int db;

                for (db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
                    const DM2_V1_RecordPool *pool =
                        &candidate->record_pools.pools[db];
                    /* PC G1 continuation records share the source ObjectID
                     * index space; the census must budget both the primary
                     * and extension spans or it can stop before a DB4
                     * recycler candidate past the primary pool. */
                    if (pool->record_count > 0)
                        budget += (size_t)pool->record_count;
                    if (pool->extension_count > 0)
                        budget += (size_t)pool->extension_count;
                }
                while (cursor != DM2_V1_RECORD_HANDLE_END &&
                       cursor != DM2_V1_RECORD_HANDLE_NULL && budget-- != 0u) {
                    const uint8_t *record = dm2_v1_record_pool_address(
                        &candidate->record_pools, cursor);
                    int16_t next;
                    if (!record || !dm2_v1_record_pool_next_link(
                            &candidate->record_pools, cursor, &next)) {
                        chain_corrupt = 1;
                        break;
                    }
                    if (next == DM2_V1_RECORD_HANDLE_NULL) {
                        chain_corrupt = 1;
                    }
                    if (dm2_v1_record_handle_pool(cursor) == 4) {
                        if (examined != UINT16_MAX) ++examined;
                        /* Source DB4 recycler admission at c_record.cpp:
                         * record word@2 != OBJECT_END and byte@0xf bit 2 is
                         * clear before DELETE_CREATURE_RECORD is called. */
                        if (dm2_v1_game_load_owner_read_u16le(record + 2) !=
                                (uint16_t)DM2_V1_RECORD_HANDLE_END &&
                            (record[15] & 0x04u) == 0u)
                            {
                                found = 1;
                                if (found_record == DM2_V1_RECORD_HANDLE_NULL) {
                                    found_record = cursor;
                                    found_map = (int16_t)map;
                                    found_x = (int16_t)x;
                                    found_y = (int16_t)y;
                                    found_possession_root =
                                        (int16_t)dm2_v1_game_load_owner_read_u16le(
                                            record + 2);
                                    if (candidate->asset_loader &&
                                        candidate->asset_loader->loaded) {
                                        uint16_t word;
                                        int slot;
                                        found_gdat_word1_known =
                                            dm2_v1_asset_load_word_value(
                                                candidate->asset_loader,
                                                DM2_GDAT_CATEGORY_CREATURES,
                                                record[4], 0x01u,
                                                &found_gdat_word1);
                                        found_drop_slots_loaded = 1;
                                        for (slot = 0;
                                             slot < DM2_DROP_SLOT_COUNT; ++slot) {
                                            if (!dm2_v1_asset_load_word_value(
                                                    candidate->asset_loader,
                                                    DM2_GDAT_CATEGORY_CREATURES,
                                                    record[4],
                                                    DM2_DROP_SLOT_FIRST + slot,
                                                    &word)) {
                                                found_drop_slots_loaded = 0;
                                                break;
                                            }
                                        }
                                    }
                                    {
                                        const DM2_AIDefinition *definition = NULL;
                                        if (dm2_v1_caii_source_owner_ai_spec_def(
                                                &candidate->caii_source, record[4],
                                                &definition) && definition) {
                                            found_ai_flags = definition->w0AIFlags;
                                        found_ai_flags_known = 1;
                                        }
                                    }
                                    found_caii_slot = (int16_t)record[5];
                                    if (candidate->caii_slots.valid &&
                                        candidate->caii_slots.slots &&
                                        found_caii_slot >= 0 &&
                                        found_caii_slot < candidate->caii_slots.capacity) {
                                        const uint8_t *slot = candidate->caii_slots.slots +
                                            (size_t)found_caii_slot * DM2_V1_CAII_SLOT_SIZE;
                                        found_caii_match =
                                            dm2_v1_game_load_owner_read_u16le(slot) ==
                                                (uint16_t)(cursor & 0x03ff) ? 1 : 0;
                                        found_timer_slot =
                                            dm2_v1_game_load_owner_read_u16le(slot + 2);
                                        found_pending_timer = found_timer_slot != 0xffffu &&
                                            found_timer_slot != 0u;
                                    }
                                    found_delete_inputs_ready =
                                        found_ai_flags_known &&
                                        found_drop_slots_loaded &&
                                        (!found_pending_timer) &&
                                        (record[5] == 0xffu ||
                                         found_caii_match) &&
                                        (((found_ai_flags & 1u) != 0u) ||
                                         found_gdat_word1_known);
                                }
                            }
                    }
                    cursor = next;
                    if (chain_corrupt) break;
                }
                if (cursor != DM2_V1_RECORD_HANDLE_END &&
                    cursor != DM2_V1_RECORD_HANDLE_NULL && budget == 0u)
                    chain_corrupt = 1;
                if (chain_corrupt) break;
            }
            if (chain_corrupt) break;
        }
        if (chain_corrupt) break;
    }
    if (chain_corrupt) {
        /* A census that cannot verify every traversed tile chain must not
         * leave an earlier DB4 candidate admissible for destructive reuse. */
        found = 0;
        found_record = DM2_V1_RECORD_HANDLE_NULL;
        found_map = -1;
        found_x = -1;
        found_y = -1;
        found_caii_slot = -1;
        found_caii_match = 0;
        found_timer_slot = 0xffffu;
        found_pending_timer = 0;
        found_possession_root = DM2_V1_RECORD_HANDLE_END;
        found_ai_flags = 0u;
        found_ai_flags_known = 0;
        found_gdat_word1 = 0u;
        found_gdat_word1_known = 0;
        found_drop_slots_loaded = 0;
        found_delete_inputs_ready = 0;
    }
    if (out_examined) *out_examined = examined;
    if (out_found) *out_found = found;
    if (out_record) *out_record = found_record;
    if (out_map) *out_map = found_map;
    if (out_x) *out_x = found_x;
    if (out_y) *out_y = found_y;
    if (out_caii_slot) *out_caii_slot = found_caii_slot;
    if (out_caii_match) *out_caii_match = found_caii_match;
    if (out_timer_slot) *out_timer_slot = found_timer_slot;
    if (out_pending_timer) *out_pending_timer = found_pending_timer;
    if (out_possession_root) *out_possession_root = found_possession_root;
    if (out_ai_flags) *out_ai_flags = found_ai_flags;
    if (out_ai_flags_known) *out_ai_flags_known = found_ai_flags_known;
    if (out_gdat_word1) *out_gdat_word1 = found_gdat_word1;
    if (out_gdat_word1_known) *out_gdat_word1_known = found_gdat_word1_known;
    if (out_drop_slots_loaded) *out_drop_slots_loaded = found_drop_slots_loaded;
    if (out_delete_inputs_ready) *out_delete_inputs_ready = found_delete_inputs_ready;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_alloc_creature_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadAllocCreatureTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadAllocCreatureTimerReceipt receipt;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t *dungeon_backup = NULL;
    uint8_t *caii_backup = NULL;
    DM2_V1_GameLoadSoundOwner sound_backup;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_DropRng rng_backup;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_SourceTimerQueue delete_timer_shadow;
    DM2_V1_DeleteCreatureFullReceipt delete_receipt;
    DM2_V1_GameLoadMoverecCaiiReceipt caii_receipt;
    DM2_V1_CreatureSomethingReceipt something;
    const uint8_t *animation = NULL;
    const DM2_AIDefinition *ai = NULL;
    uint8_t *record = NULL;
    DM2_V1_SoundQueueState sound_state;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_SoundQueueReceipt sound_receipt;
    int16_t timer_slot;
    int16_t record_handle;
    int16_t head;
    int16_t adj[2] = {0, 0};
    int map;
    int x;
    int y;
    int dir;
    int base_hp;
    int hp;
    int rand_dir;
    int dx;
    int dy;
    int adx;
    int ady;
    int old_map;
    int caii_alloc_count_backup;
    size_t caii_bytes;
    uint16_t delete_drop_slots[DM2_DROP_SLOT_COUNT];

    memset(&receipt, 0, sizeof(receipt));
    memset(&sound_backup, 0, sizeof(sound_backup));
    memset(&pool_backup, 0, sizeof(pool_backup));
    memset(&caii_receipt, 0, sizeof(caii_receipt));
    memset(&something, 0, sizeof(something));
    memset(&delete_receipt, 0, sizeof(delete_receipt));
    dm2_v1_source_timer_queue_init(&delete_timer_shadow);
    memset(delete_drop_slots, 0, sizeof(delete_drop_slots));
    receipt.game_tick = game_tick;
    receipt.timer_type = 0x5eu;
    receipt.caii_slot = -1;
    receipt.think_timer_slot = -1;
    receipt.recycler_candidate_record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.recycler_candidate_map = -1;
    receipt.recycler_candidate_x = -1;
    receipt.recycler_candidate_y = -1;
    receipt.recycler_candidate_caii_slot = -1;
    receipt.recycler_candidate_timer_slot = 0xffffu;
    receipt.recycler_candidate_possession_root = DM2_V1_RECORD_HANDLE_END;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        goto alloc_creature_fail;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        goto alloc_creature_fail;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    rng_backup = candidate->caii_rng;
    caii_alloc_count_backup = candidate->caii_slots.alloc_count;
    if (!dm2_v1_record_pool_set_clone(&pool_backup, &candidate->record_pools) ||
        candidate->dungeon.raw_size == 0u || !candidate->dungeon.raw_data)
        goto alloc_creature_rollback;
    dungeon_backup = malloc((size_t)candidate->dungeon.raw_size);
    timer_backup = malloc((size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = malloc((size_t)candidate->timer_capacity * sizeof(*index_backup));
    caii_bytes = (size_t)candidate->caii_slots.capacity * DM2_V1_CAII_SLOT_SIZE;
    if (caii_bytes != 0u) caii_backup = malloc(caii_bytes);
    if (!dungeon_backup || !timer_backup || !index_backup ||
        (caii_bytes != 0u && !caii_backup) ||
        !candidate->caii_slots.valid || !candidate->caii_slots.slots ||
        !dm2_v1_game_load_runtime_candidate_sound_clone(
            &sound_backup, &candidate->sound_owner))
        goto alloc_creature_rollback;
    memcpy(dungeon_backup, candidate->dungeon.raw_data,
           (size_t)candidate->dungeon.raw_size);
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (caii_bytes != 0u)
        memcpy(caii_backup, candidate->caii_slots.slots, caii_bytes);
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto alloc_creature_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x5eu) {
        receipt.blocked_unsupported_type = 1;
        goto alloc_creature_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(int8_t)timer_entry.xA;
    y = (int)(int8_t)timer_entry.yA;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    receipt.creature_type = (uint8_t)timer_entry.wvalueB;
    receipt.health_multiplier = 7u;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        x < 0 || y < 0 || x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto alloc_creature_rollback;
    }
    if (dm2_v1_get_creature_at(&candidate->record_pools, &candidate->dungeon,
                               map, x, y) != DM2_V1_RECORD_HANDLE_NULL) {
        receipt.blocked_tile = 1;
        goto alloc_creature_rollback;
    }
    if (map == candidate->source_party_map &&
        x == candidate->source_party_x && y == candidate->source_party_y) {
        receipt.blocked_tile = 1;
        goto alloc_creature_rollback;
    }
    if (!dm2_v1_caii_source_owner_ai_spec_def(
            &candidate->caii_source, receipt.creature_type, &ai) || !ai ||
        ai->BaseHP == 0u) {
        receipt.blocked_ai = 1;
        goto alloc_creature_rollback;
    }
    rand_dir = (int)dm2_v1_drops_randdir(&candidate->caii_rng);
    if (rand_dir == 0) {
        dir = (int)dm2_v1_drops_randdir(&candidate->caii_rng);
    } else {
        dx = x - (int)candidate->source_party_x;
        dy = y - (int)candidate->source_party_y;
        adx = dx < 0 ? -dx : dx;
        ady = dy < 0 ? -dy : dy;
        if (adx == ady) {
            if (dm2_v1_drops_randbit(&candidate->caii_rng) == 0u) ++ady;
            else ++adx;
        }
        dir = adx >= ady ? (dx <= 0 ? 1 : 3) : (dy <= 0 ? 2 : 0);
    }
    dir &= 3;
    receipt.direction = (uint8_t)dir;
    record_handle = dm2_v1_record_pool_alloc_new_record(
        &candidate->record_pools, 4u);
    record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                             record_handle);
    if (record_handle < 0 || !record ||
        candidate->record_pools.pools[4].record_size < 16) {
        receipt.blocked_record = 1;
        dm2_v1_game_load_candidate_recycler_census(
            candidate, &receipt.recycler_candidates_examined,
            &receipt.recycler_candidate_found,
            &receipt.recycler_candidate_record,
            &receipt.recycler_candidate_map,
            &receipt.recycler_candidate_x,
            &receipt.recycler_candidate_y,
            &receipt.recycler_candidate_caii_slot,
            &receipt.recycler_candidate_caii_match,
            &receipt.recycler_candidate_timer_slot,
            &receipt.recycler_candidate_pending_timer,
            &receipt.recycler_candidate_possession_root,
            &receipt.recycler_candidate_ai_flags,
            &receipt.recycler_candidate_ai_flags_known,
            &receipt.recycler_candidate_gdat_word1,
            &receipt.recycler_candidate_gdat_word1_known,
            &receipt.recycler_candidate_drop_slots_loaded,
            &receipt.recycler_candidate_delete_inputs_ready);
        if (receipt.recycler_candidate_found &&
            receipt.recycler_candidate_delete_inputs_ready) {
            const uint8_t *candidate_record = dm2_v1_record_pool_address(
                &candidate->record_pools, receipt.recycler_candidate_record);
            int slot;
            int restored_map = 0;
            if (!candidate_record || !candidate->asset_loader ||
                !dm2_v1_game_load_runtime_candidate_change_current_map(
                    candidate, receipt.recycler_candidate_map, 0))
                goto alloc_creature_rollback;
            for (slot = 0; slot < DM2_DROP_SLOT_COUNT; ++slot) {
                if (!dm2_v1_asset_load_word_value(
                        candidate->asset_loader, DM2_GDAT_CATEGORY_CREATURES,
                        candidate_record[4], DM2_DROP_SLOT_FIRST + slot,
                        &delete_drop_slots[slot]))
                    goto alloc_creature_rollback;
            }
            restored_map = dm2_v1_delete_creature_record_full_with_context_and_invoke(
                &candidate->record_pools, &candidate->dungeon,
                &candidate->caii_slots, &delete_timer_shadow,
                &candidate->caii_rng, receipt.recycler_candidate_map,
                (unsigned long)game_tick, receipt.recycler_candidate_x,
                receipt.recycler_candidate_y, 0, 0,
                candidate->source_party_x, candidate->source_party_y,
                candidate->source_party_direction, delete_drop_slots,
                dm2_v1_game_load_candidate_delete_ai_flags, candidate,
                dm2_v1_game_load_candidate_delete_gdat_word1, candidate,
                dm2_v1_game_load_candidate_delete_invoke, candidate,
                &delete_receipt);
            if (!restored_map || !delete_receipt.valid ||
                !delete_receipt.dealloc_performed ||
                !dm2_v1_game_load_runtime_candidate_change_current_map(
                    candidate, old_map, 0))
                goto alloc_creature_rollback;
            record_handle = dm2_v1_record_pool_alloc_new_record(
                &candidate->record_pools, 4u);
            record = dm2_v1_record_pool_address_mut(
                &candidate->record_pools, record_handle);
        }
        if (record_handle < 0 || !record ||
            candidate->record_pools.pools[4].record_size < 16)
            goto alloc_creature_rollback;
    }
    base_hp = ((int)receipt.health_multiplier * (int)ai->BaseHP) >> 3;
    hp = base_hp + (int)dm2_v1_drops_rand16(
        &candidate->caii_rng, (uint16_t)((base_hp >> 3) + 1));
    if (hp <= 0 || hp > 0xffff) {
        receipt.blocked_ai = 1;
        goto alloc_creature_rollback;
    }
    record[2] = 0xfeu; record[3] = 0xffu;
    record[4] = receipt.creature_type;
    record[5] = 0xffu;
    record[6] = (uint8_t)hp; record[7] = (uint8_t)(hp >> 8);
    record[8] = 0xffu; record[9] = 0xffu;
    record[10] = 0u; record[11] = 0u;
    record[12] = 0u; record[13] = 0u;
    record[14] = 0u;
    record[15] = (uint8_t)(0xf8u | (uint8_t)dir);
    head = (int16_t)dm2_v1_dungeon_get_first_thing(&candidate->dungeon,
                                                    map, x, y);
    if (!dm2_v1_record_pool_append_to_list(&candidate->record_pools, &head,
                                           record_handle) ||
        dm2_v1_dungeon_set_first_thing(&candidate->dungeon, map, x, y,
                                       (uint16_t)head) != 0) {
        receipt.blocked_tile = 1;
        goto alloc_creature_rollback;
    }
    if (!dm2_v1_game_load_runtime_session_candidate_activate_moverec_caii(
            candidate, record_handle, (int16_t)x, (int16_t)y,
            &caii_receipt) || !caii_receipt.valid) {
        receipt.blocked_caii = 1;
        goto alloc_creature_rollback;
    }
    receipt.caii_slot = caii_receipt.caii_slot;
    receipt.think_timer_slot = caii_receipt.timer_slot;
    adj[0] = (int16_t)dm2_v1_game_load_owner_read_u16le(
        candidate->caii_slots.slots + (size_t)caii_receipt.caii_slot *
        DM2_V1_CAII_SLOT_SIZE + 8u);
    adj[1] = (int16_t)dm2_v1_game_load_owner_read_u16le(
        candidate->caii_slots.slots + (size_t)caii_receipt.caii_slot *
        DM2_V1_CAII_SLOT_SIZE + 10u);
    if (dm2_v1_creature_something_1c9a_0a48_with_ai_spec(
            &candidate->record_pools, &candidate->caii_slots,
            candidate->asset_loader, ai, &candidate->caii_rng, record_handle,
            adj, &animation, map, candidate->source_party_map, 0, 0, 0, x, y,
            (unsigned long)game_tick, &something) < 0 || !something.valid) {
        receipt.blocked_caii = 1;
        goto alloc_creature_rollback;
    }
    /* c_1c9a.cpp:5539-5545 — 0a48 requests GEN1 with the creature's
     * animation index. Keep the sound owner in this same transaction; a
     * missing sample/map binding rolls back the summon rather than dropping
     * the source side effect. */
    if (something.noise_would_queue) {
        if (!candidate->sound_owner.valid ||
            !candidate->sound_owner.spatial_context_valid ||
            !candidate->sound_owner.queue_entries ||
            candidate->sound_owner.queue_entry_count == 0u ||
            !candidate->sound_owner.sample_bindings ||
            candidate->sound_owner.sample_binding_count == 0u) {
            receipt.blocked_sound = 1;
            goto alloc_creature_rollback;
        }
        dm2_v1_sound_queue_state_init(&sound_state, 0u);
        if (!dm2_v1_sound_queue_bind_entries(
                &sound_state, candidate->sound_owner.queue_entries,
                candidate->sound_owner.queue_entry_count,
                candidate->sound_owner.queue_capacity)) {
            receipt.blocked_sound = 1;
            goto alloc_creature_rollback;
        }
        sound_state.positional_count = candidate->sound_owner.positional_count;
        sound_state.immediate_count = candidate->sound_owner.immediate_count;
        sound_state.sound_enabled = candidate->sound_owner.sound_enabled;
        sound_state.master_sfx_volume = candidate->sound_owner.master_sfx_volume;
        memcpy(sound_state.positional, candidate->sound_owner.positional,
               sizeof(sound_state.positional));
        memcpy(sound_state.immediate, candidate->sound_owner.immediate,
               sizeof(sound_state.immediate));
        memcpy(sound_state.delayed, candidate->sound_owner.delayed,
               sizeof(sound_state.delayed));
        memcpy(sound_state.sample_slots, candidate->sound_owner.sample_slots,
               sizeof(sound_state.sample_slots));
        memset(&sound_env, 0, sizeof(sound_env));
        sound_env.current_map = (int16_t)candidate->current_map;
        sound_env.gate_map_a = candidate->sound_owner.spatial_audible_map;
        sound_env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
        sound_env.facing = candidate->source_party_direction;
        sound_env.party_x = candidate->source_party_x;
        sound_env.party_y = candidate->source_party_y;
        sound_env.gametick = (int32_t)game_tick;
        memset(&sound_receipt, 0, sizeof(sound_receipt));
        if (!dm2_v1_sound_queue_noise_gen1(
                &sound_state, 0x0f, (int8_t)receipt.creature_type,
                (int8_t)something.noise_index, 0x46, 0x80,
                receipt.x, receipt.y, 1, &sound_env, &sound_receipt)) {
            receipt.blocked_sound = 1;
            goto alloc_creature_rollback;
        }
        candidate->sound_owner.positional_count = sound_state.positional_count;
        candidate->sound_owner.immediate_count = sound_state.immediate_count;
        memcpy(candidate->sound_owner.positional, sound_state.positional,
               sizeof(candidate->sound_owner.positional));
        memcpy(candidate->sound_owner.immediate, sound_state.immediate,
               sizeof(candidate->sound_owner.immediate));
        memcpy(candidate->sound_owner.delayed, sound_state.delayed,
               sizeof(candidate->sound_owner.delayed));
        memcpy(candidate->sound_owner.sample_slots, sound_state.sample_slots,
               sizeof(candidate->sound_owner.sample_slots));
        receipt.sound_queued = 1;
        receipt.sound_index = (uint8_t)something.noise_index;
    }
    receipt.hit_points = (uint16_t)hp;
    receipt.record = record_handle;
    receipt.valid = 1;
    receipt.consumed = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record_handle ^ ((uint32_t)(uint16_t)x << 16) ^
        (uint32_t)(uint16_t)y ^ ((uint32_t)receipt.creature_type << 24) ^
        ((uint32_t)receipt.sound_index << 8));
    dm2_v1_record_pool_set_free(&pool_backup);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    free(dungeon_backup); free(caii_backup);
    free(index_backup); free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

alloc_creature_rollback:
    if (pool_backup.valid) {
        dm2_v1_record_pool_set_free(&candidate->record_pools);
        candidate->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (sound_backup.valid) {
        dm2_v1_game_load_runtime_candidate_sound_free(&candidate->sound_owner);
        candidate->sound_owner = sound_backup;
        memset(&sound_backup, 0, sizeof(sound_backup));
    }
    if (dungeon_backup && candidate && candidate->dungeon.raw_data)
        memcpy(candidate->dungeon.raw_data, dungeon_backup,
               (size_t)candidate->dungeon.raw_size);
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
    }
    if (caii_backup && candidate->caii_slots.slots)
        memcpy(candidate->caii_slots.slots, caii_backup, caii_bytes);
    candidate->caii_slots.alloc_count = caii_alloc_count_backup;
    candidate->caii_rng = rng_backup;
    if (candidate && candidate->current_map != old_map)
        (void)dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, old_map, 0);
    free(dungeon_backup); free(caii_backup);
    free(index_backup); free(timer_backup);
alloc_creature_fail:
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    dm2_v1_record_pool_set_free(&pool_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_ench_power_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadEnchPowerTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadEnchPowerTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_Party party_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    int16_t timer_slot;
    int old_map;
    int map;
    int16_t amount;
    uint8_t actor_mask;

    memset(&receipt, 0, sizeof(receipt));
    receipt.game_tick = game_tick;
    receipt.timer_type = 0x48u;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    party_backup = candidate->party;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto ench_power_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto ench_power_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto ench_power_rollback;
    }
    if (candidate->timer_entries[timer_slot].ttype != 0x48u) {
        receipt.blocked_unsupported_type = 1;
        goto ench_power_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    receipt.map = map;
    receipt.map_switch = old_map != map;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto ench_power_rollback;
    }
    if (candidate->party.heros_in_party < 0 ||
        candidate->party.heros_in_party > DM2_MAX_HEROES) {
        receipt.blocked_party = 1;
        goto ench_power_rollback;
    }
    actor_mask = timer_entry.actor;
    amount = (int16_t)((uint16_t)(uint8_t)timer_entry.xA |
                       ((uint16_t)(uint8_t)timer_entry.yA << 8));
    receipt.actor_mask = actor_mask;
    receipt.amount = amount;
    receipt.heroes_seen = candidate->party.heros_in_party;
    for (int hero_index = 0;
         hero_index < candidate->party.heros_in_party; ++hero_index) {
        DM2_V1_Hero *hero = &candidate->party.hero[hero_index];
        if ((actor_mask & (uint8_t)(1u << hero_index)) == 0u)
            continue;
        if (hero->curHP == 0) {
            ++receipt.heroes_skipped_dead;
            continue;
        }
        {
            int32_t next = (int32_t)hero->ench_power - amount;
            if (next < 0) next = 0;
            hero->ench_power = (int16_t)next;
        }
        ++receipt.heroes_mutated;
    }
    receipt.valid = 1;
    receipt.consumed = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        dm2_v1_game_load_owner_hash_step(0x454e4838u, actor_mask),
        (uint16_t)amount);
    for (int hero_index = 0;
         hero_index < candidate->party.heros_in_party; ++hero_index) {
        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
            receipt.transaction_hash,
            (uint16_t)candidate->party.hero[hero_index].ench_power);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

ench_power_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->party = party_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_poison_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadPoisonTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadPoisonTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry next_timer;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_Party party_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    int16_t timer_slot;
    int old_map;
    int map;
    int actor;
    int16_t counters;
    int32_t wound;
    int32_t next_poison;
    DM2_V1_Hero *hero;

    memset(&receipt, 0, sizeof(receipt));
    receipt.timer_type = 0x4bu;
    receipt.game_tick = game_tick;
    receipt.actor = -1;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    party_backup = candidate->party;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto poison_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto poison_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x4bu) {
        receipt.blocked_unsupported_type = 1;
        goto poison_rollback;
    }
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    actor = (int)(int8_t)candidate->timer_entries[timer_slot].actor;
    receipt.map = map;
    receipt.actor = (int16_t)actor;
    receipt.map_switch = old_map != map;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto poison_rollback;
    }
    if (candidate->party.heros_in_party <= 0 ||
        candidate->party.heros_in_party > DM2_MAX_HEROES) {
        receipt.blocked_party = 1;
        goto poison_rollback;
    }
    if (actor < 0 || actor >= candidate->party.heros_in_party) {
        receipt.blocked_actor = 1;
        goto poison_rollback;
    }
    /* c_startend.cpp retains ddat.v1e0288 as the next champion number.  The
     * source excludes only the hero whose one-based index matches that
     * authenticated value; do not substitute party count or a host index. */
    if (candidate->source_next_champion_number <= 0 ||
        candidate->source_next_champion_number > DM2_MAX_HEROES) {
        receipt.blocked_last_hero_owner = 1;
        goto poison_rollback;
    }
    if (actor + 1 == candidate->source_next_champion_number) {
        receipt.blocked_last_hero_owner = 1;
        goto poison_rollback;
    }
    hero = &candidate->party.hero[actor];
    receipt.poison_before = hero->poison;
    receipt.poisoned_before = hero->poisoned;
    receipt.cur_hp_before = hero->curHP;
    receipt.hero_flags_before = (uint16_t)hero->heroflag;
    counters = (int16_t)((uint16_t)(uint8_t)
        candidate->timer_entries[timer_slot].xA |
        ((uint16_t)(uint8_t)candidate->timer_entries[timer_slot].yA << 8));
    receipt.amount = counters;
    if (hero->poisoned <= 0) {
        receipt.blocked_actor = 1;
        goto poison_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);

    /* c_tim_proc.cpp:4165-4178: remove the timer's poison contribution,
     * then enter c_hero.cpp:3397 DM2_PROCESS_POISON. */
    hero->poisoned = (int8_t)(hero->poisoned - 1);
    hero->poison = (int16_t)(hero->poison - counters);
    wound = ((int32_t)counters + 0x1e) >> 6;
    if (wound < 1) wound = 1;
    receipt.wound_amount = (int16_t)(wound > INT16_MAX ? INT16_MAX : wound);
    if (hero->curHP != 0) {
        int32_t pending = (int32_t)hero->damagesuffered + wound;
        if (pending > INT16_MAX) pending = INT16_MAX;
        hero->damagesuffered = (int16_t)pending;
    }
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag | 0x2800u);
    counters = (int16_t)(counters - 1);
    if (counters > 0) {
        next_poison = (int32_t)hero->poison + counters;
        if (next_poison > 0x0c00) {
            counters = (int16_t)(0x0c00 - hero->poison);
            if (counters < 0) counters = 0;
        }
        if (counters > 0) {
            hero->poison = (int16_t)(hero->poison + counters);
            hero->poisoned = (int8_t)(hero->poisoned + 1);
            dm2_v1_timer_entry_init(&next_timer);
            dm2_v1_timer_set_mticks(&next_timer, (int16_t)map,
                                     game_tick + 0x24u);
            next_timer.ttype = 0x4bu;
            next_timer.actor = (uint8_t)actor;
            next_timer.xA = (int8_t)(uint8_t)counters;
            next_timer.yA = (int8_t)((uint16_t)counters >> 8);
            if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
                goto poison_rollback;
            receipt.requeued = 1;
        }
    }
    receipt.poison_after = hero->poison;
    receipt.poisoned_after = hero->poisoned;
    receipt.cur_hp_after = hero->curHP;
    receipt.hero_flags_after = (uint16_t)hero->heroflag;
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)actor ^ ((uint32_t)(uint16_t)receipt.poison_after << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

poison_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->party = party_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_light_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadLightTimerReceipt *out_receipt)
{
    static const int16_t light_steps[16] = {
        0, 5, 12, 24, 33, 40, 46, 51,
        59, 68, 76, 82, 89, 94, 97, 100
    };
    DM2_V1_GameLoadLightTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry next_timer;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    int16_t timer_slot;
    int old_map;
    int map;
    int16_t amount;
    int abs_amount;
    int16_t remaining;
    int delta;
    int16_t light_backup;

    memset(&receipt, 0, sizeof(receipt));
    receipt.timer_type = 0x46u;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    light_backup = candidate->source_light_level;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto light_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto light_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto light_rollback;
    }
    if (candidate->timer_entries[timer_slot].ttype != 0x46u) {
        receipt.blocked_unsupported_type = 1;
        goto light_rollback;
    }
    amount = (int16_t)((uint16_t)(uint8_t)candidate->timer_entries[timer_slot].xA |
                       ((uint16_t)(uint8_t)candidate->timer_entries[timer_slot].yA << 8));
    abs_amount = amount < 0 ? -(int)amount : (int)amount;
    receipt.amount = amount;
    if (abs_amount >= 16) {
        receipt.blocked_value = 1;
        goto light_rollback;
    }
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    receipt.map = map;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto light_rollback;
    }
    receipt.map_switch = old_map != map;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    receipt.light_before = candidate->source_light_level;
    if (amount == 0) {
        delta = 0;
        remaining = 0;
    } else {
        delta = light_steps[abs_amount] - light_steps[abs_amount - 1];
        if (amount > 0) delta *= 2;
        else delta = -delta;
        remaining = (int16_t)(amount < 0 ? -(abs_amount - 1) : abs_amount - 1);
    }
    candidate->source_light_level = (int16_t)(candidate->source_light_level + delta);
    receipt.light_after = candidate->source_light_level;
    receipt.remaining = remaining;
    if (remaining != 0) {
        dm2_v1_timer_entry_init(&next_timer);
        dm2_v1_timer_set_mticks(&next_timer, (int16_t)map, game_tick + 8u);
        next_timer.ttype = 0x46u;
        next_timer.actor = 0u;
        next_timer.xA = (int8_t)(uint8_t)remaining;
        next_timer.yA = (int8_t)(uint8_t)((uint16_t)remaining >> 8);
        if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
            goto light_rollback;
        receipt.requeued = 1;
    }
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)amount ^ ((uint32_t)(uint16_t)candidate->source_light_level << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

light_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->source_light_level = light_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_ornate_noise_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadOrnateNoiseTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadOrnateNoiseTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry next_timer;
    DM2_V1_GameLoadSoundOwner sound_backup;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_GameLoadSoundNoiseReceipt sound_receipt;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t record_backup[16];
    uint8_t *record;
    int16_t timer_slot;
    int16_t record_handle;
    int old_map;
    int map;
    int x;
    int y;
    int pool;
    int record_size = 0;
    int raw_tile;
    int wall;
    int category;
    uint8_t graphic_index;
    uint8_t decoration;
    uint16_t actuator_word;
    uint16_t animation_length;
    int active;

    memset(&receipt, 0, sizeof(receipt));
    memset(&sound_backup, 0, sizeof(sound_backup));
    memset(&sound_env, 0, sizeof(sound_env));
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    receipt.record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.timer_type = 0x5au;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto ornate_noise_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto ornate_noise_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x5au) {
        receipt.blocked_unsupported_type = 1;
        goto ornate_noise_rollback;
    }
    record_handle = candidate->timer_entries[timer_slot].wvalueB;
    pool = dm2_v1_record_handle_pool(record_handle);
    record_size = (pool >= 0 && pool < DM2_V1_RECORD_POOL_COUNT)
        ? candidate->record_pools.pools[pool].record_size : 0;
    record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                             record_handle);
    if (!record || record_size <= 0 || record_size > (int)sizeof(record_backup)) {
        receipt.blocked_record = 1;
        goto ornate_noise_rollback;
    }
    memcpy(record_backup, record, (size_t)record_size);
    receipt.record = record_handle;
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    x = (int)(int8_t)candidate->timer_entries[timer_slot].xA;
    y = (int)(int8_t)candidate->timer_entries[timer_slot].yA;
    receipt.map = map;
    if (map < 0 || map >= candidate->dungeon.level_count || x < 0 || y < 0 ||
        x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto ornate_noise_rollback;
    }
    receipt.map_switch = old_map != map;
    receipt.word2_before = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    active = (record[4] & 0x01u) != 0;
    if (active) {
        raw_tile = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon, map, x, y);
        if (raw_tile < 0 || map != candidate->local_level_graphics.map) {
            receipt.blocked_map = 1;
            goto ornate_noise_rollback;
        }
        wall = ((raw_tile >> 5) & 7) == 0;
        category = wall ? 0x09 : 0x0a;
        actuator_word = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
        graphic_index = (uint8_t)((actuator_word >> 12) & 0x0fu);
        decoration = 0xffu;
        if (wall) {
            if (graphic_index > 0u &&
                graphic_index <= candidate->local_level_graphics.wall_count)
                decoration = candidate->local_level_graphics.wall_gfx[graphic_index - 1u];
        } else if (graphic_index > 0u &&
                   graphic_index <= candidate->local_level_graphics.floor_count) {
            decoration = candidate->local_level_graphics.floor_gfx[graphic_index - 1u];
        }
        if (!candidate->asset_loader || decoration == 0xffu) {
            receipt.blocked_record = 1;
            goto ornate_noise_rollback;
        }
        {
            DM2_V1_GetOrnateAnimLenReceipt anim;
            if (!dm2_v1_get_ornate_anim_len_receipt(
                    candidate->asset_loader, category, decoration, 0, &anim) ||
                !anim.accepted || anim.length == 0u) {
                receipt.blocked_record = 1;
                goto ornate_noise_rollback;
            }
            animation_length = anim.length;
        }
        receipt.category = (uint8_t)category;
        receipt.decoration = decoration;
        receipt.animation_length = animation_length;
        /* The source continues the timer path when the optional sound queue
         * rejects a sample/map gate. Clone it when possible so a later timer
         * queue failure still restores the complete candidate. */
        (void)dm2_v1_game_load_runtime_candidate_sound_clone(
            &sound_backup, &candidate->sound_owner);
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    if (!active) {
        record[2] = (uint8_t)(receipt.word2_before & 0xffu);
        record[3] = 0u;
        receipt.word2_after = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
        receipt.frame_cleared = 1;
    } else {
        sound_env.current_map = (int16_t)map;
        sound_env.gate_map_a = candidate->sound_owner.spatial_audible_map;
        sound_env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
        sound_env.facing = candidate->source_party_direction;
        sound_env.party_x = candidate->source_party_x;
        sound_env.party_y = candidate->source_party_y;
        sound_env.gametick = (int32_t)game_tick;
        next_timer = timer_entry;
        dm2_v1_timer_set_mticks(&next_timer, (int16_t)map,
            (uint32_t)dm2_v1_timer_get_ticks(&timer_entry) + animation_length);
        if (candidate->sound_owner.valid &&
            dm2_v1_game_load_sound_owner_queue_noise_gen2(
                &candidate->sound_owner, (int8_t)category, (int8_t)decoration,
                (int8_t)0x88, (int8_t)0xfe, (int16_t)x, (int16_t)y, 1,
                (int16_t)0x8c, (int16_t)0x80, &sound_env, &sound_receipt))
            receipt.sound_queued = 1;
        if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
            goto ornate_noise_rollback;
        receipt.requeued = 1;
    }
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record_handle ^ ((uint32_t)receipt.word2_after << 16));
    free(index_backup);
    free(timer_backup);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

ornate_noise_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (record && record_size > 0 && record_size <= (int)sizeof(record_backup))
            memcpy(record, record_backup, (size_t)record_size);
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    if (sound_backup.valid) {
        dm2_v1_game_load_runtime_candidate_sound_free(&candidate->sound_owner);
        candidate->sound_owner = sound_backup;
        memset(&sound_backup, 0, sizeof(sound_backup));
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_ornate_animator_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadOrnateAnimatorTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadOrnateAnimatorTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry next_timer;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t record_backup[16];
    uint8_t *record = NULL;
    int16_t timer_slot;
    int16_t record_handle;
    int old_map;
    int map;
    int wall;
    int category;
    int record_size;
    uint8_t decoration;
    uint16_t word2;
    uint16_t frame;
    uint16_t word4;
    uint8_t graphic_index;
    DM2_V1_GetOrnateAnimLenReceipt anim;

    memset(&receipt, 0, sizeof(receipt));
    receipt.record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.timer_type = 0x55u;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        goto ornate_animator_rollback;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        goto ornate_animator_rollback;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = malloc((size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = malloc((size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto ornate_animator_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto ornate_animator_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x55u) {
        receipt.blocked_unsupported_type = 1;
        goto ornate_animator_rollback;
    }
    timer_entry = candidate->timer_entries[timer_slot];
    record_handle = (int16_t)((uint8_t)timer_entry.xA |
                              ((uint16_t)(uint8_t)timer_entry.yA << 8));
    receipt.record = record_handle;
    if (dm2_v1_record_handle_pool(record_handle) != 3) {
        receipt.blocked_record = 1;
        goto ornate_animator_rollback;
    }
    record = dm2_v1_record_pool_address_mut(&candidate->record_pools, record_handle);
    record_size = candidate->record_pools.pools[3].record_size;
    if (!record || record_size <= 0 || record_size > (int)sizeof(record_backup)) {
        receipt.blocked_record = 1;
        goto ornate_animator_rollback;
    }
    memcpy(record_backup, record, (size_t)record_size);
    map = dm2_v1_timer_get_map(&timer_entry);
    receipt.map = map;
    if (map != candidate->local_level_graphics.map ||
        map < 0 || map >= candidate->dungeon.level_count ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto ornate_animator_rollback;
    }
    receipt.animation_mode = (uint8_t)(timer_entry.wvalueB & 0xff);
    /* c_tim_proc.cpp passes the wall/floor selector as the timer's mode;
     * unlike 0x5A, 0x55 must not infer the GDAT category from the tile. */
    wall = (timer_entry.wvalueB & 1) != 0;
    category = wall ? 0x09 : 0x0a;
    word4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
    graphic_index = (uint8_t)((word4 >> 12) & 0x0fu);
    decoration = 0xffu;
    if (wall) {
        if (graphic_index > 0u && graphic_index <= candidate->local_level_graphics.wall_count)
            decoration = candidate->local_level_graphics.wall_gfx[graphic_index - 1u];
    } else if (graphic_index > 0u && graphic_index <= candidate->local_level_graphics.floor_count) {
        decoration = candidate->local_level_graphics.floor_gfx[graphic_index - 1u];
    }
    if (!candidate->asset_loader || decoration == 0xffu ||
        !dm2_v1_get_ornate_anim_len_receipt(candidate->asset_loader, category,
                                             decoration, 0, &anim) ||
        !anim.accepted || anim.length == 0u) {
        receipt.blocked_asset = 1;
        goto ornate_animator_rollback;
    }
    receipt.decoration = decoration;
    receipt.animation_length = anim.length;
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    frame = (uint16_t)((word2 >> 7) & 0x1ffu);
    receipt.word2_before = word2;
    receipt.frame_before = frame;
    frame = (uint16_t)((frame + 1u) & 0x1ffu);
    word2 = (uint16_t)((word2 & 0x007fu) | (frame << 7));
    record[2] = (uint8_t)word2;
    record[3] = (uint8_t)(word2 >> 8);
    receipt.frame_after = frame;
    receipt.word2_after = word2;
    receipt.frame_advanced = 1;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    next_timer = timer_entry;
    dm2_v1_timer_set_mticks(&next_timer, (int16_t)map,
                             (uint32_t)dm2_v1_timer_get_ticks(&timer_entry) + 1u);
    if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
        goto ornate_animator_rollback;
    receipt.requeued = 1;
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record_handle ^ ((uint32_t)word2 << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

ornate_animator_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (record && record_size > 0 && record_size <= (int)sizeof(record_backup))
            memcpy(record, record_backup, (size_t)record_size);
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_hero_ench_flag_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadHeroEnchFlagTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadHeroEnchFlagTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_Party party_backup;
    uint8_t countdown_backup;
    int16_t target_backup;
    int16_t timer_slot;
    int old_map;
    int map;
    int target;
    int hero_index;

    memset(&receipt, 0, sizeof(receipt));
    receipt.target_slot = -1;
    receipt.timer_type = 0x47u;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    party_backup = candidate->party;
    countdown_backup = candidate->source_hero_ench_countdown;
    target_backup = candidate->source_hero_ench_target;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto hero_ench_flag_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto hero_ench_flag_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto hero_ench_flag_rollback;
    }
    if (candidate->timer_entries[timer_slot].ttype != 0x47u) {
        receipt.blocked_unsupported_type = 1;
        goto hero_ench_flag_rollback;
    }
    target = candidate->source_hero_ench_target;
    if (target < 0 || target > DM2_MAX_HEROES ||
        (target != 0 && target > candidate->party.heros_in_party)) {
        receipt.blocked_party = 1;
        goto hero_ench_flag_rollback;
    }
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    receipt.map = map;
    receipt.target_slot = (int16_t)(target == 0 ? -1 : target - 1);
    if (map < 0 || map >= candidate->dungeon.level_count ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto hero_ench_flag_rollback;
    }
    receipt.map_switch = old_map != map;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    receipt.countdown_before = candidate->source_hero_ench_countdown;
    candidate->source_hero_ench_countdown--;
    receipt.countdown_after = candidate->source_hero_ench_countdown;
    receipt.countdown_decremented = 1;
    receipt.countdown_expired = candidate->source_hero_ench_countdown == 0u;
    if (receipt.countdown_expired && target != 0) {
        hero_index = target - 1;
        receipt.hero_flags_before = (uint16_t)candidate->party.hero[hero_index].heroflag;
        if (candidate->party.hero[hero_index].curHP != 0) {
            candidate->party.hero[hero_index].heroflag |= (int16_t)0x4000;
            receipt.hero_flag_set = 1;
        } else {
            receipt.hero_skipped_dead = 1;
        }
        receipt.hero_flags_after = (uint16_t)candidate->party.hero[hero_index].heroflag;
    }
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)timer_entry.l_00 ^
        ((uint32_t)candidate->source_hero_ench_countdown << 16) ^
        (uint32_t)(uint16_t)receipt.target_slot);
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

hero_ench_flag_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->party = party_backup;
        candidate->source_hero_ench_countdown = countdown_backup;
        candidate->source_hero_ench_target = target_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

/* c_cloud.cpp::DM2_075f_0182 admission for the bounded resurrection cloud.
 * The origin scan does not treat every DB3 as a cloud actuator: only DB3
 * type 0x26 records whose spell match survives the w4 gates are invoked.
 * The candidate has no authenticated invoke-owner for that positive branch,
 * so callers reject only that exact case and may still create a cloud on a
 * chain containing unrelated DB3 records. */
static int dm2_v1_resurrection_cloud_has_matching_actuator(
    const DM2_V1_RecordPoolSet *pools, int16_t head, int16_t cloud_spell,
    int16_t intensity, int chain_limit, int apply)
{
    int steps = 0;
    int16_t link = head;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        uint8_t *record_mut = NULL;
        int16_t next;
        if (link == DM2_V1_RECORD_HANDLE_NULL || steps++ >= chain_limit ||
            !(record = dm2_v1_record_pool_address(pools, link)) ||
            !dm2_v1_record_pool_next_link(pools, link, &next))
            return -1;
        if (apply) {
            record_mut = dm2_v1_record_pool_address_mut(
                (DM2_V1_RecordPoolSet *)pools, link);
            if (!record_mut) return -1;
        }
        if (dm2_v1_record_handle_pool(link) == DM2_DB_ACTUATOR) {
            uint16_t w2;
            uint16_t w4;
            int match;
            if (pools->pools[DM2_DB_ACTUATOR].record_size < 6)
                return -1;
            w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
            if ((w2 & 0x7fu) == 0x26u) {
                match = (w2 & 0xff80u) == 0xff80u ||
                    (uint16_t)(w2 >> 7) ==
                        (uint16_t)((cloud_spell - (int16_t)0xff80) & 0x7f);
                w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
                match ^= (int)((w4 >> 5) & 1u);
                if ((w4 & 0x0004u) != 0u &&
                    intensity < ((uint16_t)cloud_spell == 0xff82u ? 0x7f : 0xff))
                    match = 0;
                if ((w4 & 0x0018u) == 0x0018u)
                    match = !match;
                if (match) {
                    if (apply) {
                        uint8_t action = (uint8_t)((w4 >> 3) & 3u);
                        uint8_t once = (uint8_t)((w4 >> 2) & 1u);
                        uint8_t next_once;
                        if (action == 0u) next_once = 1u;
                        else if (action == 1u) next_once = 0u;
                        else if (action == 2u) next_once = (uint8_t)(once ^ 1u);
                        else next_once = once; /* source default: unchanged */
                        w4 = (uint16_t)((w4 & (uint16_t)~0x0004u) |
                                        ((uint16_t)next_once << 2));
                        record_mut[4] = (uint8_t)w4;
                        record_mut[5] = (uint8_t)(w4 >> 8);
                    }
                    if (!apply) return 1;
                }
            }
        }
        link = next;
    }
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_resurrection_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadResurrectionTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadResurrectionTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_RecordPoolSet pool_backup;
    uint8_t *raw_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_Party party_backup;
    int16_t timer_slot;
    int old_map;
    int map;
    int hero_index;
    uint8_t phase;
    int16_t new_max;
    int x;
    int y;
    int16_t cursor;
    int16_t next;
    int found_record = DM2_V1_RECORD_HANDLE_NULL;

    memset(&receipt, 0, sizeof(receipt));
    memset(&pool_backup, 0, sizeof(pool_backup));
    receipt.timer_type = 0x0du;
    receipt.game_tick = game_tick;
    receipt.hero_index = -1;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    party_backup = candidate->party;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto resurrection_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto resurrection_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x0du) {
        receipt.blocked_unsupported_type = 1;
        goto resurrection_rollback;
    }
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    hero_index = candidate->timer_entries[timer_slot].actor;
    phase = (uint8_t)((uint16_t)candidate->timer_entries[timer_slot].wvalueB >> 8);
    receipt.map = map;
    receipt.hero_index = (int16_t)hero_index;
    receipt.phase = phase;
    if (map < 0 || map >= candidate->dungeon.level_count) {
        receipt.blocked_map = 1;
        goto resurrection_rollback;
    }
    if (hero_index < 0 || hero_index >= candidate->party.heros_in_party ||
        hero_index >= DM2_MAX_HEROES) {
        receipt.blocked_actor = 1;
        goto resurrection_rollback;
    }
    if (phase != 0u) {
        if (phase == 2u) {
            DM2_V1_TimerEntry cloud_timer;
            DM2_V1_SoundQueueEnv sound_env;
            DM2_V1_GameLoadSoundNoiseReceipt sound_receipt;
            int16_t head;
            int16_t cloud_record;
            int chain_limit = 0;
            int direction;
            uint8_t *cloud;
            int matching_actuator;

            x = (int)(int8_t)candidate->timer_entries[timer_slot].xA;
            y = (int)(int8_t)candidate->timer_entries[timer_slot].yA;
            direction = (int)(int8_t)(uint8_t)
                ((uint16_t)candidate->timer_entries[timer_slot].wvalueB & 0xffu);
            if (x < 0 || y < 0 || map < 0 ||
                map >= candidate->dungeon.level_count ||
                x >= candidate->dungeon.level_widths[map] ||
                y >= candidate->dungeon.level_heights[map] ||
                !candidate->sound_owner.valid ||
                !candidate->sound_owner.runtime_queue_initialized ||
                !candidate->sound_owner.queue_entries ||
                !candidate->sound_owner.sample_bindings) {
                receipt.blocked_cloud_owner = 1;
                goto resurrection_rollback;
            }
            /* c_cloud.cpp scans the origin chain for matching DB3 type-0x26
             * actuators after creating the DB15 record.  Unrelated DB3
             * records are not part of this cloud owner and must not block it;
             * a matching actuator remains fail-closed until its invoke-owner
             * is authenticated. */
            head = (int16_t)dm2_v1_dungeon_get_first_thing(
                &candidate->dungeon, map, x, y);
            for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
                const DM2_V1_RecordPool *pool =
                    &candidate->record_pools.pools[db];
                if (pool->record_count < 0 || pool->extension_count < 0 ||
                    chain_limit > INT_MAX - pool->record_count -
                        pool->extension_count) {
                    receipt.blocked_cloud_actuator = 1;
                    goto resurrection_rollback;
                }
                chain_limit += pool->record_count + pool->extension_count;
            }
            if (chain_limit <= 0) {
                receipt.blocked_cloud_actuator = 1;
                goto resurrection_rollback;
            }
            {
                matching_actuator =
                    dm2_v1_resurrection_cloud_has_matching_actuator(
                        &candidate->record_pools, head, 0xffe4, 0,
                        chain_limit, 0);
                if (matching_actuator < 0) {
                    receipt.blocked_cloud_actuator = 1;
                    goto resurrection_rollback;
                }
            }
            if (!dm2_v1_record_pool_set_clone(&pool_backup,
                    &candidate->record_pools) || candidate->dungeon.raw_size <= 0) {
                receipt.blocked_cloud_owner = 1;
                goto resurrection_rollback;
            }
            raw_backup = (uint8_t *)malloc((size_t)candidate->dungeon.raw_size);
            if (!raw_backup) goto resurrection_rollback;
            memcpy(raw_backup, candidate->dungeon.raw_data,
                   (size_t)candidate->dungeon.raw_size);
            cloud_record = dm2_v1_record_pool_alloc_new_record(
                &candidate->record_pools, 15u);
            cloud = dm2_v1_record_pool_address_mut(
                &candidate->record_pools, cloud_record);
            if (cloud_record < 0 || !cloud ||
                candidate->record_pools.pools[15].record_size < 4) {
                receipt.blocked_cloud_owner = 1;
                goto resurrection_rollback;
            }
            /* DM2_CREATE_CLOUD(0xffe4, 0, xA, yA, xB): cloud type is
             * (0xffe4 - 0xff80) & 0x7f == 0x64, strength is zero, and the
             * low direction byte selects directional versus omnidirectional
             * mode through bit 7 of DB15 word 2. */
            cloud[0] = 0xfeu;
            cloud[1] = 0xffu;
            cloud[2] = (uint8_t)(0x64u | (direction == 0xff ? 0x80u : 0u));
            cloud[3] = 0u;
            {
                int append_ok = dm2_v1_record_pool_append_to_list(
                    &candidate->record_pools, &head, cloud_record);
                int tile_ok = dm2_v1_dungeon_set_first_thing(
                    &candidate->dungeon, map, x, y, (uint16_t)head);
                if (!append_ok || tile_ok != 0) {
                    receipt.blocked_cloud_owner = 1;
                    goto resurrection_rollback;
                }
            }
            if (matching_actuator != 0 &&
                dm2_v1_resurrection_cloud_has_matching_actuator(
                    &candidate->record_pools, head, 0xffe4, 0,
                    chain_limit, 1) < 0) {
                receipt.blocked_cloud_actuator = 1;
                goto resurrection_rollback;
            }
            memset(&sound_env, 0, sizeof(sound_env));
            sound_env.current_map = (int16_t)map;
            sound_env.gate_map_a = candidate->sound_owner.spatial_audible_map;
            sound_env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
            sound_env.facing = candidate->source_party_direction;
            sound_env.party_x = candidate->source_party_x;
            sound_env.party_y = candidate->source_party_y;
            sound_env.gametick = (int32_t)game_tick;
            memset(&sound_receipt, 0, sizeof(sound_receipt));
            if (candidate->sound_owner.spatial_context_valid &&
                dm2_v1_game_load_sound_owner_queue_noise_gen2(
                    &candidate->sound_owner, 0x0d, 0x64, (int8_t)0x81,
                    (int8_t)0xfe, (int16_t)x, (int16_t)y, 1, 0x6c, 1,
                    &sound_env, &sound_receipt)) {
                receipt.cloud_sound_queued = 1;
            }
            dm2_v1_timer_entry_init(&cloud_timer);
            dm2_v1_timer_set_mticks(&cloud_timer, (int16_t)map,
                                     game_tick + 5u);
            cloud_timer.ttype = 0x19u;
            cloud_timer.actor = 0u;
            cloud_timer.xA = (int8_t)(uint8_t)x;
            cloud_timer.yA = (int8_t)(uint8_t)y;
            cloud_timer.wvalueB = cloud_record;
            if (dm2_v1_timer_queue(&candidate->timer_queue, &cloud_timer) < 0)
                goto resurrection_rollback;
            dm2_v1_timer_get_and_delete_next(&candidate->timer_queue,
                                             &timer_entry);
            receipt.cloud_record = cloud_record;
            receipt.cloud_created = 1;
            receipt.cloud_timer_queued = 1;
            receipt.consumed = 1;
            receipt.valid = 1;
            receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                candidate->source_transaction_hash ^ candidate->candidate_hash,
                (uint32_t)(uint16_t)cloud_record ^
                    ((uint32_t)map << 16) ^ ((uint32_t)x << 8) ^
                    (uint32_t)y);
            dm2_v1_record_pool_set_free(&pool_backup);
            free(raw_backup);
            free(index_backup);
            free(timer_backup);
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (phase != 1u) {
            receipt.blocked_phase = 1;
            goto resurrection_rollback;
        }
        x = (int)(int8_t)candidate->timer_entries[timer_slot].xA;
        y = (int)(int8_t)candidate->timer_entries[timer_slot].yA;
        if (x < 0 || y < 0 || x >= candidate->map_context.width ||
            y >= candidate->map_context.height) {
            receipt.blocked_chain = 1;
            goto resurrection_rollback;
        }
        cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
            &candidate->dungeon, map, x, y);
        for (int step = 0; cursor >= 0 && cursor != DM2_V1_RECORD_HANDLE_END &&
             step < 65536; ++step) {
            const uint8_t *record = dm2_v1_record_pool_address(
                &candidate->record_pools, cursor);
            if (!record || !dm2_v1_record_pool_next_link(
                    &candidate->record_pools, cursor, &next)) {
                receipt.blocked_chain = 1;
                goto resurrection_rollback;
            }
            if (dm2_v1_record_handle_pool(cursor) == 10 &&
                dm2_v1_record_handle_index(cursor) == 0 &&
                (((uint16_t)record[2] |
                  ((uint16_t)record[3] << 8)) >> 14) ==
                    (uint16_t)hero_index) {
                found_record = cursor;
                break;
            }
            cursor = next;
        }
        if (found_record < 0) {
            receipt.blocked_chain = 1;
            goto resurrection_rollback;
        }
        if (!dm2_v1_record_pool_set_clone(&pool_backup,
                &candidate->record_pools) || candidate->dungeon.raw_size <= 0) {
            receipt.blocked_chain = 1;
            goto resurrection_rollback;
        }
        raw_backup = (uint8_t *)malloc((size_t)candidate->dungeon.raw_size);
        if (!raw_backup) goto resurrection_rollback;
        memcpy(raw_backup, candidate->dungeon.raw_data,
               (size_t)candidate->dungeon.raw_size);
        if (!dm2_v1_record_pool_cut_from_tile(&candidate->record_pools,
                &candidate->dungeon, map, x, y, found_record)) {
            receipt.blocked_chain = 1;
            goto resurrection_rollback;
        }
        {
            uint8_t *record = dm2_v1_record_pool_address_mut(
                &candidate->record_pools, found_record);
            if (!record) goto resurrection_rollback;
            record[0] = 0xffu;
            record[1] = 0xffu;
        }
        dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
        receipt.altar_record = found_record;
        receipt.record_removed = 1;
        receipt.consumed = 1;
        receipt.valid = 1;
        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
            candidate->source_transaction_hash ^ candidate->candidate_hash,
            (uint32_t)(uint16_t)found_record ^ ((uint32_t)map << 16));
        dm2_v1_record_pool_set_free(&pool_backup);
        free(raw_backup);
        free(index_backup);
        free(timer_backup);
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if (candidate->current_map != map &&
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0))
        goto resurrection_rollback;
    receipt.map_switch = old_map != map;
    receipt.max_hp_before = candidate->party.hero[hero_index].maxHP;
    if (receipt.max_hp_before <= 0)
        goto resurrection_rollback;
    new_max = (int16_t)(receipt.max_hp_before - receipt.max_hp_before / 64 - 1);
    if (new_max < 25) new_max = 25;
    receipt.hero_flags_before = (uint16_t)candidate->party.hero[hero_index].heroflag;
    candidate->party.hero[hero_index].weight = 0;
    for (int i = 0; i < DM2_NUM_ITEMS; ++i)
        candidate->party.hero[hero_index].item[i] = -1;
    candidate->party.hero[hero_index].maxHP = new_max;
    candidate->party.hero[hero_index].curHP = (int16_t)(new_max / 2);
    candidate->party.hero[hero_index].heroflag |= 0x4000;
    candidate->party.hero[hero_index].ench_aura = 0;
    candidate->party.hero[hero_index].ench_power = 0;
    receipt.max_hp_after = new_max;
    receipt.cur_hp_after = candidate->party.hero[hero_index].curHP;
    receipt.hero_flags_after = (uint16_t)candidate->party.hero[hero_index].heroflag;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    receipt.champion_revived = 1;
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)hero_index ^
            ((uint32_t)receipt.hero_flags_after << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

resurrection_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->party = party_backup;
        if (raw_backup) {
            memcpy(candidate->dungeon.raw_data, raw_backup,
                   (size_t)candidate->dungeon.raw_size);
            dm2_v1_record_pool_set_free(&candidate->record_pools);
            candidate->record_pools = pool_backup;
            memset(&pool_backup, 0, sizeof(pool_backup));
        }
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_process_0c_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadProcess0cTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadProcess0cTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_Party party_backup;
    int16_t timer_slot;
    int old_map;
    int map;
    int hero_index;

    memset(&receipt, 0, sizeof(receipt));
    receipt.timer_type = 0x0cu;
    receipt.game_tick = game_tick;
    receipt.hero_index = -1;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    party_backup = candidate->party;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto process_0c_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto process_0c_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x0cu) {
        receipt.blocked_unsupported_type = 1;
        goto process_0c_rollback;
    }
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    hero_index = candidate->timer_entries[timer_slot].actor;
    receipt.map = map;
    receipt.hero_index = (int16_t)hero_index;
    if (map < 0 || map >= candidate->dungeon.level_count) {
        receipt.blocked_map = 1;
        goto process_0c_rollback;
    }
    if (hero_index < 0 || hero_index >= candidate->party.heros_in_party ||
        hero_index >= DM2_MAX_HEROES) {
        receipt.blocked_actor = 1;
        goto process_0c_rollback;
    }
    if (candidate->current_map != map &&
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0))
        goto process_0c_rollback;
    receipt.map_switch = old_map != map;
    receipt.timer_index_before = candidate->party.hero[hero_index].timeridx;
    receipt.hero_flags_before = (uint16_t)candidate->party.hero[hero_index].heroflag;
    candidate->party.hero[hero_index].timeridx = -1;
    if (candidate->party.hero[hero_index].curHP != 0)
        candidate->party.hero[hero_index].heroflag |= 0x0800;
    receipt.timer_index_after = candidate->party.hero[hero_index].timeridx;
    receipt.hero_flags_after = (uint16_t)candidate->party.hero[hero_index].heroflag;
    receipt.hero_mutated = receipt.timer_index_before != receipt.timer_index_after ||
        receipt.hero_flags_before != receipt.hero_flags_after;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)hero_index ^
            ((uint32_t)receipt.hero_flags_after << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

process_0c_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        candidate->party = party_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_destroy_door_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadDestroyDoorTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadDestroyDoorTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    int16_t timer_slot;
    int old_map;
    int map;
    int x;
    int y;
    int raw;

    memset(&receipt, 0, sizeof(receipt));
    receipt.timer_type = 0x02u;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto destroy_door_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto destroy_door_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x02u) {
        receipt.blocked_unsupported_type = 1;
        goto destroy_door_rollback;
    }
    map = dm2_v1_timer_get_map(&candidate->timer_entries[timer_slot]);
    x = (int)(int8_t)candidate->timer_entries[timer_slot].xA;
    y = (int)(int8_t)candidate->timer_entries[timer_slot].yA;
    receipt.map = map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    if (map < 0 || dm2_v1_dungeon_get_tile_raw(&candidate->dungeon, map, x, y) < 0) {
        receipt.blocked_map = 1;
        goto destroy_door_rollback;
    }
    if (candidate->current_map != map &&
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0))
        goto destroy_door_rollback;
    receipt.map_switch = old_map != map;
    raw = dm2_v1_dungeon_get_tile_raw(&candidate->dungeon, map, x, y);
    if (raw < 0 || dm2_v1_dungeon_set_tile_raw(
            &candidate->dungeon, map, x, y, (uint16_t)((raw & 0xfff8) | 0x0005)) < 0)
        goto destroy_door_rollback;
    receipt.tile_before = (uint16_t)raw;
    receipt.tile_after = (uint16_t)((raw & 0xfff8) | 0x0005);
    receipt.tile_mutated = receipt.tile_before != receipt.tile_after;
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)receipt.tile_after ^ ((uint32_t)(uint16_t)map << 16));
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

destroy_door_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (candidate->current_map != old_map)
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

static uint8_t *dm2_game_load_candidate_cloud_get_record(
    void *ctx, uint16_t record)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;

    if (!candidate || !candidate->record_pools.valid)
        return NULL;
    return dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                          (int16_t)record);
}

static int16_t dm2_game_load_candidate_cloud_rand16(void *ctx, int16_t n)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;

    if (!candidate || n <= 0 || !candidate->caii_rng_initialized)
        return 0;
    return (int16_t)dm2_v1_drops_rand16(&candidate->caii_rng,
                                        (uint16_t)n);
}

static bool dm2_game_load_candidate_cloud_randbit(void *ctx)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;

    return candidate && candidate->caii_rng_initialized &&
           dm2_v1_drops_randbit(&candidate->caii_rng) != 0u;
}

static int16_t dm2_game_load_candidate_cloud_min16(int16_t a, int16_t b)
{
    return a < b ? a : b;
}

static int16_t dm2_game_load_candidate_cloud_max16(int16_t a, int16_t b)
{
    return a > b ? a : b;
}

static int16_t dm2_game_load_candidate_cloud_max16_ctx(
    void *ctx, int16_t a, int16_t b)
{
    (void)ctx;
    return dm2_game_load_candidate_cloud_max16(a, b);
}

static int dm2_game_load_candidate_attack_ai_flags(
    void *ctx, int creature_type, uint16_t *out_flags)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    const DM2_AIDefinition *definition = NULL;
    if (out_flags) *out_flags = 0u;
    if (!candidate || !out_flags ||
        !dm2_v1_caii_source_owner_ai_spec_def(&candidate->caii_source,
                                               creature_type, &definition) ||
        !definition)
        return 0;
    *out_flags = definition->w0AIFlags;
    return 1;
}

static int dm2_game_load_candidate_attack_base_hp(
    void *ctx, int creature_type, uint16_t *out_value)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    const DM2_AIDefinition *definition = NULL;
    if (out_value) *out_value = 0u;
    if (!candidate || !out_value ||
        !dm2_v1_caii_source_owner_ai_spec_def(&candidate->caii_source,
                                               creature_type, &definition) ||
        !definition)
        return 0;
    *out_value = definition->BaseHP;
    return 1;
}

static int dm2_game_load_candidate_attack_gdat_word1(
    void *ctx, int creature_type, uint16_t *out_value)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    if (out_value) *out_value = 0u;
    if (!candidate || !candidate->asset_loader || !out_value)
        return 0;
    return dm2_v1_asset_load_word_value(
        candidate->asset_loader, DM2_GDAT_CATEGORY_CREATURES,
        creature_type, 0x01u, out_value);
}

static const uint8_t *dm2_game_load_candidate_cloud_ai_spec_from_type_const(
    void *ctx, uint16_t creature_type)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    const DM2_AIDefinition *definition = NULL;
    if (!candidate || !dm2_v1_caii_source_owner_ai_spec_def(
            &candidate->caii_source, (int)creature_type, &definition) ||
        !definition)
        return NULL;
    return (uint8_t *)(uintptr_t)definition;
}

static uint8_t *dm2_game_load_candidate_cloud_ai_spec_from_type(
    void *ctx, uint16_t creature_type)
{
    return (uint8_t *)(uintptr_t)
        dm2_game_load_candidate_cloud_ai_spec_from_type_const(
            ctx, creature_type);
}

static int16_t dm2_game_load_candidate_cloud_ai_flags(
    void *ctx, uint16_t creature_type)
{
    uint16_t flags = 0u;
    return dm2_game_load_candidate_attack_ai_flags(
        ctx, (int)creature_type, &flags) ? (int16_t)flags : 0;
}

static uint16_t dm2_game_load_candidate_cloud_randdir(void *ctx)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    return candidate && candidate->caii_rng_initialized
        ? dm2_v1_drops_randdir(&candidate->caii_rng) : 0u;
}

static int16_t dm2_game_load_candidate_cloud_poison_resistance(
    void *ctx, uint16_t creature_type, uint16_t damage)
{
    DM2_V1_CreaturePoisonCallbacks poison_cb;
    poison_cb.query_ai_spec =
        dm2_game_load_candidate_cloud_ai_spec_from_type_const;
    poison_cb.rand_dir = dm2_game_load_candidate_cloud_randdir;
    return dm2_v1_apply_creature_poison_resistance(
        creature_type, (int16_t)damage, &poison_cb, ctx);
}

static int dm2_game_load_candidate_attack_allocate(
    void *ctx, DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon, DM2_V1_CaiiArray *caii,
    int16_t record_handle, int map_id, unsigned long game_tick,
    int x, int y)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    DM2_V1_GameLoadMoverecCaiiReceipt receipt;
    (void)pool_set; (void)dungeon; (void)caii; (void)game_tick;
    if (!candidate || candidate->current_map != map_id)
        return 0;
    memset(&receipt, 0, sizeof(receipt));
    return dm2_v1_game_load_runtime_session_candidate_activate_moverec_caii(
        candidate, record_handle, (int16_t)x, (int16_t)y, &receipt) &&
        receipt.valid;
}

static int dm2_game_load_candidate_attack_delete_timer(
    void *ctx, DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon, DM2_V1_CaiiArray *caii,
    int16_t record_handle, int map_id, unsigned long game_tick,
    int x, int y)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    uint8_t *record;
    uint8_t *slot;
    uint16_t timer_slot;
    (void)pool_set; (void)dungeon; (void)caii;
    (void)map_id; (void)game_tick; (void)x; (void)y;
    if (!candidate || dm2_v1_record_handle_pool(record_handle) != 4 ||
        !(record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                                   record_handle)) ||
        record[5] == 0xffu || record[5] >= candidate->caii_slots.capacity)
        return 0;
    slot = candidate->caii_slots.slots +
        (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
    timer_slot = dm2_v1_game_load_owner_read_u16le(slot + 2u);
    if (timer_slot == 0xffffu || timer_slot >= candidate->timer_capacity ||
        !candidate->timer_entries ||
        candidate->timer_entries[timer_slot].ttype == 0u)
        return 0;
    dm2_v1_timer_delete(&candidate->timer_queue, (int16_t)timer_slot);
    dm2_v1_game_load_owner_write_u16le(slot + 2u, 0xffffu);
    return 1;
}

static int dm2_game_load_candidate_attack_schedule(
    void *ctx, DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon, DM2_V1_CaiiArray *caii,
    int16_t record_handle, int map_id, unsigned long game_tick,
    int x, int y)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    uint8_t *record;
    uint8_t *slot;
    DM2_V1_TimerEntry timer;
    int16_t timer_slot;
    (void)pool_set; (void)dungeon; (void)caii;
    if (!candidate || candidate->current_map != map_id ||
        dm2_v1_record_handle_pool(record_handle) != 4 ||
        !(record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                                   record_handle)) ||
        record[5] == 0xffu || record[5] >= candidate->caii_slots.capacity)
        return 0;
    slot = candidate->caii_slots.slots +
        (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
    dm2_v1_timer_entry_init(&timer);
    dm2_v1_timer_set_mticks(&timer, (int16_t)map_id, (int32_t)game_tick + 1);
    timer.ttype = dm2_v1_game_load_owner_read_u16le(record + 8u) != 0xffffu
        ? 0x22u : 0x21u;
    timer.actor = record[4];
    timer.xA = (int8_t)x;
    timer.yA = (int8_t)y;
    timer_slot = dm2_v1_timer_queue(&candidate->timer_queue, &timer);
    if (timer_slot < 0)
        return 0;
    dm2_v1_game_load_owner_write_u16le(slot + 2u, (uint16_t)timer_slot);
    return 1;
}

static int16_t dm2_game_load_candidate_cloud_wound_party(
    void *ctx, int hero_idx, int16_t damage, int body_parts,
    int damage_type)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate = ctx;
    DM2_V1_Hero *hero;
    int32_t pending;

    (void)body_parts;
    (void)damage_type;
    if (!candidate || hero_idx < 0 ||
        hero_idx >= candidate->party.heros_in_party || damage <= 0)
        return 0;
    hero = &candidate->party.hero[hero_idx];
    if (hero->curHP == 0)
        return 0;
    pending = (int32_t)hero->damagesuffered + damage;
    if (pending > INT16_MAX)
        pending = INT16_MAX;
    hero->damagesuffered = (int16_t)pending;
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag |
                               DM2_V1_HERO_FLAG_0800);
    return damage;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_cloud_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCloudTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadCloudTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerEntry next_timer;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_GameLoadSoundOwner sound_backup;
    DM2_V1_Party party_backup;
    DM2_V1_DropRng rng_backup;
    uint8_t *caii_backup = NULL;
    size_t caii_bytes = 0u;
    int party_backed_up = 0;
    int rng_backed_up = 0;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_GameLoadSoundNoiseReceipt sound_receipt;
    uint8_t *cloud = NULL;
    int16_t timer_slot;
    int16_t cloud_record;
    int16_t cloud_head;
    int cloud_in_chain = 0;
    int old_map;
    int map;
    int x;
    int y;
    int raw_before = -1;
    int raw_mutated = 0;
    uint16_t w2;

    memset(&receipt, 0, sizeof(receipt));
    memset(&pool_backup, 0, sizeof(pool_backup));
    memset(&sound_backup, 0, sizeof(sound_backup));
    receipt.cloud_record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.timer_type = 0x19u;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    party_backup = candidate->party;
    rng_backup = candidate->caii_rng;
    party_backed_up = 1;
    rng_backed_up = candidate->caii_rng_initialized;
    queue_backup = candidate->timer_queue;
    caii_bytes = (size_t)candidate->caii_slots.capacity *
        DM2_V1_CAII_SLOT_SIZE;
    if (caii_bytes != 0u)
        caii_backup = malloc(caii_bytes);
    timer_backup = malloc((size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = malloc((size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup ||
        (caii_bytes != 0u && !caii_backup) ||
        !dm2_v1_record_pool_set_clone(&pool_backup, &candidate->record_pools) ||
        !dm2_v1_game_load_runtime_candidate_sound_clone(
            &sound_backup, &candidate->sound_owner))
        goto cloud_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (caii_bytes != 0u)
        memcpy(caii_backup, candidate->caii_slots.slots, caii_bytes);
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto cloud_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto cloud_rollback;
    }
    if (candidate->timer_entries[timer_slot].ttype != 0x19u) {
        receipt.blocked_unsupported_type = 1;
        goto cloud_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(int8_t)timer_entry.xA;
    y = (int)(int8_t)timer_entry.yA;
    receipt.map = map;
    cloud_record = timer_entry.wvalueB;
    receipt.cloud_record = cloud_record;
    if (map < 0 || map >= candidate->dungeon.level_count || x < 0 || y < 0 ||
        x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        dm2_v1_record_handle_pool(cloud_record) != 15 ||
        !(cloud = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, cloud_record)) ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto cloud_rollback;
    }
    w2 = (uint16_t)cloud[2] | ((uint16_t)cloud[3] << 8);
    receipt.cloud_type_before = (uint8_t)(w2 & 0x7fu);
    receipt.cloud_strength_before = (uint8_t)(w2 >> 8);
    if ((receipt.cloud_type_before >= 8u &&
         receipt.cloud_type_before != 0x28u &&
         receipt.cloud_type_before != 0x64u &&
         receipt.cloud_type_before != 0x65u)) {
        receipt.blocked_unsupported_type = 1;
        goto cloud_rollback;
    }
    /* The source timer names a DB15 record, but the authoritative owner is
     * the tile-rooted chain.  Preflight the complete chain before either the
     * 0x64->0x65 write or the final 0x65 cut; a malformed tail must leave the
     * candidate's timer, pool and map state untouched. */
    cloud_head = (int16_t)dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, map, x, y);
    if (!dm2_v1_game_load_record_chain_is_bounded(
            &candidate->record_pools, cloud_head, cloud_record,
            &cloud_in_chain) || !cloud_in_chain) {
        receipt.blocked_record = 1;
        goto cloud_rollback;
    }
    /* The candidate owns the source-sized party and RNG state, so the
     * ordinary party target can use the same damage/hero owners as runtime.
     * Creature and door targets remain fail-closed until their candidate
     * attack owners are complete.  Types 0 and 2 are source no-effect cases
     * and may also expire on a populated cell. */
    if (receipt.cloud_type_before < 8u ||
        receipt.cloud_type_before == 0x28u) {
        int raw_tile = dm2_v1_dungeon_get_tile_raw(
            &candidate->dungeon, map, x, y);
        int party_here = map == candidate->source_party_map &&
            x == candidate->source_party_x && y == candidate->source_party_y;
        int16_t creature_here = dm2_v1_get_creature_at(
            &candidate->record_pools, &candidate->dungeon, map, x, y);
        int door_here = raw_tile >= 0 && (((unsigned int)raw_tile >> 5) & 7u) == 4u;
        raw_before = raw_tile;
        if (door_here) {
            int16_t door_link = (int16_t)dm2_v1_dungeon_get_first_thing(
                &candidate->dungeon, map, x, y);
            uint8_t *door_record;
            DM2_V1_CloudCallbacks damage_cb;
            DM2_V1_CalcCloudDamageReceipt damage_receipt;
            uint16_t old_raw = (uint16_t)raw_tile;
            int door_state = dm2_door_get_state(old_raw);
            int door_type;

            if (door_link == DM2_V1_RECORD_HANDLE_NULL ||
                door_link == DM2_V1_RECORD_HANDLE_END ||
                dm2_v1_record_handle_pool(door_link) != DM2_DB_DOOR ||
                !(door_record = dm2_v1_record_pool_address_mut(
                    &candidate->record_pools, door_link)))
                goto cloud_rollback;
            memset(&damage_cb, 0, sizeof(damage_cb));
            damage_cb.ctx = candidate;
            damage_cb.get_address_of_record =
                dm2_game_load_candidate_cloud_get_record;
            damage_cb.rand16 = dm2_game_load_candidate_cloud_rand16;
            damage_cb.randbit = dm2_game_load_candidate_cloud_randbit;
            damage_cb.min16 = dm2_game_load_candidate_cloud_min16;
            damage_cb.max16 = dm2_game_load_candidate_cloud_max16;
            damage_cb.query_creature_ai_spec_from_type =
                dm2_game_load_candidate_cloud_ai_spec_from_type;
            damage_cb.query_creature_ai_spec_flags =
                dm2_game_load_candidate_cloud_ai_flags;
            damage_cb.apply_creature_poison_resistance =
                dm2_game_load_candidate_cloud_poison_resistance;
            damage_receipt = dm2_v1_calc_cloud_damage(
                &damage_cb, (uint16_t)cloud_record, door_link);
            receipt.door_damage = damage_receipt.damage;
            door_type = (int)((uint16_t)door_record[2] & 0x0001u);
            if (damage_receipt.damage > 0 &&
                dm2_door_check_destruction(
                    door_type, door_state, damage_receipt.damage, 1,
                    door_record[2]) == DM2_DOOR_DESTROYED_YES) {
                if (dm2_v1_dungeon_set_tile_raw(
                        &candidate->dungeon, map, x, y,
                        dm2_door_set_state(old_raw,
                                           DM2_DOOR_STATE_DESTROYED)) != 0) {
                    raw_mutated = 1;
                    receipt.door_destroyed = 1;
                } else {
                    goto cloud_rollback;
                }
            }
        }
        if (creature_here != DM2_V1_RECORD_HANDLE_NULL &&
            receipt.cloud_type_before != 0u &&
            receipt.cloud_type_before != 2u) {
            DM2_V1_CloudCallbacks damage_cb;
            DM2_V1_CalcCloudDamageReceipt damage_receipt;
            DM2_V1_CaiiAttackContext attack_context;
            DM2_V1_CaiiAttackReceipt attack_receipt;
            memset(&damage_cb, 0, sizeof(damage_cb));
            damage_cb.ctx = candidate;
            damage_cb.get_address_of_record =
                dm2_game_load_candidate_cloud_get_record;
            damage_cb.rand16 = dm2_game_load_candidate_cloud_rand16;
            damage_cb.randbit = dm2_game_load_candidate_cloud_randbit;
            damage_cb.min16 = dm2_game_load_candidate_cloud_min16;
            damage_cb.max16 = dm2_game_load_candidate_cloud_max16;
            damage_cb.query_creature_ai_spec_from_type =
                dm2_game_load_candidate_cloud_ai_spec_from_type;
            damage_cb.query_creature_ai_spec_flags =
                dm2_game_load_candidate_cloud_ai_flags;
            damage_cb.apply_creature_poison_resistance =
                dm2_game_load_candidate_cloud_poison_resistance;
            damage_receipt = dm2_v1_calc_cloud_damage(
                &damage_cb, (uint16_t)cloud_record, creature_here);
            memset(&attack_context, 0, sizeof(attack_context));
            attack_context.ctx = candidate;
            attack_context.ai_spec_flags_context =
                dm2_game_load_candidate_attack_ai_flags;
            attack_context.ai_base_hp_context =
                dm2_game_load_candidate_attack_base_hp;
            attack_context.gdat_word1_context =
                dm2_game_load_candidate_attack_gdat_word1;
            attack_context.allocate = dm2_game_load_candidate_attack_allocate;
            attack_context.delete_timer =
                dm2_game_load_candidate_attack_delete_timer;
            attack_context.schedule = dm2_game_load_candidate_attack_schedule;
            if (damage_receipt.damage > 0 &&
                !dm2_v1_caii_attack_creature_context(
                    &candidate->record_pools, &candidate->dungeon,
                    &candidate->caii_slots, NULL, &candidate->caii_rng,
                    map, (unsigned long)game_tick, creature_here, x, y,
                    x, y, 0x200du, 0x64, damage_receipt.damage,
                    &attack_context, &attack_receipt)) {
                receipt.blocked_unsupported_type = 1;
                goto cloud_rollback;
            }
            receipt.creature_damage = damage_receipt.damage;
            receipt.creature_attacked = damage_receipt.damage > 0;
        }
        if (party_here && receipt.cloud_type_before != 0u &&
            receipt.cloud_type_before != 2u) {
            DM2_V1_CloudCallbacks damage_cb;
            DM2_V1_HeroAttackPartyCallbacks party_cb;
            DM2_V1_CalcCloudDamageReceipt damage_receipt;

            memset(&damage_cb, 0, sizeof(damage_cb));
            damage_cb.ctx = candidate;
            damage_cb.get_address_of_record =
                dm2_game_load_candidate_cloud_get_record;
            damage_cb.rand16 = dm2_game_load_candidate_cloud_rand16;
            damage_cb.randbit = dm2_game_load_candidate_cloud_randbit;
            damage_cb.min16 = dm2_game_load_candidate_cloud_min16;
            damage_cb.max16 = dm2_game_load_candidate_cloud_max16;
            damage_receipt = dm2_v1_calc_cloud_damage(
                &damage_cb, (uint16_t)cloud_record, -1);
            if (damage_receipt.damage > 0) {
                memset(&party_cb, 0, sizeof(party_cb));
                party_cb.hero_count = candidate->party.heros_in_party;
                party_cb.wound_player =
                    dm2_game_load_candidate_cloud_wound_party;
                party_cb.rand16 = dm2_game_load_candidate_cloud_rand16;
                party_cb.max16 = dm2_game_load_candidate_cloud_max16_ctx;
                receipt.party_damage = damage_receipt.damage;
                receipt.party_wounded_mask = dm2_v1_hero_attack_party(
                    damage_receipt.damage, 0, 0, &party_cb, candidate);
            }
        }
        if (receipt.cloud_type_before == 7u && (w2 >> 8) >= 6u) {
            w2 = (uint16_t)((w2 & 0x00ffu) |
                (uint16_t)(((w2 >> 8) - 3u) << 8));
            cloud[2] = (uint8_t)w2;
            cloud[3] = (uint8_t)(w2 >> 8);
            receipt.cloud_strength_after = (uint8_t)(w2 >> 8);
            next_timer = timer_entry;
            next_timer.l_00 += 1;
            if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
                goto cloud_rollback;
            receipt.cloud_type_after = 7u;
            receipt.requeued = 1;
            receipt.consumed = 1;
            receipt.valid = 1;
            receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                candidate->source_transaction_hash ^ candidate->candidate_hash,
                (uint32_t)(uint16_t)cloud_record ^ 0x070000u);
            dm2_v1_record_pool_set_free(&pool_backup);
            dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
            free(index_backup);
            free(timer_backup);
            free(caii_backup);
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (receipt.cloud_type_before == 0x28u && (w2 >> 8) > 0x37u) {
            w2 = (uint16_t)((w2 & 0x00ffu) |
                (uint16_t)(((w2 >> 8) - 0x28u) << 8));
            cloud[2] = (uint8_t)w2;
            cloud[3] = (uint8_t)(w2 >> 8);
            receipt.cloud_strength_after = (uint8_t)(w2 >> 8);
            next_timer = timer_entry;
            next_timer.l_00 += 1;
            if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
                goto cloud_rollback;
            receipt.cloud_type_after = 0x28u;
            receipt.requeued = 1;
            receipt.consumed = 1;
            receipt.valid = 1;
            receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
                candidate->source_transaction_hash ^ candidate->candidate_hash,
                (uint32_t)(uint16_t)cloud_record ^ 0x280000u);
            dm2_v1_record_pool_set_free(&pool_backup);
            dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
            free(index_backup);
            free(timer_backup);
            free(caii_backup);
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        w2 = (uint16_t)((w2 & 0xff80u) | 0x65u);
        cloud[2] = (uint8_t)w2;
        cloud[3] = (uint8_t)(w2 >> 8);
        receipt.cloud_strength_after = (uint8_t)(w2 >> 8);
        receipt.cloud_type_after = 0x65u;
    }
    if (receipt.cloud_type_before == 0x65u ||
        receipt.cloud_type_before < 8u ||
        receipt.cloud_type_before == 0x28u) {
        if (!dm2_v1_record_pool_cut_from_tile(
                &candidate->record_pools, &candidate->dungeon,
                map, x, y, cloud_record)) {
            receipt.blocked_record = 1;
            goto cloud_rollback;
        }
        cloud[0] = 0xffu;
        cloud[1] = 0xffu;
        receipt.cloud_strength_after = (uint8_t)(w2 >> 8);
        receipt.cloud_type_after = 0x65u;
        receipt.deallocated = 1;
        receipt.consumed = 1;
        receipt.valid = 1;
        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
            candidate->source_transaction_hash ^ candidate->candidate_hash,
            (uint32_t)(uint16_t)cloud_record ^ 0x650000u);
        dm2_v1_record_pool_set_free(&pool_backup);
        dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
        free(index_backup);
        free(timer_backup);
        free(caii_backup);
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    w2 = (uint16_t)((w2 & 0xff80u) | 0x65u);
    cloud[2] = (uint8_t)w2;
    cloud[3] = (uint8_t)(w2 >> 8);
    receipt.cloud_strength_after = (uint8_t)(w2 >> 8);
    receipt.cloud_type_after = 0x65u;
    memset(&sound_env, 0, sizeof(sound_env));
    sound_env.current_map = (int16_t)map;
    sound_env.gate_map_a = candidate->sound_owner.spatial_audible_map;
    sound_env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
    sound_env.facing = candidate->source_party_direction;
    sound_env.party_x = candidate->source_party_x;
    sound_env.party_y = candidate->source_party_y;
    sound_env.gametick = (int32_t)game_tick;
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    if (candidate->sound_owner.spatial_context_valid &&
        dm2_v1_game_load_sound_owner_queue_noise_gen2(
            &candidate->sound_owner, 0x0d, 0x64, (int8_t)0x81,
            (int8_t)0xfe, (int16_t)x, (int16_t)y, 1, 0x6c, 0xc8,
            &sound_env, &sound_receipt))
        receipt.sound_queued = 1;
    next_timer = timer_entry;
    next_timer.l_00 += 1;
    if (dm2_v1_timer_queue(&candidate->timer_queue, &next_timer) < 0)
        goto cloud_rollback;
    receipt.requeued = 1;
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)cloud_record ^
            ((uint32_t)receipt.cloud_type_before << 8) ^
            ((uint32_t)receipt.cloud_type_after << 16));
    dm2_v1_record_pool_set_free(&pool_backup);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    free(index_backup);
    free(timer_backup);
    free(caii_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

cloud_rollback:
    if (caii_backup && candidate->caii_slots.slots)
        memcpy(candidate->caii_slots.slots, caii_backup, caii_bytes);
    free(caii_backup);
    if (raw_mutated && raw_before >= 0)
        (void)dm2_v1_dungeon_set_tile_raw(
            &candidate->dungeon, map, x, y, (uint16_t)raw_before);
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
    }
    if (pool_backup.valid) {
        dm2_v1_record_pool_set_free(&candidate->record_pools);
        candidate->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (sound_backup.valid) {
        dm2_v1_game_load_runtime_candidate_sound_free(&candidate->sound_owner);
        candidate->sound_owner = sound_backup;
        memset(&sound_backup, 0, sizeof(sound_backup));
    }
    if (party_backed_up)
        candidate->party = party_backup;
    if (rng_backed_up)
        candidate->caii_rng = rng_backup;
    if (candidate->current_map != old_map)
        (void)dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, old_map, 0);
    dm2_v1_record_pool_set_free(&pool_backup);
    dm2_v1_game_load_runtime_candidate_sound_free(&sound_backup);
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_missile_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateMissileReceipt *out_receipt)
{
    DM2_V1_GameLoadCandidateMissileReceipt receipt;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t *raw_backup = NULL;
    DM2_V1_TimerEntry timer_entry;
    int16_t timer_slot;
    int16_t missile;
    int16_t cursor;
    int16_t source_prev = DM2_V1_RECORD_HANDLE_NULL;
    int16_t source_next = DM2_V1_RECORD_HANDLE_END;
    int16_t destination_prev = DM2_V1_RECORD_HANDLE_NULL;
    int old_map;
    int map;
    int x;
    int y;
    int next_x;
    int next_y;
    int raw_size = 0;
    int raw_type = -1;
    int budget = 0;
    int steps;
    int found = 0;
    int16_t continuation_slot;
    uint16_t packed_b;
    uint16_t energy_step;
    uint8_t *record;
    uint8_t *raw_missile;
    uint8_t *raw_source_prev = NULL;
    uint8_t *raw_destination_prev = NULL;
    uint8_t *raw_cursor;
    int raw_chain_steps;
    int raw_found = 0;

    memset(&receipt, 0, sizeof(receipt));
    memset(&pool_backup, 0, sizeof(pool_backup));
    receipt.timer_type = 0x1eu;
    receipt.game_tick = game_tick;
    receipt.missile_record = DM2_V1_RECORD_HANDLE_NULL;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        goto missile_fail;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        goto missile_fail;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup || candidate->dungeon.raw_size <= 0 ||
        !candidate->dungeon.raw_data ||
        !dm2_v1_record_pool_set_clone(&pool_backup, &candidate->record_pools) ||
        !(raw_backup = (uint8_t *)malloc((size_t)candidate->dungeon.raw_size)))
        goto missile_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    memcpy(raw_backup, candidate->dungeon.raw_data,
           (size_t)candidate->dungeon.raw_size);
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto missile_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity ||
        candidate->timer_entries[timer_slot].ttype != 0x1eu) {
        receipt.blocked_unsupported_type = 1;
        goto missile_rollback;
    }
    timer_entry = candidate->timer_entries[timer_slot];
    missile = (int16_t)((uint16_t)(uint8_t)timer_entry.xA |
                        ((uint16_t)(uint8_t)timer_entry.yA << 8));
    packed_b = (uint16_t)timer_entry.wvalueB;
    map = dm2_v1_timer_get_map(&timer_entry);
    x = (int)(packed_b & 0x1fu);
    y = (int)((packed_b >> 5) & 0x1fu);
    receipt.map = map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    receipt.missile_record = missile;
    receipt.energy_step = (uint16_t)(packed_b >> 12);
    receipt.direction = (uint8_t)((packed_b >> 10) & 3u);
    if (map < 0 || map >= candidate->dungeon.level_count || x < 0 || y < 0 ||
        x >= candidate->dungeon.level_widths[map] ||
        y >= candidate->dungeon.level_heights[map] ||
        dm2_v1_record_handle_pool(missile) != 14 ||
        !(record = dm2_v1_record_pool_address_mut(
            &candidate->record_pools, missile)) ||
        candidate->record_pools.pools[14].record_size < 8 ||
        ((uint16_t)record[6] | ((uint16_t)record[7] << 8)) !=
            (uint16_t)timer_slot) {
        receipt.blocked_record = 1;
        goto missile_rollback;
    }
    raw_missile = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        &candidate->dungeon, (uint16_t)missile, &raw_type, NULL, &raw_size);
    if (!raw_missile || raw_size < 8 ||
        ((uint16_t)raw_missile[6] | ((uint16_t)raw_missile[7] << 8)) !=
            (uint16_t)timer_slot) {
        receipt.blocked_timer_owner = 1;
        goto missile_rollback;
    }
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &candidate->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            budget > INT_MAX - pool->record_count - pool->extension_count)
            goto missile_chain_fail;
        budget += pool->record_count + pool->extension_count;
    }
    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        &candidate->dungeon, map, x, y);
    for (steps = 0; cursor != DM2_V1_RECORD_HANDLE_END; ++steps) {
        int16_t next;
        if (cursor == DM2_V1_RECORD_HANDLE_NULL || steps >= budget ||
            !dm2_v1_record_pool_address(&candidate->record_pools, cursor) ||
            !dm2_v1_record_pool_next_link(
                &candidate->record_pools, cursor, &next))
            goto missile_chain_fail;
        if (cursor == missile) found = 1;
        cursor = next;
    }
    if (!found) goto missile_chain_fail;
    if (dm2_v1_get_creature_at(&candidate->record_pools,
                               &candidate->dungeon, map, x, y) !=
        DM2_V1_RECORD_HANDLE_NULL) {
        receipt.blocked_creature_collision = 1;
        goto missile_rollback;
    }
    /* The raw mirror must be admitted before either terminal dealloc or
     * relocation; the pool helper owns only the pool-side link words. */
    raw_chain_steps = 0;
    {
        int16_t raw_link = (int16_t)dm2_v1_dungeon_get_first_thing(
            &candidate->dungeon, map, x, y);
        while (raw_link != DM2_V1_RECORD_HANDLE_END &&
               raw_link != DM2_V1_RECORD_HANDLE_NULL &&
               raw_chain_steps++ < budget) {
            raw_cursor = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    &candidate->dungeon, (uint16_t)raw_link,
                    NULL, NULL, NULL);
            if (!raw_cursor) goto missile_chain_fail;
            if (raw_link == missile) {
                source_next = (int16_t)((uint16_t)raw_cursor[0] |
                                        ((uint16_t)raw_cursor[1] << 8));
                raw_found = 1;
                break;
            }
            source_prev = raw_link;
            raw_link = (int16_t)((uint16_t)raw_cursor[0] |
                                 ((uint16_t)raw_cursor[1] << 8));
        }
    }
    if (!raw_found) goto missile_chain_fail;
    if (source_prev != DM2_V1_RECORD_HANDLE_NULL) {
        raw_source_prev = (uint8_t *)(uintptr_t)
            dm2_v1_dungeon_get_thing_record(
                &candidate->dungeon, (uint16_t)source_prev,
                NULL, NULL, NULL);
        if (!raw_source_prev) goto missile_chain_fail;
    }
    {
        int16_t raw_tail = source_next;
        while (raw_tail != DM2_V1_RECORD_HANDLE_END &&
               raw_tail != DM2_V1_RECORD_HANDLE_NULL &&
               raw_chain_steps++ < budget) {
            raw_cursor = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    &candidate->dungeon, (uint16_t)raw_tail,
                    NULL, NULL, NULL);
            if (!raw_cursor) goto missile_chain_fail;
            raw_tail = (int16_t)((uint16_t)raw_cursor[0] |
                                 ((uint16_t)raw_cursor[1] << 8));
        }
        if (raw_tail != DM2_V1_RECORD_HANDLE_END) goto missile_chain_fail;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    receipt.energy_before = record[4];
    energy_step = receipt.energy_step;
    if ((uint16_t)record[4] <= energy_step) {
        if (!dm2_v1_record_pool_cut_from_tile(
                &candidate->record_pools, &candidate->dungeon,
                map, x, y, missile)) goto missile_rollback;
        if (source_prev == DM2_V1_RECORD_HANDLE_NULL) {
            if (dm2_v1_dungeon_set_first_thing(
                    &candidate->dungeon, map, x, y,
                    (uint16_t)source_next) != 0) goto missile_rollback;
        } else {
            raw_source_prev[0] = (uint8_t)source_next;
            raw_source_prev[1] = (uint8_t)((uint16_t)source_next >> 8);
        }
        record = dm2_v1_record_pool_address_mut(&candidate->record_pools, missile);
        if (!record) goto missile_rollback;
        record[0] = 0xffu; record[1] = 0xffu;
        raw_missile[0] = 0xffu; raw_missile[1] = 0xffu;
        receipt.energy_after = 0u;
        receipt.deallocated = 1;
        receipt.consumed = 1;
        receipt.valid = 1;
        receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
            candidate->source_transaction_hash ^ candidate->candidate_hash,
            (uint32_t)(uint16_t)missile ^ ((uint32_t)map << 16) ^
            ((uint32_t)x << 8) ^ (uint32_t)y);
        goto missile_success;
    }
    next_x = x + ((int[]){0, 1, 0, -1})[receipt.direction];
    next_y = y + ((int[]){-1, 0, 1, 0})[receipt.direction];
    if (next_x < 0 || next_y < 0 ||
        next_x >= candidate->dungeon.level_widths[map] ||
        next_y >= candidate->dungeon.level_heights[map] ||
        !dm2_v1_dungeon_c_map_is_tile_passage(
            &candidate->dungeon, map, next_x, next_y)) {
        receipt.blocked_teleporter = 1;
        goto missile_rollback;
    }
    {
        const int raw_tile = dm2_v1_dungeon_get_tile_raw(
            &candidate->dungeon, map, next_x, next_y);
        const int tile_type = candidate->dungeon.square_bytes == 1
            ? ((unsigned int)raw_tile >> 5) & 7
            : raw_tile & 0x1f;
        if (raw_tile < 0 || tile_type < 1 || tile_type > 3) {
            receipt.blocked_teleporter = 1;
            goto missile_rollback;
        }
    }
    if (dm2_v1_get_creature_at(&candidate->record_pools,
                               &candidate->dungeon, map, next_x, next_y) !=
        DM2_V1_RECORD_HANDLE_NULL) {
        receipt.blocked_creature_collision = 1;
        goto missile_rollback;
    }
    /* Read-only raw destination-chain admission. */
    raw_cursor = NULL;
    raw_chain_steps = 0;
    {
        int16_t raw_link = (int16_t)dm2_v1_dungeon_get_first_thing(
            &candidate->dungeon, map, next_x, next_y);
        while (raw_link != DM2_V1_RECORD_HANDLE_END &&
               raw_link != DM2_V1_RECORD_HANDLE_NULL &&
               raw_chain_steps++ < budget) {
            raw_cursor = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    &candidate->dungeon, (uint16_t)raw_link,
                    NULL, NULL, NULL);
            if (!raw_cursor) goto missile_chain_fail;
            destination_prev = raw_link;
            raw_link = (int16_t)((uint16_t)raw_cursor[0] |
                                 ((uint16_t)raw_cursor[1] << 8));
        }
        if (raw_link != DM2_V1_RECORD_HANDLE_END) goto missile_chain_fail;
    }
    if (!dm2_v1_record_pool_cut_from_tile(
            &candidate->record_pools, &candidate->dungeon, map, x, y, missile))
        goto missile_rollback;
    if (source_prev == DM2_V1_RECORD_HANDLE_NULL) {
        if (dm2_v1_dungeon_set_first_thing(
                &candidate->dungeon, map, x, y, (uint16_t)source_next) != 0)
            goto missile_rollback;
    } else {
        raw_source_prev = (uint8_t *)(uintptr_t)
            dm2_v1_dungeon_get_thing_record(&candidate->dungeon,
                                             (uint16_t)source_prev,
                                             NULL, NULL, NULL);
        if (!raw_source_prev) goto missile_rollback;
        raw_source_prev[0] = (uint8_t)source_next;
        raw_source_prev[1] = (uint8_t)((uint16_t)source_next >> 8);
    }
    if (!dm2_v1_record_pool_append_to_list(
            &candidate->record_pools, &cursor, missile)) goto missile_rollback;
    if (destination_prev == DM2_V1_RECORD_HANDLE_NULL) {
        if (dm2_v1_dungeon_set_first_thing(
                &candidate->dungeon, map, next_x, next_y,
                (uint16_t)missile) != 0) goto missile_rollback;
    } else {
        raw_destination_prev = (uint8_t *)(uintptr_t)
            dm2_v1_dungeon_get_thing_record(&candidate->dungeon,
                                             (uint16_t)destination_prev,
                                             NULL, NULL, NULL);
        if (!raw_destination_prev) goto missile_rollback;
        raw_destination_prev[0] = (uint8_t)missile;
        raw_destination_prev[1] = (uint8_t)((uint16_t)missile >> 8);
    }
    record = dm2_v1_record_pool_address_mut(&candidate->record_pools, missile);
    if (!record) goto missile_rollback;
    record[4] = (uint8_t)(record[4] - (uint8_t)energy_step);
    record[5] = record[5] >= (uint8_t)energy_step
        ? (uint8_t)(record[5] - (uint8_t)energy_step) : 0u;
    raw_missile[4] = record[4]; raw_missile[5] = record[5];
    dm2_v1_timer_set_mticks(&timer_entry, (int16_t)map, game_tick + 1u);
    timer_entry.xA = (int8_t)(uint8_t)missile;
    timer_entry.yA = (int8_t)((uint16_t)missile >> 8);
    timer_entry.wvalueB = (int16_t)((uint16_t)next_x |
                                    ((uint16_t)next_y << 5) |
                                    ((uint16_t)receipt.direction << 10) |
                                    ((uint16_t)energy_step << 12));
    continuation_slot = dm2_v1_timer_queue(&candidate->timer_queue,
                                             &timer_entry);
    if (continuation_slot < 0) goto missile_rollback;
    record[6] = (uint8_t)continuation_slot;
    record[7] = (uint8_t)((uint16_t)continuation_slot >> 8);
    raw_missile[6] = record[6]; raw_missile[7] = record[7];
    receipt.next_x = (int16_t)next_x;
    receipt.next_y = (int16_t)next_y;
    receipt.energy_after = record[4];
    receipt.moved = 1;
    receipt.requeued = 1;
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)missile ^ ((uint32_t)map << 16) ^
        ((uint32_t)next_x << 8) ^ (uint32_t)next_y);
    goto missile_success;

missile_chain_fail:
    receipt.blocked_incomplete_chain = 1;
missile_rollback:
    if (candidate && raw_backup && candidate->dungeon.raw_data)
        memcpy(candidate->dungeon.raw_data, raw_backup,
               (size_t)candidate->dungeon.raw_size);
    if (candidate && timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
    }
    if (candidate && pool_backup.valid) {
        dm2_v1_record_pool_set_free(&candidate->record_pools);
        candidate->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (candidate && candidate->current_map != old_map)
        (void)dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, old_map, 0);
    receipt.valid = 0;
    goto missile_fail_cleanup;

missile_success:
    dm2_v1_record_pool_set_free(&pool_backup);
missile_fail_cleanup:
    free(raw_backup);
    free(index_backup);
    free(timer_backup);
missile_fail:
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

int dm2_v1_game_load_runtime_session_candidate_process_next_due_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateTimerProcessReceipt *out_receipt)
{
    DM2_V1_GameLoadCandidateTimerProcessReceipt receipt;
    DM2_V1_TimerQueue queue;
    int16_t timer_slot;
    uint8_t timer_type;
    int result;

    memset(&receipt, 0, sizeof(receipt));
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    queue = candidate->timer_queue;
    queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&queue)) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    timer_slot = queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    timer_type = candidate->timer_entries[timer_slot].ttype;
    receipt.timer_type = timer_type;
    if (timer_type == 0x21u || timer_type == 0x22u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_think_timer(
            candidate, game_tick, &receipt.think);
        receipt.valid = result && receipt.think.valid;
        receipt.consumed = result && receipt.think.consumed;
    } else if (timer_type == 0x0eu) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_process_0e_timer(
            candidate, game_tick, &receipt.process_0e);
        receipt.valid = result && receipt.process_0e.valid;
        receipt.consumed = result && receipt.process_0e.consumed;
    } else if (timer_type == 0x15u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_process_sound_timer(
            candidate, game_tick, &receipt.process_sound);
        receipt.valid = result && receipt.process_sound.valid;
        receipt.consumed = result && receipt.process_sound.consumed;
    } else if (timer_type == 0x5eu) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_alloc_creature_timer(
            candidate, game_tick, &receipt.alloc_creature);
        receipt.valid = result && receipt.alloc_creature.valid;
        receipt.consumed = result && receipt.alloc_creature.consumed;
    } else if (timer_type == 0x46u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_light_timer(
            candidate, game_tick, &receipt.light);
        receipt.valid = result && receipt.light.valid;
        receipt.consumed = result && receipt.light.consumed;
    } else if (timer_type == 0x5au) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_ornate_noise_timer(
            candidate, game_tick, &receipt.ornate_noise);
        receipt.valid = result && receipt.ornate_noise.valid;
        receipt.consumed = result && receipt.ornate_noise.consumed;
    } else if (timer_type == 0x0du) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_resurrection_timer(
            candidate, game_tick, &receipt.resurrection);
        receipt.valid = result && receipt.resurrection.valid;
        receipt.consumed = result && receipt.resurrection.consumed;
    } else if (timer_type == 0x0cu) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_process_0c_timer(
            candidate, game_tick, &receipt.process_0c);
        receipt.valid = result && receipt.process_0c.valid;
        receipt.consumed = result && receipt.process_0c.consumed;
    } else if (timer_type == 0x02u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_destroy_door_timer(
            candidate, game_tick, &receipt.destroy_door);
        receipt.valid = result && receipt.destroy_door.valid;
        receipt.consumed = result && receipt.destroy_door.consumed;
    } else if (timer_type == 0x47u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_hero_ench_flag_timer(
            candidate, game_tick, &receipt.hero_ench_flag);
        receipt.valid = result && receipt.hero_ench_flag.valid;
        receipt.consumed = result && receipt.hero_ench_flag.consumed;
    } else if (timer_type == 0x48u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_ench_power_timer(
            candidate, game_tick, &receipt.ench_power);
        receipt.valid = result && receipt.ench_power.valid;
        receipt.consumed = result && receipt.ench_power.consumed;
    } else if (timer_type == 0x4bu) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_poison_timer(
            candidate, game_tick, &receipt.poison);
        receipt.valid = result && receipt.poison.valid;
        receipt.consumed = result && receipt.poison.consumed;
    } else if (timer_type == 0x55u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_ornate_animator_timer(
            candidate, game_tick, &receipt.ornate_animator);
        receipt.valid = result && receipt.ornate_animator.valid;
        receipt.consumed = result && receipt.ornate_animator.consumed;
    } else if (timer_type == 0x56u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_tick_generator_timer(
            candidate, game_tick, &receipt.tick_generator);
        receipt.valid = result && receipt.tick_generator.valid;
        receipt.consumed = result && receipt.tick_generator.valid;
    } else if (timer_type == 0x04u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_actuate_timer(
            candidate, game_tick, &receipt.actuate);
        receipt.valid = result && receipt.actuate.valid;
        receipt.consumed = result && receipt.actuate.consumed;
    } else if (timer_type == 0x1eu) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_missile_timer(
            candidate, game_tick, &receipt.missile);
        receipt.valid = result && receipt.missile.valid;
        receipt.consumed = result && receipt.missile.consumed;
    } else if (timer_type == 0x01u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_door_step_timer(
            candidate, game_tick, &receipt.door_step);
        receipt.valid = result && receipt.door_step.valid;
        receipt.consumed = result && receipt.door_step.consumed;
    } else if (timer_type == 0x58u || timer_type == 0x59u ||
               timer_type == 0x5bu || timer_type == 0x5cu) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_record_flag_timer(
            candidate, game_tick, &receipt.record_flag);
        receipt.valid = result && receipt.record_flag.valid;
        receipt.consumed = result && receipt.record_flag.consumed;
    } else if (timer_type == 0x19u) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_cloud_timer(
            candidate, game_tick, &receipt.cloud);
        receipt.valid = result && receipt.cloud.valid;
        receipt.consumed = result && receipt.cloud.consumed;
    } else if (timer_type == 0x3cu || timer_type == 0x3du) {
        result = dm2_v1_game_load_runtime_session_candidate_proceed_moverec_timer(
            candidate, game_tick, &receipt.moverec);
        receipt.valid = result && receipt.moverec.valid;
        receipt.consumed = result && receipt.moverec.consumed;
    } else {
        receipt.blocked_unsupported_type = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

int dm2_v1_game_load_runtime_session_candidate_proceed_record_flag_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadRecordFlagTimerReceipt *out_receipt)
{
    DM2_V1_GameLoadRecordFlagTimerReceipt receipt;
    DM2_V1_TimerEntry timer_entry;
    DM2_V1_TimerQueue queue_backup;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    uint8_t *record;
    int16_t timer_slot;
    int16_t record_handle;
    int old_map;
    int map;
    uint8_t before;

    memset(&receipt, 0, sizeof(receipt));
    receipt.record = DM2_V1_RECORD_HANDLE_NULL;
    receipt.game_tick = game_tick;
    if (!candidate || !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        receipt.blocked_invalid_candidate = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (!candidate->timer_entries || !candidate->timer_indices ||
        candidate->timer_capacity == 0u || candidate->timer_queue.num_timers <= 0) {
        receipt.blocked_no_due_timer = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    old_map = candidate->current_map;
    queue_backup = candidate->timer_queue;
    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)candidate->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) goto record_flag_rollback;
    memcpy(timer_backup, candidate->timer_entries,
           (size_t)candidate->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, candidate->timer_indices,
           (size_t)candidate->timer_capacity * sizeof(*index_backup));
    candidate->timer_queue.gametick = (int32_t)game_tick;
    if (!dm2_v1_timer_is_due(&candidate->timer_queue)) {
        receipt.blocked_no_due_timer = 1;
        goto record_flag_rollback;
    }
    timer_slot = candidate->timer_queue.indices[0];
    if (timer_slot < 0 || timer_slot >= candidate->timer_capacity) {
        receipt.blocked_no_due_timer = 1;
        goto record_flag_rollback;
    }
    receipt.timer_type = candidate->timer_entries[timer_slot].ttype;
    if (receipt.timer_type != 0x58u && receipt.timer_type != 0x59u &&
        receipt.timer_type != 0x5bu && receipt.timer_type != 0x5cu) {
        receipt.blocked_unsupported_type = 1;
        goto record_flag_rollback;
    }
    /* Source c_timer::getA is the packed xA/yA word (payload A). */
    if (receipt.timer_type == 0x59u) {
        record_handle = candidate->timer_entries[timer_slot].wvalueB;
    } else {
        record_handle = (int16_t)((uint8_t)candidate->timer_entries[timer_slot].xA |
                                  ((uint16_t)(uint8_t)candidate->timer_entries[timer_slot].yA << 8));
    }
    record = dm2_v1_record_pool_address_mut(&candidate->record_pools,
                                             (uint16_t)record_handle);
    if (!record) {
        receipt.blocked_record = 1;
        goto record_flag_rollback;
    }
    dm2_v1_timer_get_and_delete_next(&candidate->timer_queue, &timer_entry);
    map = dm2_v1_timer_get_map(&timer_entry);
    receipt.record = record_handle;
    receipt.map = map;
    if (map < 0 || map >= candidate->dungeon.level_count ||
        !dm2_v1_game_load_runtime_candidate_change_current_map(
            candidate, map, 0)) {
        receipt.blocked_map = 1;
        goto record_flag_rollback;
    }
    receipt.map_switch = old_map != map;
    if (receipt.timer_type == 0x58u) {
        receipt.byte_offset = 3u;
        before = record[3];
        record[3] &= (uint8_t)~0x08u;
    } else if (receipt.timer_type == 0x59u) {
        receipt.byte_offset = 4u;
        before = record[4];
        receipt.redraw_unbound = map == candidate->source_party_map;
        if (record[4] & 0x04u) {
            receipt.guard_skipped = 1;
        } else {
            record[4] &= (uint8_t)~0x01u;
        }
    } else if (receipt.timer_type == 0x5bu) {
        receipt.byte_offset = 4u;
        before = record[4];
        record[4] &= (uint8_t)~0x01u;
    } else {
        receipt.byte_offset = 2u;
        before = record[2];
        record[2] |= 0x01u;
    }
    receipt.value_before = before;
    receipt.value_after = record[receipt.byte_offset];
    receipt.consumed = 1;
    receipt.valid = 1;
    receipt.transaction_hash = dm2_v1_game_load_owner_hash_step(
        candidate->source_transaction_hash ^ candidate->candidate_hash,
        (uint32_t)(uint16_t)record_handle ^
        ((uint32_t)receipt.timer_type << 24) ^ receipt.value_after);
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 1;

record_flag_rollback:
    if (timer_backup && index_backup) {
        memcpy(candidate->timer_entries, timer_backup,
               (size_t)candidate->timer_capacity * sizeof(*timer_backup));
        memcpy(candidate->timer_indices, index_backup,
               (size_t)candidate->timer_capacity * sizeof(*index_backup));
        candidate->timer_queue = queue_backup;
        if (candidate->current_map != old_map) {
            (void)dm2_v1_game_load_runtime_candidate_change_current_map(
                candidate, old_map, 0);
        }
    }
    free(index_backup);
    free(timer_backup);
    if (out_receipt) *out_receipt = receipt;
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_change_current_map_to(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int new_map)
{
    if (!candidate || !dm2_v1_game_load_runtime_candidate_is_initialized(candidate) ||
        !candidate->valid) return 0;
    return dm2_v1_game_load_runtime_candidate_change_current_map(candidate,
                                                                   new_map, 0);
}

/* DM2_LOAD_LOCALLEVEL_DYN allocates its 400-entry c_swww table before the
 * first tile/record walk.  Keep that prefix in the private candidate exactly
 * as source data, but stop before the walk: its record/actuator side effects
 * and the following DYN4/weather/light consumers require the same complete
 * GAME_LOAD transaction.  Source: SKULLWIN/c_loadlevel.cpp (203-327). */
static int dm2_v1_game_load_local_dyn_mark(DM2_V1_DynLoadState *queue,
                                           int32_t resource_id)
{
    int16_t before;
    if (!queue) return 0;
    before = queue->count;
    if (dm2_v1_mark_dyn_load(queue, resource_id).entry_index != before ||
        queue->count != before + 1) return 0;
    return 1;
}

static int dm2_v1_game_load_local_dyn_set_last_flags(
    DM2_V1_DynLoadState *queue, int16_t flags)
{
    if (!queue || queue->count <= 0 ||
        queue->count > DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES) return 0;
    queue->entries[queue->count - 1].flags = flags;
    return 1;
}

static int dm2_v1_game_load_local_dyn_mark_flag(
    DM2_V1_DynLoadState *queue, int32_t resource_id, int32_t flag)
{
    int16_t before;
    if (!queue) return 0;
    before = queue->count;
    dm2_v1_mark_dyn_load_with_flag(queue, resource_id, flag);
    return queue->count == before + 2;
}

static uint32_t dm2_v1_game_load_local_dyn_hash(const DM2_V1_DynLoadState *queue)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!queue || queue->count < 0 ||
        queue->count > DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES) return 0u;
    for (i = 0; i < queue->count; ++i) {
        const DM2_V1_DynLoadEntry *entry = &queue->entries[i];
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)entry->flags);
        hash = dm2_v1_game_load_owner_hash_step(hash, entry->cat);
        hash = dm2_v1_game_load_owner_hash_step(hash, entry->type);
        hash = dm2_v1_game_load_owner_hash_step(hash, entry->sub1);
        hash = dm2_v1_game_load_owner_hash_step(hash, entry->sub2);
    }
    return hash;
}

static int dm2_v1_game_load_candidate_tile_raw(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int map, int x, int y)
{
    if (!candidate) return -1;
    if (candidate->sksave_map_owner_valid) {
        const DM2_V1_SksaveMapOwner *map_owner =
            &candidate->sksave_map_owner;
        const DM2_V1_OriginalRawDungeonReceipt *dungeon = map_owner->dungeon;
        size_t offset;
        if (!dungeon || map < 0 || map >= dungeon->map_count ||
            x < 0 || y < 0 || x >= dungeon->map_widths[map] ||
            y >= dungeon->map_heights[map] ||
            map_owner->map_tile_offsets[map] >= map_owner->map_tiles_size)
        {
            return -1;
        }
        offset = (size_t)map_owner->map_tile_offsets[map] +
            (size_t)x * (size_t)dungeon->map_heights[map] + (size_t)y;
        if (offset >= map_owner->map_tiles_size) return -1;
        return map_owner->map_tiles[offset];
    }
    return dm2_v1_dungeon_get_tile_raw(&candidate->dungeon, map, x, y);
}

static int dm2_v1_game_load_candidate_first_thing(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int map, int x, int y)
{
    if (!candidate) return -1;
    if (candidate->sksave_map_owner_valid) {
        uint16_t link;
        if (!dm2_v1_sksave_map_owner_tile_record_link(
                &candidate->sksave_map_owner, map, x, y, &link)) return -1;
        return (int)(int16_t)link;
    }
    return dm2_v1_dungeon_get_first_thing(&candidate->dungeon, map, x, y);
}

/* c_loadlevel.cpp::DM2_LOAD_LOCALLEVEL_DYN (518-626) first walks the
 * current map x-major/y-minor and follows every marked-square chain. Keep a
 * lossless trace over the candidate's *mutable* RecordPoolSet so the later
 * text/actuator/hero selector branches cannot accidentally be rebuilt from
 * pristine DUNGEON.DAT. This is only the common traversal predecessor: it
 * deliberately does not apply the record-specific branches or call DYN4. */
static int dm2_v1_game_load_local_dyn_map_scan_init(
    DM2_V1_GameLoadLocalDynMapScan *out,
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate)
{
    DM2_V1_FileHeaderRuntimeMapReceipt map_receipt;
    DM2_V1_GameLoadLocalDynMapScan scan;
    uint32_t hash = 0x4c44534eu; /* "LDSN" */
    int x;
    int y;

    if (!out || !candidate || !candidate->dungeon.raw_data ||
        !candidate->record_pools.valid || candidate->current_map < 0 ||
        candidate->current_map >= candidate->dungeon.level_count) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&map_receipt, 0, sizeof(map_receipt));
    if (!dm2_v1_dungeon_validate_file_header_runtime_map(
        &candidate->dungeon, candidate->current_map, &map_receipt) ||
        !map_receipt.committed || map_receipt.width <= 0 ||
        map_receipt.height <= 0 || map_receipt.record_count <= 0 ||
        map_receipt.record_count > UINT16_MAX) {
        return 0;
    }
    memset(&scan, 0, sizeof(scan));
    scan.map = (int16_t)candidate->current_map;
    scan.width = (uint16_t)map_receipt.width;
    scan.height = (uint16_t)map_receipt.height;
    scan.record_capacity = 0u;
    for (int pool_index = 0; pool_index < DM2_V1_RECORD_POOL_COUNT;
         ++pool_index) {
        if (candidate->record_pools.pools[pool_index].record_count >
            UINT16_MAX - scan.record_capacity) {
            return 0;
        }
        scan.record_capacity = (uint16_t)(scan.record_capacity +
            candidate->record_pools.pools[pool_index].record_count);
    }
    scan.records = calloc(scan.record_capacity, sizeof(*scan.records));
    if (!scan.records) goto fail;
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)scan.map);
    hash = dm2_v1_game_load_owner_hash_step(hash, scan.width);
    hash = dm2_v1_game_load_owner_hash_step(hash, scan.height);
    for (x = 0; x < map_receipt.width; ++x) {
        for (y = 0; y < map_receipt.height; ++y) {
            int raw = dm2_v1_game_load_candidate_tile_raw(
                candidate, candidate->current_map, x, y);
            int16_t link;

            if (raw < 0) {
                goto fail;
            }
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)x);
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)y);
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)raw);
            if ((raw & 0x10) == 0) continue;
            ++scan.marked_tile_count;
            link = (int16_t)dm2_v1_game_load_candidate_first_thing(
                candidate, candidate->current_map, x, y);
            if (link == DM2_V1_RECORD_HANDLE_NULL) {
                goto fail;
            }
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
            if (link == DM2_V1_RECORD_HANDLE_END) continue;
            ++scan.root_count;
            /* A File_header record may be reached from more than one marked
             * tile. Only a repeat within the same chain is a cycle. */
            {
                uint16_t chain_start = scan.record_count;
            while (link != DM2_V1_RECORD_HANDLE_END) {
                DM2_V1_GameLoadLocalDynRecordVisit *visit;
                const DM2_V1_RecordPool *pool;
                const uint8_t *record = NULL;
                int16_t next;
                int type = -1;
                int byte;
                uint32_t record_hash = 0x52454344u; /* "RECD" */
                int seen = 0;

                for (int prior = (int)chain_start;
                     prior < scan.record_count; ++prior) {
                    if (scan.records[prior].object_id == (uint16_t)link) {
                        seen = 1;
                        break;
                    }
                }
                if (seen) break;

                if (link == DM2_V1_RECORD_HANDLE_NULL ||
                    scan.record_count >= scan.record_capacity ||
                    (type = dm2_v1_record_handle_pool(link)) < 0 ||
                    type >= DM2_V1_RECORD_POOL_COUNT ||
                    !(record = dm2_v1_record_pool_address(
                        &candidate->record_pools, link))) {
                    goto fail;
                }
                pool = &candidate->record_pools.pools[type];
                /* GenericRecord::w0/w2 are both read below; a shorter pool
                 * cannot be a source-valid LOAD_LOCALLEVEL_DYN visit. */
                if (pool->record_size < 4 || pool->record_size > UINT8_MAX ||
                    (!candidate->dungeon.g1_w0_chains_disabled &&
                     !dm2_v1_record_pool_next_link(&candidate->record_pools,
                                                   link, &next))) {
                    goto fail;
                }
                if (candidate->dungeon.g1_w0_chains_disabled)
                    next = DM2_V1_RECORD_HANDLE_END;
                visit = &scan.records[scan.record_count++];
                visit->x = (int16_t)x;
                visit->y = (int16_t)y;
                visit->object_id = (uint16_t)link;
                visit->next_object_id = (uint16_t)next;
                visit->word2 = candidate->dungeon.source_words_big_endian
                    ? (uint16_t)(((uint16_t)record[2] << 8) | record[3])
                    : (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                visit->type = (uint8_t)type;
                visit->record_size = (uint8_t)pool->record_size;
                for (byte = 0; byte < pool->record_size; ++byte) {
                    record_hash = dm2_v1_game_load_owner_hash_step(
                        record_hash, record[byte]);
                }
                visit->record_hash = record_hash;
                ++scan.type_count[type];
                hash = dm2_v1_game_load_owner_hash_step(hash,
                                                         visit->object_id);
                hash = dm2_v1_game_load_owner_hash_step(hash,
                                                         visit->next_object_id);
                hash = dm2_v1_game_load_owner_hash_step(hash,
                                                         visit->word2);
                hash = dm2_v1_game_load_owner_hash_step(hash,
                                                         visit->type);
                hash = dm2_v1_game_load_owner_hash_step(hash, record_hash);
                link = next;
            }
            }
        }
    }
    if ((!candidate->sksave_map_owner_valid &&
         scan.record_count != (uint16_t)map_receipt.record_count) ||
        scan.record_count > (uint16_t)map_receipt.record_count ||
        scan.root_count == 0u || scan.marked_tile_count == 0u || hash == 0u) {
        goto fail;
    }
    scan.source_trace_hash = hash;
    scan.valid = 1;
    *out = scan;
    return 1;
fail:
    free(scan.records);
    return 0;
}

/* c_loadlevel.cpp::DM2_LOAD_LOCALLEVEL_DYN (538-626).  The original walk
 * writes exactly three cleared 0xfa-byte temporary arrays while following
 * marked-square record chains.  Preserve that byte-level result over the
 * candidate's mutable pools.  This deliberately stops before DB3/0x27's
 * cross-map tmpmap traversal: its destination byte list is not owned by the
 * File_header candidate, so manufacturing it would be a synthetic selector.
 */
static int dm2_v1_game_load_local_dyn_record_effects_init(
    DM2_V1_GameLoadLocalDynRecordEffects *out,
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate)
{
    DM2_V1_GameLoadLocalDynRecordEffects effects;
    const DM2_V1_GameLoadLocalDynMapScan *scan;
    uint32_t hash = 0x4c445246u; /* "LDRF" */
    uint16_t index;

    if (!out || !candidate || !candidate->local_dyn_prelude.valid ||
        !candidate->local_dyn_map_scan.valid ||
        candidate->local_dyn_map_scan.map != candidate->current_map ||
        candidate->local_dyn_prelude.source_map != candidate->current_map ||
        (candidate->local_dyn_prelude.queue.count <= 0 &&
         !candidate->sksave_map_owner_valid) ||
        candidate->local_dyn_prelude.queue.count >
            DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&effects, 0, sizeof(effects));
    scan = &candidate->local_dyn_map_scan;
    effects.map = (int16_t)candidate->current_map;
    effects.selector_queue = candidate->local_dyn_prelude.queue;
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)effects.map);
    hash = dm2_v1_game_load_owner_hash_step(hash, scan->record_count);

    for (index = 0u; index < scan->record_count; ++index) {
        const DM2_V1_GameLoadLocalDynRecordVisit *visit = &scan->records[index];
        int raw;
        uint8_t tile_class;

        if (visit->type >= DM2_V1_RECORD_POOL_COUNT ||
            visit->x < 0 || visit->x >= (int16_t)scan->width ||
            visit->y < 0 || visit->y >= (int16_t)scan->height) {
            return 0;
        }
        raw = dm2_v1_game_load_candidate_tile_raw(
            candidate, candidate->current_map, visit->x, visit->y);
        if (raw < 0 || (raw & 0x10) == 0) return 0;
        tile_class = (uint8_t)((unsigned int)raw >> 5);
        hash = dm2_v1_game_load_owner_hash_step(hash, visit->object_id);
        hash = dm2_v1_game_load_owner_hash_step(hash, visit->word2);
        hash = dm2_v1_game_load_owner_hash_step(hash, tile_class);

        if (visit->type == 2u && (visit->word2 & 0x0006u) == 0x0002u) {
            const uint8_t text_class = (uint8_t)((visit->word2 >> 11) & 0x1fu);
            const uint8_t text_index = (uint8_t)((visit->word2 >> 3) & 0xffu);
            int temp_mark = 0;
            int creature_mark = 0;

            ++effects.text_record_count;
            switch (text_class) {
            case 0: case 2:
            case 4: case 5: case 6: case 7: case 8:
            case 10: case 13:
            case 15: case 16: case 17:
                temp_mark = 1;
                break;
            case 19: case 21: case 22:
                creature_mark = 1;
                break;
            default:
                break;
            }
            if (temp_mark) {
                uint8_t *marks = tile_class != 0u ? effects.floor_text_marks :
                                                   effects.wall_text_marks;
                if (marks[text_index] == 0u) ++effects.text_temp_mark_count;
                marks[text_index] = 1u;
            }
            if (creature_mark) {
                if (effects.creature_marks[text_index] == 0u)
                    ++effects.creature_mark_count;
                effects.creature_marks[text_index] = 1u;
            }
        } else if (visit->type == 3u &&
                   candidate->local_dyn_prelude.source_v1e13fe[0] == 0u) {
            const uint8_t subtype = (uint8_t)(visit->word2 & 0x007fu);
            const uint8_t selector = (uint8_t)((visit->word2 >> 7) & 0xffu);

            if (subtype == 0x27u && tile_class == 5u) {
                /* Original ORs its other-map tmpmap byte list into RG51p.
                 * That list is deliberately not inferred here. */
                ++effects.cross_map_actuator_count;
            } else if (subtype == 0x2eu && tile_class == 0u) {
                if (effects.creature_marks[selector] == 0u)
                    ++effects.creature_mark_count;
                effects.creature_marks[selector] = 1u;
            } else if (subtype == 0x7eu) {
                const int32_t resource_id =
                    ((int32_t)selector << 16) | (int32_t)0x1600ffffu;
                if (!dm2_v1_game_load_local_dyn_mark(&effects.selector_queue,
                                                      resource_id)) {
                    return 0;
                }
                ++effects.mirror_selector_count;
            }
        }
    }
    /* The source's 0x27 branch is a real dependency.  Do not allow a later
     * DYN4 consumer to mistake the partial RG51p table for the final table. */
    if ((effects.cross_map_actuator_count != 0u &&
         !candidate->sksave_map_owner_valid) ||
        (effects.selector_queue.count <= 0 &&
         !candidate->sksave_map_owner_valid) ||
        effects.selector_queue.count > DM2_V1_LOADLEVEL_MAX_DYN_ENTRIES) {
        return 0;
    }
    for (index = 0u; index < DM2_V1_LOADLEVEL_CREATURE_TABLE_SIZE; ++index) {
        hash = dm2_v1_game_load_owner_hash_step(hash, effects.creature_marks[index]);
        hash = dm2_v1_game_load_owner_hash_step(hash, effects.wall_text_marks[index]);
        hash = dm2_v1_game_load_owner_hash_step(hash, effects.floor_text_marks[index]);
    }
    hash = dm2_v1_game_load_owner_hash_step(hash, effects.text_record_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, effects.text_temp_mark_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, effects.creature_mark_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, effects.mirror_selector_count);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                             (uint16_t)effects.selector_queue.count);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, dm2_v1_game_load_local_dyn_hash(&effects.selector_queue));
    if (hash == 0u) return 0;
    effects.source_effect_hash = hash;
    effects.valid = 1;
    *out = effects;
    return 1;
}

static int dm2_v1_game_load_local_dyn_prelude_init(
    DM2_V1_GameLoadLocalDynPrelude *out,
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate)
{
    DM2_V1_GameLoadLocalDynPrelude prelude;
    int16_t map_selector_index;

    if (!out || !candidate || candidate->current_map < 0 ||
        candidate->current_map >= 64 || candidate->party.heros_in_party <= 0 ||
        candidate->party.heros_in_party > DM2_MAX_HEROES) return 0;
    memset(&prelude, 0, sizeof(prelude));
    /* c_dballoc.cpp::c_dballoc (204-208): New Game starts with both entries
     * clear.  Resume is a separate SKSAVE transaction and must not use this
     * constructor. */
    prelude.source_v1e13fe[0] = 0u;
    prelude.source_v1e13fe[1] = 0u;
    prelude.source_map = (int16_t)candidate->current_map;
    prelude.source_music_type = dm2_v1_music_map[candidate->current_map];
    prelude.source_party_count = (uint8_t)candidate->party.heros_in_party;

    if (!dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x01ff02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x18ff02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x07ff02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d0002ffu) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue, 1) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d2f02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d7e02ffu) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue, 1) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d9f02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x10ff02ffu) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue, 1) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x15ff02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x030002ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x08fe02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x16fe02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x09fe02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0afe02ffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0fff08fbu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0fff07fcu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x01ffffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x01000400u) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue,
                                                    DM2_V1_LOADLEVEL_DYN_FLAG_HIRES) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x01000600u) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue,
                                                    DM2_V1_LOADLEVEL_DYN_FLAG_HIRES) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0100070au) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue,
                                                    DM2_V1_LOADLEVEL_DYN_FLAG_HIRES) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x1a80ffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x1a81ffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0300ffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0700ffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d00ffffu) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue, 1) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d2fffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d7effffu) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue, 1) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x0d9fffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x10ffffffu) ||
        !dm2_v1_game_load_local_dyn_set_last_flags(&prelude.queue, 1) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0x15ffffffu) ||
        !dm2_v1_game_load_local_dyn_mark(&prelude.queue, (int32_t)0xffff01f9u) ||
        !dm2_v1_game_load_local_dyn_mark_flag(&prelude.queue,
                                               (int32_t)0x0fff0510u, 0x39) ||
        prelude.queue.count < 2) return 0;
    prelude.queue.entries[prelude.queue.count - 2].flags &=
        (int16_t)~DM2_V1_LOADLEVEL_DYN_FLAG_HIRES;
    map_selector_index = prelude.queue.count;
    if (!dm2_v1_game_load_local_dyn_mark(&prelude.queue,
        ((int32_t)(candidate->current_map + 1) << 16) | (int32_t)0x030002ffu)) {
        return 0;
    }
    prelude.fixed_prefix_count = (uint16_t)prelude.queue.count;
    prelude.map_selector_index = (uint16_t)map_selector_index;
    prelude.record_traversal_pending = 1;
    prelude.dyn4_pending = 1;
    prelude.source_resource_hash = dm2_v1_game_load_local_dyn_hash(&prelude.queue);
    if (prelude.source_resource_hash == 0u) return 0;
    prelude.valid = 1;
    *out = prelude;
    return 1;
}

/* This is deliberately a clone, not RESET_CAII/FILL_CAII replay.  A future
 * source-complete owner must have established the exact private state before
 * this point. Replaying either against the source owner would rewrite DB4
 * byte@5 and consume its timer/RNG transaction. */
int dm2_v1_game_load_runtime_session_candidate_init(
    DM2_V1_GameLoadRuntimeSessionCandidate *out,
    const DM2_V1_GameLoadWorldOwner *source)
{
    DM2_V1_GameLoadRuntimeSessionCandidate candidate;
    uint32_t hash = 0x47534c44u; /* "GSLD" */
    const int out_was_initialized =
        dm2_v1_game_load_runtime_candidate_is_initialized(out);

    if (!out) return 0;
    if (!out_was_initialized) memset(out, 0, sizeof(*out));
    memset(&candidate, 0, sizeof(candidate));
    candidate.lifecycle_tag =
        DM2_V1_GAME_LOAD_RUNTIME_CANDIDATE_LIFECYCLE_TAG;
    if (!dm2_v1_game_load_world_owner_is_prepared(source) ||
        source->committed || !source->source_map_context_materialized ||
        !source->champion_selection_materialized ||
        !source->source_event_queue_materialized ||
        !source->source_startend_first_champion_released ||
        source->selected_party.heros_in_party <= 0 ||
        source->selected_party.heros_in_party > DM2_MAX_HEROES ||
        source->selected_party.curactevhero < 0 ||
        source->selected_party.curactevhero >= source->selected_party.heros_in_party ||
        !source->record_pools.valid || !source->dungeon.raw_data ||
        source->dungeon.raw_size <= 0 || !source->dungeon.record_graph_complete ||
        !source->timer_entries || !source->timer_indices || source->timer_capacity == 0u ||
        source->timer_queue.entries != source->timer_entries ||
        source->timer_queue.indices != source->timer_indices ||
        source->timer_queue.max_timers != (int16_t)source->timer_capacity ||
        !source->caii_static_animation.valid || !source->caii_dynamic.valid ||
        source->caii_dynamic.dynamic_candidate_count !=
            source->caii_map_receipt.dynamic_candidate_count ||
        source->caii_dynamic.allocated_slot_count !=
            source->caii_dynamic.dynamic_candidate_count ||
        source->caii_dynamic.think_timer_count !=
            source->caii_dynamic.dynamic_candidate_count ||
        source->caii_dynamic.source_hash == 0u ||
        !source->caii_slots.valid || source->caii_slots.capacity <= 0 ||
        !source->caii_slots.slots || !source->caii_rng_initialized ||
        !source->caii_source.valid || !source->asset_loader ||
        !source->asset_loader->loaded ||
        !source->preselection_init_game_ui_materialized ||
        !source->preselection_init_game_ui.valid ||
        !source->preselection_local_graphics.valid ||
        source->preselection_local_graphics.map != source->current_map ||
        !source->sound_owner.valid || !source->sound_owner.runtime_queue_initialized) {
        return 0;
    }
    if (!source->source_moverec.valid ||
        source->source_moverec.v1d3248_before_final_change != -1 ||
        source->source_moverec.v1d3248 != source->current_map) {
        return 0;
    }
    if (source->source_event_queue.event_heroidx !=
        source->source_event_hero_index ||
        source->source_event_queue.entries != 0 ||
        source->source_event_queue.idx != 0 ||
        source->source_event_queue.out_idx != 0 ||
        source->source_event_queue.fetch_busy ||
        source->source_event_queue.singleevent_available ||
        source->source_event_queue.button0x2) {
        return 0;
    }
    if (dm2_v1_dungeon_load(&candidate.dungeon, source->dungeon.raw_data,
                            source->dungeon.raw_size) != 0 ||
        !candidate.dungeon.record_graph_complete ||
        !dm2_v1_record_pool_set_clone(&candidate.record_pools,
                                      &source->record_pools) ||
        !candidate.record_pools.valid ||
        !candidate.record_pools.record_graph_complete ||
        !dm2_v1_game_load_runtime_candidate_materialize_dynamic_pools(
            &candidate, source)) goto fail;
    memcpy(candidate.record_capacities, source->record_capacities,
           sizeof(candidate.record_capacities));
    candidate.timer_entries = calloc(source->timer_capacity,
                                     sizeof(*candidate.timer_entries));
    candidate.timer_indices = calloc(source->timer_capacity,
                                     sizeof(*candidate.timer_indices));
    if (!candidate.timer_entries || !candidate.timer_indices) goto fail;
    memcpy(candidate.timer_entries, source->timer_entries,
           (size_t)source->timer_capacity * sizeof(*candidate.timer_entries));
    memcpy(candidate.timer_indices, source->timer_indices,
           (size_t)source->timer_capacity * sizeof(*candidate.timer_indices));
    candidate.timer_queue = source->timer_queue;
    candidate.timer_queue.entries = candidate.timer_entries;
    candidate.timer_queue.indices = candidate.timer_indices;
    candidate.timer_capacity = source->timer_capacity;
    dm2_v1_caii_array_init(&candidate.caii_slots, source->caii_slots.capacity);
    if (!candidate.caii_slots.valid || !candidate.caii_slots.slots) goto fail;
    memcpy(candidate.caii_slots.slots, source->caii_slots.slots,
           (size_t)source->caii_slots.capacity * DM2_V1_CAII_SLOT_SIZE);
    candidate.caii_slots.alloc_count = source->caii_slots.alloc_count;
    if (!dm2_v1_caii_source_owner_clone(&candidate.caii_source,
                                        &source->caii_source)) goto fail;
    candidate.asset_loader = source->asset_loader;
    if (!dm2_v1_game_load_runtime_candidate_sound_clone(&candidate.sound_owner,
                                                         &source->sound_owner)) goto fail;
    candidate.party = source->selected_party;
    candidate.leader_hand_record = source->load_new_dungeon_reset.leader_hand_record;
    dm2_v1_game_load_runtime_candidate_init_movement_state(&candidate.movement);
    candidate.moverec = source->source_moverec;
    candidate.event_queue = source->source_event_queue;
    candidate.source_event_hero_index = source->source_event_hero_index;
    candidate.source_next_champion_number = source->source_next_champion_number;
    /* DM2__INIT_GAME enters DM2_1031_0541(5) before DM2_2f3f_0789 selects
     * the first champion. Clone that already-materialised empty-party state;
     * rebuilding it from candidate.party here would change predicate inputs
     * and silently reorder INIT_GAME. */
    candidate.init_game_ui = source->preselection_init_game_ui;
    candidate.caii_rng = source->caii_rng;
    candidate.caii_rng_initialized = 1;
    candidate.source_party_map = source->source_party_map;
    candidate.source_party_x = source->source_party_x;
    candidate.source_party_y = source->source_party_y;
    candidate.source_party_direction = source->source_party_direction;
    candidate.source_party_absdir = source->source_party_absdir;
    candidate.source_staircase_flag = source->source_staircase_flag;
    candidate.source_teleporter_map = source->source_teleporter_map;
    candidate.source_display_x = source->source_display_x;
    candidate.source_display_y = source->source_display_y;
    candidate.source_teleporter_probe_direction =
        source->source_teleporter_probe_direction;
    candidate.source_teleporter_source_direction =
        source->source_teleporter_source_direction;
    candidate.source_teleporter_destination_direction =
        source->source_teleporter_destination_direction;
    candidate.source_display_pose_valid = source->source_display_pose_valid;
    candidate.source_transaction_hash = source->source_transaction_hash;
    /* DM2_move_2fcf_0b8b has explicitly set v1d3248=-1 before its final
     * CHANGE_CURRENT_MAP_TO. Recreate that forced initial selection over the
     * clone; a same-map shortcut would leave c_map's descriptor pointers
     * unmaterialized. */
    if (!dm2_v1_game_load_runtime_candidate_change_current_map(
        &candidate, source->current_map, 1)) {
        goto fail;
    }
    /* The source receipt is built by LOAD_LOCALLEVEL_GRAPHICS_TABLE from the
     * same map descriptor selected above.  Copy its arrays rather than
     * regenerating a graphics set from host defaults. */
    candidate.local_level_graphics = source->preselection_local_graphics;
    if (!dm2_v1_game_load_local_dyn_prelude_init(&candidate.local_dyn_prelude,
                                                  &candidate)) {
        goto fail;
    }
    if (!dm2_v1_game_load_local_dyn_map_scan_init(&candidate.local_dyn_map_scan,
                                                  &candidate)) {
        goto fail;
    }
    if (!dm2_v1_game_load_local_dyn_record_effects_init(
            &candidate.local_dyn_record_effects, &candidate)) {
        goto fail;
    }
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.source_transaction_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        (uint32_t)source->dungeon.raw_size);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        (uint32_t)source->dungeon.level_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, source->sound_owner.receipt_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, source->caii_source.source_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.timer_capacity);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)candidate.party.heros_in_party);
    /* Fresh GAME_LOAD owns the source c_wbbb block from zero. Keep the
     * complete block in the candidate even though its b_04 consumer is the
     * first runtime effect currently admitted. */
    memset(candidate.source_savegames1, 0,
           sizeof(candidate.source_savegames1));
    candidate.source_savegames1_valid = 1;
    hash = dm2_v1_game_load_owner_hash_step(hash,
        candidate.source_savegames1_valid);
    if (hash == 0u) goto fail;
    candidate.candidate_hash = hash;
    candidate.valid = 1;
    dm2_v1_game_load_runtime_session_candidate_free(out);
    *out = candidate;
    return 1;
fail:
    dm2_v1_game_load_runtime_session_candidate_free(&candidate);
    if (!out_was_initialized) memset(out, 0, sizeof(*out));
    return 0;
}

int dm2_v1_game_load_runtime_session_candidate_init_from_sksave(
    DM2_V1_GameLoadRuntimeSessionCandidate *out,
    const DM2_V1_SksaveGameLoadOwner *source)
{
    DM2_V1_GameLoadRuntimeSessionCandidate candidate;
    uint32_t hash = 0x52534b53u; /* "RSKS" */
    int i;

    if (!out || !source || !source->valid ||
        !source->source_dungeon_valid || !source->source_dungeon.raw_data ||
        source->source_dungeon.raw_size <= 0 ||
        !source->record_pools.valid || !source->record_pools.record_graph_complete ||
        !source->caii_slots_valid || !source->caii_slots.valid ||
        !source->caii_source.valid || !source->asset_loader ||
        !source->asset_loader->loaded || !source->source_timer_owner_ready ||
        !source->runtime_timer_entries || !source->runtime_timer_indices ||
        source->runtime_timer_capacity == 0u || !source->source_sound_materialized ||
        !source->sound_owner.valid || !source->source_savegames1_valid ||
        source->state.champion_count == 0u ||
        source->state.champion_count > DM2_MAX_HEROES ||
        source->state.leader_index >= source->state.champion_count) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate.lifecycle_tag =
        DM2_V1_GAME_LOAD_RUNTIME_CANDIDATE_LIFECYCLE_TAG;
    if (dm2_v1_dungeon_load(&candidate.dungeon,
                            source->source_dungeon.raw_data,
                            source->source_dungeon.raw_size) != 0) goto fail;
    if (!candidate.dungeon.record_graph_complete) goto fail;
    if (!dm2_v1_record_pool_set_clone(&candidate.record_pools,
                                      &source->record_pools)) goto fail;
    if (!dm2_v1_caii_source_owner_clone(&candidate.caii_source,
                                         &source->caii_source)) goto fail;
    if (!dm2_v1_game_load_sound_owner_clone(&candidate.sound_owner,
                                             &source->sound_owner)) goto fail;
    if (!dm2_v1_game_load_runtime_candidate_sksave_map_clone(
            &candidate.sksave_map_owner, &source->map_owner)) goto fail;
    if (!source->map_owner.dungeon) goto fail;
    candidate.sksave_dungeon_receipt = *source->map_owner.dungeon;
    candidate.sksave_map_owner.dungeon = &candidate.sksave_dungeon_receipt;
    candidate.sksave_map_owner_valid = 1;
    candidate.timer_capacity = source->runtime_timer_capacity;
    candidate.timer_entries = calloc(candidate.timer_capacity,
                                     sizeof(*candidate.timer_entries));
    candidate.timer_indices = calloc(candidate.timer_capacity,
                                     sizeof(*candidate.timer_indices));
    if (!candidate.timer_entries || !candidate.timer_indices) goto fail;
    memcpy(candidate.timer_entries, source->runtime_timer_entries,
           (size_t)candidate.timer_capacity * sizeof(*candidate.timer_entries));
    memcpy(candidate.timer_indices, source->runtime_timer_indices,
           (size_t)candidate.timer_capacity * sizeof(*candidate.timer_indices));
    candidate.timer_queue = source->runtime_timer_queue;
    candidate.timer_queue.entries = candidate.timer_entries;
    candidate.timer_queue.indices = candidate.timer_indices;
    dm2_v1_caii_array_init(&candidate.caii_slots,
                           source->caii_slots.capacity);
    if (!candidate.caii_slots.valid) goto fail;
    memcpy(candidate.caii_slots.slots, source->caii_slots.slots,
           (size_t)source->caii_slots.capacity * DM2_V1_CAII_SLOT_SIZE);
    candidate.caii_slots.alloc_count = source->caii_slots.alloc_count;
    candidate.asset_loader = source->asset_loader;
    candidate.party.heros_in_party = (int16_t)source->state.champion_count;
    candidate.party.absdir = (int16_t)source->state.party_direction;
    candidate.party.curactevhero = (DM2_HeroIndex)source->state.leader_index;
    candidate.party.curacthero = (int16_t)source->state.leader_index;
    for (i = 0; i < (int)source->state.champion_count; ++i)
        candidate.party.hero[i] = source->heroes[i];
    candidate.leader_hand_record = (int16_t)source->leader_hand_root;
    if (candidate.leader_hand_record == DM2_V1_RECORD_HANDLE_END ||
        candidate.leader_hand_record == DM2_V1_RECORD_HANDLE_NULL) {
        int leader = (int)source->state.leader_index;
        if (leader >= 0 && leader < (int)source->state.champion_count) {
            uint16_t object = (uint16_t)source->heroes[leader].item[0];
            if (object != 0xffffu)
                candidate.leader_hand_record = (int16_t)object;
        }
    }
    candidate.source_event_hero_index = (int16_t)source->state.leader_index;
    candidate.source_next_champion_number = 0;
    candidate.source_party_map = (int)source->state.party_map;
    candidate.source_party_x = (uint8_t)source->state.party_x;
    candidate.source_party_y = (uint8_t)source->state.party_y;
    candidate.source_party_direction = (uint8_t)source->state.party_direction;
    candidate.source_party_absdir = candidate.source_party_direction;
    candidate.source_teleporter_map = -1;
    candidate.source_display_pose_valid = 1;
    candidate.source_display_map = (int16_t)source->state.party_map;
    candidate.source_display_x = (int16_t)source->state.party_x;
    candidate.source_display_y = (int16_t)source->state.party_y;
    candidate.source_runtime_display_direction = candidate.source_party_direction;
    candidate.source_hero_ench_countdown = source->source_savegames1[2];
    candidate.source_hero_ench_target = -1;
    memcpy(candidate.source_savegames1, source->source_savegames1,
           sizeof(candidate.source_savegames1));
    candidate.source_savegames1_valid = 1;
    dm2_v1_game_load_moverec_state_init(&candidate.moverec,
                                        (int16_t)source->state.party_map);
    {
        int map_ok = dm2_v1_game_load_runtime_candidate_change_current_map(
            &candidate, (int)source->state.party_map, 1);
        int prelude_ok = map_ok && dm2_v1_game_load_local_dyn_prelude_init(
            &candidate.local_dyn_prelude, &candidate);
        int scan_ok = prelude_ok && dm2_v1_game_load_local_dyn_map_scan_init(
            &candidate.local_dyn_map_scan, &candidate);
        int effects_ok = scan_ok && dm2_v1_game_load_local_dyn_record_effects_init(
            &candidate.local_dyn_record_effects, &candidate);
        if (!map_ok || !prelude_ok || !scan_ok || !effects_ok) {
            goto fail;
        }
    }
    candidate.source_transaction_hash = source->state.fixed_sections_hash ^
        source->state.timers_hash ^ source->state.dungeon.prefix_hash;
    if (candidate.source_transaction_hash == 0u) goto fail;
    hash = dm2_v1_game_load_owner_hash_step(hash,
        candidate.source_transaction_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        source->sound_owner.receipt_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        source->caii_source.source_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        candidate.timer_capacity);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        (uint32_t)candidate.party.heros_in_party);
    if (hash == 0u) goto fail;
    candidate.candidate_hash = hash;
    candidate.valid = 1;
    dm2_v1_game_load_runtime_session_candidate_free(out);
    *out = candidate;
    out->sksave_map_owner.dungeon = &out->sksave_dungeon_receipt;
    return 1;
fail:
    dm2_v1_game_load_runtime_session_candidate_free(&candidate);
    return 0;
}

/* Build the source-owned half of GAME_LOAD that precedes the mirror event
 * loop.  The receipts here are deliberately selection-free: a title click
 * must never turn the first roster entry into an invented party choice.
 * Source: SKProject SKULLWIN/c_savegame.cpp::DM2_GAME_LOAD (1415-1565),
 * skhero.cpp::DM2_SELECT_CHAMPION (1054-1168). */
int dm2_v1_game_load_world_owner_prepare_new_game(
    DM2_V1_GameLoadWorldOwner *owner, const DM2_V1_BootProfile *profile)
{
    const DM2_V1_DungeonData *source;
    DM2_V1_GameLoadWorldOwner candidate;
    uint32_t hash = 0x4e475052u; /* "NGPR": New Game preparation. */
    const int owner_was_initialized =
        dm2_v1_game_load_world_owner_is_initialized(owner);
#define DM2_OWNER_PREPARE_DEBUG(label, expression) (expression)

    if (!owner) return 0;
    if (!owner_was_initialized) memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    candidate.lifecycle_tag = DM2_V1_GAME_LOAD_WORLD_OWNER_LIFECYCLE_TAG;
    if (!DM2_OWNER_PREPARE_DEBUG("profile", profile && profile->assets_verified) ||
        !DM2_OWNER_PREPARE_DEBUG("source", (source = (const DM2_V1_DungeonData *)profile->dungeon_data) != NULL) ||
        !DM2_OWNER_PREPARE_DEBUG("source-bytes", source->raw_data && source->raw_size > 0) ||
        !DM2_OWNER_PREPARE_DEBUG("source-graph", source->record_graph_complete) ||
        !DM2_OWNER_PREPARE_DEBUG("entrance", dm2_v1_boot_new_game_entrance_receipt(profile, &candidate.preselection_entrance)) ||
        !DM2_OWNER_PREPARE_DEBUG("entrance-fields", candidate.preselection_entrance.valid && candidate.preselection_entrance.incomplete_game_load) ||
        !DM2_OWNER_PREPARE_DEBUG("world", dm2_v1_boot_file_header_world_interaction_receipt(profile, &candidate.preselection_world_interactions) && candidate.preselection_world_interactions.valid && candidate.preselection_world_interactions.incomplete_world) ||
        !DM2_OWNER_PREPARE_DEBUG("generators", dm2_v1_boot_file_header_actuator_generator_receipt(profile, &candidate.preselection_actuator_generators) && candidate.preselection_actuator_generators.valid && candidate.preselection_actuator_generators.incomplete_game_load) ||
        !DM2_OWNER_PREPARE_DEBUG("map", dm2_v1_boot_file_header_runtime_map_receipt(profile, candidate.preselection_entrance.map, &candidate.preselection_entrance_map) && candidate.preselection_entrance_map.committed && candidate.preselection_entrance_map.incomplete_world) ||
        !DM2_OWNER_PREPARE_DEBUG("census", dm2_v1_boot_champion_selection_census(profile, &candidate.preselection_mirror_roster) && candidate.preselection_mirror_roster.valid && candidate.preselection_mirror_roster.incomplete_game_load && candidate.preselection_mirror_roster.candidate_count > 0 && candidate.preselection_mirror_roster.candidate_count <= DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES) ||
        !DM2_OWNER_PREPARE_DEBUG("dyn4", dm2_v1_boot_champion_dyn4_roster_receipt(profile, &candidate.preselection_dyn4_roster) && candidate.preselection_dyn4_roster.valid && candidate.preselection_dyn4_roster.incomplete_champion_activation) ||
        !DM2_OWNER_PREPARE_DEBUG("map-range", candidate.preselection_entrance.map >= 0 && candidate.preselection_entrance.map < source->level_count) ||
        !DM2_OWNER_PREPARE_DEBUG("map-count", candidate.preselection_world_interactions.map_count == source->level_count && candidate.preselection_actuator_generators.map_count == source->level_count) ||
        !DM2_OWNER_PREPARE_DEBUG("map-match", candidate.preselection_entrance_map.map == candidate.preselection_entrance.map) ||
        !DM2_OWNER_PREPARE_DEBUG("dyn4-count", candidate.preselection_mirror_roster.candidate_count == candidate.preselection_dyn4_roster.selector_count) ||
        !DM2_OWNER_PREPARE_DEBUG("reload", dm2_v1_dungeon_load(&candidate.dungeon, source->raw_data, source->raw_size) == 0) ||
        !DM2_OWNER_PREPARE_DEBUG("reload-graph", candidate.dungeon.record_graph_complete) ||
        !DM2_OWNER_PREPARE_DEBUG("pose", candidate.dungeon.initial_party_pose_valid && candidate.dungeon.initial_party_x == candidate.preselection_entrance.x && candidate.dungeon.initial_party_y == candidate.preselection_entrance.y && candidate.dungeon.initial_party_dir == candidate.preselection_entrance.direction) ||
        !DM2_OWNER_PREPARE_DEBUG("pools", dm2_v1_record_pool_set_init_from_dungeon(&candidate.record_pools, &candidate.dungeon) && candidate.record_pools.valid && candidate.record_pools.record_graph_complete) ||
        !DM2_OWNER_PREPARE_DEBUG("timer", dm2_v1_game_load_owner_prepare_timer_capacity(&candidate))) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    hash = dm2_v1_game_load_owner_hash_step(
        hash, candidate.preselection_entrance.receipt_hash);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, candidate.preselection_world_interactions.interaction_hash);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, candidate.preselection_actuator_generators.candidate_hash);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, candidate.preselection_entrance_map.map_data_hash);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, candidate.preselection_dyn4_roster.selector_roster_hash);
    hash = dm2_v1_game_load_owner_hash_step(
        hash, candidate.preselection_mirror_roster.roster_hash);
    if (hash == 0u) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    candidate.current_map = candidate.preselection_entrance.map;
    candidate.boot_profile = profile;
    candidate.asset_loader = dm2_v1_boot_asset_loader(profile);
    candidate.preselection_hash = hash;
    candidate.source_transaction_hash = hash;
    candidate.source_preselection_ready = 1;
    if (!DM2_OWNER_PREPARE_DEBUG("asset-loader", candidate.asset_loader && candidate.asset_loader->loaded) ||
        !DM2_OWNER_PREPARE_DEBUG("caii-capacity", dm2_v1_game_load_owner_materialize_caii_capacity(&candidate)) ||
        !DM2_OWNER_PREPARE_DEBUG("caii-candidates", dm2_v1_game_load_owner_materialize_caii_map_candidates(&candidate)) ||
        !DM2_OWNER_PREPARE_DEBUG("caii-storage", dm2_v1_game_load_owner_materialize_caii_storage(&candidate)) ||
        !DM2_OWNER_PREPARE_DEBUG("world-maps", dm2_v1_game_load_owner_validate_world_maps(&candidate)) ||
        !DM2_OWNER_PREPARE_DEBUG("new-dungeon-reset", dm2_v1_game_load_owner_materialize_new_dungeon_reset(&candidate)) ||
        !DM2_OWNER_PREPARE_DEBUG("dyn4", dm2_v1_game_load_owner_materialize_dyn4(&candidate)) ||
        !DM2_OWNER_PREPARE_DEBUG("sound", dm2_v1_game_load_owner_materialize_sound(&candidate))) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
#undef DM2_OWNER_PREPARE_DEBUG
    candidate.prepared = 1;
    candidate.committed = 0;
    dm2_v1_game_load_world_owner_free(owner);
    *owner = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_init_new_game(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootProfile *profile,
    const DM2_V1_BootNewGamePartySelection *selections,
    int selection_count)
{
    DM2_V1_GameLoadWorldOwner candidate;

    if (!owner) return 0;
    /* Constructor callers pass a fresh/zeroed owner.  Do not inspect a
     * possibly uninitialised caller buffer here; reinitialising an existing
     * owner is explicitly free-then-init so no source RAM owner leaks. */
    memset(&candidate, 0, sizeof(candidate));
    if (!selections || selection_count <= 0 ||
        selection_count > DM2_MAX_HEROES ||
        !dm2_v1_game_load_world_owner_prepare_new_game(&candidate, profile) ||
        !dm2_v1_game_load_world_owner_materialize_preselection_init_game_ui(
            &candidate) ||
        !dm2_v1_boot_new_game_transaction_receipt(
            profile, selections, selection_count, &candidate.transaction) ||
        !candidate.transaction.valid ||
        !candidate.transaction.incomplete_game_load ||
        candidate.transaction.entrance.receipt_hash !=
            candidate.preselection_entrance.receipt_hash ||
        candidate.transaction.world_interactions.interaction_hash !=
            candidate.preselection_world_interactions.interaction_hash ||
        candidate.transaction.dyn4_roster.selector_roster_hash !=
            candidate.preselection_dyn4_roster.selector_roster_hash ||
        !dm2_v1_game_load_owner_validate_possessions(&candidate)) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    /* The source transaction identity becomes more specific only after real
     * mirror clicks have been admitted.  The selection-free preparation hash
     * remains retained separately for the title-to-entrance boundary. */
    candidate.source_transaction_hash = candidate.transaction.transaction_hash;
    dm2_v1_game_load_world_owner_free(owner);
    *owner = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_select_champion(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootNewGamePartySelection *selection)
{
    DM2_V1_BootNewGameTransactionReceipt transaction;
    DM2_V1_BootNewGameTransactionReceipt old_transaction;
    DM2_V1_BootNewGamePartySelection next[DM2_MAX_HEROES];
    DM2_V1_BootNewGamePartySelection old_selected[DM2_MAX_HEROES];
    DM2_V1_Party old_party;
    DM2_V1_SksaveItemBonusReceipt old_bonus;
    DM2_V1_GameLoadChampionSelectionReceipt old_receipt;
    DM2_V1_EventQueue old_event_queue;
    int16_t old_next_number;
    int16_t old_event_hero;
    uint8_t old_count;
    int old_materialized;
    int old_event_queue_materialized;
    uint8_t count;
    int i;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) || !selection ||
        !owner->source_preselection_ready || !owner->boot_profile ||
        !owner->source_map_context_materialized || owner->committed ||
        owner->selected_mirror_count >= DM2_MAX_HEROES) return 0;
    count = owner->selected_mirror_count;
    for (i = 0; i < count; ++i) {
        if (owner->selected_mirrors[i].mirror_object_id ==
            selection->mirror_object_id) return 0;
        next[i] = owner->selected_mirrors[i];
    }
    next[count] = *selection;
    memset(&transaction, 0, sizeof(transaction));
    if (owner->boot_profile->platform == DM2_PLATFORM_MAC_EN) {
        transaction.entrance = owner->preselection_entrance;
        transaction.world_interactions = owner->preselection_world_interactions;
        transaction.actuator_generators = owner->preselection_actuator_generators;
        transaction.entrance_map = owner->preselection_entrance_map;
        transaction.dyn4_roster = owner->preselection_dyn4_roster;
        if (!dm2_v1_boot_new_game_party_receipt(owner->boot_profile, next,
                (int)count + 1, &transaction.party) ||
            !transaction.party.valid ||
            !dm2_v1_boot_new_game_possession_receipt(
                owner->boot_profile, &transaction.party,
                &transaction.possessions) || !transaction.possessions.valid) {
            return 0;
        }
        transaction.hero_count = (int)count + 1;
        transaction.incomplete_game_load = 1;
        transaction.valid = 1;
    } else if (!dm2_v1_boot_new_game_transaction_receipt(owner->boot_profile, next,
            (int)count + 1, &transaction) || !transaction.valid ||
        !transaction.incomplete_game_load ||
        transaction.entrance.receipt_hash !=
            owner->preselection_entrance.receipt_hash ||
        transaction.world_interactions.interaction_hash !=
            owner->preselection_world_interactions.interaction_hash ||
        transaction.actuator_generators.candidate_hash !=
            owner->preselection_actuator_generators.candidate_hash ||
        transaction.dyn4_roster.selector_roster_hash !=
            owner->preselection_dyn4_roster.selector_roster_hash) {
        return 0;
    }

    old_transaction = owner->transaction;
    memcpy(old_selected, owner->selected_mirrors, sizeof(old_selected));
    old_count = owner->selected_mirror_count;
    old_party = owner->selected_party;
    old_bonus = owner->champion_item_bonus;
    old_receipt = owner->champion_selection_receipt;
    old_event_queue = owner->source_event_queue;
    old_next_number = owner->source_next_champion_number;
    old_event_hero = owner->source_event_hero_index;
    old_materialized = owner->champion_selection_materialized;
    old_event_queue_materialized = owner->source_event_queue_materialized;
    owner->transaction = transaction;
    memcpy(owner->selected_mirrors, next, sizeof(next));
    owner->selected_mirror_count = (uint8_t)(count + 1u);
    owner->champion_selection_materialized = 0;
    if (!dm2_v1_game_load_owner_validate_possessions(owner) ||
        !dm2_v1_game_load_world_owner_materialize_champion_selection(owner)) {
        owner->transaction = old_transaction;
        memcpy(owner->selected_mirrors, old_selected, sizeof(old_selected));
        owner->selected_mirror_count = old_count;
        owner->selected_party = old_party;
        owner->champion_item_bonus = old_bonus;
        owner->champion_selection_receipt = old_receipt;
        owner->source_event_queue = old_event_queue;
        owner->source_next_champion_number = old_next_number;
        owner->source_event_hero_index = old_event_hero;
        owner->champion_selection_materialized = old_materialized;
        owner->source_event_queue_materialized = old_event_queue_materialized;
        return 0;
    }
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_champion_selection(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_Party candidate;
    DM2_V1_GameLoadChampionSelectionReceipt selection_receipt;
    DM2_V1_EventQueue source_event_queue;
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
    /* c_eventqueue::init initializes the full queue before STARTEND drives
     * the first mirror. SELECT_CHAMPION_LEADER keeps hero 0 selected here;
     * do not substitute a host queue or enqueue an invented input event.
     * Source: SKULLWIN/c_eventqueue.cpp::init (10-31), startend.cpp::
     * DM2_2f3f_0789 (1164-1189). */
    dm2_v1_eventqueue_init(&source_event_queue);
    if (source_event_queue.event_heroidx != 0) {
        return 0;
    }
    source_event_queue.event_heroidx = 0;
    owner->selected_party = candidate;
    owner->source_next_champion_number = candidate.heros_in_party;
    owner->source_event_hero_index = 0;
    owner->source_event_queue = source_event_queue;
    owner->source_event_queue_materialized = 1;
    owner->champion_selection_receipt = selection_receipt;
    owner->champion_selection_materialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_source_map_context(
    DM2_V1_GameLoadWorldOwner *owner)
{
    const DM2_V1_BootNewGameEntranceReceipt *entrance = owner &&
        owner->source_preselection_ready ? &owner->preselection_entrance :
        (owner ? &owner->transaction.entrance : NULL);
    const int map = entrance ? entrance->map : -1;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->fresh_game_mode || !owner->actuator_generators_processed ||
        owner->source_map_context_materialized || owner->committed ||
        map < 0 || map >= owner->dungeon.level_count ||
        !owner->dungeon.initial_party_pose_valid ||
        owner->dungeon.initial_party_x != entrance->x ||
        owner->dungeon.initial_party_y != entrance->y ||
        owner->dungeon.initial_party_dir != entrance->direction) {
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

int dm2_v1_game_load_world_owner_materialize_preselection_init_game_ui(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_Party empty_party;
    DM2_V1_EventQueue initial_event_queue;
    DM2_V1_InitGameUiOwner candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->fresh_game_mode || !owner->source_preselection_ready ||
        owner->committed ||
        owner->champion_selection_materialized ||
        owner->preselection_init_game_ui_materialized ||
        !owner->load_new_dungeon_reset.valid ||
        owner->load_new_dungeon_reset.party_count != 0 ||
        owner->load_new_dungeon_reset.leader_hand_record !=
            DM2_V1_RECORD_HANDLE_NULL ||
        owner->selected_mirror_count != 0u) {
        return 0;
    }
    /* c_party/c_eventqueue initialisers precede STARTEND.  Do not derive
     * this from selected_party: that state belongs to a later mirror click.
     * Source: dm2data.cpp::c_dm2data; c_eventqueue.cpp::init (10-31);
     * startend.cpp::DM2__INIT_GAME_38c8_03ad (1171-1194). */
    dm2_v1_party_state_init(&empty_party);
    dm2_v1_eventqueue_init(&initial_event_queue);
    if (empty_party.heros_in_party != 0 || empty_party.curacthero != 0 ||
        empty_party.curactevhero != DM2_HERO_NONE ||
        initial_event_queue.event_heroidx != 0) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_init_game_ui_owner_init(&candidate, &empty_party,
                                        &initial_event_queue) ||
        !candidate.valid || candidate.runtime.active_tree != 5u ||
        !candidate.initial_tree.valid ||
        candidate.initial_tree.selected_tree != 5 ||
        candidate.predicates.champion_inventory != 0u ||
        candidate.predicates.champion_hp[0] != 0u) {
        return 0;
    }
    owner->preselection_init_game_ui = candidate;
    owner->preselection_init_game_ui_materialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_local_graphics(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadLocalLevelGraphicsReceipt candidate;
    int wall_count;
    int floor_count;
    int ornate_count;
    uint32_t hash = 0x4c4c4758u; /* "LLGX": local-level graphics. */

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        owner->preselection_local_graphics.valid ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    wall_count = dm2_v1_dungeon_get_map_wall_gfx_list(&owner->dungeon,
        owner->source_party_map, candidate.wall_gfx,
        (int)sizeof(candidate.wall_gfx));
    floor_count = dm2_v1_dungeon_get_map_floor_gfx_list(&owner->dungeon,
        owner->source_party_map, candidate.floor_gfx,
        (int)sizeof(candidate.floor_gfx));
    ornate_count = dm2_v1_dungeon_get_map_door_ornate_list(&owner->dungeon,
        owner->source_party_map, candidate.door_ornate_gfx,
        (int)sizeof(candidate.door_ornate_gfx));
    if (wall_count < 0 || floor_count < 0 || ornate_count < 0 ||
        wall_count > (int)sizeof(candidate.wall_gfx) ||
        floor_count > (int)sizeof(candidate.floor_gfx) ||
        ornate_count > (int)sizeof(candidate.door_ornate_gfx)) {
        return 0;
    }
    /* c_loadlevel.cpp walks current-map lists directly after map change.
     * Retain the exact list lengths as part of the owner identity, including
     * valid source maps where a list is empty. */
    candidate.map = owner->source_party_map;
    candidate.wall_count = (uint8_t)wall_count;
    candidate.floor_count = (uint8_t)floor_count;
    candidate.door_ornate_count = (uint8_t)ornate_count;
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)candidate.map);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.wall_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.floor_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.door_ornate_count);
    for (int i = 0; i < candidate.wall_count; ++i)
        hash = dm2_v1_game_load_owner_hash_step(hash, candidate.wall_gfx[i]);
    for (int i = 0; i < candidate.floor_count; ++i)
        hash = dm2_v1_game_load_owner_hash_step(hash, candidate.floor_gfx[i]);
    for (int i = 0; i < candidate.door_ornate_count; ++i)
        hash = dm2_v1_game_load_owner_hash_step(hash,
                                                 candidate.door_ornate_gfx[i]);
    if (hash == 0u) return 0;
    candidate.source_list_hash = hash;
    candidate.valid = 1;
    owner->preselection_local_graphics = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_map_doors(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_G1RuntimeMapDoorReceipt candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        owner->preselection_map_doors.committed ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count ||
        owner->preselection_local_graphics.map != owner->source_party_map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    /* This reads only direct DB0 roots after the File_header map validator
     * has admitted the clone.  It does not follow GenericRecord links, so a
     * visible door cannot acquire a guessed owner from a neighbouring chain.
     * Source: SKWIN/DME.h::Door; c_map.cpp::GET_ADDRESS_OF_TILE_RECORD. */
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_doors(
            &owner->dungeon, owner->source_party_map, &candidate) ||
        !candidate.committed || candidate.map != owner->source_party_map) {
        return 0;
    }
    owner->preselection_map_doors = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_map_objects(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_FileHeaderRuntimeObjectReceipt candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        owner->preselection_map_objects.committed ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count ||
        owner->preselection_local_graphics.map != owner->source_party_map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    /* LOAD_LOCALLEVEL_DYN reaches DB5..DB15 through c_map's validated
     * ground stacks and GenericRecord::w0.  Preserve those exact source
     * addresses first; constructing an inventory or a placement list here
     * would lose the source-owned record-chain semantics. */
    if (!dm2_v1_dungeon_collect_file_header_runtime_map_objects(
            &owner->dungeon, owner->source_party_map, &candidate) ||
        !candidate.committed || candidate.map != owner->source_party_map ||
        candidate.object_record_reads != candidate.object_record_count) {
        return 0;
    }
    owner->preselection_map_objects = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_map_texts(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_FileHeaderRuntimeTextReceipt candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        owner->preselection_map_texts.committed ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count ||
        owner->preselection_local_graphics.map != owner->source_party_map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    /* QUERY_MESSAGE_TEXT and special floor markers use DME.h::Text::w2
     * through the original c_map -> c_record chain.  Keep those exact
     * fields in RAM, but do not turn text indices into translated strings or
     * mutate TextVisibility before the relevant source consumers are owned. */
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_texts(
            &owner->dungeon, owner->source_party_map, &candidate) ||
        !candidate.committed || candidate.map != owner->source_party_map ||
        candidate.text_record_reads != candidate.text_record_count) {
        return 0;
    }
    owner->preselection_map_texts = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_map_teleporters(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_FileHeaderRuntimeTeleporterReceipt candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        owner->preselection_map_teleporters.committed ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count ||
        owner->preselection_local_graphics.map != owner->source_party_map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    /* c_moverec consumes the direct DME.h::Teleporter payload.  The map
     * owner only preserves those bytes; changing the party map, coordinates,
     * facing or playing teleporter sound requires the missing live session. */
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_teleporters(
            &owner->dungeon, owner->source_party_map, &candidate) ||
        !candidate.committed || candidate.map != owner->source_party_map ||
        candidate.teleporter_record_reads != candidate.teleporter_root_count) {
        return 0;
    }
    owner->preselection_map_teleporters = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_map_actuators(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_G1RuntimeMapActuatorReceipt candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        owner->preselection_map_actuators.committed ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count ||
        owner->preselection_local_graphics.map != owner->source_party_map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    /* DME.h::Actuator w2/w4/w6 belongs to the validated File_header map
     * chain.  Preserve its source target and timing fields but never invoke
     * it through an incomplete event queue or substitute a host callback. */
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_actuators(
            &owner->dungeon, owner->source_party_map, &candidate) ||
        !candidate.committed || candidate.map != owner->source_party_map ||
        candidate.actuator_record_reads != candidate.actuator_root_count) {
        return 0;
    }
    owner->preselection_map_actuators = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_map_creatures(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_FileHeaderRuntimeCreatureReceipt candidate;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        owner->preselection_map_creatures.committed ||
        owner->champion_selection_materialized || owner->committed ||
        owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count ||
        owner->preselection_local_graphics.map != owner->source_party_map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    /* DME.h::Creature data stays record-owned here.  In particular, do not
     * turn its info slot into a host CAII slot or traverse possession roots
     * as drops before c_record/c_tim/CAII share one live session owner. */
    if (!dm2_v1_dungeon_materialize_file_header_runtime_map_creatures(
            &owner->dungeon, owner->source_party_map, &candidate) ||
        !candidate.committed || candidate.map != owner->source_party_map ||
        candidate.creature_record_reads != candidate.creature_record_count) {
        return 0;
    }
    owner->preselection_map_creatures = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_creature_possessions(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadCreaturePossessionReceipt candidate[
        DM2_V1_FILE_HEADER_RUNTIME_MAX_CREATURE_RECORDS];
    int count;
    int i;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->preselection_map_creatures.committed ||
        owner->preselection_creature_possessions_materialized ||
        owner->champion_selection_materialized || owner->committed) {
        return 0;
    }
    count = owner->preselection_map_creatures.creature_record_count;
    if (count < 0 || count > DM2_V1_FILE_HEADER_RUNTIME_MAX_CREATURE_RECORDS)
        return 0;
    memset(candidate, 0, sizeof(candidate));
    for (i = 0; i < count; ++i) {
        const DM2_V1_FileHeaderCreatureRecord *creature =
            &owner->preselection_map_creatures.creatures[i];
        DM2_V1_GameLoadCreaturePossessionReceipt *entry = &candidate[i];

        entry->valid = 1;
        entry->creature_object_id = creature->object_id;
        if (creature->possession_object_id == DM2_THING_NULL_MARKER) {
            /* DME.h::Creature::GetPossessionObject has an authentic null
             * root. Keep it distinct from OBJECT_END and a zero-length list. */
            entry->has_possession = 0u;
            continue;
        }
        if (!dm2_v1_dungeon_collect_file_header_creature_possession_chain(
                &owner->dungeon, &owner->preselection_map_creatures, i,
                &entry->receipt) || !entry->receipt.committed ||
            entry->receipt.map != owner->source_party_map ||
            entry->receipt.creature_object_id != creature->object_id ||
            entry->receipt.possession_root != creature->possession_object_id) {
            return 0;
        }
        entry->has_possession = 1u;
    }
    memcpy(owner->preselection_creature_possessions, candidate,
           (size_t)count * sizeof(candidate[0]));
    owner->preselection_creature_possession_count = (uint16_t)count;
    owner->preselection_creature_possessions_materialized = 1u;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_light(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadPreselectionLightReceipt candidate;
    int graphicsset;
    uint32_t hash = 0x4c495447u; /* "LITG": c_light GAME_LOAD inputs. */

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->fresh_game_mode || !owner->actuator_generators_processed ||
        !owner->source_map_context_materialized ||
        owner->champion_selection_materialized || owner->committed ||
        owner->preselection_light.valid || owner->source_party_map < 0 ||
        owner->source_party_map >= owner->dungeon.level_count) {
        return 0;
    }
    graphicsset = dm2_v1_dungeon_get_map_graphics_style(&owner->dungeon,
                                                          owner->source_party_map);
    if (graphicsset < 0 || graphicsset > UINT8_MAX) return 0;

    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_dungeon_c_light_map_descriptor_receipt(
            &owner->dungeon, owner->source_party_map,
            &candidate.map_descriptor) ||
        !candidate.map_descriptor.valid ||
        candidate.map_descriptor.level != owner->source_party_map ||
        candidate.map_descriptor.descriptor_hash == 0u) {
        return 0;
    }
    /* c_dm2data initialises every field below before DM2_GAME_LOAD.  LOAD_NEW_DUNGEON
     * then establishes a zero-hero party and OBJECT_NULL leader hand; only
     * LOAD_LOCALLEVEL_DYN derives v1d6c02 from this map descriptor.  Keep
     * these terminal source inputs explicit instead of accepting a host's
     * zero-filled struct as equivalent dynamic illumination. */
    candidate.map = owner->source_party_map;
    candidate.graphicsset = (uint8_t)graphicsset;
    candidate.party_count = 0u;
    candidate.leader_hand_record = DM2_V1_RECORD_HANDLE_NULL;
    candidate.savegame_light = 0u;
    candidate.v1e0974 = 0u;
    candidate.v1e0978 = 0u;
    candidate.weather_active = 0u;
    candidate.weather_index = 0u;
    candidate.weather_delta = 0u;
    candidate.weather_darkness_active = 0u;

    hash = dm2_v1_game_load_owner_hash_step(hash,
        candidate.map_descriptor.descriptor_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)candidate.map);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.graphicsset);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.party_count);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.leader_hand_record);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.savegame_light);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.v1e0974);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.v1e0978);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.weather_active);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.weather_index);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.weather_delta);
    hash = dm2_v1_game_load_owner_hash_step(hash,
                                             candidate.weather_darkness_active);
    if (hash == 0u) return 0;
    candidate.source_state_hash = hash;
    candidate.valid = 1;
    owner->preselection_light = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_light_visibility(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadLightVisibilityOwner candidate;
    const int primary_map = owner ? owner->source_party_map : -1;
    /* move_2fcf_0b8b first writes v1e027c=-1, then replaces it only when
     * the real teleporter probe finds an alternate display map.  c_light
     * uses exactly that post-probe value for its optional second surface;
     * c_dm2data::init's earlier map-zero value is not a GAME_LOAD input. */
    const int secondary_map = owner ? owner->source_teleporter_map : -2;
    int primary_width;
    int primary_height;
    int secondary_width;
    int secondary_height;
    size_t primary_bytes;
    size_t secondary_bytes;
    uint32_t hash = 0x4c564953u; /* "LVIS" */

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->fresh_game_mode || !owner->source_map_context_materialized ||
        !owner->preselection_light.valid || owner->committed ||
        owner->champion_selection_materialized ||
        owner->preselection_light_visibility.valid ||
        primary_map < 0 || primary_map >= owner->dungeon.level_count ||
        secondary_map < -1 || secondary_map >= owner->dungeon.level_count) {
        return 0;
    }
    primary_width = owner->dungeon.level_widths[primary_map];
    primary_height = owner->dungeon.level_heights[primary_map];
    secondary_width = secondary_map >= 0 ?
        owner->dungeon.level_widths[secondary_map] : 0;
    secondary_height = secondary_map >= 0 ?
        owner->dungeon.level_heights[secondary_map] : 0;
    /* c_light indexes `x << 5 | y`; DUNGEON's File_header dimensions are
     * bounded to that same 32-column stride. */
    if (primary_width <= 0 || primary_width > 32 || primary_height <= 0 ||
        primary_height > 32 ||
        (secondary_map >= 0 &&
         (secondary_width <= 0 || secondary_width > 32 ||
          secondary_height <= 0 || secondary_height > 32))) {
        return 0;
    }
    primary_bytes = (size_t)primary_width * 32u;
    secondary_bytes = secondary_map >= 0 ?
        (size_t)secondary_width * 32u : 0u;
    memset(&candidate, 0, sizeof(candidate));
    candidate.primary_cells = calloc(primary_bytes, 1u);
    if (secondary_bytes != 0u)
        candidate.secondary_cells = calloc(secondary_bytes, 1u);
    if (!candidate.primary_cells ||
        (secondary_bytes != 0u && !candidate.secondary_cells)) {
        dm2_v1_game_load_owner_light_visibility_free(&candidate);
        return 0;
    }
    candidate.primary_map = (int16_t)primary_map;
    candidate.secondary_map = (int16_t)secondary_map;
    candidate.primary_width = (uint8_t)primary_width;
    candidate.primary_height = (uint8_t)primary_height;
    candidate.secondary_width = (uint8_t)secondary_width;
    candidate.secondary_height = (uint8_t)secondary_height;
    candidate.primary_cell_bytes = primary_bytes;
    candidate.secondary_cell_bytes = secondary_bytes;
    /* LOAD_LOCALLEVEL_DYN sets bit 2 before CHECK_RECOMPUTE_LIGHT.  The
     * later GAME_LOAD tail's 3 is deliberately not used as this call's
     * input: CHECK_RECOMPUTE_LIGHT clears its dirty state before returning. */
    candidate.dirty_flags_before_check = 2u;
    candidate.walk_path_pending = 1u;
    hash = dm2_v1_game_load_owner_hash_step(hash,
        owner->preselection_light.source_state_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)primary_map);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)secondary_map);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)primary_width);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)primary_height);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)secondary_width);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)secondary_height);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.dirty_flags_before_check);
    if (hash == 0u) {
        dm2_v1_game_load_owner_light_visibility_free(&candidate);
        return 0;
    }
    candidate.source_state_hash = hash;
    candidate.valid = 1;
    owner->preselection_light_visibility = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_sound_spatial(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadSoundOwner *sound;
    const int current_map = owner ? owner->current_map : -1;
    const int audible_map = owner && owner->source_teleporter_map >= 0 ?
        owner->source_teleporter_map : (owner ? owner->source_party_map : -1);
    const int alternate_map = owner ? owner->source_teleporter_map : -1;
    uint32_t hash = 0x53504154u; /* "SPAT" */

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized || owner->committed ||
        owner->champion_selection_materialized || !owner->sound_owner.valid ||
        !owner->sound_owner.runtime_queue_initialized ||
        owner->sound_owner.spatial_context_valid ||
        current_map < 0 || current_map >= owner->dungeon.level_count ||
        audible_map < 0 || audible_map >= owner->dungeon.level_count ||
        alternate_map < -1 || alternate_map >= owner->dungeon.level_count ||
        owner->dungeon.map_offset_x[current_map] < 0 ||
        owner->dungeon.map_offset_x[current_map] > UINT8_MAX ||
        owner->dungeon.map_offset_y[current_map] < 0 ||
        owner->dungeon.map_offset_y[current_map] > UINT8_MAX ||
        owner->dungeon.map_offset_x[audible_map] < 0 ||
        owner->dungeon.map_offset_x[audible_map] > UINT8_MAX ||
        owner->dungeon.map_offset_y[audible_map] < 0 ||
        owner->dungeon.map_offset_y[audible_map] > UINT8_MAX) {
        return 0;
    }
    sound = &owner->sound_owner;
    /* Map_definitions::MapOffsetX/Y are b6/b7, represented by the parsed
     * File_header fields below. c_sfx.cpp translates a noise by exactly
     * current-origin minus audible-origin before its direction transform. */
    sound->spatial_current_map = (int16_t)current_map;
    sound->spatial_audible_map = (int16_t)audible_map;
    sound->spatial_alternate_map = (int16_t)alternate_map;
    sound->spatial_current_origin_x =
        (uint8_t)owner->dungeon.map_offset_x[current_map];
    sound->spatial_current_origin_y =
        (uint8_t)owner->dungeon.map_offset_y[current_map];
    sound->spatial_audible_origin_x =
        (uint8_t)owner->dungeon.map_offset_x[audible_map];
    sound->spatial_audible_origin_y =
        (uint8_t)owner->dungeon.map_offset_y[audible_map];
    hash = dm2_v1_game_load_owner_hash_step(hash, sound->receipt_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)current_map);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)audible_map);
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)(uint16_t)alternate_map);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        sound->spatial_current_origin_x);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        sound->spatial_current_origin_y);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        sound->spatial_audible_origin_x);
    hash = dm2_v1_game_load_owner_hash_step(hash,
        sound->spatial_audible_origin_y);
    if (hash == 0u) return 0;
    sound->spatial_context_hash = hash;
    sound->spatial_context_valid = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_scene(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GdatSceneM11CommandPlan scene_plan;
    DM2_V1_GdatSceneLightM11Receipt scene_light;
    DM2_V1_CLightSourceState source_light;
    DM2_V1_CLightM11Receipt c_light;


    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_light.valid ||
        owner->preselection_scene_materialized ||
        owner->champion_selection_materialized || owner->committed ||
        !owner->asset_loader || !owner->asset_loader->loaded ||
        owner->preselection_light.graphicsset > 15u) {
        return 0;
    }
    /* The source entrance descriptor has difficulty zero, so c_light takes
     * its fixed-map branch: level one, then v1e0978 and clamp.  The preceding
     * receipt retains every terminal field and guards against accidentally
     * applying that branch to a dynamic map. Source: sklight.cpp:24-198. */
    if (owner->preselection_light.map_descriptor.dynamic_light ||
        owner->preselection_light.v1e0978 > 12u) {
        return 0;
    }
    memset(&scene_plan, 0, sizeof(scene_plan));
    memset(&scene_light, 0, sizeof(scene_light));
    memset(&source_light, 0, sizeof(source_light));
    memset(&c_light, 0, sizeof(c_light));
    if (!dm2_v1_gdat_scene_m11_command_plan_build(owner->asset_loader,
            owner->preselection_light.graphicsset, &scene_plan) ||
        !scene_plan.valid ||
        scene_plan.graphicsset != owner->preselection_light.graphicsset ||
        !dm2_v1_gdat_scene_light_m11_receipt(&scene_plan, &scene_light) ||
        !scene_light.valid) {
        dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
        return 0;
    }
    source_light.valid = 1;
    source_light.dynamic_map = 0u;
    source_light.base_light = 1u;
    source_light.darkness_offset = (uint8_t)owner->preselection_light.v1e0978;
    source_light.source_state_hash = owner->preselection_light.source_state_hash;
    if (!dm2_v1_c_light_m11_receipt_build_for_map(&scene_light,
            &owner->preselection_light.map_descriptor, &source_light,
            &c_light) || !c_light.valid || c_light.light_level != 1u) {
        dm2_v1_gdat_scene_m11_command_plan_free(&scene_plan);
        return 0;
    }
    owner->preselection_scene_plan = scene_plan;
    owner->preselection_scene_light = scene_light;
    owner->preselection_c_light = c_light;
    owner->preselection_scene_materialized = 1;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_view(
    DM2_V1_GameLoadWorldOwner *owner)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const struct {
        uint8_t view_square;
        int8_t forward;
        int8_t lateral;
    } source_cells[DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT] = {
        { DM2_SQ_D0C, 1,  0 }, { DM2_SQ_D1C, 2,  0 },
        { DM2_SQ_D2C, 3,  0 }, { DM2_SQ_D0L, 0, -1 },
        { DM2_SQ_D0R, 0,  1 }, { DM2_SQ_D1L, 1, -1 },
        { DM2_SQ_D1R, 1,  1 }, { DM2_SQ_D2L, 2, -1 },
        { DM2_SQ_D2R, 2,  1 }, { DM2_SQ_D3L, 5, -2 },
        { DM2_SQ_D3R, 5,  2 },
    };
    DM2_V1_GameLoadPreselectionViewReceipt candidate;
    const int map = owner ? owner->source_party_map : -1;
    const int direction = owner ? owner->source_party_direction : -1;
    uint32_t hash = 0x56494557u; /* "VIEW" */

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_map_context_materialized ||
        !owner->preselection_local_graphics.valid ||
        !owner->preselection_light.valid ||
        !owner->preselection_scene_materialized ||
        owner->preselection_view.valid || owner->champion_selection_materialized ||
        owner->committed || map < 0 || map >= owner->dungeon.level_count ||
        direction < 0 || direction > 3 ||
        owner->preselection_local_graphics.map != map ||
        owner->preselection_light.map != map) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.map = map;
    candidate.party_x = owner->source_party_x;
    candidate.party_y = owner->source_party_y;
    candidate.party_direction = (uint8_t)direction;
    candidate.cell_count = DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT;
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)map);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.party_x);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.party_y);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.party_direction);

    for (int i = 0; i < DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT; ++i) {
        DM2_V1_GameLoadPreselectionViewCell *cell = &candidate.cells[i];
        const int map_x = (int)owner->source_party_x +
            dx[direction] * source_cells[i].forward -
            dy[direction] * source_cells[i].lateral;
        const int map_y = (int)owner->source_party_y +
            dy[direction] * source_cells[i].forward +
            dx[direction] * source_cells[i].lateral;
        const int raw = dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map,
                                                      map_x, map_y);
        const int tile_class = dm2_v1_dungeon_get_square_type(
            &owner->dungeon, map, map_x, map_y);
        const int square_type = tile_class < 0 ? -1 :
            dm2_v1_viewport_g1_tile_class_to_square_type((uint8_t)tile_class);
        const int ground_stack = dm2_v1_dungeon_get_first_thing(
            &owner->dungeon, map, map_x, map_y);

        if (map_x < INT16_MIN || map_x > INT16_MAX ||
            map_y < INT16_MIN || map_y > INT16_MAX) return 0;
        cell->view_square = source_cells[i].view_square;
        cell->map_x = (int16_t)map_x;
        cell->map_y = (int16_t)map_y;
        /* Out-of-map cells are part of the original projection.  Mark them
         * unavailable rather than supplying a wall or floor that the map did
         * not own.  A later renderer must preserve this no-draw state. */
        if (raw < 0 || square_type < 0 || ground_stack < INT16_MIN ||
            ground_stack > UINT16_MAX) {
            hash = dm2_v1_game_load_owner_hash_step(hash, cell->view_square);
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)cell->map_x);
            hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)cell->map_y);
            hash = dm2_v1_game_load_owner_hash_step(hash, 0u);
            hash = dm2_v1_game_load_owner_hash_step(hash, 0u);
            hash = dm2_v1_game_load_owner_hash_step(hash, 0u);
            hash = dm2_v1_game_load_owner_hash_step(hash, 0u);
            continue;
        }
        cell->source_available = 1u;
        cell->raw_tile = (uint16_t)raw;
        cell->ground_stack_root = (uint16_t)ground_stack;
        cell->square_type = (uint8_t)square_type;
        hash = dm2_v1_game_load_owner_hash_step(hash, cell->view_square);
        hash = dm2_v1_game_load_owner_hash_step(hash, cell->source_available);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)cell->map_x);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)cell->map_y);
        hash = dm2_v1_game_load_owner_hash_step(hash, cell->raw_tile);
        hash = dm2_v1_game_load_owner_hash_step(hash, cell->ground_stack_root);
        hash = dm2_v1_game_load_owner_hash_step(hash, cell->square_type);
    }
    if (hash == 0u) return 0;
    candidate.source_view_hash = hash;
    candidate.valid = 1;
    owner->preselection_view = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_materialize_preselection_viewport(
    DM2_V1_GameLoadWorldOwner *owner)
{
    DM2_V1_GameLoadPreselectionViewportReceipt candidate;
    uint32_t hash = 0x56505254u; /* "VPRT" */
    int i;

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->preselection_view.valid ||
        !owner->preselection_map_doors.committed ||
        !owner->preselection_scene_materialized ||
        !owner->preselection_scene_plan.valid ||
        !owner->preselection_c_light.valid ||
        owner->preselection_viewport.valid || owner->committed) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate.map = owner->preselection_view.map;
    candidate.map_data_hash = owner->preselection_entrance_map.map_data_hash;
    candidate.scene_control_hash = owner->preselection_c_light.scene_control_hash;
    candidate.c_light_hash = owner->preselection_c_light.receipt_hash;
    if (candidate.map != owner->source_party_map ||
        candidate.map_data_hash == 0u || candidate.scene_control_hash == 0u ||
        candidate.c_light_hash == 0u) return 0;
    hash = dm2_v1_game_load_owner_hash_step(hash, (uint32_t)candidate.map);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.map_data_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.scene_control_hash);
    hash = dm2_v1_game_load_owner_hash_step(hash, candidate.c_light_hash);
    for (i = 0; i < owner->preselection_view.cell_count; ++i) {
        const DM2_V1_GameLoadPreselectionViewCell *source =
            &owner->preselection_view.cells[i];
        DM2_ViewSquare *square;
        if (source->view_square >= DM2_SQ_COUNT) return 0;
        hash = dm2_v1_game_load_owner_hash_step(hash, source->view_square);
        hash = dm2_v1_game_load_owner_hash_step(hash, source->source_available);
        if (!source->source_available) continue;
        square = &candidate.squares[source->view_square];
        square->square_type = source->square_type;
        square->light_level = owner->preselection_c_light.light_level;
        square->sprite_depth = (int16_t)source->view_square;
        if (source->square_type == DM2_SQUARE_WALL) {
            square->flags = DM2_SQF_HAS_WALL;
        } else if (source->square_type == DM2_SQUARE_FLOOR ||
                   source->square_type == DM2_SQUARE_TELEPORTER) {
            square->flags = DM2_SQF_NONE;
        } else if (source->square_type == DM2_SQUARE_DOOR) {
            const DM2_V1_G1DirectDoorRoot *door = NULL;
            int door_state = (int)(source->raw_tile & 7u);

            if (!dm2_v1_g1_runtime_map_door_at(
                    &owner->preselection_map_doors, source->map_x,
                    source->map_y, &door) || !door ||
                door->door_type > 1u) {
                return 0;
            }
            if (door_state > 5) door_state = 4;
            square->flags = DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL;
            /* The renderer uses this bit as an authenticated direct-DB0
             * presence marker.  File_header has the same Door payload, not
             * a G1 fallback. */
            square->door_direct_g1_root = 1u;
            square->door_button = door->button;
            square->door_button_state = door->button_state;
            square->door_record_type = door->door_type;
            square->door_opening_dir = door->opening_dir;
            square->ornament_index = door->ornate_index;
            square->door_state = (uint8_t)door_state;
            square->door_open_pct = (uint8_t)(
                door_state == 5 ? 100 : (4 - door_state) * 25);
            if (owner->dungeon.map_use_door0[candidate.map] &&
                door->door_type == 0u) {
                square->door_gfx_index = (uint8_t)(
                    owner->dungeon.map_door_set0[candidate.map] & 0xff);
                square->door_gfx_admitted = 1u;
            } else if (owner->dungeon.map_use_door1[candidate.map] &&
                       door->door_type == 1u) {
                square->door_gfx_index = (uint8_t)(
                    owner->dungeon.map_door_set1[candidate.map] & 0xff);
                square->door_gfx_admitted = 1u;
            }
            if (door->ornate_index > 0u &&
                door->ornate_index <=
                    owner->preselection_local_graphics.door_ornate_count) {
                square->door_ornate_gfx_index =
                    owner->preselection_local_graphics.door_ornate_gfx[
                        door->ornate_index - 1u];
            }
        } else {
            /* Door, pit and trick-wall rendering needs the complete direct
             * record chain. Do not turn a raw tile byte into a drawable
             * surface in this pre-session owner. */
            return 0;
        }
        ++candidate.source_cell_count;
        hash = dm2_v1_game_load_owner_hash_step(hash, source->raw_tile);
        hash = dm2_v1_game_load_owner_hash_step(hash, source->ground_stack_root);
        hash = dm2_v1_game_load_owner_hash_step(hash, source->square_type);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->flags);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_gfx_index);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_gfx_admitted);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->ornament_index);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_ornate_gfx_index);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_button);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_button_state);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_record_type);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_opening_dir);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_state);
        hash = dm2_v1_game_load_owner_hash_step(hash, square->door_open_pct);
    }
    if (candidate.source_cell_count == 0u || hash == 0u) return 0;
    candidate.source_viewport_hash = hash;
    candidate.valid = 1;
    owner->preselection_viewport = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_turn_preselection(
    DM2_V1_GameLoadWorldOwner *owner, int source_event)
{
    DM2_V1_SkprojectTeleporterDetail direct_teleporter;
    DM2_V1_GameLoadPreselectionViewReceipt old_view;
    DM2_V1_GameLoadPreselectionViewportReceipt old_viewport;
    const int map = owner ? owner->source_party_map : -1;
    const int x = owner ? owner->source_party_x : -1;
    const int y = owner ? owner->source_party_y : -1;
    const int old_direction = owner ? owner->source_party_direction : -1;
    const int tile_class = owner ? dm2_v1_dungeon_get_square_type(
        &owner->dungeon, map, x, y) : -1;
    const int square_type = tile_class < 0 ? -1 :
        dm2_v1_viewport_g1_tile_class_to_square_type((uint8_t)tile_class);
    const int8_t old_staircase_flag = owner ? owner->source_staircase_flag : 0;
    const int8_t old_teleporter_map = owner ? owner->source_teleporter_map : -1;
    const int16_t old_display_x = owner ? owner->source_display_x : 0;
    const int16_t old_display_y = owner ? owner->source_display_y : 0;
    const uint8_t old_absdir = owner ? owner->source_party_absdir : 0u;
    const int8_t old_probe_direction = owner ?
        owner->source_teleporter_probe_direction : -1;
    const uint8_t old_source_direction = owner ?
        owner->source_teleporter_source_direction : 0u;
    const uint8_t old_destination_direction = owner ?
        owner->source_teleporter_destination_direction : 0u;
    const uint8_t old_display_pose_valid = owner ?
        owner->source_display_pose_valid : 0u;
    const DM2_V1_GameLoadMoverecState old_moverec = owner ?
        owner->source_moverec : (DM2_V1_GameLoadMoverecState){0};

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_preselection_ready ||
        !owner->source_map_context_materialized ||
        !owner->preselection_view.valid ||
        !owner->preselection_viewport.valid || owner->committed ||
        owner->champion_selection_materialized ||
        owner->selected_mirror_count != 0u ||
        (source_event != 1 && source_event != 2) ||
        map < 0 || map >= owner->dungeon.level_count ||
        x < 0 || y < 0 || old_direction < 0 || old_direction > 3 ||
        square_type != DM2_SQUARE_FLOOR) {
        return 0;
    }

    /* DM2_PERFORM_TURN_SQUAD asks GET_TELEPORTER_DETAIL before the first
     * moverec/rotate call.  A positive result invokes DM2_map_3BF83 and
     * returns instead of turning.  That map transition needs the complete
     * mutable session owner, so preserve this source branch by rejecting it
     * before any private pose or receipt mutation. */
    if (dm2_v1_game_load_owner_get_teleporter_detail(owner, map, x, y,
                                                      &direct_teleporter)) {
        return 0;
    }

    /* Before SELECT_CHAMPION, RESET_SQUAD_DIR and party.rotate have no hero
     * fields to mutate.  The rest is the source event's orientation change
     * and DM2_move_2fcf_0b8b's read-only teleporter/display probe against the
     * owned File_header map and record pools. */
    old_view = owner->preselection_view;
    old_viewport = owner->preselection_viewport;
    owner->source_party_direction = (uint8_t)((old_direction +
        (source_event == 2 ? 1 : 3)) & 3);
    if (!dm2_v1_game_load_owner_materialize_move_2fcf_0b8b(owner)) {
        goto rollback;
    }
    memset(&owner->preselection_view, 0, sizeof(owner->preselection_view));
    memset(&owner->preselection_viewport, 0,
           sizeof(owner->preselection_viewport));
    if (dm2_v1_game_load_world_owner_materialize_preselection_view(owner) &&
        dm2_v1_game_load_world_owner_materialize_preselection_viewport(owner)) {
        return 1;
    }

rollback:
    owner->source_party_direction = (uint8_t)old_direction;
    owner->source_staircase_flag = old_staircase_flag;
    owner->source_teleporter_map = old_teleporter_map;
    owner->source_display_x = old_display_x;
    owner->source_display_y = old_display_y;
    owner->source_party_absdir = old_absdir;
    owner->source_teleporter_probe_direction = old_probe_direction;
    owner->source_teleporter_source_direction = old_source_direction;
    owner->source_teleporter_destination_direction = old_destination_direction;
    owner->source_display_pose_valid = old_display_pose_valid;
    owner->source_moverec = old_moverec;
    owner->preselection_view = old_view;
    owner->preselection_viewport = old_viewport;
    return 0;
}

int dm2_v1_game_load_world_owner_move_preselection(
    DM2_V1_GameLoadWorldOwner *owner, int source_event)
{
    static const int8_t direction_x[4] = { 0, 1, 0, -1 };
    static const int8_t direction_y[4] = { -1, 0, 1, 0 };
    DM2_V1_SkprojectTeleporterDetail target_teleporter;
    DM2_V1_GameLoadPreselectionViewReceipt old_view;
    DM2_V1_GameLoadPreselectionViewportReceipt old_viewport;
    const int map = owner ? owner->source_party_map : -1;
    const int x = owner ? owner->source_party_x : -1;
    const int y = owner ? owner->source_party_y : -1;
    const int party_direction = owner ? owner->source_party_direction : -1;
    const int direction = party_direction < 0 || party_direction > 3 ? -1 :
        (party_direction + (source_event == 4 ? 1 :
                            source_event == 5 ? 2 :
                            source_event == 6 ? 3 : 0)) & 3;
    const int target_x = direction < 0 || direction > 3 ? -1 :
        x + direction_x[direction];
    const int target_y = direction < 0 || direction > 3 ? -1 :
        y + direction_y[direction];
    const int source_raw_tile = owner ? dm2_v1_dungeon_get_tile_raw(
        &owner->dungeon, map, x, y) : -1;
    const int raw_tile = owner ? dm2_v1_dungeon_get_tile_raw(
        &owner->dungeon, map, target_x, target_y) : -1;
    const int source_tile_class = source_raw_tile < 0 ? -1 :
        (source_raw_tile >> 5) & 7;
    const int tile_class = raw_tile < 0 ? -1 : (raw_tile >> 5) & 7;
    const int source_first_thing = source_raw_tile < 0 ? -2 :
        dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y);
    const int first_thing = raw_tile < 0 ? -2 :
        dm2_v1_dungeon_get_first_thing(&owner->dungeon, map,
                                       target_x, target_y);
    const uint8_t old_x = owner ? owner->source_party_x : 0u;
    const uint8_t old_y = owner ? owner->source_party_y : 0u;
    const int8_t old_staircase_flag = owner ? owner->source_staircase_flag : 0;
    const int8_t old_teleporter_map = owner ? owner->source_teleporter_map : -1;
    const int16_t old_display_x = owner ? owner->source_display_x : 0;
    const int16_t old_display_y = owner ? owner->source_display_y : 0;
    const uint8_t old_absdir = owner ? owner->source_party_absdir : 0u;
    const int8_t old_probe_direction = owner ?
        owner->source_teleporter_probe_direction : -1;
    const uint8_t old_source_direction = owner ?
        owner->source_teleporter_source_direction : 0u;
    const uint8_t old_destination_direction = owner ?
        owner->source_teleporter_destination_direction : 0u;
    const uint8_t old_display_pose_valid = owner ?
        owner->source_display_pose_valid : 0u;
    const DM2_V1_GameLoadMoverecState old_moverec = owner ?
        owner->source_moverec : (DM2_V1_GameLoadMoverecState){0};

    if (!dm2_v1_game_load_world_owner_is_prepared(owner) ||
        !owner->source_preselection_ready ||
        !owner->source_map_context_materialized ||
        !owner->preselection_view.valid ||
        !owner->preselection_viewport.valid || owner->committed ||
        owner->champion_selection_materialized ||
        owner->selected_mirror_count != 0u || map < 0 ||
        map >= owner->dungeon.level_count || x < 0 || y < 0 ||
        party_direction < 0 || party_direction > 3 ||
        (source_event < 3 || source_event > 6) || source_raw_tile < 0 ||
        raw_tile < 0 ||
        /* MOVE_RECORD_TO(OBJECT_NULL) runs c_moverec's departure path before
         * it examines the destination.  It is only source-no-op when the
         * departure has the same empty class-1 chain proof; otherwise a
         * type-14 missile or another linked record can invoke a real timer,
         * cut/append or actuator path. */
        source_tile_class != 1 || (source_raw_tile & 0x10) != 0 ||
        source_first_thing != DM2_V1_RECORD_HANDLE_END ||
        /* c_querydb.cpp::DM2_IS_TILE_BLOCKED accepts the tile type one
         * floor branch.  c_move.cpp::DM2_12b4_0881 may then reach its
         * no-creature result 6.  Retain only that exact no-record path;
         * a set 0x10 flag would require c_moverec/record ownership. */
        tile_class != 1 || (raw_tile & 0x10) != 0 ||
        first_thing != DM2_V1_RECORD_HANDLE_END) {
        return 0;
    }

    /* DM2_PERFORM_MOVE calls GET_TELEPORTER_DETAIL on the destination after
     * its no-creature path.  A positive detail enters DM2_map_3BF83 rather
     * than a plain coordinate step, so do not approximate that transition
     * in the pre-session owner. */
    if (dm2_v1_game_load_owner_get_teleporter_detail(owner, map, target_x,
                                                      target_y,
                                                      &target_teleporter)) {
        return 0;
    }

    old_view = owner->preselection_view;
    old_viewport = owner->preselection_viewport;
    /* DM2_PERFORM_MOVE's no-creature exit calls MOVE_RECORD_TO with
     * OBJECT_NULL and then establishes the new c_map pose.  The private
     * owner has no party record to cut or insert, so the source-visible
     * mutation here is exactly its authenticated pose; no tile/DB byte is
     * fabricated or changed.  Source: SKProject c_move.cpp:389-427,
     * 505-516. */
    owner->source_party_x = (uint8_t)target_x;
    owner->source_party_y = (uint8_t)target_y;
    if (!dm2_v1_game_load_owner_materialize_move_2fcf_0b8b(owner)) {
        goto rollback;
    }
    memset(&owner->preselection_view, 0, sizeof(owner->preselection_view));
    memset(&owner->preselection_viewport, 0,
           sizeof(owner->preselection_viewport));
    if (dm2_v1_game_load_world_owner_materialize_preselection_view(owner) &&
        dm2_v1_game_load_world_owner_materialize_preselection_viewport(owner)) {
        return 1;
    }

rollback:
    owner->source_party_x = old_x;
    owner->source_party_y = old_y;
    owner->source_staircase_flag = old_staircase_flag;
    owner->source_teleporter_map = old_teleporter_map;
    owner->source_display_x = old_display_x;
    owner->source_display_y = old_display_y;
    owner->source_party_absdir = old_absdir;
    owner->source_teleporter_probe_direction = old_probe_direction;
    owner->source_teleporter_source_direction = old_source_direction;
    owner->source_teleporter_destination_direction = old_destination_direction;
    owner->source_display_pose_valid = old_display_pose_valid;
    owner->source_moverec = old_moverec;
    owner->preselection_view = old_view;
    owner->preselection_viewport = old_viewport;
    return 0;
}

int dm2_v1_game_load_world_owner_advance_preselection(
    DM2_V1_GameLoadWorldOwner *owner)
{
    return dm2_v1_game_load_world_owner_move_preselection(owner, 3);
}

int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    return owner != NULL && dm2_v1_game_load_world_owner_is_initialized(owner) &&
        owner->prepared &&
        !owner->committed &&
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
                if ((uint16_t)raw_link == DM2_THING_NULL_MARKER) {
                    if (owner->dungeon.source_words_big_endian) continue;
                    goto fail;
                }
                link = (int16_t)(uint16_t)raw_link;
                /* Dungeon roots carry 0xfffe, while c_record returns the
                 * same terminal link as signed int16_t -2. Keep the walk in
                 * source link representation so it cannot chase -2 as DB15.
                 * SKProject c_record.cpp::DM2_GET_NEXT_RECORD_LINK. */
                while (link != DM2_V1_RECORD_HANDLE_END) {
                    int16_t next;
                    const int pool = dm2_v1_record_handle_pool(link);
                    if (++steps > max_chain_steps) goto fail;
                    if (owner->dungeon.source_words_big_endian) {
                        const uint8_t *link_record =
                            dm2_v1_record_pool_address(&owner->record_pools, link);
                        if (!link_record) goto fail;
                        next = (int16_t)(((uint16_t)link_record[0] << 8) |
                                         (uint16_t)link_record[1]);
                    } else if (!dm2_v1_record_pool_next_link(
                                   &owner->record_pools, link, &next)) {
                        goto fail;
                    }
                    if (pool == 3) {
                        uint8_t *record = dm2_v1_record_pool_address_mut(
                            &owner->record_pools, link);
                        uint16_t attributes;
                        uint8_t subtype;
                        int multiplier;
                        if (!record) goto fail;
                        attributes = owner->dungeon.source_words_big_endian
                            ? (uint16_t)(((uint16_t)record[2] << 8) | record[3])
                            : (uint16_t)record[2] |
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
                                    if (cadence == 0u) {
                                        goto fail;
                                    }
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
                                        &timer) < 0) {
                                        goto fail;
                                    }
                                    record[4] |= 0x01u;
                                    ++receipt.queued_timer_count;
                                }
                            }
                        }
                    }
                    if (next == DM2_V1_RECORD_HANDLE_NULL) {
                        goto fail;
                    }
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
    /* DME.h Actuator::Direction/Xcoord/Ycoord share w6.  Keep the timer
     * message on the same source-locked accessors as all other actuator
     * paths; the low bits of w6 are not the target x coordinate. */
    receipt.target_x = dm2_actu_xcoord(record);
    receipt.target_y = dm2_actu_ycoord(record);
    receipt.target_direction = dm2_actu_direction(record);
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
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        if (dm2_actu_type(record) != DM2_ACTU_CROSS_MAP) {
            result = count > 0 ? -1 : 0;
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

/* Source-complete COUNTER subset for an active WALL/FLOOR record chain.
 *
 * skevent.cpp::ACTUATE_{WALL,FLOOR}_MECHA (1834-1863) changes the DB3 Data
 * word, then may call DM2_INVOKE_ACTUATOR.  The latter only creates a 0x04
 * c_tim message from the same record's authenticated target fields.  This
 * private path therefore accepts a chain only when every resident record is
 * DB3 and every direction-matching actuator is a COUNTER.  Any other record
 * would need a different source subsystem, so it rejects before touching a
 * record or timer. */
static int dm2_v1_game_load_owner_dispatch_counter_chain(
    DM2_V1_GameLoadWorldOwner *owner, int map, int x, int y,
    uint8_t direction, uint8_t incoming_action,
    DM2_V1_GameLoadActuateReceipt *receipt)
{
    int chain_limit = 0;
    int16_t link;
    int16_t *links = NULL;
    uint16_t *new_data = NULL;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    int count = 0;
    int steps = 0;
    int result = 0;
    uint32_t hash = 0x434e5452u; /* "CNTR" */

    if (!owner || !receipt || incoming_action > 2u ||
        map < 0 || map >= owner->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    links = (int16_t *)calloc((size_t)chain_limit, sizeof(*links));
    new_data = (uint16_t *)calloc((size_t)chain_limit, sizeof(*new_data));
    messages = (DM2_V1_TimerEntry *)calloc((size_t)chain_limit,
                                            sizeof(*messages));
    if (!links || !new_data || !messages) goto done;

    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint16_t old_data;
        uint16_t data_after;
        int before_active;
        int after_active;
        int action_to_queue = -1;

        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= chain_limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&owner->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        if (dm2_actu_type(record) != DM2_ACTU_COUNTER) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        old_data = dm2_actu_data(record);
        data_after = old_data;
        before_active = old_data == 0u || (old_data & 0x0100u) != 0u;
        if (incoming_action == 1u) {
            data_after = (uint16_t)(old_data + 1u);
        } else if (incoming_action == 0u &&
                   (!dm2_actu_once_only(record) || old_data != 0u)) {
            data_after = (uint16_t)(old_data - 1u);
        }
        after_active = data_after == 0u || (data_after & 0x0100u) != 0u;
        if (after_active != before_active) {
            const uint8_t configured_action = dm2_actu_action_type(record);
            if (configured_action == 3u) {
                action_to_queue = dm2_actu_revert_effect(record) == after_active
                    ? 1 : 0;
            } else if (after_active) {
                action_to_queue = configured_action;
            }
            if (action_to_queue < 0 || action_to_queue > 2 ||
                dm2_actu_xcoord(record) >= owner->dungeon.level_widths[map] ||
                dm2_actu_ycoord(record) >= owner->dungeon.level_heights[map]) {
                result = -1;
                goto done;
            }
        }
        if (!dm2_v1_record_pool_address_mut(&owner->record_pools, link)) {
            result = -1;
            goto done;
        }
        links[count] = link;
        new_data[count] = data_after;
        if (action_to_queue >= 0) {
            DM2_V1_TimerEntry *message = &messages[count];
            dm2_v1_timer_entry_init(message);
            dm2_v1_timer_set_mticks(message, (int16_t)map,
                                    owner->timer_queue.gametick);
            message->ttype = 0x04u;
            message->actor = action_to_queue == 0 ? 1u :
                             (action_to_queue == 1 ? 3u : 2u);
            message->xA = (int8_t)dm2_actu_xcoord(record);
            message->yA = (int8_t)dm2_actu_ycoord(record);
            message->wvalueB = (int16_t)((uint16_t)dm2_actu_direction(record) |
                ((uint16_t)action_to_queue << 8));
        }
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, old_data);
        hash = dm2_v1_game_load_owner_hash_step(hash, data_after);
        hash = dm2_v1_game_load_owner_hash_step(hash,
            (uint32_t)(action_to_queue + 1));
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
        if (messages[i].ttype != 0u &&
            dm2_v1_timer_queue(&owner->timer_queue, &messages[i]) < 0) {
            memcpy(owner->timer_entries, timer_backup,
                   (size_t)owner->timer_capacity * sizeof(*timer_backup));
            memcpy(owner->timer_indices, index_backup,
                   (size_t)owner->timer_capacity * sizeof(*index_backup));
            owner->timer_queue = queue_backup;
            result = -1;
            goto done;
        }
    }
    for (int i = 0; i < count; ++i) {
        uint8_t *record = dm2_v1_record_pool_address_mut(&owner->record_pools,
                                                           links[i]);
        uint16_t old_data;
        if (!record) {
            memcpy(owner->timer_entries, timer_backup,
                   (size_t)owner->timer_capacity * sizeof(*timer_backup));
            memcpy(owner->timer_indices, index_backup,
                   (size_t)owner->timer_capacity * sizeof(*index_backup));
            owner->timer_queue = queue_backup;
            result = -1;
            goto done;
        }
        old_data = dm2_actu_data(record);
        if (old_data != new_data[i]) {
            dm2_actu_set_data(record, new_data[i]);
            ++receipt->counter_records_mutated;
        }
        if (messages[i].ttype != 0u) ++receipt->counter_messages_queued;
    }
    receipt->counter_actuators_seen = count;
    receipt->private_counter_hash = hash;
    result = 1;

done:
    free(index_backup);
    free(timer_backup);
    free(messages);
    free(new_data);
    free(links);
    return result;
}

/* Source-complete RELAY_1/RELAY_3 subset for a WALL/FLOOR DB3 chain.
 *
 * SKProject SKULLWIN/skevent.cpp::ACTIVATE_RELAY1 (1154-1183) has no
 * independent world mutation.  It applies the once/revert gate and creates
 * a delayed 0x04 message from the same authenticated actuator fields.
 * Keeping that exact message in the owner's dynamic c_tim heap is complete;
 * dispatching an unowned target family is deliberately still deferred. */
static int dm2_v1_game_load_owner_dispatch_relay_chain(
    DM2_V1_GameLoadWorldOwner *owner, int map, int x, int y,
    uint8_t direction, uint8_t incoming_action,
    DM2_V1_GameLoadActuateReceipt *receipt)
{
    int chain_limit = 0;
    int16_t link;
    DM2_V1_TimerEntry *messages = NULL;
    DM2_V1_TimerEntry *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    int count = 0;
    int message_count = 0;
    int steps = 0;
    int result = 0;
    uint32_t hash = 0x524c5931u; /* "RLY1" */

    if (!owner || !receipt || incoming_action > 2u ||
        map < 0 || map >= owner->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    messages = (DM2_V1_TimerEntry *)calloc((size_t)chain_limit,
                                            sizeof(*messages));
    if (!messages) goto done;

    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint8_t type;
        uint8_t action;
        uint32_t fire_tick;
        int blocked_by_source_gate = 0;

        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= chain_limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&owner->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if ((uint8_t)(link & 3) != direction) {
            ++steps;
            link = next;
            continue;
        }
        type = dm2_actu_type(record);
        if (type != DM2_ACTU_RELAY_1 && type != DM2_ACTU_RELAY_3) {
            result = count > 0 ? -1 : 0;
            goto done;
        }
        if (dm2_actu_once_only(record) != 0u &&
            (dm2_actu_revert_effect(record) != 0u || incoming_action != 0u) &&
            (dm2_actu_revert_effect(record) == 0u || incoming_action != 1u)) {
            blocked_by_source_gate = 1;
        }
        action = dm2_actu_once_only(record) != 0u ?
            dm2_actu_action_type(record) : incoming_action;
        if (!blocked_by_source_gate) {
            if (action > 2u ||
                dm2_actu_xcoord(record) >= owner->dungeon.level_widths[map] ||
                dm2_actu_ycoord(record) >= owner->dungeon.level_heights[map]) {
                result = -1;
                goto done;
            }
            if (type == DM2_ACTU_RELAY_3) {
                fire_tick = owner->timer_queue.gametick +
                    ((uint32_t)dm2_actu_data(record) << dm2_actu_delay(record));
            } else {
                fire_tick = owner->timer_queue.gametick +
                    (uint32_t)dm2_actu_data(record) + dm2_actu_delay(record);
            }
            dm2_v1_timer_entry_init(&messages[message_count]);
            dm2_v1_timer_set_mticks(&messages[message_count], (int16_t)map,
                                    fire_tick);
            messages[message_count].ttype = 0x04u;
            messages[message_count].actor = action == 0u ? 1u :
                (action == 1u ? 3u : 2u);
            messages[message_count].xA = (int8_t)dm2_actu_xcoord(record);
            messages[message_count].yA = (int8_t)dm2_actu_ycoord(record);
            messages[message_count].wvalueB = (int16_t)((uint16_t)
                dm2_actu_direction(record) | ((uint16_t)action << 8));
            ++message_count;
        }
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, dm2_actu_data(record));
        hash = dm2_v1_game_load_owner_hash_step(hash, type);
        hash = dm2_v1_game_load_owner_hash_step(hash, blocked_by_source_gate);
        ++count;
        ++steps;
        link = next;
    }
    if (count == 0 || hash == 0u) goto done;

    timer_backup = (DM2_V1_TimerEntry *)malloc(
        (size_t)owner->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc(
        (size_t)owner->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) { result = -1; goto done; }
    memcpy(timer_backup, owner->timer_entries,
           (size_t)owner->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, owner->timer_indices,
           (size_t)owner->timer_capacity * sizeof(*index_backup));
    queue_backup = owner->timer_queue;
    for (int i = 0; i < message_count; ++i) {
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
    receipt->relay_actuators_seen = count;
    receipt->relay_messages_queued = message_count;
    receipt->private_relay_hash = hash;
    result = 1;

done:
    free(index_backup);
    free(timer_backup);
    free(messages);
    return result;
}

/* FINITE_RELAY's deterministic arm in ACTUATE_{WALL,FLOOR}_MECHA decrements
 * Data only in [1,400], then invokes the record's source target immediately.
 * Values above 400 use RAND16 in the original and remain outside this private
 * owner until its random state belongs to the same GAME_LOAD transaction. */
static int dm2_v1_game_load_owner_dispatch_finite_relay_chain(
    DM2_V1_GameLoadWorldOwner *owner, int map, int x, int y,
    uint8_t direction, uint8_t incoming_action,
    DM2_V1_GameLoadActuateReceipt *receipt)
{
    int limit = 0, count = 0, message_count = 0, steps = 0, result = 0;
    int16_t link;
    int16_t *links = NULL;
    uint16_t *new_data = NULL;
    DM2_V1_TimerEntry *messages = NULL, *timer_backup = NULL;
    int16_t *index_backup = NULL;
    DM2_V1_TimerQueue queue_backup;
    uint32_t hash = 0x464e524cu; /* "FNRL" */

    if (!owner || !receipt || incoming_action > 2u || map < 0 ||
        map >= owner->dungeon.level_count) return -1;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            limit > INT_MAX - pool->record_count - pool->extension_count)
            return -1;
        limit += pool->record_count + pool->extension_count;
    }
    if (limit <= 0) return -1;
    link = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_END || link == DM2_V1_RECORD_HANDLE_NULL)
        return 0;
    links = (int16_t *)calloc((size_t)limit, sizeof(*links));
    new_data = (uint16_t *)calloc((size_t)limit, sizeof(*new_data));
    messages = (DM2_V1_TimerEntry *)calloc((size_t)limit, sizeof(*messages));
    if (!links || !new_data || !messages) goto done;
    while (link != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        uint16_t data;
        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= limit ||
            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
            !(record = dm2_v1_record_pool_address(&owner->record_pools, link)) ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, link, &next)) {
            result = count > 0 ? -1 : 0; goto done;
        }
        if ((uint8_t)(link & 3) != direction) { ++steps; link = next; continue; }
        if (dm2_actu_type(record) != DM2_ACTU_FINITE_RELAY) {
            result = count > 0 ? -1 : 0; goto done;
        }
        data = dm2_actu_data(record);
        if (data > 400u) { result = -1; goto done; }
        if (data != 0u) {
            if (dm2_actu_xcoord(record) >= owner->dungeon.level_widths[map] ||
                dm2_actu_ycoord(record) >= owner->dungeon.level_heights[map] ||
                !dm2_v1_record_pool_address_mut(&owner->record_pools, link)) {
                result = -1; goto done;
            }
            dm2_v1_timer_entry_init(&messages[message_count]);
            dm2_v1_timer_set_mticks(&messages[message_count], (int16_t)map,
                                    owner->timer_queue.gametick);
            messages[message_count].ttype = 0x04u;
            messages[message_count].actor = incoming_action == 0u ? 1u :
                (incoming_action == 1u ? 3u : 2u);
            messages[message_count].xA = (int8_t)dm2_actu_xcoord(record);
            messages[message_count].yA = (int8_t)dm2_actu_ycoord(record);
            messages[message_count].wvalueB = (int16_t)((uint16_t)
                dm2_actu_direction(record) | ((uint16_t)incoming_action << 8));
            ++message_count;
        }
        links[count] = link; new_data[count] = data == 0u ? 0u : (uint16_t)(data - 1u);
        hash = dm2_v1_game_load_owner_hash_step(hash, (uint16_t)link);
        hash = dm2_v1_game_load_owner_hash_step(hash, data);
        ++count; ++steps; link = next;
    }
    if (count == 0 || hash == 0u) goto done;
    timer_backup = (DM2_V1_TimerEntry *)malloc((size_t)owner->timer_capacity * sizeof(*timer_backup));
    index_backup = (int16_t *)malloc((size_t)owner->timer_capacity * sizeof(*index_backup));
    if (!timer_backup || !index_backup) { result = -1; goto done; }
    memcpy(timer_backup, owner->timer_entries, (size_t)owner->timer_capacity * sizeof(*timer_backup));
    memcpy(index_backup, owner->timer_indices, (size_t)owner->timer_capacity * sizeof(*index_backup));
    queue_backup = owner->timer_queue;
    for (int i = 0; i < message_count; ++i) if (dm2_v1_timer_queue(&owner->timer_queue, &messages[i]) < 0) {
        memcpy(owner->timer_entries, timer_backup, (size_t)owner->timer_capacity * sizeof(*timer_backup));
        memcpy(owner->timer_indices, index_backup, (size_t)owner->timer_capacity * sizeof(*index_backup));
        owner->timer_queue = queue_backup; result = -1; goto done;
    }
    for (int i = 0; i < count; ++i) {
        uint8_t *record = dm2_v1_record_pool_address_mut(&owner->record_pools, links[i]);
        if (!record) { result = -1; goto done; }
        if (dm2_actu_data(record) != new_data[i]) {
            dm2_actu_set_data(record, new_data[i]); ++receipt->finite_relay_records_mutated;
        }
    }
    receipt->finite_relay_actuators_seen = count;
    receipt->finite_relay_messages_queued = message_count;
    receipt->private_finite_relay_hash = hash;
    result = 1;
done:
    free(index_backup); free(timer_backup); free(messages); free(new_data); free(links);
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

    if (receipt.tile_class == 0u || receipt.tile_class == 1u) {
        const int counter = dm2_v1_game_load_owner_dispatch_counter_chain(
            owner, map, x, y, direction, action, &receipt);
        if (counter > 0) {
            receipt.valid = 1;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (counter < 0) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
    }

    if (receipt.tile_class == 0u || receipt.tile_class == 1u) {
        const int relay = dm2_v1_game_load_owner_dispatch_relay_chain(
            owner, map, x, y, direction, action, &receipt);
        if (relay > 0) {
            receipt.valid = 1;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (relay < 0) {
            receipt.blocked_incomplete_chain = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
    }

    if (receipt.tile_class == 0u || receipt.tile_class == 1u) {
        const int finite_relay = dm2_v1_game_load_owner_dispatch_finite_relay_chain(
            owner, map, x, y, direction, action, &receipt);
        if (finite_relay > 0) {
            receipt.valid = 1;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (finite_relay < 0) {
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
