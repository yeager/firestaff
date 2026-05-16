#include "dm1_v2_item_render_pc34.h"

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
    char path[1024];
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

    CHECK(hand != NULL);
    CHECK(floor != NULL);
    CHECK(strcmp(hand->assetId, "fs.v2.item.starter.empty-hand") == 0);
    CHECK(hand->surface == DM1_V2_ITEM_SURFACE_ACTION_HAND);
    CHECK(hand->supportsSubcellOffset == 0);

    CHECK(strcmp(floor->assetId, "fs.v2.item.starter.floor-item-placeholder") == 0);
    CHECK(floor->surface == DM1_V2_ITEM_SURFACE_FLOOR);
    CHECK(floor->drawLayer == DM1_V2_CELL_LAYER_FLOOR_ITEM);
    CHECK(floor->sourceCellOrdinal == 1);
    CHECK(floor->supportsSubcellOffset == 1);
}

static void test_manifest_and_source_evidence(void) {
    CHECK(strstr(dm1_v2_item_render_source_evidence(), "DUNVIEW.C") != NULL);
    CHECK(strstr(dm1_v2_item_render_source_evidence(), "F0115") != NULL);
    CHECK(file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "fs.v2.item.starter.empty-hand"));
    CHECK(file_contains("assets-v2/manifests/firestaff-v2-wave1-items-starter.manifest.json", "fs.v2.item.starter.floor-item-placeholder"));
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
