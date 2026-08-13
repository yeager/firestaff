#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_data_tables_pc34_compat.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"
#include "dm2_v1_creature_something_pc34_compat.h"
#include "dm2_v1_fmtowns_graphics_dat.h"

#include <stdlib.h>
#include <limits.h>
#include <string.h>

/* Constructors accept caller storage that might not yet be initialised.
 * Inspect the first object-representation bytes through memcpy, rather than
 * reading an indeterminate uint32_t lvalue. */
static int dm2_v1_sksave_game_load_owner_is_initialized(
    const DM2_V1_SksaveGameLoadOwner *owner)
{
    uint32_t tag = 0u;

    if (!owner) return 0;
    memcpy(&tag, owner, sizeof(tag));
    return tag == DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG;
}

static int dm2_v1_sksave_owner_hero_alive(void *ctx, int hero_index)
{
    const DM2_V1_SksaveGameLoadOwner *owner =
        (const DM2_V1_SksaveGameLoadOwner *)ctx;
    return owner && hero_index >= 0 &&
        hero_index < (int)owner->state.champion_count &&
        owner->heroes[hero_index].curHP != 0;
}

static void dm2_v1_sksave_owner_add_ench_power(void *ctx, int hero_index,
                                                 int16_t amount)
{
    DM2_V1_SksaveGameLoadOwner *owner = (DM2_V1_SksaveGameLoadOwner *)ctx;
    if (owner && hero_index >= 0 &&
        hero_index < (int)owner->state.champion_count)
        owner->heroes[hero_index].ench_power = (int16_t)(
            owner->heroes[hero_index].ench_power + amount);
}

static void dm2_v1_sksave_owner_increment_poisoned(void *ctx, int hero_index)
{
    DM2_V1_SksaveGameLoadOwner *owner = (DM2_V1_SksaveGameLoadOwner *)ctx;
    if (owner && hero_index >= 0 &&
        hero_index < (int)owner->state.champion_count)
        owner->heroes[hero_index].poisoned = (int8_t)(
            owner->heroes[hero_index].poisoned + 1);
}

static void dm2_v1_sksave_owner_add_poison(void *ctx, int hero_index,
                                             int16_t amount)
{
    DM2_V1_SksaveGameLoadOwner *owner = (DM2_V1_SksaveGameLoadOwner *)ctx;
    if (owner && hero_index >= 0 &&
        hero_index < (int)owner->state.champion_count)
        owner->heroes[hero_index].poison = (int16_t)(
            owner->heroes[hero_index].poison + amount);
}

typedef struct {
    DM2_V1_SksaveGameLoadOwner *owner;
    int invalid;
} DM2_V1_SksaveOwnerTimerRebuildContext;

static void dm2_v1_sksave_owner_set_hero_timeridx(
    void *ctx, int hero_index, int16_t timer_index)
{
    DM2_V1_SksaveOwnerTimerRebuildContext *context =
        (DM2_V1_SksaveOwnerTimerRebuildContext *)ctx;
    if (!context || !context->owner || hero_index < 0 ||
        hero_index >= (int)context->owner->state.champion_count) {
        if (context) context->invalid = 1;
        return;
    }
    context->owner->heroes[hero_index].timeridx = timer_index;
}

static void dm2_v1_sksave_owner_set_record_timer_backlink(
    void *ctx, uint16_t link, int16_t timer_index)
{
    DM2_V1_SksaveOwnerTimerRebuildContext *context =
        (DM2_V1_SksaveOwnerTimerRebuildContext *)ctx;
    const int pool = dm2_v1_record_handle_pool((int16_t)link);
    uint8_t *record;

    if (!context || !context->owner || pool < 0 ||
        pool >= DM2_V1_RECORD_POOL_COUNT ||
        context->owner->record_pools.pools[pool].record_size < 8) {
        if (context) context->invalid = 1;
        return;
    }
    record = dm2_v1_record_pool_address_mut(&context->owner->record_pools,
                                             (int16_t)link);
    if (!record) {
        context->invalid = 1;
        return;
    }
    /* SKProject c_savegame.cpp::DM2_3a15_020f writes timer index at +6
     * for type 0x1d/0x1e records. */
    record[6] = (uint8_t)((uint16_t)timer_index & 0xffu);
    record[7] = (uint8_t)((uint16_t)timer_index >> 8);
}

static int dm2_v1_sksave_owner_rebuild_timer_backlinks(
    DM2_V1_SksaveGameLoadOwner *owner)
{
    DM2_V1_SksaveOwnerTimerRebuildContext context;
    DM2_V1_TimerRebuildCallbacks callbacks;
    DM2_V1_TimerRebuildReceipt receipt;

    if (!owner || !owner->state.valid || !owner->record_pools.valid ||
        owner->state.champion_count > DM2_MAX_HEROES ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX) return 0;
    memset(&context, 0, sizeof(context));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&receipt, 0, sizeof(receipt));
    context.owner = owner;
    callbacks.ctx = &context;
    callbacks.set_hero_timeridx = dm2_v1_sksave_owner_set_hero_timeridx;
    callbacks.set_record_timer_backlink =
        dm2_v1_sksave_owner_set_record_timer_backlink;
    if (dm2_v1_post_load_timer_rebuild(
            (const uint8_t *)owner->timers, owner->state.timer_count,
            owner->state.champion_count, &callbacks, &receipt) != 0 ||
        !receipt.valid || context.invalid) return 0;
    owner->receipt.hero_timeridx_cleared =
        (uint16_t)receipt.hero_timeridx_cleared;
    owner->receipt.hero_timeridx_set = (uint16_t)receipt.hero_timeridx_set;
    owner->receipt.ornate_timer_backlinks_set =
        (uint16_t)receipt.ornate_backlinks_set;
    return 1;
}

static int dm2_v1_sksave_owner_init_recycler_context(
    DM2_V1_SksaveGameLoadOwner *owner)
{
    const DM2_V1_OriginalRawDungeonReceipt *dungeon;
    const DM2_V1_SksaveMapOwner *map_owner;

    if (!owner || !owner->state.valid || !owner->map_owner.valid ||
        !owner->map_owner.dungeon) return 0;
    dungeon = &owner->state.dungeon;
    map_owner = &owner->map_owner;
    /* DM2_GAME_LOAD assigns ddat.v1e0266 from s_savegamebuffer.w_10 before
     * it calls DM2_READ_SKSAVE_DUNGEON. The current c_map can temporarily
     * differ while a resident chain asks ALLOC_NEW_RECORD; the recycler saves
     * and restores it, but skips v1e0266's party map during its first pass. */
    if (!dungeon->valid || dungeon != map_owner->dungeon ||
        owner->state.party_map >= dungeon->map_count ||
        owner->state.party_x >= dungeon->map_widths[owner->state.party_map] ||
        owner->state.party_y >= dungeon->map_heights[owner->state.party_map] ||
        map_owner->current_map < 0 || map_owner->current_map >= dungeon->map_count ||
        map_owner->column_index_count != dungeon->column_index_count ||
        map_owner->map_tiles_size != dungeon->map_data_byte_count) {
        return 0;
    }
    memset(&owner->recycler_context, 0, sizeof(owner->recycler_context));
    owner->recycler_context.valid = 1;
    owner->recycler_context.map_count = dungeon->map_count;
    owner->recycler_context.current_map = (uint16_t)map_owner->current_map;
    owner->recycler_context.party_map = owner->state.party_map;
    owner->recycler_context.party_x = owner->state.party_x;
    owner->recycler_context.party_y = owner->state.party_y;
    owner->recycler_context.party_direction = owner->state.party_direction;
    /* SKProject dm2data.cpp::init sets v1e0234=0 and v1e027c=0.  The
     * resume branch has not called DM2_move_2fcf_0b8b yet when it enters
     * DM2_READ_SKSAVE_DUNGEON, so c_record's recycler receives no protected
     * alternate map in this phase (c_savegame.cpp::DM2_GAME_LOAD:1476-1528;
     * c_move.cpp::DM2_move_2fcf_0b8b:961-1016). */
    owner->recycler_context.protected_map_active = 0;
    owner->recycler_context.protected_map = -1;
    owner->recycler_context.column_index_count = dungeon->column_index_count;
    owner->recycler_context.ground_stack_count =
        (uint16_t)map_owner->ground_stack_count;
    owner->recycler_context.map_data_byte_count = dungeon->map_data_byte_count;
    owner->recycler_context.column_index_hash = dungeon->column_index_hash;
    owner->recycler_context.ground_stack_hash = dungeon->ground_stack_hash;
    owner->recycler_context.map_data_hash = dungeon->map_data_hash;
    /* dm2data.cpp resets all 18 source cursors.  Do not derive cursors from
     * record counts or from a host free list. */
    memset(owner->recycler_context.map_cursors, 0,
           sizeof(owner->recycler_context.map_cursors));
    owner->recycler_context.recycle_blocked = 1;
    return 1;
}

int dm2_v1_sksave_game_load_owner_apply_post_load_global_effects(
    DM2_V1_SksaveGameLoadOwner *owner)
{
    DM2_V1_SaveGlobalEffectCallbacks callbacks;
    DM2_V1_GlobalEffectReceipt receipt;
    DM2_V1_Hero heroes_before[DM2_MAX_HEROES];
    uint8_t savegames1_before[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE];

    if (!owner || !owner->state.valid ||
        owner->state.champion_count > DM2_MAX_HEROES ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX) return 0;
    memcpy(heroes_before, owner->heroes, sizeof(heroes_before));
    memcpy(savegames1_before, owner->savegames1, sizeof(savegames1_before));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&receipt, 0, sizeof(receipt));
    callbacks.ctx = owner;
    callbacks.hero_is_alive = dm2_v1_sksave_owner_hero_alive;
    callbacks.add_hero_ench_power = dm2_v1_sksave_owner_add_ench_power;
    callbacks.increment_hero_poisoned = dm2_v1_sksave_owner_increment_poisoned;
    callbacks.add_hero_poison = dm2_v1_sksave_owner_add_poison;

    /* sksvgame.cpp:1047 clears exactly c_wbbb/savegames1 before it walks
     * c_tim.  No 0x0e callback is supplied: spell effects need a complete
     * dungeon/CCM/runtime owner and thus reject this private transaction. */
    memset(owner->savegames1, 0, sizeof(owner->savegames1));
    if (dm2_v1_post_load_global_effects(
            (const uint8_t *)owner->timers, owner->state.timer_count,
            owner->state.champion_count, &callbacks, &receipt) != 0) {
        memcpy(owner->heroes, heroes_before, sizeof(heroes_before));
        memcpy(owner->savegames1, savegames1_before, sizeof(savegames1_before));
        owner->global_effect_receipt = receipt;
        owner->global_effects_complete = 0;
        return 0;
    }
    owner->savegames1[0] = (uint8_t)((uint16_t)receipt.light_accumulator);
    owner->savegames1[1] = (uint8_t)((uint16_t)receipt.light_accumulator >> 8);
    owner->savegames1[2] = (uint8_t)receipt.attack_count;
    owner->global_effect_receipt = receipt;
    owner->global_effects_complete = 1;
    /* c_party::calc_player_weight also consults curacthero/curactmode and
     * hand_container for an open chest.  This owner retains neither active
     * eventqueue nor c_party hand state, so recomputation cannot be proven. */
    owner->weight_recompute_blocked = 1;
    return 1;
}

/* GAME_LOAD's three fixed globals sections, sksvgame.cpp:1512-1514:
 *     SUPPRESS_READER(v1e0104, mask, 1, 0x8)
 *     SUPPRESS_READER(globalb, mask, 1, 0x40)
 *     SUPPRESS_READER(globalw, mask, 2, 0x40)
 * i.e. 8, 64 and 128 bytes, matching the writer at :2235-2239 and the
 * orchestrator's ORCH_SUPPRESS(data, mask, elem, count) loops.
 *
 * dm2_suppress_reader_read indexes mask[i] for i < count, so count is a byte
 * total AND the mask must be that long. Passing the element sizes (1, 1, 2)
 * read only the first element of each section -- 4 bytes where the writer
 * emitted 200, leaving the stream 1568 bits short so heroes, savegames1,
 * timers and the record-link stream all decoded from the wrong offset.
 * Passing the totals instead would have run off the end of the 2-byte mask.
 * Loop per element, exactly like the writer. */
static int dm2_sksave_read_fixed_globals(
    DM2_SuppressReader *reader, const uint8_t *full_mask,
    uint8_t *v1e0104, size_t v1e0104_len,
    uint8_t *globalb, size_t globalb_len,
    uint8_t *globalw, size_t globalw_len)
{
    size_t i;
    for (i = 0u; i < v1e0104_len; ++i)
        if (dm2_suppress_reader_read(reader, full_mask, 1u, v1e0104 + i, 0u))
            return 1;
    for (i = 0u; i < globalb_len; ++i)
        if (dm2_suppress_reader_read(reader, full_mask, 1u, globalb + i, 0u))
            return 1;
    for (i = 0u; i + 1u < globalw_len; i += 2u)
        if (dm2_suppress_reader_read(reader, full_mask, 2u, globalw + i, 0u))
            return 1;
    return 0;
}

