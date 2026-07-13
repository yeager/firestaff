#ifndef FIRESTAFF_DM2_V1_GDAT_SCENE_M11_COMMAND_H
#define FIRESTAFF_DM2_V1_GDAT_SCENE_M11_COMMAND_H

#include "dm2_v1_asset_loader.h"

typedef struct {
    uint8_t field;
    uint8_t *pixels;
    uint16_t width;
    uint16_t height;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t raw_hash;
    uint32_t palette_hash;
} DM2_V1_GdatSceneM11Command;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint16_t scene_colorkey;
    uint16_t scene_flags;
    uint16_t highest_light_level;
    uint16_t ambient_darkness;
    uint32_t command_hash;
    DM2_V1_GdatSceneM11Command commands[2];
} DM2_V1_GdatSceneM11CommandPlan;

/* skproject c_gui_vp: the active MapGraphicsStyle's control words and floor/
 * ceiling IMG3s form one transaction. Missing any source record emits no M11
 * scene command and never borrows another graphics set. */
int dm2_v1_gdat_scene_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatSceneM11CommandPlan *out_plan);
void dm2_v1_gdat_scene_m11_command_plan_free(
    DM2_V1_GdatSceneM11CommandPlan *plan);

#endif
