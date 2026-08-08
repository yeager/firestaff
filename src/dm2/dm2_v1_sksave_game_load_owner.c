#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"

#include <string.h>

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
        dm2_suppress_reader_read(&reader, full_mask, 1u, owner->v1e0104, 0u) ||
        dm2_suppress_reader_read(&reader, full_mask, 1u, owner->globalb, 0u) ||
        dm2_suppress_reader_read(&reader, full_mask, 2u, owner->globalw, 0u))
        return 0;
    /* The downstream materializer repeats the exact hero/timer stream into
     * its source-owned owners. These reads establish the retained fixed
     * globals without seeking across a partially consumed SUPPRESS byte. */
    for (uint16_t i = 0; i < owner->state.champion_count; ++i)
        if (dm2_suppress_reader_read(&reader, hero_mask, sizeof(DM2_V1_Hero),
                (uint8_t *)&owner->heroes[i], 0u)) return 0;
    if (dm2_suppress_reader_read(&reader, state_mask, sizeof(owner->savegames1),
            owner->savegames1, 0u)) return 0;
    for (uint16_t i = 0; i < owner->state.timer_count; ++i)
        if (dm2_suppress_reader_read(&reader, timer_mask, 12u,
                owner->timers[i].bytes, 0u)) return 0;
    return reader.position == owner->state.record_link_bitstream_offset -
        owner->state.dungeon.suppress_state_offset &&
        reader.bits_remaining == owner->state.record_link_bitstream_bits_remaining;
}

int dm2_v1_sksave_game_load_owner_init(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx)
{
    DM2_V1_SksaveGameLoadOwner candidate;

    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    candidate.timer_queue_count = -1;
    candidate.timer_free_head = -1;
    candidate.leader_hand_root = DM2_V1_RECORD_HANDLE_END;
    if (!raw_body || !savegamew7 ||
        !dm2_v1_original_raw_sksave_fixed_state_receipt(raw_body,
            raw_body_size, &candidate.state) || !candidate.state.valid ||
        !dm2_v1_sksave_owner_decode_fixed(&candidate, raw_body, raw_body_size) ||
        !dm2_v1_record_pool_materialize_raw_sksave_game_load_owner(
            &candidate, raw_body, raw_body_size, savegamew7, asset_loader,
            query_creature_ai_flags, query_creature_ai_flags_ctx,
            &candidate.receipt) ||
        !dm2_v1_sksave_game_load_owner_apply_post_load_global_effects(
            &candidate)) {
        dm2_v1_sksave_game_load_owner_free(&candidate);
        return 0;
    }
    candidate.savegamew7 = savegamew7;
    candidate.valid = 1;
    candidate.source_game_load_session_ready = 0;
    *owner = candidate;
    return 1;
}

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner)
{
    if (!owner) return;
    dm2_v1_sksave_map_owner_free(&owner->map_owner);
    dm2_v1_record_pool_set_free(&owner->record_pools);
    memset(owner, 0, sizeof(*owner));
}