static int dm2_v1_sksave_owner_decode_fixed(
    DM2_V1_SksaveGameLoadOwner *owner, const uint8_t *raw_body,
    size_t raw_body_size)
{
    DM2_SuppressReader reader;
    uint8_t full_mask[2] = { 0xffu, 0xffu };
    const uint8_t *hero_mask;
    const uint8_t *state_mask;
    const uint8_t *timer_mask;
    size_t timer_mask_size = 0u;

    if (!owner || !raw_body || !owner->state.valid ||
        owner->state.dungeon.suppress_state_offset >= raw_body_size ||
        owner->state.champion_count > DM2_MAX_HEROES ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX) return 0;
    hero_mask = dm2_v1_save_mask_hero();
    state_mask = dm2_v1_save_mask_save_state();
    timer_mask = dm2_v1_save_vsgame_raw(&timer_mask_size);
    if (!hero_mask || !state_mask || !timer_mask || timer_mask_size < 12u)
        return 0;
    dm2_suppress_reader_init(&reader,
        raw_body + owner->state.dungeon.suppress_state_offset,
        raw_body_size - owner->state.dungeon.suppress_state_offset);
    if (dm2_suppress_reader_read(&reader, dm2_v1_save_mask_savegame_buffer(),
            sizeof(owner->savegame_buffer), owner->savegame_buffer, 0u) ||
        dm2_sksave_read_fixed_globals(&reader, full_mask,
                                      owner->v1e0104, sizeof(owner->v1e0104),
                                      owner->globalb, sizeof(owner->globalb),
                                      owner->globalw, sizeof(owner->globalw)))
        return 0;
    /* The downstream materializer repeats the exact hero/timer stream into
     * its source-owned owners. These reads establish the retained fixed
     * globals without seeking across a partially consumed SUPPRESS byte. */
    for (uint16_t i = 0; i < owner->state.champion_count; ++i)
        if (dm2_suppress_reader_read(&reader, hero_mask, sizeof(DM2_V1_Hero),
                (uint8_t *)&owner->heroes[i], 0u)) return 0;
    for (uint16_t i = 0; i < owner->state.champion_count; ++i)
        dm2_v1_hero_normalize_original_words(&owner->heroes[i],
            owner->state.dungeon.words_big_endian);
    if (dm2_suppress_reader_read(&reader, state_mask, sizeof(owner->savegames1),
            owner->savegames1, 0u)) return 0;
    memcpy(owner->source_savegames1, owner->savegames1,
           sizeof(owner->source_savegames1));
    owner->source_savegames1_valid = 1;
    for (uint16_t i = 0; i < owner->state.timer_count; ++i)
        if (dm2_suppress_reader_read(&reader, timer_mask, 12u,
                owner->timers[i].bytes, 0u)) return 0;
    if (owner->state.dungeon.words_big_endian) {
        for (uint16_t i = 0; i < owner->state.timer_count; ++i) {
            uint8_t *b = owner->timers[i].bytes;
            uint8_t t;
            t = b[0]; b[0] = b[3]; b[3] = t;
            t = b[1]; b[1] = b[2]; b[2] = t;
            t = b[6]; b[6] = b[7]; b[7] = t;
            t = b[8]; b[8] = b[9]; b[9] = t;
        }
    }
    return reader.position == owner->state.record_link_bitstream_offset -
        owner->state.dungeon.suppress_state_offset &&
        reader.bits_remaining == owner->state.record_link_bitstream_bits_remaining;
}

static int dm2_v1_sksave_owner_retain_creature_ai_flags(
    DM2_V1_SksaveGameLoadOwner *owner,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags, void *ctx,
    int strict)
{
    const DM2_V1_RecordPool *pool;
    if (!owner || !owner->record_pools.valid || !query_creature_ai_flags)
        return 0;
    pool = &owner->record_pools.pools[4];
    if (!pool->bytes || pool->record_size < 5 || pool->record_count < 0)
        return 0;
    for (size_t i = 0; i < (size_t)pool->record_count; ++i) {
        const uint8_t *record = pool->bytes + i * pool->record_size;
        const uint16_t link = (uint16_t)(0x1000u | (uint16_t)i);
        uint16_t flags = 0;
        /* The boundary owner is captured after the source has cleared the
         * dynamic DB4+ pools.  Its remaining bytes still carry the source
         * creature type, which the recycler may encounter through a static
         * possession tail.  Keep those authenticated type bytes for the
         * read-only audit; a complete owner remains limited to live rows. */
        if (record[0] == 0xffu && record[1] == 0xffu && strict) continue;
        {
            const DM2_AIDefinition *definition = NULL;
            const int source_ai_ok = owner->caii_source.valid &&
                dm2_v1_caii_source_owner_ai_spec_def(&owner->caii_source,
                    record[4], &definition) && definition;
            const int callback_ai_ok = !owner->caii_source.valid &&
                query_creature_ai_flags(ctx, link, record[4], &flags);
            if (source_ai_ok) {
                flags = definition->w0AIFlags;
            } else if (!callback_ai_ok) {
            /* A failed map restore can leave pool records whose source
             * liveness has not yet been reconstructed. For the narrow
             * recycler inspection we retain only admitted AI rows and reject
             * later if traversal actually needs a missing row. A complete
             * owner remains strict over every retained DB4 record. */
                if (strict) return 0;
                continue;
            }
        }
        owner->retained_creature_ai_flags[record[4]] = flags;
        owner->retained_creature_ai_valid[record[4]] = 1u;
    }
    return 1;
}

static int dm2_v1_sksave_owner_materialize_caii_capacity(
    DM2_V1_SksaveGameLoadOwner *owner) {
    const DM2_V1_RecordPool *db4;
    int nonstatic = 0;

    if (!owner || !owner->caii_source.valid ||
        !owner->record_pools.valid || owner->caii_capacity_valid)
        return 0;
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 6 || db4->record_count <= 0 ||
        db4->record_count > UINT16_MAX)
        return 0;
    for (int index = 0; index < db4->record_count; ++index) {
        const uint8_t *record = db4->bytes +
            (size_t)index * (size_t)db4->record_size;
        const DM2_AIDefinition *ai = NULL;
        uint16_t word = (uint16_t)record[0] |
            ((uint16_t)record[1] << 8);
        if (word == 0xffffu) continue;
        if (!dm2_v1_caii_source_owner_ai_spec_def(
                &owner->caii_source, record[4], &ai) || !ai)
            return 0;
        if ((ai->w0AIFlags & 1u) == 0u) ++nonstatic;
    }
    if (nonstatic > INT_MAX - 0x64) return 0;
    owner->caii_nonstatic_creature_count = (uint16_t)nonstatic;
    owner->caii_source_capacity = (uint16_t)((nonstatic + 0x64 <
        db4->record_count) ? nonstatic + 0x64 : db4->record_count);
    if (owner->caii_source_capacity == 0u) return 0;
    owner->caii_capacity_valid = 1;
    return 1;
}

static int16_t dm2_v1_sksave_owner_active_timer_count(
    const DM2_V1_SaveTimerRecord *timers)
{
    int16_t count = 0;
    if (!timers) return -1;
    for (uint16_t i = 0u; i < DM2_V1_SAVE_TIMER_MAX; ++i) {
        if (!dm2_v1_save_timer_is_no_type(&timers[i])) ++count;
    }
    return count;
}

static int dm2_v1_sksave_owner_find_record_position(
    const DM2_V1_SksaveGameLoadOwner *owner, uint16_t target,
    int *out_map, int *out_x, int *out_y)
{
    const DM2_V1_OriginalRawDungeonReceipt *dungeon;
    if (!owner || !owner->map_owner.valid || !owner->record_pools.valid ||
        !owner->state.dungeon.valid || !out_map || !out_x || !out_y) return 0;
    dungeon = &owner->state.dungeon;
    for (int map = 0; map < dungeon->map_count; ++map)
        for (int x = 0; x < dungeon->map_widths[map]; ++x)
            for (int y = 0; y < dungeon->map_heights[map]; ++y) {
                uint16_t link;
                int steps = 0;
                const size_t tile_offset =
                    (size_t)owner->map_owner.map_tile_offsets[map] +
                    (size_t)x * (size_t)dungeon->map_heights[map] +
                    (size_t)y;
                if ((owner->map_owner.map_tiles[tile_offset] & 0x10u) == 0u)
                    continue;
                if (!dm2_v1_sksave_map_owner_tile_record_link(
                        &owner->map_owner, map, x, y, &link)) return 0;
                while (link != (uint16_t)DM2_V1_RECORD_HANDLE_END &&
                       link != (uint16_t)DM2_V1_RECORD_HANDLE_NULL &&
                       steps++ < DM2_V1_SKSAVE_RECYCLE_MAX_STEPS) {
                    if (link == target) {
                        *out_map = map;
                        *out_x = x;
                        *out_y = y;
                        return 1;
                    }
                    {
                        int16_t next;
                        if (!dm2_v1_record_pool_next_link(
                                &owner->record_pools, (int16_t)link, &next))
                            return 0;
                        link = (uint16_t)next;
                    }
                }
                if (link != (uint16_t)DM2_V1_RECORD_HANDLE_END &&
                    link != (uint16_t)DM2_V1_RECORD_HANDLE_NULL) return 0;
            }
    return 0;
}

int dm2_v1_sksave_game_load_owner_caii_admission(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveCaiiAdmissionReceipt *out_receipt)
{
    DM2_V1_SksaveCaiiAdmissionReceipt receipt;
    const DM2_V1_RecordPool *db4;
    uint32_t hash = 2166136261u;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!owner || !owner->valid || !owner->map_owner.valid ||
        !owner->record_pools.valid || !owner->caii_source.valid ||
        owner->state.timer_count < 0 ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX) {
        return 0;
    }
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 14 || db4->record_count <= 0) {
        return 0;
    }
    receipt.map_owner_ready = 1;
    receipt.source_ai_ready = 1;
    for (int index = 0; index < db4->record_count; ++index) {
        const uint8_t *record = db4->bytes +
            (size_t)index * (size_t)db4->record_size;
        const DM2_AIDefinition *ai = NULL;
        const uint16_t handle = (uint16_t)(0x1000u | (uint16_t)index);
        int map;
        int x;
        int y;
        int timer_matches = 0;

        if (((uint16_t)record[0] | ((uint16_t)record[1] << 8)) == 0xffffu ||
            !dm2_v1_sksave_owner_find_record_position(owner, handle,
                                                       &map, &x, &y)) {
            continue;
        }
        ++receipt.live_db4_count;
        if (!dm2_v1_caii_source_owner_ai_spec_def(
                &owner->caii_source, record[4], &ai) || !ai) {
            receipt.source_ai_ready = 0;
            continue;
        }
        hash ^= (uint32_t)handle;
        hash *= 16777619u;
        hash ^= (uint32_t)((map << 16) | (x << 8) | y);
        hash *= 16777619u;
        hash ^= ai->w0AIFlags;
        hash *= 16777619u;
        if ((ai->w0AIFlags & 1u) != 0u) {
            ++receipt.static_candidate_count;
            /* SKProject c_1c9a.cpp::DM2_FILL_CAII_CUR_MAP asks the allocator
             * for a static creature whose DB4 byte@5 is 0xff, but
             * RESET_CAII clears that byte on the live load owner first.  The
             * saved value is therefore only a raw marker census here; an
             * existing byte@5 is not proof of a retained CAII slot. */
            if (record[5] == 0xffu)
                ++receipt.static_lazy_fill_candidate_count;
            else
                ++receipt.static_already_assigned_count;
            continue;
        }
        ++receipt.dynamic_candidate_count;
        for (int timer = 0; timer < owner->state.timer_count; ++timer) {
            const DM2_V1_SaveTimerRecord *saved = &owner->timers[timer];
            const uint8_t type = dm2_v1_save_timer_get_type(saved);
            const uint16_t packed = (uint16_t)dm2_v1_save_timer_get_a(saved);
            if ((type != 0x21u && type != 0x22u) ||
                dm2_v1_save_timer_get_map(saved) != (uint8_t)map ||
                dm2_v1_save_timer_get_actor(saved) != record[4] ||
                (packed & 0xffu) != (uint16_t)x ||
                ((packed >> 8) & 0xffu) != (uint16_t)y) continue;
            ++timer_matches;
        }
        if (timer_matches == 1) ++receipt.dynamic_timer_match_count;
        else if (timer_matches == 0) ++receipt.dynamic_timer_missing_count;
        else ++receipt.dynamic_timer_ambiguous_count;
    }
    receipt.source_hash = hash;
    receipt.valid = hash != 0u &&
        receipt.static_candidate_count ==
            receipt.static_lazy_fill_candidate_count +
            receipt.static_already_assigned_count;
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

