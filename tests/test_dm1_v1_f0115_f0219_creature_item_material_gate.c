/* Real PC34 D1-D3 F0115 creature/item admission. C14/C15 are absent by
 * construction: this test only owns normal object and creature surfaces. */

#include "asset_loader_m11.h"
#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_f0115_f0219_creature_item_material_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kMaterialCount = 4 };

static const char* data_path(const char* name, char path[2048])
{
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* home;
    if (root && root[0]) {
        snprintf(path, 1024, "%s/%s", root, name);
        return path;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    snprintf(path, 1024, "%s/.firestaff/data/dm1/%s", home, name);
    return path;
}

static unsigned char* read_file(const char* path, int* outSize)
{
    FILE* file;
    long size;
    unsigned char* bytes;
    if (outSize) *outSize = 0;
    if (!path || !(file = fopen(path, "rb"))) return 0;
    if (fseek(file, 0, SEEK_END) || (size = ftell(file)) <= 0 ||
        size > 0x7fffffffL || fseek(file, 0, SEEK_SET)) {
        fclose(file);
        return 0;
    }
    bytes = (unsigned char*)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
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
    out->pixelsFNV1a = DM1_V1_FloorFeatureFNV1aPc34(slot->pixels,
                                                     out->indexedPixelCount);
    return out->pixelsFNV1a != 0u;
}

int main(void)
{
    char graphicsPath[1024], dungeonPath[1024];
    M11_AssetLoader loader;
    DM1_V1_FloorFeatureSourceMaterialPc34 materials[kMaterialCount];
    DM1_V1_F0115F0219DungeonProvenancePc34 provenance;
    unsigned char* dungeon;
    int dungeonSize;
    int corridorOffset;
    int graphics[kMaterialCount];
    int depth;
    int i;

    if (!data_path("GRAPHICS.DAT", graphicsPath) ||
        !data_path("DUNGEON.DAT", dungeonPath)) return 0;
    dungeon = read_file(dungeonPath, &dungeonSize);
    if (!dungeon || !M11_AssetLoader_Init(&loader, graphicsPath)) {
        free(dungeon);
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) {
            fputs("configured PC34 data is unavailable\n", stderr);
            return 1;
        }
        puts("SKIP: PC34 GRAPHICS.DAT/DUNGEON.DAT not installed");
        return 0;
    }
    graphics[0] = (int)dm1_item_sprite_index(5, 0);
    for (depth = 1; depth <= 3; ++depth) {
        graphics[depth] = (int)dm1_creature_sprite_for_view(
            DM1_CREATURE_MUMMY, depth, 0, 0, 0, 0);
    }
    memset(materials, 0, sizeof(materials));
    for (i = 0; i < kMaterialCount; ++i) {
        if (!load_material(&loader, graphics[i], &materials[i])) goto fail;
    }
    corridorOffset = find_corridor(dungeon, dungeonSize);
    if (corridorOffset < 0) {
        fputs("real PC34 corridor provenance was not found\n", stderr);
        goto fail;
    }
    memset(&provenance, 0, sizeof(provenance));
    provenance.dungeonDatOwned = 1;
    provenance.rawBytes = dungeon;
    provenance.rawByteCount = dungeonSize;
    provenance.rawBytesFNV1a = DM1_V1_FloorFeatureFNV1aPc34(dungeon, dungeonSize);
    provenance.squareByteOffset = corridorOffset;
    provenance.squareByte = dungeon[corridorOffset];

    for (depth = 1; depth <= 3; ++depth) {
        DM1_V1_F0115F0219MaterialRequestPc34 item;
        DM1_V1_F0115F0219MaterialRequestPc34 creature;
        DM1_V1_F0115F0219MaterialReceiptPc34 itemReceipt;
        DM1_V1_F0115F0219MaterialReceiptPc34 creatureReceipt;
        memset(&item, 0, sizeof(item));
        item.kind = DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34;
        item.relForward = depth;
        item.relativeCell = 2;
        item.sourceCellOwner = 2;
        item.thingType = 5;
        item.pileIndex = 0;
        item.viewportW = 224;
        item.viewportH = 136;
        memset(&creature, 0, sizeof(creature));
        creature.kind = DM1_V1_F0115_F0219_MATERIAL_CREATURE_PC34;
        creature.relForward = depth;
        creature.relativeCell = 2;
        creature.sourceCellOwner = 2;
        creature.creatureType = DM1_CREATURE_MUMMY;
        creature.groupCount = 1;
        creature.creatureCount = 1;
        creature.viewportW = 224;
        creature.viewportH = 136;
        if (!dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(
                &item, materials, kMaterialCount, &provenance, &itemReceipt)) {
            fprintf(stderr, "real PC34 F0115 D%d item admission failed (graphic %d %dx%d)\n",
                    depth, graphics[0], materials[0].width, materials[0].height);
            goto fail;
        }
        if (!dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(
                &creature, materials, kMaterialCount, &provenance,
                &creatureReceipt) || !itemReceipt.valid || !creatureReceipt.valid ||
            itemReceipt.drawOrder >= creatureReceipt.drawOrder ||
            itemReceipt.cellOwner != 2 || creatureReceipt.cellOwner != 2 ||
            itemReceipt.cropW <= 0 || itemReceipt.cropH <= 0 ||
            creatureReceipt.cropW <= 0 || creatureReceipt.cropH <= 0) {
            fprintf(stderr, "real PC34 F0115 D%d creature admission failed\n", depth);
            goto fail;
        }
    }
    ++materials[0].pixelsFNV1a;
    {
        DM1_V1_F0115F0219MaterialRequestPc34 item;
        DM1_V1_F0115F0219MaterialReceiptPc34 receipt;
        memset(&item, 0, sizeof(item));
        item.kind = DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34;
        item.relForward = 1;
        item.relativeCell = 2;
        item.sourceCellOwner = 2;
        item.thingType = 5;
        item.viewportW = 224;
        item.viewportH = 136;
        if (dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(
                &item, materials, kMaterialCount, &provenance, &receipt)) {
            fputs("tampered PC34 item material was admitted\n", stderr);
            goto fail;
        }
    }
    --materials[0].pixelsFNV1a;
    {
        DM1_V1_F0115F0219MaterialRequestPc34 item;
        DM1_V1_F0115F0219MaterialReceiptPc34 receipt;
        memset(&item, 0, sizeof(item));
        item.kind = DM1_V1_F0115_F0219_MATERIAL_ITEM_PC34;
        item.relForward = 1;
        item.relativeCell = 2;
        item.sourceCellOwner = 1;
        item.thingType = 5;
        item.viewportW = 224;
        item.viewportH = 136;
        if (dm1_v1_f0115_f0219_creature_item_material_receipt_pc34(
                &item, materials, kMaterialCount, &provenance, &receipt)) {
            fputs("foreign view-cell item material was admitted\n", stderr);
            goto fail;
        }
    }
    M11_AssetLoader_Shutdown(&loader);
    free(dungeon);
    puts("ok: real PC34 F0115 D1-D3 creature/item material gate");
    return 0;
fail:
    M11_AssetLoader_Shutdown(&loader);
    free(dungeon);
    return 1;
}
