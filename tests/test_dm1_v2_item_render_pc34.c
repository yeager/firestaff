#include "dm1_v2_item_render_pc34.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_SOURCE_DIR
#define FIRESTAFF_SOURCE_DIR "."
#endif

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int file_contains(const char* rel, const char* needle) {
    char path[2048];
    snprintf(path, sizeof(path), "%s/%s", FIRESTAFF_SOURCE_DIR, rel);
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* data = (char*)malloc((size_t)size + 1u);
    if (!data) { fclose(f); return 0; }
    size_t got = fread(data, 1, (size_t)size, f);
    fclose(f);
    data[got] = '\0';
    int ok = strstr(data, needle) != NULL;
    free(data);
    return ok;
}

static void test_source_locked_cell_layer_order(void) {
    CHECK(dm1_v2_item_render_layer_precedes(DM1_V2_CELL_LAYER_FLOOR_ITEM, DM1_V2_CELL_LAYER_CREATURE));
    CHECK(dm1_v2_item_render_layer_precedes(DM1_V2_CELL_LAYER_CREATURE, DM1_V2_CELL_LAYER_PROJECTILE));
    CHECK(dm1_v2_item_render_layer_precedes(DM1_V2_CELL_LAYER_PROJECTILE, DM1_V2_CELL_LAYER_EXPLOSION));
    CHECK(dm1_v2_item_render_layer_precedes(DM1_V2_CELL_LAYER_EXPLOSION, DM1_V2_CELL_LAYER_FLUXCAGE));
    CHECK(!dm1_v2_item_render_layer_precedes(DM1_V2_CELL_LAYER_FLUXCAGE, DM1_V2_CELL_LAYER_FLOOR_ITEM));
    CHECK(!dm1_v2_item_render_layer_precedes((DM1_V2_CellLayer)-1, DM1_V2_CELL_LAYER_FLOOR_ITEM));
}

static void test_item_surface_bindings(void) {
    const DM1_V2_ItemRenderBinding* hand = dm1_v2_item_render_empty_hand_binding();
    const DM1_V2_ItemRenderBinding* floor = dm1_v2_item_render_floor_item_binding();
    DM1_V1_ObjectIconSourceZonePc34 sourceZone;

    CHECK(hand != NULL);
    CHECK(strcmp(hand->assetId, "C201_ICON_ACTION_ICON_EMPTY_HAND") == 0);
    CHECK(hand->surface == DM1_V2_ITEM_SURFACE_ACTION_HAND);
    CHECK(hand->supportsSubcellOffset == 0);
    CHECK(hand->sourceGraphicIndex == 48);
    CHECK(hand->sourceIconIndex == 201);
    CHECK(hand->sourceX == 144);
    CHECK(hand->sourceY == 0);
    CHECK(hand->sourceWidth == 16);
    CHECK(hand->sourceHeight == 16);
    CHECK(dm1_v1_object_icon_source_zone_pc34(hand->sourceIconIndex, &sourceZone));
    CHECK(sourceZone.graphic_index == hand->sourceGraphicIndex);
    CHECK(sourceZone.x == hand->sourceX);
    CHECK(sourceZone.y == hand->sourceY);
    CHECK(sourceZone.w == hand->sourceWidth);
    CHECK(sourceZone.h == hand->sourceHeight);

    CHECK(floor == NULL);
}

static void test_manifest_and_source_evidence(void) {
    CHECK(strstr(dm1_v2_item_render_source_evidence(), "DUNVIEW.C") != NULL);
    CHECK(strstr(dm1_v2_item_render_source_evidence(), "F0115") != NULL);
    CHECK(strstr(dm1_v2_item_render_source_evidence(), "OBJECT.C F0033") != NULL);
    CHECK(file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "C201_ICON_ACTION_ICON_EMPTY_HAND"));
    CHECK(file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"));
    CHECK(!file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "fs.v2.item."));
    CHECK(file_contains("assets-v2/items/wave1/specs/starter-icons.md", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"));
    CHECK(file_contains("assets-v2/items/wave1/specs/starter-icons.md", "G0219"));
}

int main(void) {
    test_source_locked_cell_layer_order();
    test_item_surface_bindings();
    test_manifest_and_source_evidence();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_item_render_pc34: ok");
    return 0;
}