int dm2_v1_sksave_game_load_owner_caii_reset_fill_preview(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveCaiiResetFillPreview *out_preview)
{
    DM2_V1_SksaveCaiiAdmissionReceipt admission;
    DM2_V1_SksaveCaiiResetFillPreview preview;
    const DM2_V1_RecordPool *db4;
    uint32_t hash = 2166136261u;

    memset(&preview, 0, sizeof(preview));
    if (out_preview) *out_preview = preview;
    memset(&admission, 0, sizeof(admission));
    if (!owner || !dm2_v1_sksave_game_load_owner_caii_admission(
            owner, &admission) || !admission.valid ||
        !owner->state.dungeon.valid || owner->state.dungeon.map_count == 0u) {
        return 0;
    }
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 6 || db4->record_count <= 0)
        return 0;

    /* Count the raw markers separately.  RESET_CAII will clear all of them
     * on the live owner, so this is provenance about the save bytes, not a
     * claim that any slot survives into the post-load CAII array. */
    for (int index = 0; index < db4->record_count; ++index) {
        const uint8_t *record = db4->bytes +
            (size_t)index * (size_t)db4->record_size;
        int map;
        int x;
        int y;
        const uint16_t handle = (uint16_t)(0x1000u | (uint16_t)index);
        if (((uint16_t)record[0] | ((uint16_t)record[1] << 8)) == 0xffffu ||
            !dm2_v1_sksave_owner_find_record_position(owner, handle,
                                                       &map, &x, &y))
            continue;
        if (record[5] != 0xffu) ++preview.raw_slot_marker_count;
    }
    preview.map_count = owner->state.dungeon.map_count;
    preview.creature_count = admission.live_db4_count;
    preview.static_fill_count = admission.static_candidate_count;
    preview.dynamic_activation_count = admission.dynamic_candidate_count;
    preview.think_timer_count_required =
        (uint16_t)(preview.static_fill_count +
                   preview.dynamic_activation_count);
    hash ^= preview.map_count; hash *= 16777619u;
    hash ^= preview.creature_count; hash *= 16777619u;
    hash ^= preview.static_fill_count; hash *= 16777619u;
    hash ^= preview.dynamic_activation_count; hash *= 16777619u;
    hash ^= preview.think_timer_count_required; hash *= 16777619u;
    hash ^= preview.raw_slot_marker_count; hash *= 16777619u;
    hash ^= admission.source_hash; hash *= 16777619u;
    preview.source_hash = hash;
    preview.valid = hash != 0u &&
        preview.creature_count == preview.static_fill_count +
            preview.dynamic_activation_count;
    if (out_preview) *out_preview = preview;
    return preview.valid;
}

int dm2_v1_sksave_game_load_owner_materialize_static_caii(
    DM2_V1_SksaveGameLoadOwner *owner)
{
    DM2_V1_RecordPool *db4;
    uint8_t *snapshot = NULL;
    size_t db4_bytes;
    uint32_t hash = 0x43535453u; /* "CSTS" */
    DM2_V1_DropRng rng_snapshot;
    int static_count = 0;

    if (!owner || !owner->valid || !owner->map_owner.valid ||
        !owner->record_pools.valid || !owner->record_pools.record_graph_complete ||
        !owner->caii_source.valid || !owner->asset_loader ||
        !owner->caii_capacity_valid || owner->caii_static_animation_valid)
        return 0;
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 14 || db4->record_count <= 0)
        return 0;
    db4_bytes = (size_t)db4->record_count * (size_t)db4->record_size;
    snapshot = malloc(db4_bytes);
    if (!snapshot) return 0;
    memcpy(snapshot, db4->bytes, db4_bytes);
    rng_snapshot = owner->caii_rng;
    if (!owner->caii_rng_initialized) {
        dm2_v1_drops_rng_init(&owner->caii_rng);
        owner->caii_rng_initialized = 1;
    }
    for (int index = 0; index < db4->record_count; ++index)
        db4->bytes[(size_t)index * (size_t)db4->record_size + 5u] = 0xffu;

    for (int index = 0; index < db4->record_count; ++index) {
        const uint16_t handle = (uint16_t)(0x1000u | (uint16_t)index);
        uint8_t *record = dm2_v1_record_pool_address_mut(
            &owner->record_pools, (int16_t)handle);
        const DM2_AIDefinition *ai = NULL;
        DM2_V1_SksaveRecordPositionReceipt position;
        uint16_t old_word;
        uint16_t adjacent_base;
        int16_t animation_word;
        const uint8_t *animation = NULL;
        DM2_V1_CreatureAnimFrameReceipt frame;

        if (!record || ((uint16_t)record[0] |
                        ((uint16_t)record[1] << 8)) == 0xffffu ||
            !dm2_v1_sksave_game_load_owner_record_position(
                owner, handle, &position) || !position.valid ||
            !dm2_v1_caii_source_owner_ai_spec_def(
                &owner->caii_source, record[4], &ai) || !ai ||
            (ai->w0AIFlags & 1u) == 0u)
            continue;
        ++static_count;
        old_word = (uint16_t)record[10] | ((uint16_t)record[11] << 8);
        adjacent_base = (uint16_t)record[8] | ((uint16_t)record[9] << 8);
        animation_word = (int16_t)old_word;
        memset(&frame, 0, sizeof(frame));
        if (owner->asset_loader->gdat_version == DM2_FMTOWNS_GDAT_VERSION)
            continue;
        if (dm2_v1_creature_get_animation_frame_with_ai_spec(
                owner->asset_loader, &owner->caii_rng, ai, record[4], 0x11,
                &adjacent_base, &animation_word, &animation,
                (uint16_t)record[12] | ((uint16_t)record[13] << 8),
                &frame) != 1 || !frame.valid || !frame.static_path ||
            animation != NULL) {
            if (frame.valid && !frame.static_path &&
                !frame.attribution_found && frame.return_value == 0) {
                continue;
            }
            goto rollback;
        }
        {
            uint16_t merged = (uint16_t)animation_word;
            merged |= (uint16_t)((old_word ^ merged) & 0x0060u);
            if ((old_word & 0x803fu) == 0x8001u)
                merged = (uint16_t)((merged & 0x7fc0u) | 0x8001u);
            record[10] = (uint8_t)merged;
            record[11] = (uint8_t)(merged >> 8);
            hash ^= handle; hash *= 16777619u;
            hash ^= old_word; hash *= 16777619u;
            hash ^= merged; hash *= 16777619u;
        }
    }
    if (hash == 0u || owner->caii_rng.random != 0u)
        goto rollback;
    hash ^= (uint32_t)static_count; hash *= 16777619u;
    owner->caii_static_animation_valid = 1;
    owner->caii_static_animation_hash = hash;
    free(snapshot);
    return 1;
rollback:
    memcpy(db4->bytes, snapshot, db4_bytes);
    owner->caii_rng = rng_snapshot;
    free(snapshot);
    return 0;
}

int dm2_v1_sksave_game_load_owner_materialize_dynamic_caii(
    DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDynamicCaiiReceipt *out_receipt)
{
    DM2_V1_GameLoadWorldOwner bridge;
    DM2_V1_GameLoadCaiiDynamicReceipt dynamic_receipt;
    DM2_V1_RecordPool *db4;
    DM2_V1_GameLoadCaiiMapCandidate *candidates = NULL;
    uint16_t candidate_count = 0u;
    uint16_t static_count = 0u;
    uint16_t dynamic_count = 0u;
    uint32_t transaction_hash;
    int ok = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&bridge, 0, sizeof(bridge));
    memset(&dynamic_receipt, 0, sizeof(dynamic_receipt));
    if (!owner || !owner->valid || !owner->source_dungeon_valid ||
        !owner->source_dungeon.raw_data || !owner->record_pools.valid ||
        !owner->record_pools.record_graph_complete ||
        !owner->caii_source.valid || !owner->asset_loader ||
        !owner->source_timer_owner_ready || !owner->runtime_timer_entries ||
        !owner->runtime_timer_indices || !owner->source_sound_materialized ||
        !owner->sound_owner.valid || !owner->sound_owner.runtime_queue_initialized ||
        !owner->caii_static_animation_valid || owner->caii_slots_valid) {
        return 0;
    }
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 14 || db4->record_count <= 0)
        return 0;
    candidates = calloc((size_t)db4->record_count, sizeof(*candidates));
    if (!candidates) return 0;

    for (int index = 0; index < db4->record_count; ++index) {
        const uint16_t handle = (uint16_t)(0x1000u | (uint16_t)index);
        uint8_t *record = dm2_v1_record_pool_address_mut(
            &owner->record_pools, (int16_t)handle);
        DM2_V1_SksaveRecordPositionReceipt position;
        const DM2_AIDefinition *ai = NULL;
        DM2_V1_GameLoadCaiiMapCandidate *candidate;

        if (!record || ((uint16_t)record[0] |
                        ((uint16_t)record[1] << 8)) == 0xffffu ||
            !dm2_v1_sksave_game_load_owner_record_position(
                owner, handle, &position) || !position.valid ||
            !dm2_v1_caii_source_owner_ai_spec_def(
                &owner->caii_source, record[4], &ai) || !ai)
            continue;
        candidate = &candidates[candidate_count++];
        candidate->map = (int16_t)position.map;
        candidate->x = position.x;
        candidate->y = position.y;
        candidate->record_handle = (int16_t)handle;
        candidate->creature_type = record[4];
        candidate->static_ai = (uint8_t)((ai->w0AIFlags & 1u) != 0u);
        candidate->record_word_a = (uint16_t)record[10] |
            ((uint16_t)record[11] << 8);
        candidate->packed_position = (uint16_t)record[12] |
            ((uint16_t)record[13] << 8);
        candidate->static_animation_frame = 0xffffu;
        if (candidate->static_ai) ++static_count;
        else ++dynamic_count;
    }
    if (candidate_count == 0u) {
        goto done;
    }

    bridge.lifecycle_tag = 0x474c574fu; /* GAME_LOAD_WORLD_OWNER */
    bridge.prepared = 1;
    bridge.source_preselection_ready = 1;
    bridge.current_map = owner->map_owner.current_map;
    bridge.source_transaction_hash = owner->state.fixed_sections_hash ^
        owner->state.timers_hash ^ owner->state.dungeon.prefix_hash;
    bridge.dungeon = owner->source_dungeon;
    bridge.record_pools = owner->record_pools;
    bridge.caii_source = owner->caii_source;
    bridge.caii_capacity.valid = owner->caii_capacity_valid;
    bridge.caii_capacity.db4_record_count = (uint16_t)db4->record_count;
    bridge.caii_capacity.nonstatic_creature_count =
        owner->caii_nonstatic_creature_count;
    bridge.caii_capacity.source_capacity = owner->caii_source_capacity;
    bridge.caii_capacity.source_hash = owner->state.dungeon.prefix_hash;
    bridge.caii_map_receipt.valid = 1;
    bridge.caii_map_receipt.map_count = owner->state.dungeon.map_count;
    bridge.caii_map_receipt.candidate_count = candidate_count;
    bridge.caii_map_receipt.static_candidate_count = static_count;
    bridge.caii_map_receipt.dynamic_candidate_count = dynamic_count;
    bridge.caii_map_receipt.source_hash = owner->state.dungeon.prefix_hash;
    bridge.caii_map_candidates = candidates;
    candidates = NULL;
    bridge.asset_loader = owner->asset_loader;
    bridge.dyn4_materialized = 1;
    bridge.dyn4_selector_count = 1u;
    bridge.validated_map_count = owner->state.dungeon.map_count;
    bridge.validated_world_hash = owner->state.dungeon.prefix_hash;
    bridge.sound_owner = owner->sound_owner;
    /* GAME_LOAD's SOUND9 tail binds the spatial gate before RESET_CAII.
     * SKSAVE already authenticated the current party map and the parsed
     * dungeon offsets, so retain that source-derived context on the bridge. */
    if (owner->state.party_map < 0 ||
        owner->state.party_map >= owner->source_dungeon.level_count ||
        owner->source_dungeon.map_offset_x[owner->state.party_map] < 0 ||
        owner->source_dungeon.map_offset_y[owner->state.party_map] < 0) {
        goto done;
    }
    bridge.sound_owner.spatial_current_map = (int16_t)owner->state.party_map;
    bridge.sound_owner.spatial_audible_map = (int16_t)owner->state.party_map;
    bridge.sound_owner.spatial_alternate_map = -1;
    bridge.sound_owner.spatial_current_origin_x =
        (uint8_t)owner->source_dungeon.map_offset_x[owner->state.party_map];
    bridge.sound_owner.spatial_current_origin_y =
        (uint8_t)owner->source_dungeon.map_offset_y[owner->state.party_map];
    bridge.sound_owner.spatial_audible_origin_x =
        bridge.sound_owner.spatial_current_origin_x;
    bridge.sound_owner.spatial_audible_origin_y =
        bridge.sound_owner.spatial_current_origin_y;
    bridge.sound_owner.spatial_context_hash = owner->state.dungeon.prefix_hash;
    bridge.sound_owner.spatial_context_valid = 1;
    bridge.timer_entries = owner->runtime_timer_entries;
    bridge.timer_indices = owner->runtime_timer_indices;
    bridge.timer_queue = owner->runtime_timer_queue;
    bridge.timer_capacity = owner->runtime_timer_capacity;
    bridge.source_party_map = owner->state.party_map;
    bridge.source_party_x = (uint8_t)owner->state.party_x;
    bridge.source_party_y = (uint8_t)owner->state.party_y;
    bridge.source_party_direction = (uint8_t)owner->state.party_direction;
    bridge.preselection_entrance.map = owner->state.party_map;
    bridge.caii_static_animation.valid = 1;
    bridge.caii_rng = owner->caii_rng;
    bridge.caii_rng_initialized = owner->caii_rng_initialized;
    dm2_v1_caii_array_init(&bridge.caii_slots,
                           (int)owner->caii_source_capacity);
    if (!bridge.caii_slots.valid) goto done;
    if (dynamic_count == 0u) {
        /* This save has only static CAII candidates.  RESET_CAII's dynamic
         * half has no work to perform; retain the authenticated empty slot
         * array so the later runtime candidate can still represent the
         * source state without inventing think timers or creature slots. */
        owner->caii_slots = bridge.caii_slots;
        memset(&bridge.caii_slots, 0, sizeof(bridge.caii_slots));
        owner->caii_slots_valid = 1;
        owner->caii_dynamic_valid = 1;
        owner->caii_dynamic_candidate_count = 0u;
        owner->caii_dynamic_allocated_slot_count = 0u;
        owner->caii_dynamic_think_timer_count = 0u;
        owner->caii_dynamic_noise_queue_count = 0u;
        owner->caii_dynamic_hash = owner->state.dungeon.prefix_hash;
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->source_hash = owner->caii_dynamic_hash;
        }
        ok = 1;
        goto done;
    }
    if (!dm2_v1_game_load_world_owner_materialize_caii_local_context(&bridge)) {
        goto done;
    }
    if (!dm2_v1_game_load_world_owner_materialize_dynamic_caii(
            &bridge, &dynamic_receipt)) {
        if (out_receipt) {
            out_receipt->blocked_unowned_0a48 =
                dynamic_receipt.blocked_unowned_0a48;
            out_receipt->failed_record_handle =
                dynamic_receipt.failed_record_handle;
            out_receipt->failed_creature_type =
                dynamic_receipt.failed_creature_type;
            out_receipt->failed_map = dynamic_receipt.failed_map;
            out_receipt->failed_x = dynamic_receipt.failed_x;
            out_receipt->failed_y = dynamic_receipt.failed_y;
            out_receipt->failed_0a48_result =
                dynamic_receipt.failed_0a48_result;
        }
        goto done;
    }

    owner->caii_slots = bridge.caii_slots;
    memset(&bridge.caii_slots, 0, sizeof(bridge.caii_slots));
    owner->caii_slots_valid = 1;
    owner->caii_rng = bridge.caii_rng;
    owner->caii_rng_initialized = bridge.caii_rng_initialized;
    owner->runtime_timer_queue = bridge.timer_queue;
    owner->sound_owner = bridge.sound_owner;
    owner->caii_dynamic_valid = 1;
    owner->caii_dynamic_candidate_count = dynamic_receipt.dynamic_candidate_count;
    owner->caii_dynamic_allocated_slot_count = dynamic_receipt.allocated_slot_count;
    owner->caii_dynamic_think_timer_count = dynamic_receipt.think_timer_count;
    owner->caii_dynamic_noise_queue_count = dynamic_receipt.noise_queue_count;
    owner->caii_dynamic_hash = dynamic_receipt.source_hash;
    transaction_hash = dynamic_receipt.source_hash;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->dynamic_candidate_count =
            dynamic_receipt.dynamic_candidate_count;
        out_receipt->allocated_slot_count =
            dynamic_receipt.allocated_slot_count;
        out_receipt->think_timer_count = dynamic_receipt.think_timer_count;
        out_receipt->noise_queue_count = dynamic_receipt.noise_queue_count;
        out_receipt->source_hash = transaction_hash;
    }
    ok = 1;
