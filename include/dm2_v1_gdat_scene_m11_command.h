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
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t geometry_hash;
} DM2_V1_GdatSceneM11Command;

typedef struct {
    uint16_t rect_number;
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} DM2_V1_GdatSceneBlitRect;

typedef struct {
    int valid;
    uint16_t floor_rect_number;
    uint16_t ceiling_rect_number;
    uint32_t table_hash;
    uint32_t floor_row_hash;
    uint32_t ceiling_row_hash;
} DM2_V1_GdatSceneQueryBlitRectReceipt;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint16_t scene_colorkey;
    uint16_t scene_flags;
    uint16_t ambient_light;
    uint16_t highest_light_level;
    uint16_t ambient_darkness;
    uint32_t command_hash;
    DM2_V1_GdatSceneM11Command commands[2];
    /* QUERY_BLIT_RECT's source-backed destinations, indexed like commands:
     * floor first, ceiling second. The viewport consumes ceiling then floor. */
    DM2_V1_GdatSceneBlitRect rects[2];
    /* Exact INTERFACE_GENERAL/0/dt04 program that resolved rects. M11 must
     * reject a plan that cannot retain this source receipt. */
    DM2_V1_GdatSceneQueryBlitRectReceipt query_blit_rect;
    uint32_t query_blit_rect_hash;
} DM2_V1_GdatSceneM11CommandPlan;

/* skproject CHECK_RECOMPUTE_LIGHT consumes these GRAPHICSSET control words
 * with the selected scene transaction.  This receipt retains their exact
 * source identity for the M11 frame boundary; it never manufactures a light
 * level, palette, or weather state. */
typedef struct {
    int valid;
    uint8_t graphicsset;
    uint16_t ambient_light;
    uint16_t highest_light_level;
    uint16_t ambient_darkness;
    uint32_t scene_control_hash;
    uint32_t receipt_hash;
} DM2_V1_GdatSceneLightM11Receipt;

/* Exact terminal inputs to SKProject c_light.cpp::DM2_RECALC_LIGHT_LEVEL.
 * `base_light` is either the observed v1e0974 accumulator for a map whose
 * descriptor admitted dynamic light, or the source's fixed non-dynamic base
 * level. Callers must retain an authenticated raw-state hash; this API never
 * substitutes GRAPHICSSET fields for dynamic c_light state. */
typedef struct {
    int valid;
    uint8_t dynamic_map;
    uint8_t base_light;
    uint8_t darkness_offset;
    uint32_t source_state_hash;
} DM2_V1_CLightSourceState;

typedef struct {
    int valid;
    uint8_t graphicsset;
    uint8_t light_level;
    uint8_t dynamic_map;
    uint32_t scene_control_hash;
    uint32_t source_state_hash;
    uint32_t receipt_hash;
} DM2_V1_CLightM11Receipt;

/* SKProject QUERY_BLIT_RECT obtains the ceiling and floor destinations from
 * INTERFACE_GENERAL/0/dt04/0 in that order. */
#define DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER   701u
#define DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER 700u

/* skproject c_gui_vp: the active MapGraphicsStyle's control words and floor/
 * ceiling IMG3s form one transaction. Missing any source record emits no M11
 * scene command and never borrows another graphics set. */
int dm2_v1_gdat_scene_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatSceneM11CommandPlan *out_plan);
int dm2_v1_gdat_scene_light_m11_receipt(
    const DM2_V1_GdatSceneM11CommandPlan *plan,
    DM2_V1_GdatSceneLightM11Receipt *out_receipt);
int dm2_v1_c_light_m11_receipt_build(
    const DM2_V1_GdatSceneLightM11Receipt *scene,
    const DM2_V1_CLightSourceState *source,
    DM2_V1_CLightM11Receipt *out_receipt);

/* The receipt hashes records 700/701 only. Plan construction retains this
 * receipt and admits their exact x=11/14 -> x=1 -> x=9 grammar slice. */
int dm2_v1_gdat_scene_query_blit_rect_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatSceneQueryBlitRectReceipt *out_receipt);
uint32_t dm2_v1_gdat_scene_query_blit_rect_hash(
    const DM2_V1_GdatSceneQueryBlitRectReceipt *receipt);
uint32_t dm2_v1_gdat_scene_m11_command_pixel_hash(
    const DM2_V1_GdatSceneM11Command *command);
uint32_t dm2_v1_gdat_scene_m11_command_geometry_hash(
    const DM2_V1_GdatSceneM11Command *command,
    const DM2_V1_GdatSceneBlitRect *rect);
void dm2_v1_gdat_scene_m11_command_plan_free(
    DM2_V1_GdatSceneM11CommandPlan *plan);

#endif
