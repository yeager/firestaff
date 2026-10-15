#ifndef FIRESTAFF_DM2_V1_ARRANGE_DUNGEON_OWNER_H
#define FIRESTAFF_DM2_V1_ARRANGE_DUNGEON_OWNER_H

#include "dm2_v1_caii_source_owner.h"

/* Complete skmap.cpp:715-1255 private transformation. GAME_LOAD publishes its
 * synchronized dungeon/pools only after the preceding weather transaction.
 * Queries must read authenticated source data; zero is a valid GDAT value,
 * so callbacks report success separately. They must not mutate session state.
 * locate_other_level implements DM_LOCATE_OTHER_LEVEL(map,+1,&x,&y,NULL),
 * including original map offsets and level selection. No fallback is used. */
typedef struct {
    void *context;
    int (*pit_gdat_6a)(void *context, int map, uint16_t *value);
    int (*locate_other_level)(void *context, int map, int *x, int *y,
                              int *result_map);
} DM2_V1_ArrangeDungeonQueries;

typedef struct {
    DM2_V1_RecordPoolSet pools;
    DM2_V1_DropRng rng;
    uint8_t *tiles; /* owned, maps concatenated in source X/Y order */
    size_t tile_count;
    size_t map_offsets[DM2_V1_MAX_LEVELS];
    uint32_t record_visits;
    uint32_t rng_draws;
    uint32_t teleporter_merges;
    int valid;
} DM2_V1_ArrangeDungeonOwner;

void dm2_v1_arrange_dungeon_owner_free(DM2_V1_ArrangeDungeonOwner *owner);
/* out must be empty. On failure it is zeroed and all inputs are unchanged.
 * The returned owner is the complete candidate; publishing it together with
 * its pools, tiles and RNG belongs to the eventual session transaction. */
int dm2_v1_arrange_dungeon_owner_build(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_CaiiSourceOwner *caii,
    const DM2_V1_DropRng *rng,
    const DM2_V1_ArrangeDungeonQueries *queries,
    DM2_V1_ArrangeDungeonOwner *out);
/* Serialize the complete private candidate and reload its dungeon metadata.
 * out must be empty. On failure every caller-owned object, including out,
 * is unchanged. On success out owns its allocation (dungeon_free releases it).
 * This does not publish the candidate RNG or install a GAME_LOAD session. */
int dm2_v1_arrange_dungeon_owner_reload(
    const DM2_V1_DungeonData *original,
    const DM2_V1_ArrangeDungeonOwner *arranged,
    DM2_V1_DungeonData *out);
/* Bind original optional GDAT pit flags and LOCATE_OTHER_LEVEL map-order
 * queries directly to this admitted dungeon/asset allocation. */
int dm2_v1_arrange_dungeon_owner_build_from_assets(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_RecordPoolSet *pools,
    const DM2_V1_CaiiSourceOwner *caii,
    const DM2_V1_DropRng *rng,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_ArrangeDungeonOwner *out);
#endif