done:
    free(bridge.caii_local_contexts);
    free(bridge.caii_map_candidates);
    dm2_v1_caii_array_free(&bridge.caii_slots);
    free(candidates);
    return ok;
}

int dm2_v1_sksave_game_load_owner_materialize_runtime_caii(
    DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDynamicCaiiReceipt *out_receipt)
{
    DM2_V1_SksaveDynamicCaiiReceipt receipt;
    DM2_V1_RecordPool *db4;
    uint8_t *db4_snapshot = NULL;
    size_t db4_bytes = 0u;
    int reset_applied = 0;
    int prior_static_animation_valid = 0;
    uint32_t prior_static_animation_hash = 0u;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKProject startend.cpp::DM2_RESET_CAII is called only after
     * DM2_GAME_LOAD has restored the map, timers and SOUND9 prerequisites.
     * The old callers attempted dynamic CAII before those owners existed,
     * which made the otherwise source-valid 0a48 transaction roll back. */
    if (!owner || !owner->valid || !owner->source_dungeon_valid ||
        !owner->source_dungeon_tile_layout_valid ||
        !owner->source_timer_owner_ready ||
        !owner->source_sound_materialized || !owner->sound_owner.valid ||
        !owner->runtime_timer_entries || !owner->runtime_timer_indices ||
        !owner->caii_source.valid || !owner->record_pools.record_graph_complete) {
        return 0;
    }
    db4 = &owner->record_pools.pools[4];
    if (!db4->bytes || db4->record_size < 6 || db4->record_count <= 0)
        return 0;
    if (!owner->caii_slots_valid) {
        db4_bytes = (size_t)db4->record_count * (size_t)db4->record_size;
        db4_snapshot = malloc(db4_bytes);
        if (!db4_snapshot) return 0;
        memcpy(db4_snapshot, db4->bytes, db4_bytes);
        prior_static_animation_valid = owner->caii_static_animation_valid;
        prior_static_animation_hash = owner->caii_static_animation_hash;
        owner->caii_static_animation_valid = 0;
        owner->caii_static_animation_hash = 0u;
        /* startend.cpp::RESET_CAII clears the live DB4 CAII ownership byte
         * before FILL_CAII_CUR_MAP/FILL_ORPHAN_CAII walks every map.  The
         * save's byte is only a stale pre-load marker. */
        for (int index = 0; index < db4->record_count; ++index) {
            uint8_t *record = db4->bytes +
                (size_t)index * (size_t)db4->record_size;
            if (((uint16_t)record[0] | ((uint16_t)record[1] << 8)) != 0xffffu)
                record[5] = 0xffu;
        }
        reset_applied = 1;
        if (!dm2_v1_sksave_game_load_owner_materialize_static_caii(owner))
            goto rollback;
    }
    if (owner->caii_slots_valid) {
        receipt.valid = owner->caii_dynamic_valid;
        receipt.dynamic_candidate_count = owner->caii_dynamic_candidate_count;
        receipt.allocated_slot_count = owner->caii_dynamic_allocated_slot_count;
        receipt.think_timer_count = owner->caii_dynamic_think_timer_count;
        receipt.noise_queue_count = owner->caii_dynamic_noise_queue_count;
        receipt.source_hash = owner->caii_dynamic_hash;
        if (out_receipt) *out_receipt = receipt;
        free(db4_snapshot);
        return receipt.valid;
    }
    if (!dm2_v1_sksave_game_load_owner_materialize_dynamic_caii(
            owner, &receipt) || !receipt.valid) {
        goto rollback;
    }
    if (out_receipt) *out_receipt = receipt;
    free(db4_snapshot);
    return 1;

rollback:
    if (reset_applied && db4_snapshot) {
        memcpy(db4->bytes, db4_snapshot, db4_bytes);
        owner->caii_static_animation_valid = prior_static_animation_valid;
        owner->caii_static_animation_hash = prior_static_animation_hash;
    }
    free(db4_snapshot);
    return 0;
}

int dm2_v1_sksave_game_load_owner_record_position(
    const DM2_V1_SksaveGameLoadOwner *owner,
    uint16_t record_handle,
    DM2_V1_SksaveRecordPositionReceipt *out_receipt)
{
    DM2_V1_SksaveRecordPositionReceipt receipt;
    int map;
    int x;
    int y;
    uint32_t hash = 2166136261u;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!owner || !owner->valid || !owner->map_owner.valid ||
        !owner->record_pools.valid ||
        dm2_v1_record_handle_pool((int16_t)record_handle) != 4 ||
        !dm2_v1_record_pool_address(&owner->record_pools,
                                    (int16_t)record_handle) ||
        !dm2_v1_sksave_owner_find_record_position(owner, record_handle,
                                                   &map, &x, &y)) {
        return 0;
    }
    receipt.record_handle = record_handle;
    receipt.map = (uint16_t)map;
    receipt.x = (uint8_t)x;
    receipt.y = (uint8_t)y;
    hash ^= record_handle; hash *= 16777619u;
    hash ^= (uint32_t)((map << 16) | (x << 8) | y); hash *= 16777619u;
    hash ^= (uint32_t)owner->map_owner.column_index_count; hash *= 16777619u;
    hash ^= owner->map_owner.ground_stack_count; hash *= 16777619u;
    receipt.source_hash = hash;
    receipt.valid = hash != 0u;
    if (out_receipt) *out_receipt = receipt;
    return receipt.valid;
}

int dm2_v1_sksave_game_load_owner_schedule_think_timer(
    DM2_V1_SksaveGameLoadOwner *owner,
    uint16_t record_handle,
    uint16_t map,
    uint8_t x,
    uint8_t y,
    DM2_V1_CreatureScheduleReceipt *out_receipt)
{
    DM2_V1_CreatureScheduleReceipt receipt;
    DM2_V1_SksaveRecordPositionReceipt position;
    const uint8_t *record;
    DM2_V1_TimerEntry timer;
    int16_t ticket;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!owner || !owner->valid || owner->source_game_load_session_ready ||
        !owner->source_timer_owner_ready ||
        !owner->runtime_timer_queue.entries ||
        !owner->runtime_timer_queue.indices || map > 0xffu ||
        !dm2_v1_sksave_game_load_owner_record_position(
            owner, record_handle, &position) || !position.valid ||
        position.map != map || position.x != x || position.y != y ||
        !(record = dm2_v1_record_pool_address(&owner->record_pools,
                                               (int16_t)record_handle))) {
        return 0;
    }
    receipt.resolved = 1;
    receipt.map_id = map;
    receipt.creature_type = record[4];
    receipt.has_group_link =
        (((uint16_t)record[8] | ((uint16_t)record[9] << 8)) != 0xffffu);
    receipt.timer_type = receipt.has_group_link ? 0x22 : 0x21;
    receipt.due_tick = (unsigned long)owner->runtime_timer_queue.gametick + 1u;
    memset(&timer, 0, sizeof(timer));
    dm2_v1_timer_set_mticks(&timer, (int16_t)map,
                            (int32_t)receipt.due_tick);
    timer.ttype = (uint8_t)receipt.timer_type;
    timer.actor = (uint8_t)receipt.creature_type;
    timer.xA = (int8_t)x;
    timer.yA = (int8_t)y;
    ticket = dm2_v1_timer_queue(&owner->runtime_timer_queue, &timer);
    if (ticket < 0) return 0;
    receipt.timer_ticket = (uint32_t)(uint16_t)ticket;
    receipt.enqueued = 1;
    receipt.valid = 1;
    snprintf(receipt.source_evidence, sizeof(receipt.source_evidence),
             "SKSAVE owner-bound c_1c9a.cpp:5695-5728 direct-handle "
             "0x%02x/0x%02x think timer; c_map position authenticated",
             receipt.timer_type, receipt.creature_type);
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_sksave_game_load_owner_init(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    int words_big_endian,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx)
{
    DM2_V1_SksaveGameLoadOwner candidate;
    const int owner_was_initialized =
        dm2_v1_sksave_game_load_owner_is_initialized(owner);

    if (!owner) return 0;
    if (!owner_was_initialized)
        memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    candidate.lifecycle_tag = DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG;
    candidate.timer_queue_count = -1;
    candidate.timer_free_head = -1;
    candidate.leader_hand_root = DM2_V1_RECORD_HANDLE_END;
    {
        const char *failure = NULL;
        if (!raw_body || !savegamew7 ||
            !dm2_v1_original_raw_sksave_fixed_state_receipt(raw_body,
                raw_body_size, &candidate.state) || !candidate.state.valid) {
            failure = "fixed-state";
        } else if (!dm2_v1_sksave_owner_decode_fixed(
                       &candidate, raw_body, raw_body_size)) {
            failure = "fixed-decode";
        } else if (!dm2_v1_record_pool_materialize_raw_sksave_game_load_owner(
                       &candidate, raw_body, raw_body_size, savegamew7,
                       asset_loader, query_creature_ai_flags,
                       query_creature_ai_flags_ctx, &candidate.receipt)) {
            failure = "materialize";
        } else if (!dm2_v1_caii_source_owner_init_from_record_pool(
                       &candidate.caii_source, asset_loader,
                       &candidate.record_pools.pools[4])) {
            failure = "caii-source";
        } else if (!dm2_v1_sksave_owner_materialize_caii_capacity(&candidate)) {
            failure = "caii-capacity";
        } else if ((memset(candidate.retained_creature_ai_flags, 0,
                           sizeof(candidate.retained_creature_ai_flags)),
                    memset(candidate.retained_creature_ai_valid, 0,
                           sizeof(candidate.retained_creature_ai_valid)),
                    !dm2_v1_sksave_owner_retain_creature_ai_flags(
                        &candidate, query_creature_ai_flags,
                        query_creature_ai_flags_ctx, 1))) {
            failure = "creature-ai";
        } else if (!dm2_v1_sksave_game_load_owner_apply_post_load_global_effects(
                       &candidate)) {
            failure = "global-effects";
        } else if (!dm2_v1_sksave_owner_rebuild_timer_backlinks(&candidate)) {
            failure = "timer-backlinks";
        } else if (!dm2_v1_sksave_owner_init_recycler_context(&candidate)) {
            failure = "recycler-context";
        }
        if (failure) {
        dm2_v1_sksave_game_load_owner_free(&candidate);
        if (!owner_was_initialized) memset(owner, 0, sizeof(*owner));
        return 0;
        }
    }
    candidate.savegamew7 = savegamew7;
    candidate.asset_loader = asset_loader;
    candidate.valid = 1;
    candidate.source_game_load_session_ready = 0;
    if (owner_was_initialized)
        dm2_v1_sksave_game_load_owner_free(owner);
    *owner = candidate;
    /* map_owner is part of the candidate value-copy, so rebind its internal
     * dungeon receipt after the stack candidate has moved into caller-owned
     * storage. Without this, map 0 reads stale stack bytes and appears to
     * have width zero even though state.dungeon is authenticated. */
    owner->map_owner.dungeon = &owner->state.dungeon;
    return 1;
}

