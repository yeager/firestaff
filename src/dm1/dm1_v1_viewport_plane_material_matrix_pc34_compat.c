#include "dm1_v1_viewport_plane_material_matrix_pc34_compat.h"

#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <string.h>

static uint32_t fnv_bytes(const unsigned char *bytes, int count)
{
    return DM1_V1_FloorFeatureFNV1aPc34(bytes, count);
}

static uint32_t fnv_u32(uint32_t hash, uint32_t value)
{
    int shift;
    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t plane_palette_hash(const DM1_V1_ViewportPlaneBlitPc34 *plane)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!plane) return 0u;
    if (plane->paletteMapValid) {
        for (i = 0; i < 16; ++i) {
            if (plane->paletteMap[i] > 15) return 0u;
            hash ^= plane->paletteMap[i];
            hash *= 16777619u;
        }
    }
    hash = fnv_u32(hash, (uint32_t)plane->paletteMapValid);
    return fnv_u32(hash, (uint32_t)(plane->transparentColor + 1));
}

static int plane_graphic_index(int floorSet, DM1_V1_ViewportPlaneKindPc34 kind,
                               unsigned int *outIndex)
{
    unsigned int index;
    if (!outIndex || floorSet < 0 || kind < DM1_V1_VIEWPORT_PLANE_FLOOR_PC34 ||
        kind > DM1_V1_VIEWPORT_PLANE_CEILING_PC34) return 0;
    index = kind == DM1_V1_VIEWPORT_PLANE_FLOOR_PC34 ?
        dm1_floor_set_floor_graphic(floorSet) :
        dm1_floor_set_ceiling_graphic(floorSet);
    *outIndex = index;
    return 1;
}

static int plane_is_valid(const DM1_V1_ViewportPlaneBlitPc34 *plane,
                          const M11_AssetSlot *slot)
{
    if (!plane || !slot || !slot->loaded || !slot->pixels ||
        plane->srcX < 0 || plane->srcY < 0 || plane->dstX < 0 ||
        plane->dstY < 0 || plane->width <= 0 || plane->height <= 0 ||
        plane->transparentColor < -1 || plane->transparentColor > 15 ||
        plane->srcX + plane->width > (int)slot->width ||
        plane->srcY + plane->height > (int)slot->height) return 0;
    return plane_palette_hash(plane) != 0u;
}

const char *dm1_v1_viewport_plane_material_matrix_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C F0094 selects GRAPHICS.DAT 78 + floorSet * 2 "
           "and its ceiling sibling; F0098 composites their decoded PC34 pixels. "
           "Missing or substituted source planes are no draw.";
}

