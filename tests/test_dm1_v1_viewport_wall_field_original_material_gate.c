/* Real-PC34 admission for the non-door, non-inscription F0110/F0112/F0113
 * viewport lanes. No generated bitmap or dungeon byte is admitted. */

#include "asset_loader_m11.h"
#include "dm1_v1_viewport_wall_field_material_pc34_compat.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kMaterialCount = 4 };

/* D3L wall C106; global ornament 1 / G0205 view row 2 is C261
 * (C259 + 1 * 2 + native offset 0); F0113 field C076 and mask C070. */
static const int kGraphics[kMaterialCount] = { 106, 261, 76, 70 };

static unsigned char* read_dungeon_archive(int* outSize)
{
    const char* archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    size_t size = 0u;
    uint8_t* bytes = NULL;
    if (outSize) *outSize = 0;
    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive, "DATA/DUNGEON.DAT",
                                        &bytes, &size) != 0 ||
        !bytes || size == 0u || size > 0x7fffffffu) {
        free(bytes);
        return NULL;
    }
    if (outSize) *outSize = (int)size;
    return bytes;
}

static int find_visible_open_teleporter(const unsigned char* bytes, int count)
{
    int i;
    for (i = 0; i < count; ++i) {
        if (((bytes[i] >> 5) & 7) == 5 && (bytes[i] & 0x0c) == 0x0c) return i;
    }
    return -1;
}

int main(void)
{
    char graphicsPath[2048];
    const char* archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    M11_AssetLoader loader;
    DM1_V1_FloorFeatureSourceMaterialPc34 materials[kMaterialCount];
    DM1_ViewportLaneVisibilityReceiptPc34 visibility;
    DM1_ViewportSideWallHostReceiptPc34 wall;
    DM1_FieldRenderPlanPc34 field;
    DM1_V1_ViewportDungeonProvenancePc34 provenance;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 wallReceipt;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 ornamentReceipt;
    DM1_V1_ViewportWallFieldMaterialReceiptPc34 fieldReceipt;
    const int valid[3] = { 1, 1, 1 };
    const int open[3] = { 1, 1, 1 };
    const int doors[3] = { 0, 0, 0 };
    unsigned char* dungeon;
    int dungeonSize;
    int teleporterOffset;
    int i;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_DOS_PC34_ARCHIVE is not selected");
        return 77;
    }
    snprintf(graphicsPath, sizeof(graphicsPath), "%s::DATA/GRAPHICS.DAT",
             archive);
    dungeon = read_dungeon_archive(&dungeonSize);
    if (!dungeon || !M11_AssetLoader_Init(&loader, graphicsPath)) {
        free(dungeon);
        fputs("original PC34 ZIP GRAPHICS.DAT/DUNGEON.DAT is unavailable\n", stderr);
        return 1;
    }
    memset(materials, 0, sizeof(materials));
    for (i = 0; i < kMaterialCount; ++i) {
        const M11_AssetSlot* slot = M11_AssetLoader_Load(&loader, kGraphics[i]);
        if (!slot || !slot->loaded || !slot->pixels || !slot->width || !slot->height) goto fail;
        materials[i].graphicsDatOwned = 1;
        materials[i].graphicIndex = kGraphics[i];
        materials[i].width = slot->width;
        materials[i].height = slot->height;
        materials[i].indexedPixels = slot->pixels;
        materials[i].indexedPixelCount = (int)slot->width * (int)slot->height;
        materials[i].pixelsFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
            slot->pixels, materials[i].indexedPixelCount);
    }
    memset(&provenance, 0, sizeof(provenance));
    provenance.dungeonDatOwned = 1;
    provenance.rawBytes = dungeon;
    provenance.rawByteCount = dungeonSize;
    provenance.rawBytesFNV1a = DM1_V1_FloorFeatureFNV1aPc34(dungeon, dungeonSize);
    provenance.squareByteOffset = 0;
    provenance.squareByte = dungeon[0];
    visibility = dm1_viewport_3d_lane_visibility_from_cells_pc34(
        valid, open, doors, open, open);
    if (!dm1_viewport_3d_build_side_wall_host_receipt_pc34(
            DM1_VIEW_SQUARE_D3L, 0, 0, 1, 0, 3, &visibility, &wall) ||
        !dm1_v1_viewport_wall_original_material_receipt_pc34(
            &wall, materials, kMaterialCount, &provenance, 0, &wallReceipt) ||
        !dm1_v1_viewport_wall_ornament_original_material_receipt_pc34(
            1, 2, 0, materials, kMaterialCount, &provenance, 0,
            &ornamentReceipt)) {
        fputs("real PC34 wall/ornament material was rejected\n", stderr);
        goto fail;
    }
    teleporterOffset = find_visible_open_teleporter(dungeon, dungeonSize);
    if (teleporterOffset < 0 || !dm1_v1_field_render_plan_at_pc34(0, &field)) {
        fputs("real PC34 visible/open teleporter source was not found\n", stderr);
        goto fail;
    }
    provenance.squareByteOffset = teleporterOffset;
    provenance.squareByte = dungeon[teleporterOffset];
    if (!dm1_v1_viewport_field_original_material_receipt_pc34(
            &field, materials, kMaterialCount, &provenance, 0, &fieldReceipt) ||
        !(wallReceipt.drawPhase < ornamentReceipt.drawPhase &&
          ornamentReceipt.drawPhase < fieldReceipt.drawPhase) ||
        ornamentReceipt.paletteMapFNV1a == fieldReceipt.paletteMapFNV1a) {
        fputs("real PC34 field material/order was rejected\n", stderr);
        goto fail;
    }
    ++materials[3].pixelsFNV1a;
    if (dm1_v1_viewport_field_original_material_receipt_pc34(
            &field, materials, kMaterialCount, &provenance, 0, &fieldReceipt)) {
        fputs("tampered field mask was admitted\n", stderr);
        goto fail;
    }
    M11_AssetLoader_Shutdown(&loader);
    free(dungeon);
    puts("ok: original PC34 ZIP wall/ornament/field material, palette and draw-order gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    free(dungeon);
    return 1;
}
