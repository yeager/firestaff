#ifndef FIRESTAFF_DM1_V1_VIEWPORT_SOURCE_FRAME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_SOURCE_FRAME_PC34_COMPAT_H

/*
 * Source-owned wall/door/sensor viewport frame.
 *
 * ReDMCSB DUNVIEW.C F0104/F0111/F0115/F0128 gets its pixels from the
 * loaded graphics catalog and its visibility facts from DUNGEON.DAT.  This
 * adapter is deliberately below M11: it turns already-decoded PC34 source
 * rectangles into indexed pixels only after validating every source input.
 * It has no generated texture, colour, or fallback branch.
 */

#include "dm1_v1_floor_feature_material_pc34_compat.h"
#include "dm1_v1_viewport_wall_field_material_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_VIEWPORT_SOURCE_FRAME_MAX_LAYERS_PC34 = 16
};

typedef enum DM1_V1_ViewportSourceLayerKindPc34 {
    DM1_V1_VIEWPORT_SOURCE_LAYER_WALL_PC34 = 1,
    DM1_V1_VIEWPORT_SOURCE_LAYER_DOOR_PC34 = 2,
    DM1_V1_VIEWPORT_SOURCE_LAYER_SENSOR_PC34 = 3
} DM1_V1_ViewportSourceLayerKindPc34;

typedef struct DM1_V1_ViewportSourceLayerPc34 {
    DM1_V1_ViewportSourceLayerKindPc34 kind;
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
    int transparentColor; /* -1 retains every source pixel. */
    int paletteMapValid;
    unsigned char paletteMap[16];
    /* A sensor layer is enabled only with an immutable source record.  The
     * caller supplies the exact raw C15/C127 record it decoded from the live
     * DUNGEON.DAT Thing table; this adapter never fabricates one. */
    const unsigned char *sensorRecord;
    int sensorRecordByteCount;
    uint32_t sensorRecordFNV1a;
} DM1_V1_ViewportSourceLayerPc34;

typedef struct DM1_V1_ViewportSourceFrameInputPc34 {
    const DM1_V1_FloorFeatureSourceMaterialPc34 *materials;
    int materialCount;
    const DM1_V1_ViewportDungeonProvenancePc34 *dungeonProvenance;
    const DM1_V1_ViewportSourceLayerPc34 *layers;
    int layerCount;
} DM1_V1_ViewportSourceFrameInputPc34;

typedef struct DM1_V1_ViewportSourceFrameReceiptPc34 {
    int valid;
    int layerCount;
    int wallLayerCount;
    int doorLayerCount;
    int sensorLayerCount;
    uint32_t dungeonBytesFNV1a;
    uint32_t sourceFrameFNV1a;
    uint32_t sourcePixelsFNV1a[DM1_V1_VIEWPORT_SOURCE_FRAME_MAX_LAYERS_PC34];
    uint32_t sensorRecordFNV1a[DM1_V1_VIEWPORT_SOURCE_FRAME_MAX_LAYERS_PC34];
} DM1_V1_ViewportSourceFrameReceiptPc34;

/* Performs the whole preflight.  It returns no receipt unless all source
 * rectangles, palette maps, dungeon provenance, and sensor records verify. */
int dm1_v1_viewport_source_frame_preflight_pc34(
    const DM1_V1_ViewportSourceFrameInputPc34 *input,
    DM1_V1_ViewportSourceFrameReceiptPc34 *outReceipt);

/* Applies the preflighted original indexed pixels to a destination.  The
 * supplied receipt must be the exact result for input, so a changed source,
 * DUNGEON.DAT byte, or layer list fails before it touches framebuffer. */
int dm1_v1_viewport_source_frame_render_pc34(
    const DM1_V1_ViewportSourceFrameInputPc34 *input,
    const DM1_V1_ViewportSourceFrameReceiptPc34 *receipt,
    unsigned char *framebuffer,
    int framebufferWidth,
    int framebufferHeight);

const char *dm1_v1_viewport_source_frame_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_SOURCE_FRAME_PC34_COMPAT_H */