int dm1_v1_viewport_plane_material_matrix_decode_pc34(
    M11_AssetLoader *loader,
    int floorSet,
    const DM1_V1_ViewportPlaneBlitPc34 *planes,
    int planeCount,
    DM1_V1_ViewportPlaneMaterialMatrixPc34 *outMatrix,
    DM1_V1_ViewportPlaneMaterialReceiptPc34 *outReceipt)
{
    DM1_V1_ViewportPlaneMaterialMatrixPc34 matrix;
    DM1_V1_ViewportPlaneMaterialReceiptPc34 receipt;
    uint32_t matrixHash = 2166136261u;
    int seenFloor = 0;
    int seenCeiling = 0;
    int i;

    if (!outMatrix || !outReceipt) return 0;
    memset(outMatrix, 0, sizeof(*outMatrix));
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!loader || !M11_AssetLoader_IsReady(loader) || !loader->graphicsDatPath[0] ||
        !planes || planeCount <= 0 ||
        planeCount > DM1_V1_VIEWPORT_PLANE_MATERIAL_MAX_PC34 || floorSet < 0) return 0;

    memset(&matrix, 0, sizeof(matrix));
    memset(&receipt, 0, sizeof(receipt));
    matrix.floorSet = floorSet;
    matrix.planeCount = planeCount;
    receipt.floorSet = floorSet;
    receipt.planeCount = planeCount;
    receipt.graphicsPathFNV1a = fnv_bytes((const unsigned char *)loader->graphicsDatPath,
                                           (int)strlen(loader->graphicsDatPath));
    if (receipt.graphicsPathFNV1a == 0u) return 0;
    matrixHash = fnv_u32(matrixHash, receipt.graphicsPathFNV1a);

    for (i = 0; i < planeCount; ++i) {
        const DM1_V1_ViewportPlaneBlitPc34 *plane = &planes[i];
        const M11_AssetSlot *slot;
        DM1_V1_FloorFeatureSourceMaterialPc34 *surface = &matrix.surfaces[i];
        uint32_t pixelsHash;
        uint32_t paletteHash;
        unsigned int graphicIndex;

        if (!plane_graphic_index(floorSet, plane->kind, &graphicIndex)) return 0;
        if ((plane->kind == DM1_V1_VIEWPORT_PLANE_FLOOR_PC34 && seenFloor++) ||
            (plane->kind == DM1_V1_VIEWPORT_PLANE_CEILING_PC34 && seenCeiling++)) return 0;
        slot = M11_AssetLoader_Load(loader, graphicIndex);
        if (!plane_is_valid(plane, slot)) return 0;
        pixelsHash = fnv_bytes(slot->pixels, (int)slot->width * (int)slot->height);
        paletteHash = plane_palette_hash(plane);
        if (pixelsHash == 0u || paletteHash == 0u) return 0;
        surface->graphicsDatOwned = 1;
        surface->graphicIndex = (int)graphicIndex;
        surface->width = (int)slot->width;
        surface->height = (int)slot->height;
        surface->indexedPixels = slot->pixels;
        surface->indexedPixelCount = surface->width * surface->height;
        surface->pixelsFNV1a = pixelsHash;
        receipt.graphicIndex[i] = graphicIndex;
        receipt.sourcePixelsFNV1a[i] = pixelsHash;
        receipt.paletteFNV1a[i] = paletteHash;
        matrixHash = fnv_u32(matrixHash, graphicIndex);
        matrixHash = fnv_u32(matrixHash, pixelsHash);
        matrixHash = fnv_u32(matrixHash, paletteHash);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->kind);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->srcX);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->srcY);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->dstX);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->dstY);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->width);
        matrixHash = fnv_u32(matrixHash, (uint32_t)plane->height);
    }
    matrix.valid = 1;
    receipt.matrixFNV1a = matrixHash;
    receipt.valid = matrixHash != 0u;
    if (!receipt.valid) return 0;
    *outMatrix = matrix;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_viewport_plane_material_matrix_render_pc34(
    M11_AssetLoader *loader,
    int floorSet,
    const DM1_V1_ViewportPlaneBlitPc34 *planes,
    int planeCount,
    const DM1_V1_ViewportPlaneMaterialMatrixPc34 *matrix,
    const DM1_V1_ViewportPlaneMaterialReceiptPc34 *receipt,
    unsigned char *framebuffer,
    int framebufferWidth,
    int framebufferHeight)
{
    DM1_V1_ViewportPlaneMaterialMatrixPc34 verifiedMatrix;
    DM1_V1_ViewportPlaneMaterialReceiptPc34 verifiedReceipt;
    int i;
    if (!matrix || !receipt || !framebuffer || framebufferWidth <= 0 ||
        framebufferHeight <= 0 || !matrix->valid || !receipt->valid ||
        !dm1_v1_viewport_plane_material_matrix_decode_pc34(
            loader, floorSet, planes, planeCount, &verifiedMatrix, &verifiedReceipt) ||
        memcmp(matrix, &verifiedMatrix, sizeof(verifiedMatrix)) != 0 ||
        memcmp(receipt, &verifiedReceipt, sizeof(verifiedReceipt)) != 0) return 0;

    for (i = 0; i < planeCount; ++i) {
        const DM1_V1_ViewportPlaneBlitPc34 *plane = &planes[i];
        const DM1_V1_FloorFeatureSourceMaterialPc34 *surface = &matrix->surfaces[i];
        int y;
        for (y = 0; y < plane->height; ++y) {
            int x;
            int dstY = plane->dstY + y;
            if (dstY < 0 || dstY >= framebufferHeight) continue;
            for (x = 0; x < plane->width; ++x) {
                int dstX = plane->dstX + x;
                unsigned char pixel;
                if (dstX < 0 || dstX >= framebufferWidth) continue;
                pixel = surface->indexedPixels[(plane->srcY + y) * surface->width +
                                               plane->srcX + x];
                if (plane->transparentColor >= 0 &&
                    pixel == (unsigned char)plane->transparentColor) continue;
                if (plane->paletteMapValid) pixel = plane->paletteMap[pixel & 15u];
                framebuffer[dstY * framebufferWidth + dstX] = pixel;
            }
        }
    }
    return 1;
}
