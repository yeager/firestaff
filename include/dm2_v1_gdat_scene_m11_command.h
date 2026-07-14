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

/* SKProject QUERY_BLIT_RECT obtains the floor and ceiling destinations from
 * INTERFACE_GENERAL/0/dt04/0.  This receipt proves that the original table
 * owns the two named compressed records, but deliberately does not assign
 * coordinates: their compressed program grammar has not been established. */
#define DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER   700u
#define DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER 701u

typedef struct {
    int valid;
    uint16_t floor_rect_number;
    uint16_t ceiling_rect_number;
    uint32_t table_hash;
    uint32_t floor_row_hash;
    uint32_t ceiling_row_hash;
} DM2_V1_GdatSceneQueryBlitRectReceipt;

/* skproject c_gui_vp: the active MapGraphicsStyle's control words and floor/
 * ceiling IMG3s form one transaction. Missing any source record emits no M11
 * scene command and never borrows another graphics set. */
int dm2_v1_gdat_scene_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatSceneM11CommandPlan *out_plan);

/* Source-only admission for QUERY_BLIT_RECT records 700/701. This is not a
 * rectangle expander and cannot authorize a floor/ceiling draw by itself. */
int dm2_v1_gdat_scene_query_blit_rect_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatSceneQueryBlitRectReceipt *out_receipt);
void dm2_v1_gdat_scene_m11_command_plan_free(
    DM2_V1_GdatSceneM11CommandPlan *plan);

#endif