int dm2_v1_sksave_game_load_owner_init_to_recycler_boundary_ordered(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    int words_big_endian,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx)
{
    DM2_V1_SksaveGameLoadOwner candidate;
    const int owner_was_initialized =
        dm2_v1_sksave_game_load_owner_is_initialized(owner);

    if (!owner) return 0;
    if (!owner_was_initialized) memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    candidate.lifecycle_tag = DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG;
    candidate.timer_queue_count = -1;
    candidate.timer_free_head = -1;
    candidate.leader_hand_root = DM2_V1_RECORD_HANDLE_END;
    if (!raw_body || !savegamew7 ||
        !dm2_v1_original_raw_sksave_fixed_state_receipt_ordered(raw_body,
            raw_body_size, words_big_endian, &candidate.state) || !candidate.state.valid ||
        !dm2_v1_sksave_owner_decode_fixed(&candidate, raw_body, raw_body_size)) {
        dm2_v1_sksave_game_load_owner_free(&candidate);
        if (!owner_was_initialized) memset(owner, 0, sizeof(*owner));
        return 0;
    }
    {
        const int materialize_rc =
            dm2_v1_record_pool_materialize_raw_sksave_game_load_owner(
                &candidate, raw_body, raw_body_size, savegamew7, asset_loader,
                query_creature_ai_flags, query_creature_ai_flags_ctx,
                &candidate.receipt);
        if (materialize_rc || candidate.receipt.failure_stage !=
                DM2_V1_SKSAVE_PREFLIGHT_FAILURE_MAPS ||
            (candidate.receipt.recycle_required_db != 0 &&
             candidate.receipt.recycle_required_db != 2 &&
             candidate.receipt.recycle_required_db != 3 &&
             candidate.receipt.recycle_required_db != 5 &&
             candidate.receipt.recycle_required_db != 6 &&
             candidate.receipt.recycle_required_db != 7 &&
             candidate.receipt.recycle_required_db != 8 &&
             candidate.receipt.recycle_required_db != 9 &&
             candidate.receipt.recycle_required_db != 10) ||
            candidate.receipt.map_failure_record_reason !=
                DM2_READ_RECORD_FAILURE_ALLOC || !candidate.map_owner.valid ||
            !candidate.record_pools.valid ||
            !dm2_v1_sksave_owner_retain_creature_ai_flags(&candidate,
                query_creature_ai_flags, query_creature_ai_flags_ctx, 0) ||
            !dm2_v1_sksave_owner_init_recycler_context(&candidate)) {
            dm2_v1_sksave_game_load_owner_free(&candidate);
            if (!owner_was_initialized) memset(owner, 0, sizeof(*owner));
            return 0;
        }
    }
    candidate.savegamew7 = savegamew7;
    candidate.valid = 0;
    candidate.recycler_boundary_inspection_valid = 1;
    candidate.source_game_load_session_ready = 0;
    if (owner_was_initialized) dm2_v1_sksave_game_load_owner_free(owner);
    *owner = candidate;
    owner->map_owner.dungeon = &owner->state.dungeon;
    return 1;
}

int dm2_v1_sksave_game_load_owner_init(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx)
{
    return dm2_v1_sksave_game_load_owner_init_ordered(
        owner, raw_body, raw_body_size, savegamew7, 0, asset_loader,
        query_creature_ai_flags, query_creature_ai_flags_ctx);
}

int dm2_v1_sksave_game_load_owner_init_to_recycler_boundary(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx)
{
    return dm2_v1_sksave_game_load_owner_init_to_recycler_boundary_ordered(
        owner, raw_body, raw_body_size, savegamew7, 0, asset_loader,
        query_creature_ai_flags, query_creature_ai_flags_ctx);
}

int dm2_v1_sksave_game_load_owner_creature_ai_flags(
    const DM2_V1_SksaveGameLoadOwner *owner, uint8_t creature_type,
    uint16_t *out_flags)
{
    if (out_flags) *out_flags = 0;
    if (!owner || !out_flags ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid) ||
        !owner->retained_creature_ai_valid[creature_type]) return 0;
    *out_flags = owner->retained_creature_ai_flags[creature_type];
    return 1;
}

int dm2_v1_sksave_game_load_owner_copy_source_savegames1(
    const DM2_V1_SksaveGameLoadOwner *owner,
    uint8_t out_savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE]) {
    if (!owner || !out_savegames1 || !owner->source_savegames1_valid ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid))
        return 0;
    memcpy(out_savegames1, owner->source_savegames1,
           DM2_V1_ORIGINAL_SAVEGAMES1_SIZE);
    return 1;
}

int dm2_v1_sksave_game_load_owner_bind_dungeon_layout(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_dungeon, size_t raw_dungeon_size)
{
    DM2_V1_DungeonData candidate;
    uint32_t tile_hash = 2166136261u;
    if (!owner || !owner->valid || !owner->map_owner.valid ||
        !owner->state.dungeon.valid || !raw_dungeon || raw_dungeon_size == 0u ||
        raw_dungeon_size > (size_t)INT_MAX) return 0;
    memset(&candidate, 0, sizeof(candidate));
    if (dm2_v1_dungeon_load(&candidate, raw_dungeon,
                            (int)raw_dungeon_size) != 0 ||
        candidate.level_count != (int)owner->state.dungeon.map_count ||
        !candidate.record_graph_complete) {
        dm2_v1_dungeon_free(&candidate);
        return 0;
    }
    for (int map = 0; map < candidate.level_count; ++map) {
        if (candidate.level_widths[map] !=
                (int)owner->state.dungeon.map_widths[map] ||
            candidate.level_heights[map] !=
                (int)owner->state.dungeon.map_heights[map]) {
            dm2_v1_dungeon_free(&candidate);
            return 0;
        }
        for (int x = 0; x < candidate.level_widths[map]; ++x)
            for (int y = 0; y < candidate.level_heights[map]; ++y) {
                const int dungeon_tile = dm2_v1_dungeon_get_tile_raw(
                    &candidate, map, x, y);
                const uint8_t save_tile = owner->map_owner.map_tiles[
                    (size_t)owner->map_owner.map_tile_offsets[map] +
                    (size_t)x * (size_t)candidate.level_heights[map] +
                    (size_t)y];
                if (dungeon_tile < 0) {
                    dm2_v1_dungeon_free(&candidate);
                    return 0;
                }
                /* DMWeb's SKSAVE tile stream preserves the source tile type
                 * in bits 5..7, while bits 0..4 are save/runtime fields
                 * (terrain state, object-list and related flags). */
                if (((uint8_t)dungeon_tile & 0xe0u) != (save_tile & 0xe0u)) {
                    dm2_v1_dungeon_free(&candidate);
                    return 0;
                }
                tile_hash ^= (uint32_t)((uint8_t)dungeon_tile & 0xe0u);
                tile_hash *= 16777619u;
            }
    }
    if (owner->source_dungeon_valid)
        dm2_v1_dungeon_free(&owner->source_dungeon);
    owner->source_dungeon = candidate;
    owner->source_dungeon_valid = 1;
    owner->source_dungeon_tile_layout_valid = 1;
    owner->source_dungeon_tile_layout_hash = tile_hash;
    return 1;
}

int dm2_v1_sksave_game_load_owner_bind_sound(
    DM2_V1_SksaveGameLoadOwner *owner,
    const DM2_V1_AssetLoader *loader, uint16_t selector_count,
    const uint32_t *selector_ids,
    const DM2_V1_GdatDyn4MaterializedSelection *selections)
{
    DM2_V1_GameLoadSoundOwner candidate;

    if (!owner || !owner->valid || !owner->source_dungeon_valid ||
        !owner->source_dungeon_tile_layout_valid || !loader ||
        selector_count == 0u || !selector_ids || !selections) return 0;
    owner->asset_loader = loader;
    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_game_load_sound_owner_materialize_from_dyn4(
            loader, selector_count, selector_ids, selections, &candidate)) {
        return 0;
    }
    dm2_v1_game_load_sound_owner_free(&owner->sound_owner);
    owner->sound_owner = candidate;
    owner->source_sound_materialized = 1;
    return 1;
}

int dm2_v1_sksave_game_load_owner_bind_current_map_sound(
    DM2_V1_SksaveGameLoadOwner *owner,
    const DM2_V1_AssetLoader *loader)
{
    uint32_t *selector_ids = NULL;
    DM2_V1_GdatDyn4MaterializedSelection *selections = NULL;
    DM2_V1_GdatDyn4SoundState dyn4_state;
    uint16_t selector_count = 0u;
    uint16_t selector_capacity;
    int fixed_selections_ready = 0;
    int map;
    int width;
    int height;
    int x;
    int y;
    int ok = 0;

    if (!owner || !owner->valid || !owner->source_dungeon_valid ||
        !owner->source_dungeon_tile_layout_valid || !loader ||
        !loader->loaded || !owner->map_owner.valid ||
        owner->map_owner.current_map < 0 ||
        owner->map_owner.current_map >= owner->source_dungeon.level_count) {
        return 0;
    }
    map = owner->map_owner.current_map;
    width = owner->source_dungeon.level_widths[map];
    height = owner->source_dungeon.level_heights[map];
    selector_capacity = (uint16_t)owner->record_pools.pools[3].record_count;
    if (width <= 0 || height <= 0 || selector_capacity == 0u) return 0;
    selector_ids = calloc(selector_capacity, sizeof(*selector_ids));
    selections = calloc(selector_capacity, sizeof(*selections));
    if (!selector_ids || !selections) goto done;
    dm2_v1_gdat_dyn4_sound_state_init(&dyn4_state);

    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            const size_t tile_offset =
                (size_t)owner->map_owner.map_tile_offsets[map] +
                (size_t)x * (size_t)height + (size_t)y;
            uint16_t root_link;
            int16_t link;
            if ((owner->map_owner.map_tiles[tile_offset] & 0x10u) == 0u ||
                !dm2_v1_sksave_map_owner_tile_record_link(
                    &owner->map_owner, map, x, y, &root_link)) {
                continue;
            }
            link = (int16_t)root_link;
            while (link != DM2_V1_RECORD_HANDLE_END) {
                const int pool_index = dm2_v1_record_handle_pool(
                    (int16_t)link);
                const DM2_V1_RecordPool *pool;
                const uint8_t *record;
                int16_t next;
                uint16_t word2;
                uint8_t subtype;
                uint8_t selector;
                uint32_t resource_id;
                uint16_t i;

                if (link == DM2_V1_RECORD_HANDLE_NULL || pool_index < 0 ||
                    pool_index >= DM2_V1_RECORD_POOL_COUNT ||
                    !(record = dm2_v1_record_pool_address(
                        &owner->record_pools, (int16_t)link)) ||
                    (pool = &owner->record_pools.pools[pool_index],
                     pool->record_size < 4) ||
                    !dm2_v1_record_pool_next_link(
                        &owner->record_pools, link,
                        &next)) {
                    goto done;
                }
                word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                subtype = (uint8_t)(word2 & 0x7fu);
                selector = (uint8_t)((word2 >> 7) & 0xffu);
                if (pool_index == 3 && subtype == 0x7eu) {
                    resource_id = ((uint32_t)selector << 16) |
                        0x1600ffffu;
                    for (i = 0u; i < selector_count; ++i)
                        if (selector_ids[i] == resource_id) break;
                    if (i == selector_count) {
                        if (selector_count >= selector_capacity) goto done;
                        selector_ids[selector_count++] = resource_id;
                    }
                }
                link = next;
            }
        }
    }
    if (selector_count == 0u) {
        /* LOAD_LOCALLEVEL_DYN always marks this fixed prefix before the
         * map-specific DB3 0x7e selectors. A save may have no sound selector
         * on its current map, but SOUND9 still consumes the fixed source
         * population. Keep only selectors that really materialise in this
         * authenticated GRAPHICS.DAT; no selector is invented. */
        static const uint32_t fixed_selector_ids[] = {
            0x01ff02ffu, 0x18ff02ffu, 0x07ff02ffu, 0x0d0002ffu,
            0x0d2f02ffu, 0x0d7e02ffu, 0x0d9f02ffu, 0x10ff02ffu,
            0x15ff02ffu, 0x030002ffu, 0x08fe02ffu, 0x16fe02ffu,
            0x09fe02ffu, 0x0afe02ffu, 0x0fff08fbu, 0x0fff07fcu,
            0x01ffffffu, 0x01000400u, 0x01000600u, 0x0100070au,
            0x1a80ffffu, 0x1a81ffffu, 0x0300ffffu, 0x0700ffffu,
            0x0d00ffffu, 0x0d2fffffu, 0x0d7effffu, 0x0d9fffffu,
            0x10ffffffu, 0x15ffffffu, 0xffff01f9u, 0x0fff0510u
        };
        for (size_t i = 0u;
             i < sizeof(fixed_selector_ids) / sizeof(fixed_selector_ids[0]);
             ++i) {
            DM2_V1_GdatDyn4MaterializedSelection selection;
            memset(&selection, 0, sizeof(selection));
            if (selector_count >= selector_capacity) goto done;
            if (dm2_v1_gdat_dyn4_materialize_selection(
                    loader, fixed_selector_ids[i], &dyn4_state, &selection) &&
                selection.valid && selection.block_count > 0u) {
                selector_ids[selector_count] = fixed_selector_ids[i];
                selections[selector_count] = selection;
                ++selector_count;
            } else {
                dm2_v1_gdat_dyn4_materialized_selection_free(&selection);
            }
        }
        fixed_selections_ready = 1;
    }
    if (selector_count == 0u) goto done;
    if (!fixed_selections_ready) {
        for (uint16_t i = 0u; i < selector_count; ++i) {
            if (!dm2_v1_gdat_dyn4_materialize_selection(
                    loader, selector_ids[i], &dyn4_state, &selections[i]) ||
                !selections[i].valid || selections[i].block_count == 0u) {
                goto done;
            }
        }
    }
    ok = dm2_v1_sksave_game_load_owner_bind_sound(
        owner, loader, selector_count, selector_ids, selections);
