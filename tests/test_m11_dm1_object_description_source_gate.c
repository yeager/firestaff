#include "m11_game_view.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

static int file_readable(const char *path)
{
    FILE *file;
    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

static int find_real_object_icon(const struct DungeonThings_Compat *things,
                                 unsigned short *out_thing,
                                 int *out_icon)
{
    static const int object_types[] = {
        THING_TYPE_WEAPON, THING_TYPE_ARMOUR, THING_TYPE_SCROLL,
        THING_TYPE_POTION, THING_TYPE_CONTAINER, THING_TYPE_JUNK
    };
    size_t type_ordinal;

    if (!things || !out_thing || !out_icon) return 0;
    for (type_ordinal = 0; type_ordinal < sizeof(object_types) / sizeof(object_types[0]);
         ++type_ordinal) {
        const int type = object_types[type_ordinal];
        int index;
        for (index = 0; index < things->thingCounts[type]; ++index) {
            const unsigned short thing =
                (unsigned short)(((unsigned int)type << 10) | (unsigned int)index);
            const unsigned char *raw = F7018_GetThingData(things, thing);
            const int icon = dm1_v1_dungeon_get_object_icon_index_pc34(
                things, thing, 0);
            if (raw && icon >= 0) {
                *out_thing = thing;
                *out_icon = icon;
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *dungeon_path = getenv("FIRESTAFF_DM1_DUNGEON_DAT");
    const char *graphics_path = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    M11_GameViewState state;
    DM1_V1_ObjectIconSourceZonePc34 zone;
    const M11_AssetSlot *atlas;
    const char *load_path;
    unsigned short thing = THING_NONE;
    int icon = -1;

    if (!file_readable(dungeon_path) || !file_readable(graphics_path)) {
        puts("SKIP: set FIRESTAFF_DM1_DUNGEON_DAT and FIRESTAFF_DM1_GRAPHICS_DAT");
        return 0;
    }

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    CHECK(F0500_DUNGEON_LoadDatHeader_Compat(dungeon_path, &dungeon),
          "configured DUNGEON.DAT header loads");
    load_path = dungeon.decompressedPath[0] ? dungeon.decompressedPath : dungeon_path;
    CHECK(failures == 0 && F0502_DUNGEON_LoadTileData_Compat(load_path, &dungeon),
          "configured DUNGEON.DAT tile data loads");
    CHECK(failures == 0 && F0504_DUNGEON_LoadThingData_Compat(
              load_path, &dungeon, &things),
          "configured DUNGEON.DAT raw Thing data loads");
    CHECK(failures == 0 && find_real_object_icon(&things, &thing, &icon),
          "real DUNGEON.DAT contains an F0156-backed object icon");
    CHECK(failures == 0 && F7018_GetThingData(&things, thing) != NULL,
          "selected object remains backed by a raw Thing record");
    CHECK(failures == 0 && dm1_v1_object_icon_source_zone_pc34(icon, &zone),
          "F0033 icon resolves to a source atlas zone");

    M11_GameView_Init(&state);
    CHECK(failures == 0 && M11_AssetLoader_Init(&state.assetLoader, graphics_path),
          "configured GRAPHICS.DAT loads");
    atlas = failures == 0 ? M11_AssetLoader_Load(
        &state.assetLoader, (unsigned int)zone.graphic_index) : NULL;
    CHECK(atlas && atlas->loaded && atlas->pixels &&
              zone.w == 16 && zone.h == 16 && zone.x >= 0 && zone.y >= 0 &&
              zone.x + zone.w <= (int)atlas->width &&
              zone.y + zone.h <= (int)atlas->height,
          "real GRAPHICS.DAT contains the selected F0033 icon zone");

    M11_GameView_Shutdown(&state);
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    if (failures) return 1;
    puts("PASS: real DM1 DUNGEON.DAT plus GRAPHICS.DAT object source gate");
    return 0;
}
