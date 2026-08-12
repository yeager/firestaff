#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_data_tables_pc34_compat.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"

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
        map_owner->ground_stack_count < dungeon->ground_stack_count ||
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
    DM2_V1_GlobalEffectCallbacks callbacks;
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
        if (record[0] == 0xffu && record[1] == 0xffu) continue;
        if (!query_creature_ai_flags(ctx, link, record[4], &flags)) {
            /* A failed map restore can leave pool records whose source
             * liveness has not yet been reconstructed. For the narrow
             * recycler inspection we retain only admitted AI rows and reject
             * later if traversal actually needs a missing row. A complete
             * owner remains strict over every retained DB4 record. */
            if (strict) return 0;
            continue;
        }
        owner->retained_creature_ai_flags[record[4]] = flags;
        owner->retained_creature_ai_valid[record[4]] = 1u;
    }
    return 1;
}

int dm2_v1_sksave_game_load_owner_init_ordered(
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
    if (!raw_body || !savegamew7 ||
        !dm2_v1_original_raw_sksave_fixed_state_receipt_ordered(raw_body,
            raw_body_size, words_big_endian, &candidate.state) || !candidate.state.valid ||
        !dm2_v1_sksave_owner_decode_fixed(&candidate, raw_body, raw_body_size) ||
        !dm2_v1_record_pool_materialize_raw_sksave_game_load_owner(
            &candidate, raw_body, raw_body_size, savegamew7, asset_loader,
            query_creature_ai_flags, query_creature_ai_flags_ctx,
            &candidate.receipt) ||
        !dm2_v1_sksave_owner_retain_creature_ai_flags(&candidate,
            query_creature_ai_flags, query_creature_ai_flags_ctx, 1) ||
        !dm2_v1_sksave_game_load_owner_apply_post_load_global_effects(
            &candidate) ||
        !dm2_v1_sksave_owner_rebuild_timer_backlinks(&candidate) ||
        !dm2_v1_sksave_owner_init_recycler_context(&candidate)) {
        dm2_v1_sksave_game_load_owner_free(&candidate);
        if (!owner_was_initialized) memset(owner, 0, sizeof(*owner));
        return 0;
    }
    candidate.savegamew7 = savegamew7;
    candidate.valid = 1;
    candidate.source_game_load_session_ready = 0;
    if (owner_was_initialized)
        dm2_v1_sksave_game_load_owner_free(owner);
    *owner = candidate;
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
             candidate.receipt.recycle_required_db != 2) ||
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

    /* OBJECT_NULL is legal only in an unused pool slot, never as a tile or
     * possession root. The original would dereference it; reject instead of
     * silently shortening an admitted chain. */
    if (current == DM2_V1_RECORD_HANDLE_NULL) return 0;
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

        /* DB0 reaches c_record.cpp:DB88 directly.  DB2 only observes the
         * protected-text barrier at DAF9-DB2A, then advances this chain at
         * DACC.  It never reaches DB88 for a requested DB2 allocation. */
        if (pool == 0 && requested_db == 0u) {
            candidate->found = 1;
            candidate->selected_link = (uint16_t)current & 0x3fffu;
            candidate->selected_map = (uint8_t)map;
            candidate->selected_x = (uint8_t)x;
            candidate->selected_y = (uint8_t)y;
            return 2;
        }
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
        } else if (pool == 4 && !near_party &&
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
    if (!owner || !out_candidate || requested_db != 0u ||
        (!owner->valid && !owner->recycler_boundary_inspection_valid) ||
        !owner->recycler_context.valid || !owner->map_owner.valid ||
        !owner->record_pools.valid || !owner->map_owner.dungeon ||
        owner->source_game_load_session_ready) return 0;
    dungeon = owner->map_owner.dungeon;
    if (!dungeon->valid || dungeon->map_count < 2u ||
        dungeon->map_count > DM2_RAW_SKSAVE_MAX_MAPS ||
        owner->recycler_context.map_count != dungeon->map_count ||
        owner->recycler_context.current_map >= dungeon->map_count ||
        owner->recycler_context.party_map >= dungeon->map_count ||
        owner->map_owner.current_map !=
            (int)owner->recycler_context.current_map) return 0;

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

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner)
{
    if (!owner) return;
    if (!dm2_v1_sksave_game_load_owner_is_initialized(owner)) {
        memset(owner, 0, sizeof(*owner));
        return;
    }
    dm2_v1_sksave_map_owner_free(&owner->map_owner);
    dm2_v1_record_pool_set_free(&owner->record_pools);
    memset(owner, 0, sizeof(*owner));
}
