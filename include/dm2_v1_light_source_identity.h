#ifndef FIRESTAFF_DM2_V1_LIGHT_SOURCE_IDENTITY_H
#define FIRESTAFF_DM2_V1_LIGHT_SOURCE_IDENTITY_H

#include <stdint.h>
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_game.h"
#include "dm2_v1_party.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_weather.h"
#include "dm2_v1_update_weather_pc34_compat.h"

/* A fresh identity of mutable inputs, not a FIND_WALK_PATH completion proof. */
typedef struct {
    const DM2_V1_CLightMapDescriptorReceipt *map;
    int graphics_style;
    const DM2_V1_DungeonData *dungeon;
    const DM2_V1_RecordPoolSet *records;
    const DM2_V1_AssetLoader *gdat;
    const DM2_V1_GameState *game;
    const DM2_V1_Party *party;
    const DM2_V1_WeatherState *weather;
    const DM2_V1_UpdateWeatherState *weather_chain;
    int weather_light_valid;
    int weather_chain_started;
    const uint8_t *savegames1;
    uint32_t savegames1_size;
    const uint32_t *champion_inventory_objects;
    uint32_t champion_inventory_count;
    uint32_t leader_hand_object;
    int16_t source_light_level;
    uint16_t gdat_scene_rain;
    uint16_t gdat_scene_highest_light_level;
    uint16_t gdat_ambient_darkness;
    uint16_t gdat_scene_ambient_light;
} DM2_V1_LightSourceIdentityInputs;

typedef struct {
    uint64_t map;
    uint64_t dungeon;
    uint64_t records;
    uint64_t gdat;
    uint64_t party_light;
    uint64_t weather;
    uint64_t combined;
    int valid;
} DM2_V1_LightSourceIdentity;

int dm2_v1_light_source_identity(
    const DM2_V1_LightSourceIdentityInputs *inputs,
    DM2_V1_LightSourceIdentity *out);
/* Fold every byte of a live 64-bit identity into the terminal receipt ABI. */
uint32_t dm2_v1_light_source_identity_fold(uint64_t identity);

#endif
