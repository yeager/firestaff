#include "dm1_v1_viewport_source_frame_pc34_compat.h"

#include <string.h>

static uint32_t fnv_mix_u32(uint32_t hash, uint32_t value)
{
    int i;
    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static const DM1_V1_FloorFeatureSourceMaterialPc34 *find_surface(
    const DM1_V1_ViewportSourceFrameInputPc34 *input,
    const DM1_V1_ViewportSourceLayerPc34 *layer,
    uint32_t *outHash)
{
    int i;
    if (outHash) *outHash = 0u;
    if (!input || !layer || !input->materials || input->materialCount <= 0 ||
        layer->graphicIndex < 0 || layer->srcX < 0 || layer->srcY < 0 ||
        layer->width <= 0 || layer->height <= 0) return NULL;
    for (i = 0; i < input->materialCount; ++i) {
        const DM1_V1_FloorFeatureSourceMaterialPc34 *surface =
            &input->materials[i];
        uint32_t hash;
        if (!surface->graphicsDatOwned || surface->graphicIndex != layer->graphicIndex ||
            !surface->indexedPixels || surface->width <= 0 || surface->height <= 0 ||
            surface->indexedPixelCount < surface->width * surface->height ||
            layer->srcX + layer->width > surface->width ||
            layer->srcY + layer->height > surface->height) continue;
        hash = DM1_V1_FloorFeatureFNV1aPc34(surface->indexedPixels,
                                             surface->indexedPixelCount);
        if (hash == 0u || hash != surface->pixelsFNV1a) continue;
        if (outHash) *outHash = hash;
        return surface;
    }
    return NULL;
}

static int layer_is_valid(const DM1_V1_ViewportSourceFrameInputPc34 *input,
                          const DM1_V1_ViewportSourceLayerPc34 *layer,
                          uint32_t *outPixelsHash,
                          uint32_t *outSensorHash)
{
    uint32_t sensorHash = 0u;
    int i;
    if (outPixelsHash) *outPixelsHash = 0u;
    if (outSensorHash) *outSensorHash = 0u;
    if (!input || !layer || layer->kind < DM1_V1_VIEWPORT_SOURCE_LAYER_WALL_PC34 ||
        layer->kind > DM1_V1_VIEWPORT_SOURCE_LAYER_SENSOR_PC34 ||
        layer->dstX < 0 || layer->dstY < 0 || layer->transparentColor < -1 ||
        layer->transparentColor > 15 || !find_surface(input, layer, outPixelsHash)) {
        return 0;
    }
    if (layer->paletteMapValid) {
        for (i = 0; i < 16; ++i) {
            if (layer->paletteMap[i] > 15) return 0;
        }
    }
    if (layer->kind != DM1_V1_VIEWPORT_SOURCE_LAYER_SENSOR_PC34) return 1;
    if (!layer->sensorRecord || layer->sensorRecordByteCount <= 0) return 0;
    sensorHash = DM1_V1_FloorFeatureFNV1aPc34(layer->sensorRecord,
                                                layer->sensorRecordByteCount);
    if (sensorHash == 0u || sensorHash != layer->sensorRecordFNV1a) return 0;
    if (outSensorHash) *outSensorHash = sensorHash;
    return 1;
}

const char *dm1_v1_viewport_source_frame_source_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C F0104/F0111/F0115/F0128 consumes decoded "
           "GRAPHICS.DAT source rectangles after DUNGEON.C F0172 publishes "
           "the visible square; MOVESENS.C F0275 supplies live wall-sensor "
           "records. Missing or changed source material is no draw.";
}

