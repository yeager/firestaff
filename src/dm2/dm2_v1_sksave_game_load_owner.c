#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"

#include <string.h>

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
            &candidate.receipt)) {
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
