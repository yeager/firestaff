#include "dm2_v1_g1_scene_runtime_bridge.h"

#include <string.h>

static int dm2_v1_g1_creature_map_chip_gdat_index(int creature_type)
{
    if (creature_type < 0 || creature_type > 0xff) return 0;
    /* Same virtual GDAT address as viewport_renderer.c's
     * dm2_v1_viewport_creature_graphic_index(type, 0): CREATURES/type/F9. */
    return DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE -
        (creature_type << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT);
}

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
    int creature_type = -1;
    uint32_t palette_hash = 0u;

    if (!out || !dungeon || !resolve || !asset_fetch || !palette_fetch) return 0;
    memset(&candidate, 0, sizeof(candidate));
    if (!dm2_v1_dungeon_classify_g1_direct_root_scene(
            dungeon, level, x, y, &candidate.scene)) {
        return 0;
    }
    /* skproject DME.h::Creature::CreatureType is DB4 b4. Its map-chip
     * route is QUERY_DUNGEON_MAP_CHIP_PICT(CREATURES, type, F9), represented
     * by the same virtual GDAT index consumed by the viewport renderer.
     * Door selection still needs unconsumed DB0 fields and remains blocked. */
    if (candidate.scene.root_class == DM2_V1_G1_SCENE_ROOT_CREATURE) {
        const DM2_V1_G1DirectChainNode *root = &candidate.scene.chain.nodes[0];
        if (root->type != 4 || root->record_size < 5 ||
            root->record_offset < 0 || root->record_offset + root->record_size >
                dungeon->raw_size) {
            candidate.blocked = 1;
            *out = candidate;
            return 0;
        }
        creature_type = dungeon->raw_data[root->record_offset + 4];
        gdat_index = dm2_v1_g1_creature_map_chip_gdat_index(creature_type);
    } else if (!resolve(resolve_user, candidate.scene.tile_class,
                        candidate.scene.root_class, &gdat_index)) {
        candidate.blocked = 1;
        *out = candidate;
        return 0;
    }
    if (gdat_index == 0 ||
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
    candidate.creature_type = creature_type;
    candidate.material_width = width;
    candidate.material_height = height;
    candidate.material_stride = stride;
    candidate.material_pixels = pixels;
    memcpy(candidate.material_palette16, palette16,
           sizeof(candidate.material_palette16));
    candidate.material_palette_hash = palette_hash;
    candidate.valid = 1;
    *out = candidate;
    return 1;
}