int dm1_v1_viewport_source_frame_preflight_pc34(
    const DM1_V1_ViewportSourceFrameInputPc34 *input,
    DM1_V1_ViewportSourceFrameReceiptPc34 *outReceipt)
{
    DM1_V1_ViewportSourceFrameReceiptPc34 receipt;
    uint32_t hash = 2166136261u;
    int i;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!input || !input->layers || input->layerCount <= 0 ||
        input->layerCount > DM1_V1_VIEWPORT_SOURCE_FRAME_MAX_LAYERS_PC34 ||
        !dm1_v1_viewport_dungeon_provenance_is_valid_pc34(input->dungeonProvenance)) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.layerCount = input->layerCount;
    receipt.dungeonBytesFNV1a = input->dungeonProvenance->rawBytesFNV1a;
    hash = fnv_mix_u32(hash, receipt.dungeonBytesFNV1a);
    for (i = 0; i < input->layerCount; ++i) {
        const DM1_V1_ViewportSourceLayerPc34 *layer = &input->layers[i];
        uint32_t pixelsHash;
        uint32_t sensorHash;
        if (!layer_is_valid(input, layer, &pixelsHash, &sensorHash)) return 0;
        receipt.sourcePixelsFNV1a[i] = pixelsHash;
        receipt.sensorRecordFNV1a[i] = sensorHash;
        if (layer->kind == DM1_V1_VIEWPORT_SOURCE_LAYER_WALL_PC34) ++receipt.wallLayerCount;
        if (layer->kind == DM1_V1_VIEWPORT_SOURCE_LAYER_DOOR_PC34) ++receipt.doorLayerCount;
        if (layer->kind == DM1_V1_VIEWPORT_SOURCE_LAYER_SENSOR_PC34) ++receipt.sensorLayerCount;
        hash = fnv_mix_u32(hash, (uint32_t)layer->kind);
        hash = fnv_mix_u32(hash, (uint32_t)layer->graphicIndex);
        hash = fnv_mix_u32(hash, pixelsHash);
        hash = fnv_mix_u32(hash, sensorHash);
        hash = fnv_mix_u32(hash, (uint32_t)layer->srcX);
        hash = fnv_mix_u32(hash, (uint32_t)layer->srcY);
        hash = fnv_mix_u32(hash, (uint32_t)layer->dstX);
        hash = fnv_mix_u32(hash, (uint32_t)layer->dstY);
        hash = fnv_mix_u32(hash, (uint32_t)layer->width);
        hash = fnv_mix_u32(hash, (uint32_t)layer->height);
    }
    receipt.sourceFrameFNV1a = hash;
    receipt.valid = hash != 0u;
    if (!receipt.valid) return 0;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_viewport_source_frame_render_pc34(
    const DM1_V1_ViewportSourceFrameInputPc34 *input,
    const DM1_V1_ViewportSourceFrameReceiptPc34 *receipt,
    unsigned char *framebuffer,
    int framebufferWidth,
    int framebufferHeight)
{
    DM1_V1_ViewportSourceFrameReceiptPc34 verified;
    int i;
    if (!receipt || !framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0 ||
        !dm1_v1_viewport_source_frame_preflight_pc34(input, &verified) ||
        !receipt->valid || memcmp(receipt, &verified, sizeof(verified)) != 0) return 0;
    for (i = 0; i < input->layerCount; ++i) {
        const DM1_V1_ViewportSourceLayerPc34 *layer = &input->layers[i];
        const DM1_V1_FloorFeatureSourceMaterialPc34 *surface =
            find_surface(input, layer, NULL);
        int y;
        if (!surface) return 0;
        for (y = 0; y < layer->height; ++y) {
            int x;
            const int dstY = layer->dstY + y;
            if (dstY < 0 || dstY >= framebufferHeight) continue;
            for (x = 0; x < layer->width; ++x) {
                const int dstX = layer->dstX + x;
                unsigned char pixel;
                if (dstX < 0 || dstX >= framebufferWidth) continue;
                pixel = surface->indexedPixels[(layer->srcY + y) * surface->width +
                                               layer->srcX + x];
                if (layer->transparentColor >= 0 && pixel == (unsigned char)layer->transparentColor)
                    continue;
                if (layer->paletteMapValid) pixel = layer->paletteMap[pixel & 15u];
                framebuffer[dstY * framebufferWidth + dstX] = pixel;
            }
        }
    }
    return 1;
}
