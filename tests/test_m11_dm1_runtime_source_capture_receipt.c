/* Locks M11's final real-data capture receipt for source-gated DM1 routes. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int between_has(const char* begin, const char* end, const char* needle)
{
    const char* cursor;
    size_t length;
    if (!begin || !end || end < begin || !needle) return 0;
    length = strlen(needle);
    for (cursor = begin; cursor + length <= end; ++cursor) {
        if (memcmp(cursor, needle, length) == 0) return 1;
    }
    return 0;
}

int main(void)
{
    FILE* file = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char* source;
    char* begin;
    char* route;
    char* bridge;
    char* publish;
    char* draw;
    char* accessibility;
    char* ra;
    long size;
    int ok;

    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0 ||
        !(source = (char*)malloc((size_t)size + 1u)) ||
        fread(source, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        return 1;
    }
    fclose(file);
    source[size] = '\0';
    begin = strstr(source, "static void m11_dm1_runtime_capture_frame_begin(");
    route = strstr(source,
                   "static void m11_dm1_runtime_capture_route_evidence(");
    bridge = strstr(source, "M11_Dm1RuntimeCaptureEvidenceBridge");
    publish = strstr(source, "static void m11_dm1_runtime_capture_publish(");
    draw = strstr(source, "void M11_GameView_Draw(");
    accessibility = draw ? strstr(draw, "m11_screen_reader_update_ex(") : NULL;
    ra = accessibility ? strstr(accessibility, "m11_draw_ra_overlay(state,") : NULL;
    ok = begin && route && bridge && publish && draw && accessibility && ra &&
         begin < route && route < publish && publish < draw &&
         between_has(begin, route, "state->showDebugHUD") &&
         between_has(route, publish, "requiredRoutes |= route") &&
         between_has(route, publish, "acceptedRoutes |= route") &&
         between_has(route, publish, "sourceTick < bridge->lastSourceTick[index]") &&
         between_has(publish, draw, "lastRuntimeTick[index]") &&
         between_has(publish, draw, "lastMaterialFNV1a[index]") &&
         between_has(publish, draw, "routes->requiredRoutes != routes->acceptedRoutes") &&
         between_has(publish, draw, "m11_dm1_runtime_capture_fnv1a(") &&
         between_has(publish, draw, "framebuffer, pixelCount") &&
         between_has(draw, accessibility, "m11_dm1_runtime_capture_frame_begin(state)") &&
         between_has(draw, accessibility, "M11_DM1_RUNTIME_CAPTURE_C13") &&
         between_has(source, draw, "M11_DM1_RUNTIME_CAPTURE_TOP_ROW") &&
         strstr(source, "m11_dm1_v1_top_row_material_fnv1a(") != NULL &&
         strstr(source, "m11_dm1_v1_action_spell_material_fnv1a(") != NULL &&
         between_has(draw, accessibility, "M11_DM1_RUNTIME_CAPTURE_ACTION_SPELL") &&
         between_has(draw, accessibility, "M11_DM1_RUNTIME_CAPTURE_HOC") &&
         between_has(source, draw, "M11_DM1_RUNTIME_CAPTURE_F0115_FLOOR_ITEM") &&
         between_has(draw, accessibility,
                     "m11_dm1_f0115_floor_item_runtime_capture_consume(state)") &&
         between_has(draw, accessibility, "hocCapture.c040.sourceHash") &&
         between_has(draw, accessibility, "hocCapture.c026.sourceHash") &&
         between_has(draw, accessibility, "m11_dm1_runtime_capture_publish(state,") &&
         draw < accessibility && accessibility < ra;
    free(source);
    return ok ? 0 : 1;
}
