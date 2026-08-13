#ifndef FIRESTAFF_DM2_V1_GAME_LOAD_SOUND_OWNER_H
#define FIRESTAFF_DM2_V1_GAME_LOAD_SOUND_OWNER_H

#include "dm2_v1_sound.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t raw_index;
    uint16_t raw_length;
    uint32_t source_payload_hash;
} DM2_V1_GameLoadSoundSampleBinding;

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
    DM2_V1_SoundSfx positional[DM2_V1_SOUND_POSITIONAL_CAP];
    uint16_t positional_count;
    DM2_V1_SoundSfx immediate[DM2_V1_SOUND_IMMEDIATE_CAP];
    uint16_t immediate_count;
    DM2_V1_SoundDelayedSlot delayed[DM2_V1_SOUND_DELAYED_SLOT_COUNT];
    int32_t sample_slots[DM2_V1_SOUND_SAMPLE_SLOT_COUNT];
    int sound_enabled;
    int master_sfx_volume;
    int runtime_queue_initialized;
    int spatial_context_valid;
    int16_t spatial_current_map;
    int16_t spatial_audible_map;
    int16_t spatial_alternate_map;
    uint8_t spatial_current_origin_x;
    uint8_t spatial_current_origin_y;
    uint8_t spatial_audible_origin_x;
    uint8_t spatial_audible_origin_y;
    uint32_t spatial_context_hash;
} DM2_V1_GameLoadSoundOwner;

uint16_t dm2_v1_game_load_sound_owner_query_entry(
    const DM2_V1_GameLoadSoundOwner *owner,
    int8_t cls1, int8_t cls2, int8_t cls3);

int dm2_v1_game_load_sound_owner_materialize_from_dyn4(
    const DM2_V1_AssetLoader *loader, uint16_t selector_count,
    const uint32_t *selector_ids,
    const DM2_V1_GdatDyn4MaterializedSelection *selections,
    DM2_V1_GameLoadSoundOwner *out_owner);
void dm2_v1_game_load_sound_owner_free(DM2_V1_GameLoadSoundOwner *owner);
int dm2_v1_game_load_sound_owner_clone(
    DM2_V1_GameLoadSoundOwner *out,
    const DM2_V1_GameLoadSoundOwner *source);

#ifdef __cplusplus
}
#endif

#endif
