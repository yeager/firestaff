#ifndef FIRESTAFF_DM2_V1_G1_SCENE_RUNTIME_BRIDGE_H
#define FIRESTAFF_DM2_V1_G1_SCENE_RUNTIME_BRIDGE_H

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"

/* Resolves only a source-proven scene class to its GDAT image. Returning zero
 * deliberately blocks the handoff; it must never choose a generic substitute. */
typedef int (*DM2_V1_G1SceneGdatResolve)(
    void *user,
    DM2_V1_G1SceneTileClass tile_class,
    DM2_V1_G1SceneRootClass root_class,
    int *out_gdat_index);

typedef struct {
    int valid;
    int blocked;
    DM2_V1_G1DungeonSceneClassificationReceipt scene;
    int gdat_index;
    int creature_type;
    int material_width;
    int material_height;
    int material_stride;
    uint32_t material_palette_hash;
} DM2_V1_G1SceneRuntimeHandoffReceipt;

/* c_map.cpp selects the tile/root chain before c_gui_vp.cpp issues its GDAT
 * draw. This bridge preserves that boundary: classification is accepted only
 * with a resolver-selected, decoded image and its exact local palette. */
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
    DM2_V1_G1SceneRuntimeHandoffReceipt *out);

#endif