done:
    for (uint16_t i = 0u; i < selector_count; ++i)
        dm2_v1_gdat_dyn4_materialized_selection_free(&selections[i]);
    free(selections);
    free(selector_ids);
    return ok;
}

int dm2_v1_sksave_game_load_owner_materialize_timer_owner(
    DM2_V1_SksaveGameLoadOwner *owner)
{
    DM2_V1_TimerEntry *entries;
    int16_t *indices;
    int16_t active_timer_count;
    int i;

    if (!owner || !owner->valid || !owner->state.valid ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX ||
        owner->timer_queue_count < 0 ||
        owner->timer_queue_count > (int16_t)DM2_V1_SAVE_TIMER_MAX) return 0;
    entries = calloc(DM2_V1_SAVE_TIMER_MAX, sizeof(*entries));
    indices = calloc(DM2_V1_SAVE_TIMER_MAX, sizeof(*indices));
    if (!entries || !indices) {
        free(entries);
        free(indices);
        return 0;
    }
    for (i = 0; i < (int)DM2_V1_SAVE_TIMER_MAX; ++i) {
        const DM2_V1_SaveTimerRecord *source = &owner->timers[i];
        const int16_t value_a = dm2_v1_save_timer_get_a(source);
        dm2_v1_timer_entry_init(&entries[i]);
        dm2_v1_timer_set_mticks(&entries[i],
            (int16_t)dm2_v1_save_timer_get_map(source),
            dm2_v1_save_timer_get_ticks(source));
        entries[i].ttype = dm2_v1_save_timer_get_type(source);
        entries[i].actor = dm2_v1_save_timer_get_actor(source);
        entries[i].xA = (int8_t)(uint8_t)value_a;
        entries[i].yA = (int8_t)((uint16_t)value_a >> 8);
        entries[i].wvalueB = dm2_v1_save_timer_get_b(source);
        entries[i].dummya = 0;
        indices[i] = owner->timer_indices[i];
    }
    active_timer_count = 0;
    for (i = 0; i < (int)DM2_V1_SAVE_TIMER_MAX; ++i)
        if (entries[i].ttype != 0u) ++active_timer_count;
    free(owner->runtime_timer_entries);
    free(owner->runtime_timer_indices);
    owner->runtime_timer_entries = entries;
    owner->runtime_timer_indices = indices;
    dm2_v1_timer_queue_init(&owner->runtime_timer_queue, entries, indices,
                            (int16_t)DM2_V1_SAVE_TIMER_MAX);
    /* c_savegame.cpp keeps these as two different values: w_14 is the
     * active c_tim count, while DM2_REARRANGE_TIMERLIST returns the highest
     * occupied slot plus one for the index-array span.  A full-looking
     * num_timers here would reject QUEUE_TIMER even when the free chain has
     * a valid slot. */
    owner->runtime_timer_queue.num_timers = active_timer_count;
    owner->runtime_timer_queue.num_indices = owner->timer_queue_count;
    owner->runtime_timer_queue.available_idx = owner->timer_free_head;
    owner->runtime_timer_queue.gametick = owner->state.game_tick;
    owner->runtime_timer_queue.deferred_sift = 0;
    owner->runtime_timer_capacity = DM2_V1_SAVE_TIMER_MAX;
    owner->source_timer_owner_ready = 1;
    return 1;
}

