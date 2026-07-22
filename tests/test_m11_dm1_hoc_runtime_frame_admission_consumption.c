/* Locks M11's source-owned C040/C026 runtime-frame admission gate. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int between_has(const char* begin, const char* end, const char* needle)
{
    size_t length;
    const char* cursor;
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
    char* publish;
    char* current;
    char* clear;
    char* select;
    char* draw;
    char* inventory;
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
    publish = strstr(source, "static int m11_dm1_hoc_publish_runtime_frame_admission(");
    current = strstr(source, "static int m11_dm1_hoc_runtime_frame_admission_current(");
    clear = strstr(source, "static void m11_clear_dm1_hoc_runtime_frame_zones(");
    select = strstr(source, "static int m11_select_mirror_candidate_by_ordinal(");
    draw = strstr(source, "void M11_GameView_Draw(");
    inventory = draw ? strstr(draw, "m11_draw_inventory_panel(state,") : NULL;
    ok = publish && current && clear && select && draw && inventory &&
         publish < current && current < clear && clear < select && select < draw &&
         between_has(publish, current,
             "DM1_V1_HocCandidateRuntimeFrameAdmission_BuildReceiptPc34(") &&
         between_has(publish, current, "graphicIndex = 40") &&
         between_has(publish, current, "graphicIndex = 26") &&
         between_has(publish, current, "m11_dm1_hoc_source_hash(") &&
         between_has(current, clear,
             "runtime->admission.runtimeTick != (uint64_t)state->world.gameTick") &&
         between_has(current, clear, "runtime->admission.sensorGeneration") &&
         between_has(current, clear, "runtime->admission.presentedPanelGeneration") &&
         between_has(clear, select, "M11_VIEWPORT_X + panelRect.x") &&
         between_has(clear, select, "M11_VIEWPORT_X + portraitX") &&
         between_has(select, draw,
             "m11_dm1_hoc_publish_runtime_frame_admission(state)") &&
         between_has(draw, inventory,
             "m11_dm1_hoc_runtime_frame_admission_current(state,") &&
         between_has(draw, inventory,
             "m11_clear_dm1_hoc_runtime_frame_zones(");
    free(source);
    return ok ? 0 : 1;
}
