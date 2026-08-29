/* Real-data admission only: D0/D1 F0098/F0108/F0115 normal object path. */

#include "asset_loader_m11.h"
#include "dm1_v1_f0115_near_object_decoration_material_pc34_compat.h"
#include "dm1_v1_floor_ornament_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kMaterialCount = 4 };

static unsigned char* read_dungeon_archive(int* outSize)
{
    const char* archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    size_t size = 0u;
    uint8_t* bytes = NULL;
    if (outSize) *outSize = 0;
    if (!archive || !archive[0] ||
        firestaff_zip_extract_by_suffix(archive,
                                        "dmaster/DATA/DUNGEON.DAT",
                                        &bytes, &size) != 0 ||
        !bytes || size == 0u || size > 0x7fffffffu) {
        free(bytes);
        return NULL;
    }
    if (outSize) *outSize = (int)size;
    return bytes;
}

static int find_corridor(const unsigned char* bytes, int count)
{
    int i;
    for (i = 0; i < count; ++i) {
        if (((bytes[i] >> 5) & 7) == 1) return i;
    }
    return -1;
}

static int load_material(M11_AssetLoader* loader, int graphic,
                         DM1_V1_FloorFeatureSourceMaterialPc34* out)
{
    const M11_AssetSlot* slot = M11_AssetLoader_Load(loader, (unsigned int)graphic);
    if (!slot || !slot->loaded || !slot->pixels || !slot->width || !slot->height) return 0;
    out->graphicsDatOwned = 1;
    out->graphicIndex = graphic;
    out->width = slot->width;
    out->height = slot->height;
    out->indexedPixels = slot->pixels;
    out->indexedPixelCount = (int)slot->width * (int)slot->height;
    out->pixelsFNV1a = DM1_V1_FloorFeatureFNV1aPc34(
        slot->pixels, out->indexedPixelCount);
    return out->pixelsFNV1a != 0u;
}

static int receipt(const DM1_V1_F0115NearMaterialRequestPc34* request,
                   const DM1_V1_FloorFeatureSourceMaterialPc34* materials,
                   const DM1_V1_F0115NearDungeonProvenancePc34* provenance,
                   DM1_V1_F0115NearMaterialReceiptPc34* out)
{
    return dm1_v1_f0115_near_object_decoration_material_receipt_pc34(
        request, materials, kMaterialCount, provenance, out);
}

int main(void)
{
    char graphicsPath[2048];
    const char* archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    M11_AssetLoader loader;
    DM1_V1_FloorFeatureSourceMaterialPc34 materials[kMaterialCount];
    DM1_V1_F0115NearDungeonProvenancePc34 provenance;
    DM1_V1_F0115NearMaterialRequestPc34 request;
    DM1_V1_F0115NearMaterialReceiptPc34 floorReceipt, ceilingReceipt;
    DM1_V1_F0115NearMaterialReceiptPc34 ornamentReceipt, objectReceipt;
    unsigned char* dungeon;
    int dungeonSize;
    int corridorOffset;
    int graphics[kMaterialCount];
    int i;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_DOS_PC34_ARCHIVE is not selected");
        return 77;
    }
    snprintf(graphicsPath, sizeof(graphicsPath),
             "%s::dungeon-master/dmaster/DATA/GRAPHICS.DAT", archive);
    dungeon = read_dungeon_archive(&dungeonSize);
    if (!dungeon || !M11_AssetLoader_Init(&loader, graphicsPath)) {
        free(dungeon);
        fputs("original PC34 ZIP GRAPHICS.DAT/DUNGEON.DAT is unavailable\n", stderr);
        return 1;
    }
    graphics[0] = (int)dm1_floor_set_floor_graphic(0);
    graphics[1] = (int)dm1_floor_set_ceiling_graphic(0);
    graphics[2] = dm1_v1_floor_ornament_graphic_index_pc34(0, 5);
    graphics[3] = (int)dm1_item_sprite_index(5, 0);
    memset(materials, 0, sizeof(materials));
    for (i = 0; i < kMaterialCount; ++i) {
        if (graphics[i] < 0 || !load_material(&loader, graphics[i], &materials[i])) goto fail;
    }
    corridorOffset = find_corridor(dungeon, dungeonSize);
    if (corridorOffset < 0) goto fail;
    memset(&provenance, 0, sizeof(provenance));
    provenance.dungeonDatOwned = 1;
    provenance.rawBytes = dungeon;
    provenance.rawByteCount = dungeonSize;
    provenance.rawBytesFNV1a = DM1_V1_FloorFeatureFNV1aPc34(dungeon, dungeonSize);
    provenance.squareByteOffset = corridorOffset;
    provenance.squareByte = dungeon[corridorOffset];

    memset(&request, 0, sizeof(request));
    request.relForward = 1;
    request.floorSet = 0;
    request.kind = DM1_V1_F0115_NEAR_FLOOR_PC34;
    if (!receipt(&request, materials, &provenance, &floorReceipt)) goto fail;
    request.kind = DM1_V1_F0115_NEAR_CEILING_PC34;
    if (!receipt(&request, materials, &provenance, &ceilingReceipt)) goto fail;
    request.kind = DM1_V1_F0115_NEAR_FLOOR_ORNAMENT_PC34;
    request.floorOrnamentIndex = 0;
    if (!receipt(&request, materials, &provenance, &ornamentReceipt)) goto fail;
    request.kind = DM1_V1_F0115_NEAR_NORMAL_OBJECT_PC34;
    request.thingType = 5;
    request.sourceCellOwner = 2;
    request.pileIndex = 0;
    if (!receipt(&request, materials, &provenance, &objectReceipt) ||
        !floorReceipt.valid || !ceilingReceipt.valid || !ornamentReceipt.valid ||
        !objectReceipt.valid || floorReceipt.drawOrder != ceilingReceipt.drawOrder ||
        floorReceipt.drawOrder >= ornamentReceipt.drawOrder ||
        ornamentReceipt.drawOrder >= objectReceipt.drawOrder ||
        objectReceipt.cellOwner != 2 || objectReceipt.cropW != materials[3].width ||
        objectReceipt.cropH != materials[3].height) goto fail;

    request.thingType = 14;
    if (receipt(&request, materials, &provenance, &objectReceipt)) {
        fputs("C14 projectile was admitted by normal-object gate\n", stderr);
        goto fail;
    }
    request.thingType = 15;
    if (receipt(&request, materials, &provenance, &objectReceipt)) {
        fputs("C15 explosion was admitted by normal-object gate\n", stderr);
        goto fail;
    }
    ++materials[3].pixelsFNV1a;
    request.thingType = 5;
    if (receipt(&request, materials, &provenance, &objectReceipt)) {
        fputs("tampered PC34 object material was admitted\n", stderr);
        goto fail;
    }
    M11_AssetLoader_Shutdown(&loader);
    free(dungeon);
    puts("ok: original PC34 ZIP D0/D1 floor ceiling ornament object material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    free(dungeon);
    return 1;
}