int dm2_v1_sksave_game_load_owner_runtime_candidate_admission(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveRuntimeCandidateAdmissionReceipt *out_receipt) {
    DM2_V1_SksaveRuntimeCandidateAdmissionReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    if (!owner || (!owner->valid && !owner->recycler_boundary_inspection_valid)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.source_party_ready = owner->state.champion_count > 0u &&
        owner->state.champion_count <= DM2_MAX_HEROES;
    receipt.source_map_ready = owner->map_owner.valid &&
        owner->map_owner.raw_body && owner->map_owner.raw_body_size > 0u &&
        owner->state.dungeon.valid &&
        owner->state.dungeon.map_data_byte_count > 0u;
    receipt.source_record_pool_ready = owner->record_pools.valid;
    receipt.source_record_graph_ready = owner->record_pools.record_graph_complete;
    receipt.source_timer_ready = owner->source_timer_owner_ready &&
        owner->runtime_timer_entries && owner->runtime_timer_indices &&
        owner->runtime_timer_queue.entries == owner->runtime_timer_entries &&
        owner->runtime_timer_queue.indices == owner->runtime_timer_indices &&
        owner->timer_queue_count >= 0 &&
        owner->timer_queue_count <= (int16_t)DM2_V1_SAVE_TIMER_MAX;
    receipt.source_savegames1_ready = owner->source_savegames1_valid;
    /* SKSAVE currently retains the authenticated AI-definition source only;
     * it has not yet materialized the mutable CAII slot array required by
     * DM2_RESET_CAII/FILL_CAII_CUR_MAP. Do not report a false owner. */
    receipt.source_caii_ready = owner->caii_slots_valid;
    receipt.source_caii_capacity_ready = owner->caii_capacity_valid;
    /* Original SKSAVE does not serialize the runtime sound queue; it is
     * admitted only after the source DYN4/SOUND9 owner has been bound. */
    receipt.source_sound_ready = owner->source_sound_materialized &&
        owner->sound_owner.valid && owner->sound_owner.runtime_queue_initialized;
    receipt.party_count = owner->state.champion_count;
    /* The public receipt reports active c_tim records (s_savegamebuffer.w_14),
     * not the rearranged index-array span kept in timer_queue_count. */
    receipt.timer_count = owner->source_timer_owner_ready
        ? (uint16_t)owner->runtime_timer_queue.num_timers
        : (uint16_t)dm2_v1_sksave_owner_active_timer_count(owner->timers);
    receipt.caii_capacity = owner->caii_source_capacity;
    receipt.source_transaction_hash = owner->state.fixed_sections_hash ^
        owner->state.timers_hash ^ owner->state.dungeon.prefix_hash;
    receipt.runtime_candidate_ready = receipt.source_party_ready &&
        receipt.source_map_ready && receipt.source_record_pool_ready &&
        receipt.source_record_graph_ready &&
        receipt.source_timer_ready && receipt.source_savegames1_ready &&
        receipt.source_caii_ready && receipt.source_sound_ready &&
        receipt.source_transaction_hash != 0u;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static int dm2_v1_sksave_map_owner_clone_runtime_underlay(
    DM2_V1_SksaveMapOwner *out,
    const DM2_V1_SksaveMapOwner *source,
    const DM2_V1_OriginalRawDungeonReceipt *dungeon)
{
    DM2_V1_SksaveMapOwner candidate;

    if (!out || !source || !source->valid || !source->dungeon || !dungeon ||
        source->column_index_count == 0u || !source->column_indices ||
        source->ground_stack_count == 0u || !source->ground_stack_links ||
        source->map_tiles_size == 0u || !source->map_tiles) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate = *source;
    candidate.dungeon = dungeon;
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
    *out = candidate;
    return 1;
}

int dm2_v1_sksave_game_load_owner_clone_runtime_underlay(
    DM2_V1_SksaveGameLoadOwner *out,
    const DM2_V1_SksaveGameLoadOwner *source)
{
    DM2_V1_SksaveGameLoadOwner candidate;
    size_t timer_bytes;

    if (!out || !source || source->lifecycle_tag !=
        DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG || !source->valid ||
        !source->source_dungeon_valid || !source->source_dungeon.raw_data ||
        source->source_dungeon.raw_size <= 0 ||
        !source->record_pools.valid || !source->record_pools.record_graph_complete ||
        !source->caii_slots_valid || !source->caii_slots.valid ||
        !source->caii_slots.slots || source->caii_slots.capacity <= 0 ||
        !source->source_timer_owner_ready || !source->runtime_timer_entries ||
        !source->runtime_timer_indices || source->runtime_timer_capacity == 0u ||
        !source->source_sound_materialized || !source->sound_owner.valid) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate = *source;
    candidate.source_dungeon.raw_data = NULL;
    candidate.map_owner.column_indices = NULL;
    candidate.map_owner.ground_stack_links = NULL;
    candidate.map_owner.map_tiles = NULL;
    memset(&candidate.record_pools, 0, sizeof(candidate.record_pools));
    memset(&candidate.caii_slots, 0, sizeof(candidate.caii_slots));
    candidate.runtime_timer_entries = NULL;
    candidate.runtime_timer_indices = NULL;
    memset(&candidate.runtime_timer_queue, 0,
           sizeof(candidate.runtime_timer_queue));
    memset(&candidate.sound_owner, 0, sizeof(candidate.sound_owner));
    memset(&candidate.caii_source, 0, sizeof(candidate.caii_source));

    if (dm2_v1_dungeon_load(&candidate.source_dungeon,
                            source->source_dungeon.raw_data,
                            source->source_dungeon.raw_size) != 0) {
        goto fail;
    }
    if (!dm2_v1_record_pool_set_clone(&candidate.record_pools,
                                      &source->record_pools)) {
        goto fail;
    }
    if (!dm2_v1_sksave_map_owner_clone_runtime_underlay(
            &candidate.map_owner, &source->map_owner,
            &candidate.state.dungeon)) {
        goto fail;
    }
    if (!dm2_v1_caii_source_owner_clone(&candidate.caii_source,
                                         &source->caii_source)) {
        goto fail;
    }
    if (!dm2_v1_game_load_sound_owner_clone(&candidate.sound_owner,
                                             &source->sound_owner)) {
        goto fail;
    }
    dm2_v1_caii_array_init(&candidate.caii_slots,
                           source->caii_slots.capacity);
    if (!candidate.caii_slots.valid) goto fail;
    memcpy(candidate.caii_slots.slots, source->caii_slots.slots,
           (size_t)source->caii_slots.capacity * DM2_V1_CAII_SLOT_SIZE);
    candidate.caii_slots.alloc_count = source->caii_slots.alloc_count;
    timer_bytes = (size_t)source->runtime_timer_capacity;
    candidate.runtime_timer_entries = calloc(timer_bytes,
                                             sizeof(*candidate.runtime_timer_entries));
    candidate.runtime_timer_indices = calloc(timer_bytes,
                                              sizeof(*candidate.runtime_timer_indices));
    if (!candidate.runtime_timer_entries || !candidate.runtime_timer_indices)
        goto fail;
    memcpy(candidate.runtime_timer_entries, source->runtime_timer_entries,
           timer_bytes * sizeof(*candidate.runtime_timer_entries));
    memcpy(candidate.runtime_timer_indices, source->runtime_timer_indices,
           timer_bytes * sizeof(*candidate.runtime_timer_indices));
    candidate.runtime_timer_queue = source->runtime_timer_queue;
    candidate.runtime_timer_queue.entries = candidate.runtime_timer_entries;
    candidate.runtime_timer_queue.indices = candidate.runtime_timer_indices;
    /* The mutable c_map clone owns the arrays; the authenticated raw
     * descriptor remains the source receipt until a runtime candidate copies
     * it into its own storage.  candidate.state.dungeon is only the compact
     * save-state descriptor and has no map geometry. */
    candidate.map_owner.dungeon = source->map_owner.dungeon;
    candidate.source_dungeon_valid = 1;
    candidate.source_game_load_session_ready = 0;
    *out = candidate;
    return 1;

fail:
    dm2_v1_sksave_game_load_owner_free(&candidate);
    return 0;
}

static int dm2_v1_sksave_tile_has_creature(
    const DM2_V1_SksaveGameLoadOwner *owner, int map, int x, int y);

/* c_record.cpp::DM2_RECYCLE_A_RECORD_FROM_THE_WORLD has one non-mutating
 * branch which is useful before a complete runtime exists: DB0 is returned
 * to DM2_ALLOC_NEW_RECORD, which performs the actual clear afterwards.
 * Keep this walker here, beside the retained c_map/DB4-AI ownership, rather
 * than teaching the generic map diagnostic to guess at GAME_LOAD state. */
static int dm2_v1_sksave_recycler_scan_tile(
    const DM2_V1_SksaveGameLoadOwner *owner, int map, int x, int y,
    uint8_t requested_db, DM2_V1_SksaveRecyclerCandidate *candidate)
{
    uint16_t root;
    int16_t current;
    int16_t static_creature = DM2_V1_RECORD_HANDLE_END;
    int16_t static_creature_next = DM2_V1_RECORD_HANDLE_END;
    size_t steps = 0u;
    int near_party;

    if (!dm2_v1_sksave_map_owner_tile_record_link(&owner->map_owner, map,
                                                    x, y, &root)) {
        return 0;
    }
    current = (int16_t)root;
    near_party = map == (int)owner->recycler_context.party_map &&
        x >= (int)owner->recycler_context.party_x - 5 &&
        x <= (int)owner->recycler_context.party_x + 5 &&
        y >= (int)owner->recycler_context.party_y - 5 &&
        y <= (int)owner->recycler_context.party_y + 5;

    /* Empty c_map cells carry OBJECT_NULL as their chain root. They have no
     * record to recycle; continue with the next tile. OBJECT_NULL remains
     * invalid only after a real record's next-link has been admitted. */
    if (current == DM2_V1_RECORD_HANDLE_NULL) return 1;
    while (current != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *record;
        int16_t next;
        const int pool = dm2_v1_record_handle_pool(current);

        if (++steps > (size_t)DM2_V1_SKSAVE_RECYCLE_MAX_STEPS || pool < 0 ||
            pool >= DM2_V1_RECORD_POOL_COUNT ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, current,
                                           &next) ||
            !(record = dm2_v1_record_pool_address(&owner->record_pools,
                                                   current))) {
            return 0;
        }
        ++candidate->records_examined;

        /* c_record.cpp:DA34-DAF1: these records stop this tile's chain. */
        if (pool == 3) {
            const uint8_t subtype = record[2] & 0x7fu;
            if (subtype < 44u && dm2_v1_table_1d324c[subtype] != 0)
                break;
        } else if (pool == 2) {
            const uint16_t word2 = (uint16_t)record[2] |
                ((uint16_t)record[3] << 8);
            if ((word2 & 0x0006u) == 0x0002u &&
                ((word2 >> 11) & 0x001fu) == 4u) {
                break;
            }
        }
        /* c_record.cpp:DB88 returns a directly selected record after the
         * DB2/Text barrier has been evaluated. DB0, DB2, DB3, DB7 and DB9 are
         * direct-return branches; DB5/DB6/DB8/DB10 use source cut-only
         * relocation. DB14 admits only the no-creature delete shape; DB4 and
         * creature-bearing DB14 tiles still require their full owners. */
        if ((pool == 0 || pool == 2 || pool == 3 || pool == 7 || pool == 9 ||
             (pool == 14 && dm2_v1_sksave_tile_has_creature(
                 owner, map, x, y) == 0) ||
             ((pool == 5 || pool == 6 || pool == 8 || pool == 10) &&
              (record[2] & 0x80u) == 0u)) &&
            requested_db == (uint8_t)pool) {
            candidate->found = 1;
            candidate->selected_link = (uint16_t)current & 0x3fffu;
            candidate->selected_map = (uint8_t)map;
            candidate->selected_x = (uint8_t)x;
            candidate->selected_y = (uint8_t)y;
            return 2;
        }
        if (pool == 4 && !near_party &&
                   static_creature == DM2_V1_RECORD_HANDLE_END &&
                   next != DM2_V1_RECORD_HANDLE_END) {
            uint16_t ai_flags = 0u;
            /* c_record.cpp:DB31-DB62 descends only into a static creature's
             * possession chain. Missing retained CREATURES data is not an
             * invitation to treat the creature as dynamic or empty. */
            if (!dm2_v1_sksave_game_load_owner_creature_ai_flags(owner,
                                                                  record[4],
                                                                  &ai_flags)) {
                return 0;
            }
            if ((ai_flags & 0x0001u) != 0u) {
                static_creature = current;
                static_creature_next = next;
                current = next;
                ++candidate->static_possession_descents;
                continue;
            }
        }

        current = next;
        if (current == DM2_V1_RECORD_HANDLE_END &&
            static_creature != DM2_V1_RECORD_HANDLE_END) {
            /* c_record.cpp:DAE2-DAF0 resumes at the static creature, then
             * advances once through its ordinary tile-chain link. */
            current = static_creature_next;
            static_creature = DM2_V1_RECORD_HANDLE_END;
            static_creature_next = DM2_V1_RECORD_HANDLE_END;
        }
    }
    return 1;
}

/* DM2_DELETE_MISSILE_RECORD first asks GET_CREATURE_AT when its caller passes
 * a null owner. The SKSAVE recycler has no complete creature/timer owner for
 * that side effect, so admit DB14 only on a tile whose authenticated chain has
 * no DB4 creature. This is an admission guard, not a creature substitute. */
static int dm2_v1_sksave_tile_has_creature(
    const DM2_V1_SksaveGameLoadOwner *owner, int map, int x, int y)
{
    uint16_t root;
    int16_t current;
    size_t steps = 0u;

    if (!owner || !dm2_v1_sksave_map_owner_tile_record_link(
            &owner->map_owner, map, x, y, &root)) return -1;
    current = (int16_t)root;
    if (current == DM2_V1_RECORD_HANDLE_NULL) return -1;
    while (current != DM2_V1_RECORD_HANDLE_END) {
        const int pool = dm2_v1_record_handle_pool(current);
        int16_t next;
        if (++steps > (size_t)DM2_V1_SKSAVE_RECYCLE_MAX_STEPS ||
            pool < 0 || pool >= DM2_V1_RECORD_POOL_COUNT ||
            !dm2_v1_record_pool_next_link(&owner->record_pools, current,
                                           &next) ||
            !dm2_v1_record_pool_address(&owner->record_pools, current)) {
            return -1;
        }
        if (pool == 4) return 1;
        current = next;
    }
    return 0;
}

static int dm2_v1_sksave_db0_advance_map(int *map, int map_count)
{
    if (!map || map_count <= 0 || *map < 0 || *map >= map_count) return 0;
    *map = *map + 1 < map_count ? *map + 1 : 0;
    return 1;
}

int dm2_v1_sksave_game_load_owner_recycler_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    uint8_t requested_db, DM2_V1_SksaveRecyclerCandidate *out_candidate)
{
    const DM2_V1_OriginalRawDungeonReceipt *dungeon;
    DM2_V1_SksaveRecyclerCandidate candidate;
    int map;
    int anchor;
    int protected_map;
    int second_pass = 0;
    size_t pass_budget;

    if (out_candidate) memset(out_candidate, 0, sizeof(*out_candidate));
    if (!owner || !out_candidate ||
        (requested_db != 0u && requested_db != 2u && requested_db != 3u &&
         requested_db != 5u && requested_db != 6u && requested_db != 7u &&
         requested_db != 8u && requested_db != 9u && requested_db != 10u &&
         requested_db != 14u) ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid) ||
        !owner->recycler_context.valid || !owner->map_owner.valid ||
        !owner->record_pools.valid || !owner->map_owner.dungeon ||
        owner->source_game_load_session_ready) {
        return 0;
    }
    /* c_map's map-owner pointer is only a local chain-view reference when the
     * real GAME_LOAD state exists. Prefer that complete source receipt. The
     * fallback is limited to standalone unit fixtures that intentionally
     * construct only a map-owner receipt. */
    dungeon = &owner->state.dungeon;
    if (!dungeon->valid || dungeon->map_count == 0u)
        dungeon = owner->map_owner.dungeon;
    if (!dungeon->valid || dungeon->map_count < 2u ||
        dungeon->map_count > DM2_RAW_SKSAVE_MAX_MAPS ||
        owner->recycler_context.map_count != dungeon->map_count ||
        owner->recycler_context.current_map >= dungeon->map_count ||
        owner->recycler_context.party_map >= dungeon->map_count ||
        owner->map_owner.current_map !=
            (int)owner->recycler_context.current_map) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.selected_link = (uint16_t)DM2_V1_RECORD_HANDLE_END;
    candidate.requested_db = requested_db;
    candidate.cursor_before = owner->recycler_context.map_cursors[requested_db];
    map = (int)candidate.cursor_before;
    if (map < 0 || map >= (int)dungeon->map_count) return 0;
    protected_map = owner->recycler_context.protected_map_active ?
        (int)owner->recycler_context.protected_map : -1;
    if (protected_map >= (int)dungeon->map_count) return 0;
    anchor = map;

    /* c_record.cpp:D83F-D929: start outside party/protected maps. */
    if (map == (int)owner->recycler_context.party_map ||
        map == protected_map) {
        do {
            if (!dm2_v1_sksave_db0_advance_map(&map, dungeon->map_count))
                return 0;
            if (map == anchor) {
                if (protected_map >= 0) {
                    map = protected_map;
                    protected_map = -1;
                } else {
                    map = (int)owner->recycler_context.party_map;
                }
            }
        } while (map == (int)owner->recycler_context.party_map ||
                 map == protected_map);
    }
    anchor = map;
    pass_budget = (size_t)dungeon->map_count * 4u + 2u;

    while (pass_budget-- != 0u) {
        int x;
        for (x = 0; x < (int)dungeon->map_widths[map]; ++x) {
            int y;
            for (y = 0; y < (int)dungeon->map_heights[map]; ++y) {
                const int scan = dm2_v1_sksave_recycler_scan_tile(owner, map, x, y,
                                                                    requested_db,
                                                                    &candidate);
                if (scan == 0) return 0;
                if (scan == 2) {
                    candidate.valid = 1;
                    candidate.cursor_after = (uint8_t)map;
                    ++candidate.maps_scanned;
                    *out_candidate = candidate;
                    return 1;
                }
            }
        }
        ++candidate.maps_scanned;

        if (map == (int)owner->recycler_context.party_map ||
            dungeon->map_count <= 1u) {
            if (second_pass) {
                candidate.valid = 1;
                candidate.cursor_after = (uint8_t)map;
                *out_candidate = candidate;
                return 1;
            }
            /* The source re-walks the party map after setting vbool_18.
             * DB0 itself does not use the second-pass actuator type filter,
             * but the extra traversal is part of the source cursor proof. */
            second_pass = 1;
            continue;
        }

        if (protected_map < 0) {
            do {
                if (!dm2_v1_sksave_db0_advance_map(&map, dungeon->map_count))
                    return 0;
                if (map == anchor) {
                    map = (int)owner->recycler_context.party_map;
                    break;
                }
            } while (map == (int)owner->recycler_context.party_map);
        } else {
            do {
                if (!dm2_v1_sksave_db0_advance_map(&map, dungeon->map_count))
                    return 0;
                if (map == anchor) {
                    map = protected_map;
                    protected_map = -1;
                }
            } while (map == protected_map ||
                     map == (int)owner->recycler_context.party_map);
        }
    }
    /* A bounded source ring must always reach the party-map second pass. */
    return 0;
}

int dm2_v1_sksave_game_load_owner_db0_recycler_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDb0RecyclerCandidate *out_candidate)
{
    return dm2_v1_sksave_game_load_owner_recycler_candidate(owner, 0u,
        (DM2_V1_SksaveRecyclerCandidate *)out_candidate);
}

