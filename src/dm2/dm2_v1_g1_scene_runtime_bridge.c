#include "dm2_v1_g1_scene_runtime_bridge.h"

#include <string.h>

int dm2_v1_g1_scene_runtime_handoff(
    const DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    DM2_V1_G1SceneGdatResolve resolve,
    void *resolve_user,
    DM2_V1_ViewportAssetFetch asset_fetch,
    void *asset_user,
    DM2_V1_ViewportAssetPaletteFetch palette_fetch,
    void *palette_user,
    DM2_V1_G1SceneRuntimeHandoffReceipt *out)
{
    DM2_V1_G1SceneRuntimeHandoffReceipt candidate;
    const uint8_t *pixels = NULL;
    uint8_t palette16[16];
    int width = 0;
    int height = 0;
    int stride = 0;
    int gdat_index = 0;
    uint32_t palette_hash = 0u;

    if (!out || !dungeon || !resolve || !asset_fetch || !palette_fetch) return 0;
    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
            dungeon, level, x, y, &candidate.scene)) {
        return 0;
    }
    /* DME.h::Door and ::Creature need payload fields to select a graphic.
     * This bridge owns no such fields, so their resolver must fail closed. */
    if (!resolve(resolve_user, candidate.scene.tile_class,
                 candidate.scene.root_class, &gdat_index) ||
        gdat_index <= 0 ||
        asset_fetch(asset_user, gdat_index, &pixels, &width, &height,
                    &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width ||
        palette_fetch(palette_user, gdat_index, palette16, &palette_hash) != 0 ||
        palette_hash == 0u) {
        candidate.blocked = 1;
        *out = candidate;
        return 0;
    }
    candidate.gdat_index = gdat_index;
    candidate.material_width = width;
    candidate.material_height = height;
    candidate.material_stride = stride;
    candidate.material_palette_hash = palette_hash;
    candidate.valid = 1;
    *out = candidate;
    return 1;
}
