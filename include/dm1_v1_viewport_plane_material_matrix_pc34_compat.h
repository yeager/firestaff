#ifndef FIRESTAFF_DM1_V1_VIEWPORT_PLANE_MATERIAL_MATRIX_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_PLANE_MATERIAL_MATRIX_PC34_COMPAT_H

/*
 * DM1 PC34 floor/ceiling viewport material matrix.
 *
 * ReDMCSB DUNVIEW.C F0094 selects a pair of GRAPHICS.DAT bitmaps for the
 * current floor set, then F0098 composites those decoded pixels.  The
 * existing viewport source-frame contract accepts supplied surfaces; this
 * module owns the missing preceding step: it obtains that pair directly from
 * the active PC34 asset decoder, fingerprints it, and makes stale or
 * substituted surfaces fail closed before a viewport blit.
 */

#include "asset_loader_m11.h"
#include "dm1_v1_floor_feature_material_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_VIEWPORT_PLANE_MATERIAL_MAX_PC34 = 2
};

typedef enum DM1_V1_ViewportPlaneKindPc34 {
    DM1_V1_VIEWPORT_PLANE_FLOOR_PC34 = 1,
    DM1_V1_VIEWPORT_PLANE_CEILING_PC34 = 2
} DM1_V1_ViewportPlaneKindPc34;

typedef struct DM1_V1_ViewportPlaneBlitPc34 {
    DM1_V1_ViewportPlaneKindPc34 kind;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
    int transparentColor; /* -1 copies all original indexed pixels. */
    int paletteMapValid;
    unsigned char paletteMap[16];
} DM1_V1_ViewportPlaneBlitPc34;

typedef struct DM1_V1_ViewportPlaneMaterialMatrixPc34 {
    int valid;
    int floorSet;
    int planeCount;
    DM1_V1_FloorFeatureSourceMaterialPc34
        surfaces[DM1_V1_VIEWPORT_PLANE_MATERIAL_MAX_PC34];
} DM1_V1_ViewportPlaneMaterialMatrixPc34;

typedef struct DM1_V1_ViewportPlaneMaterialReceiptPc34 {
    int valid;
    int floorSet;
    int planeCount;
    unsigned int graphicIndex[DM1_V1_VIEWPORT_PLANE_MATERIAL_MAX_PC34];
    uint32_t sourcePixelsFNV1a[DM1_V1_VIEWPORT_PLANE_MATERIAL_MAX_PC34];
    uint32_t paletteFNV1a[DM1_V1_VIEWPORT_PLANE_MATERIAL_MAX_PC34];
    uint32_t graphicsPathFNV1a;
    uint32_t matrixFNV1a;
} DM1_V1_ViewportPlaneMaterialReceiptPc34;

/* Decodes the requested F0094 floor-set planes through the active original
 * GRAPHICS.DAT loader.  The requested layers must contain one or both of the
 * exact F0094 floor/ceiling members; arbitrary graphic indices are rejected. */
int dm1_v1_viewport_plane_material_matrix_decode_pc34(
    M11_AssetLoader *loader,
    int floorSet,
    const DM1_V1_ViewportPlaneBlitPc34 *planes,
    int planeCount,
    DM1_V1_ViewportPlaneMaterialMatrixPc34 *outMatrix,
    DM1_V1_ViewportPlaneMaterialReceiptPc34 *outReceipt);

/* Revalidates the complete real-data matrix and composites its requested
 * source rectangles.  A different GRAPHICS.DAT path, bitmap byte stream,
 * F0094 index, palette map, or rectangle does not draw. */
int dm1_v1_viewport_plane_material_matrix_render_pc34(
    M11_AssetLoader *loader,
    int floorSet,
    const DM1_V1_ViewportPlaneBlitPc34 *planes,
    int planeCount,
    const DM1_V1_ViewportPlaneMaterialMatrixPc34 *matrix,
    const DM1_V1_ViewportPlaneMaterialReceiptPc34 *receipt,
    unsigned char *framebuffer,
    int framebufferWidth,
    int framebufferHeight);

const char *dm1_v1_viewport_plane_material_matrix_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_PLANE_MATERIAL_MATRIX_PC34_COMPAT_H */