int dm2_v1_sksave_game_load_owner_missile_timer_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveMissileTimerCandidate *out_candidate)
{
    DM2_V1_SksaveMissileTimerCandidate receipt;
    DM2_V1_TimerQueue queue;
    DM2_V1_TimerEntry *timer;
    const uint8_t *record;
    int map;
    int x;
    int y;
    int record_map;
    int record_x;
    int record_y;
    int16_t slot;
    int16_t missile;
    uint16_t packed;

    memset(&receipt, 0, sizeof(receipt));
    receipt.timer_slot = DM2_V1_RECORD_HANDLE_NULL;
    receipt.missile_record = DM2_V1_RECORD_HANDLE_NULL;
    if (!owner || !out_candidate || !owner->valid ||
        !owner->source_timer_owner_ready ||
        !owner->runtime_timer_entries || !owner->runtime_timer_indices ||
        owner->runtime_timer_queue.num_timers <= 0) {
        receipt.blocked_no_timer = 1;
        if (out_candidate) *out_candidate = receipt;
        return 0;
    }
    queue = owner->runtime_timer_queue;
    queue.gametick = owner->state.game_tick;
    if (!dm2_v1_timer_is_due(&queue)) {
        receipt.blocked_no_timer = 1;
        if (out_candidate) *out_candidate = receipt;
        return 0;
    }
    receipt.due = 1;
    slot = queue.indices[0];
    if (slot < 0 || slot >= owner->runtime_timer_capacity) {
        receipt.blocked_no_timer = 1;
        if (out_candidate) *out_candidate = receipt;
        return 0;
    }
    receipt.timer_slot = slot;
    timer = &owner->runtime_timer_entries[slot];
    if (timer->ttype != 0x1eu) {
        receipt.blocked_wrong_type = 1;
        if (out_candidate) *out_candidate = receipt;
        return 0;
    }
    missile = (int16_t)((uint16_t)(uint8_t)timer->xA |
                        ((uint16_t)(uint8_t)timer->yA << 8));
    map = dm2_v1_timer_get_map(timer);
    packed = (uint16_t)timer->wvalueB;
    x = (int)(packed & 0x1fu);
    y = (int)((packed >> 5) & 0x1fu);
    receipt.missile_record = missile;
    receipt.map = (int16_t)map;
    receipt.x = (int16_t)x;
    receipt.y = (int16_t)y;
    receipt.direction = (uint8_t)((packed >> 10) & 3u);
    receipt.energy_step = (uint16_t)(packed >> 12);
    record = dm2_v1_record_pool_address(&owner->record_pools, missile);
    if (map < 0 || map >= owner->state.dungeon.map_count || x < 0 || y < 0 ||
        x >= owner->state.dungeon.map_widths[map] ||
        y >= owner->state.dungeon.map_heights[map] ||
        dm2_v1_record_handle_pool(missile) != 14 || !record ||
        owner->record_pools.pools[14].record_size < 8 ||
        ((uint16_t)record[6] | ((uint16_t)record[7] << 8)) !=
            (uint16_t)slot) {
        receipt.blocked_record = 1;
        if (out_candidate) *out_candidate = receipt;
        return 0;
    }
    if (!dm2_v1_sksave_owner_find_record_position(owner, (uint16_t)missile,
                                                   &record_map, &record_x,
                                                   &record_y) ||
        record_map != map || record_x != x || record_y != y ||
        dm2_v1_sksave_tile_has_creature(owner, map, x, y) != 0) {
        receipt.blocked_chain = 1;
        if (out_candidate) *out_candidate = receipt;
        return 0;
    }
    receipt.valid = 1;
    if (out_candidate) *out_candidate = receipt;
    return 1;
}

int dm2_v1_sksave_game_load_owner_db14_missile_delete_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDb14MissileDeleteCandidate *out_candidate)
{
    DM2_V1_SksaveRecyclerCandidate recycler;
    const uint8_t *record;
    uint16_t timer_index;
    int found_timer = -1;

    if (out_candidate) memset(out_candidate, 0, sizeof(*out_candidate));
    if (!owner || !out_candidate ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid) ||
        owner->source_game_load_session_ready || !owner->record_pools.valid ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX) return 0;
    memset(&recycler, 0, sizeof(recycler));
    if (!dm2_v1_sksave_game_load_owner_recycler_candidate(
            owner, 14u, &recycler) || !recycler.valid || !recycler.found ||
        dm2_v1_sksave_tile_has_creature(owner, recycler.selected_map,
                                        recycler.selected_x,
                                        recycler.selected_y) != 0 ||
        !(record = dm2_v1_record_pool_address(&owner->record_pools,
                                              (int16_t)recycler.selected_link)) ||
        dm2_v1_record_handle_pool((int16_t)recycler.selected_link) != 14) {
        return 0;
    }
    if (owner->record_pools.pools[14].record_size < 8) return 0;
    timer_index = (uint16_t)record[6] | ((uint16_t)record[7] << 8);
    if (timer_index >= owner->state.timer_count) return 0;
    {
        const DM2_V1_SaveTimerRecord *timer =
            (const DM2_V1_SaveTimerRecord *)&owner->timers[timer_index];
        if (dm2_v1_save_timer_get_type(timer) == 0x1eu &&
            (uint16_t)dm2_v1_save_timer_get_a(timer) ==
                (uint16_t)recycler.selected_link) {
            found_timer = (int)timer_index;
        }
    }
    if (found_timer < 0) return 0;
    out_candidate->valid = 1;
    out_candidate->found = 1;
    out_candidate->missile_record = recycler.selected_link;
    out_candidate->map = recycler.selected_map;
    out_candidate->x = recycler.selected_x;
    out_candidate->y = recycler.selected_y;
    out_candidate->timer_index = (int16_t)found_timer;
    return 1;
}

int dm2_v1_sksave_game_load_owner_commit_db14_missile_delete(
    DM2_V1_SksaveGameLoadOwner *owner, uint16_t *out_record)
{
    DM2_V1_SksaveDb14MissileDeleteCandidate candidate;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SksaveMapOwner map_copy;
    DM2_V1_SaveTimerRecord timers[DM2_V1_SAVE_TIMER_MAX];
    int16_t timer_indices[DM2_V1_SAVE_TIMER_MAX];
    int16_t timer_count;
    int16_t timer_free;
    uint8_t *record;
    size_t column_bytes, ground_bytes, tile_bytes;

    if (out_record) *out_record = DM2_V1_RECORD_HANDLE_END;
    if (!owner || owner->source_game_load_session_ready ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid) ||
        !owner->record_pools.valid || owner->state.timer_count == 0u ||
        owner->state.timer_count > DM2_V1_SAVE_TIMER_MAX ||
        !dm2_v1_sksave_game_load_owner_db14_missile_delete_candidate(
            owner, &candidate)) return 0;
    memset(&pools, 0, sizeof(pools));
    memset(&map_copy, 0, sizeof(map_copy));
    if (!dm2_v1_record_pool_set_clone(&pools, &owner->record_pools)) goto fail;
    map_copy = owner->map_owner;
    column_bytes = owner->map_owner.column_index_count * sizeof(uint16_t);
    ground_bytes = owner->map_owner.ground_stack_count * sizeof(uint16_t);
    tile_bytes = owner->map_owner.map_tiles_size;
    map_copy.column_indices = column_bytes ? malloc(column_bytes) : NULL;
    map_copy.ground_stack_links = ground_bytes ? malloc(ground_bytes) : NULL;
    map_copy.map_tiles = tile_bytes ? malloc(tile_bytes) : NULL;
    if ((column_bytes && !map_copy.column_indices) ||
        (ground_bytes && !map_copy.ground_stack_links) ||
        (tile_bytes && !map_copy.map_tiles)) goto fail;
    if (column_bytes) memcpy(map_copy.column_indices,
                             owner->map_owner.column_indices, column_bytes);
    if (ground_bytes) memcpy(map_copy.ground_stack_links,
                             owner->map_owner.ground_stack_links, ground_bytes);
    if (tile_bytes) memcpy(map_copy.map_tiles, owner->map_owner.map_tiles,
                           tile_bytes);
    if (!dm2_v1_sksave_map_owner_cut_tile_record(
            &map_copy, &pools, candidate.map, candidate.x, candidate.y,
            candidate.missile_record)) goto fail;
    record = dm2_v1_record_pool_address_mut(&pools,
                                             (int16_t)candidate.missile_record);
    if (!record) goto fail;
    memset(record, 0, (size_t)pools.pools[14].record_size);
    record[0] = 0xffu;
    record[1] = 0xffu;
    memcpy(timers, owner->timers, sizeof(timers));
    memcpy(timer_indices, owner->timer_indices, sizeof(timer_indices));
    dm2_v1_save_timer_clr_type(&timers[candidate.timer_index]);
    dm2_v1_save_timer_rearrange(timers, DM2_V1_SAVE_TIMER_MAX,
                                &timer_count, &timer_free);

    free(owner->map_owner.column_indices);
    free(owner->map_owner.ground_stack_links);
    free(owner->map_owner.map_tiles);
    owner->map_owner = map_copy;
    dm2_v1_record_pool_set_free(&owner->record_pools);
    owner->record_pools = pools;
    memcpy(owner->timers, timers, sizeof(timers));
    memcpy(owner->timer_indices, timer_indices, sizeof(timer_indices));
    owner->timer_queue_count = timer_count;
    owner->timer_free_head = timer_free;
    if (out_record) *out_record = candidate.missile_record;
    return 1;
fail:
    free(map_copy.column_indices);
    free(map_copy.ground_stack_links);
    free(map_copy.map_tiles);
    dm2_v1_record_pool_set_free(&pools);
    return 0;
}

int dm2_v1_sksave_game_load_owner_commit_db0_recycler(
    DM2_V1_SksaveGameLoadOwner *owner, uint16_t *out_record)
{
    DM2_V1_SksaveDb0RecyclerCandidate candidate;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SksaveMapOwner map_copy;
    uint8_t *record;
    const int16_t selected = (int16_t)DM2_V1_RECORD_HANDLE_END;
    size_t column_bytes;
    size_t ground_bytes;
    size_t tile_bytes;

    if (out_record) *out_record = (uint16_t)selected;
    if (!owner || owner->source_game_load_session_ready ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid) ||
        !owner->record_pools.valid) return 0;
    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_sksave_game_load_owner_db0_recycler_candidate(owner,
                                                               &candidate) ||
        !candidate.valid || !candidate.found ||
        dm2_v1_record_handle_pool((int16_t)candidate.selected_link) != 0) {
        return 0;
    }

    memset(&pools, 0, sizeof(pools));
    memset(&map_copy, 0, sizeof(map_copy));
    if (!dm2_v1_record_pool_set_clone(&pools, &owner->record_pools)) {
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }

    /* Clone the mutable c_map spans before the cut. The source commits the
     * ground-stack splice and record-pool splice as one ALLOC_NEW_RECORD
     * transaction; mutating the live map and then publishing an uncut pool
     * clone would leave a predecessor pointing at the cleared record. */
    map_copy = owner->map_owner;
    column_bytes = owner->map_owner.column_index_count * sizeof(uint16_t);
    ground_bytes = owner->map_owner.ground_stack_count * sizeof(uint16_t);
    tile_bytes = owner->map_owner.map_tiles_size;
    map_copy.column_indices = column_bytes != 0u
        ? (uint16_t *)malloc(column_bytes) : NULL;
    map_copy.ground_stack_links = ground_bytes != 0u
        ? (uint16_t *)malloc(ground_bytes) : NULL;
    map_copy.map_tiles = tile_bytes != 0u
        ? (uint8_t *)malloc(tile_bytes) : NULL;
    if ((column_bytes != 0u && !map_copy.column_indices) ||
        (ground_bytes != 0u && !map_copy.ground_stack_links) ||
        (tile_bytes != 0u && !map_copy.map_tiles)) {
        free(map_copy.column_indices);
        free(map_copy.ground_stack_links);
        free(map_copy.map_tiles);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }
    if (column_bytes != 0u)
        memcpy(map_copy.column_indices, owner->map_owner.column_indices,
               column_bytes);
    if (ground_bytes != 0u)
        memcpy(map_copy.ground_stack_links, owner->map_owner.ground_stack_links,
               ground_bytes);
    if (tile_bytes != 0u)
        memcpy(map_copy.map_tiles, owner->map_owner.map_tiles, tile_bytes);

    /* The direct-return branch owns a tile-chain unlink before
     * ALLOC_NEW_RECORD clears the selected DB0 slot.  Apply it to the same
     * cloned map/pool pair that will be published. */
    if (!dm2_v1_sksave_map_owner_cut_tile_record(
            &map_copy, &pools,
            candidate.selected_map, candidate.selected_x,
            candidate.selected_y, candidate.selected_link)) {
        free(map_copy.column_indices);
        free(map_copy.ground_stack_links);
        free(map_copy.map_tiles);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }

    record = dm2_v1_record_pool_address_mut(&pools,
                                             (int16_t)candidate.selected_link);
    if (!record) {
        free(map_copy.column_indices);
        free(map_copy.ground_stack_links);
        free(map_copy.map_tiles);
        dm2_v1_record_pool_set_free(&pools);
        return 0;
    }

    /* ALLOC_NEW_RECORD clears the source-sized record and then marks its
     * first link OBJECT_END_MARKER.  DB0 has no DB9 second-word special case. */
    memset(record, 0, (size_t)pools.pools[0].record_size);
    record[0] = 0xfeu;
    record[1] = 0xffu;

    /* The candidate is prospective until the clone and record mutation have
     * both succeeded.  Replacing the pool owner is the commit point. */
    free(owner->map_owner.column_indices);
    free(owner->map_owner.ground_stack_links);
    free(owner->map_owner.map_tiles);
    owner->map_owner = map_copy;
    dm2_v1_record_pool_set_free(&owner->record_pools);
    owner->record_pools = pools;
    owner->recycler_context.map_cursors[0] = candidate.cursor_after;
    if (out_record) *out_record = candidate.selected_link;
    return 1;
}

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner)
{
    if (!owner) return;
    if (!dm2_v1_sksave_game_load_owner_is_initialized(owner)) {
        memset(owner, 0, sizeof(*owner));
        return;
    }
    dm2_v1_sksave_map_owner_free(&owner->map_owner);
    dm2_v1_record_pool_set_free(&owner->record_pools);
    dm2_v1_caii_array_free(&owner->caii_slots);
    free(owner->runtime_timer_entries);
    free(owner->runtime_timer_indices);
    dm2_v1_game_load_sound_owner_free(&owner->sound_owner);
    if (owner->source_dungeon_valid)
        dm2_v1_dungeon_free(&owner->source_dungeon);
    dm2_v1_caii_source_owner_free(&owner->caii_source);
    free(owner->owned_raw_file);
    memset(owner, 0, sizeof(*owner));
}
