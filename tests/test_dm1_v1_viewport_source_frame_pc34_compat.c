/* Real PC34 wall/door/C127 frame transaction.  This exercises the dedicated
 * source-pixel adapter with installed GRAPHICS.DAT and DUNGEON.DAT, rather
 * than fixture art. */

#include "dm1_v1_viewport_source_frame_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, int *outCount)
{
    FILE *file;
    long count;
    unsigned char *bytes;
    *outCount = 0;
    file = path ? fopen(path, "rb") : NULL;
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (count = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = (unsigned char *)malloc((size_t)count);
    if (!bytes || fread(bytes, 1, (size_t)count, file) != (size_t)count) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *outCount = (int)count;
    return bytes;
}

static int find_c127(const M11_GameViewState *state, const unsigned char **outRaw)
{
    int i;
    if (outRaw) *outRaw = NULL;
    if (!state || !state->world.things || !state->world.things->sensors ||
        !state->world.things->rawThingData[THING_TYPE_SENSOR]) return 0;
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127) {
            if (outRaw) *outRaw = state->world.things->rawThingData[THING_TYPE_SENSOR] + i * 8;
            return 1;
        }
    }
    return 0;
}

static void source_from_slot(DM1_V1_FloorFeatureSourceMaterialPc34 *out,
                             int graphicIndex, const M11_AssetSlot *slot)
{
    memset(out, 0, sizeof(*out));
    out->graphicsDatOwned = 1;
    out->graphicIndex = graphicIndex;
    out->width = (int)slot->width;
    out->height = (int)slot->height;
    out->indexedPixels = slot->pixels;
    out->indexedPixelCount = out->width * out->height;
    out->pixelsFNV1a = DM1_V1_FloorFeatureFNV1aPc34(slot->pixels,
                                                      out->indexedPixelCount);
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultData[1024];
    char dungeonPath[1200];
    M11_GameViewState state;
    const M11_AssetSlot *wall;
    const M11_AssetSlot *door;
    const M11_AssetSlot *sensor;
    DM1_V1_FloorFeatureSourceMaterialPc34 materials[3];
    DM1_V1_ViewportDungeonProvenancePc34 provenance;
    DM1_V1_ViewportSourceLayerPc34 layers[3];
    DM1_V1_ViewportSourceFrameInputPc34 input;
    DM1_V1_ViewportSourceFrameReceiptPc34 receipt;
    const unsigned char *c127;
    unsigned char *dungeon;
    unsigned char framebuffer[320 * 200];
    int dungeonCount;
    int ok = 0;

    if (!dataDir || !dataDir[0]) {
        home = getenv("HOME");
        if (!home || !home[0]) return 0;
        snprintf(defaultData, sizeof(defaultData), "%s/.firestaff/data/dm1", home);
        dataDir = defaultData;
    }
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", dataDir);
    dungeon = read_file(dungeonPath, &dungeonCount);
    M11_GameView_Init(&state);
    if (!dungeon || !M11_GameView_StartDm1(&state, dataDir)) {
        free(dungeon);
        M11_GameView_Shutdown(&state);
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) return 1;
        puts("SKIP: PC34 DM1 data not installed");
        return 0;
    }
    wall = M11_AssetLoader_Load(&state.assetLoader, 97);
    door = M11_AssetLoader_Load(&state.assetLoader, 246);
    sensor = M11_AssetLoader_Load(&state.assetLoader, 102);
    if (!wall || !door || !sensor || !wall->loaded || !door->loaded || !sensor->loaded ||
        !wall->pixels || !door->pixels || !sensor->pixels || !find_c127(&state, &c127) ||
        dungeonCount < 8) goto done;
    source_from_slot(&materials[0], 97, wall);
    source_from_slot(&materials[1], 246, door);
    source_from_slot(&materials[2], 102, sensor);
    memset(&provenance, 0, sizeof(provenance));
    provenance.dungeonDatOwned = 1;
    provenance.rawBytes = dungeon;
    provenance.rawByteCount = dungeonCount;
    provenance.rawBytesFNV1a = DM1_V1_FloorFeatureFNV1aPc34(dungeon, dungeonCount);
    provenance.squareByteOffset = 0;
    provenance.squareByte = dungeon[0];
    memset(layers, 0, sizeof(layers));
    layers[0].kind = DM1_V1_VIEWPORT_SOURCE_LAYER_WALL_PC34;
    layers[0].graphicIndex = 97; layers[0].dstX = 8; layers[0].dstY = 8;
    layers[0].width = wall->width < 16 ? (int)wall->width : 16;
    layers[0].height = wall->height < 16 ? (int)wall->height : 16;
    layers[0].transparentColor = -1;
    layers[1].kind = DM1_V1_VIEWPORT_SOURCE_LAYER_DOOR_PC34;
    layers[1].graphicIndex = 246; layers[1].dstX = 32; layers[1].dstY = 8;
    layers[1].width = door->width < 16 ? (int)door->width : 16;
    layers[1].height = door->height < 16 ? (int)door->height : 16;
    layers[1].transparentColor = 10;
    layers[2].kind = DM1_V1_VIEWPORT_SOURCE_LAYER_SENSOR_PC34;
    layers[2].graphicIndex = 102; layers[2].dstX = 56; layers[2].dstY = 8;
    layers[2].width = sensor->width < 16 ? (int)sensor->width : 16;
    layers[2].height = sensor->height < 16 ? (int)sensor->height : 16;
    layers[2].transparentColor = 10;
    layers[2].sensorRecord = c127;
    layers[2].sensorRecordByteCount = 8;
    layers[2].sensorRecordFNV1a = DM1_V1_FloorFeatureFNV1aPc34(c127, 8);
    memset(&input, 0, sizeof(input));
    input.materials = materials; input.materialCount = 3;
    input.dungeonProvenance = &provenance; input.layers = layers; input.layerCount = 3;
    memset(framebuffer, 0, sizeof(framebuffer));
    if (!dm1_v1_viewport_source_frame_preflight_pc34(&input, &receipt) ||
        !receipt.valid || receipt.wallLayerCount != 1 || receipt.doorLayerCount != 1 ||
        receipt.sensorLayerCount != 1 ||
        !dm1_v1_viewport_source_frame_render_pc34(&input, &receipt, framebuffer, 320, 200) ||
        framebuffer[8 * 320 + 8] != wall->pixels[0]) goto done;
    ++layers[2].sensorRecordFNV1a;
    if (dm1_v1_viewport_source_frame_preflight_pc34(&input, &receipt)) goto done;
    --layers[2].sensorRecordFNV1a;
    ++materials[1].pixelsFNV1a;
    if (dm1_v1_viewport_source_frame_preflight_pc34(&input, &receipt)) goto done;
    ok = 1;
done:
    M11_GameView_Shutdown(&state);
    free(dungeon);
    if (!ok) return 1;
    puts("ok: PC34 wall/door/C127 source frame preflight and pixel rendering");
    return 0;
}
