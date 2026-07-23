/* Locks HoC C127 pointer selection to the actual C026 render receipt. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int has_between(const char *begin, const char *end, const char *needle)
{
    size_t length;
    const char *cursor;

    if (!begin || !end || end < begin || !needle) return 0;
    length = strlen(needle);
    for (cursor = begin; cursor + length <= end; ++cursor) {
        if (memcmp(cursor, needle, length) == 0) return 1;
    }
    return 0;
}

int main(void)
{
    FILE *file = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char *source;
    char *hitTest;
    char *hitTestEnd;
    char *clickRoute;
    char *clickRouteEnd;
    long size;
    int ok;

    if (!file || fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0 ||
        !(source = (char *)malloc((size_t)size + 1u)) ||
        fread(source, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        return 1;
    }
    fclose(file);
    source[size] = '\0';

    hitTest = strstr(source, "static int m11_front_mirror_hit_test(");
    hitTestEnd = hitTest ? strstr(hitTest,
        "static int m11_front_mirror_click_wall_cell(") : NULL;
    clickRoute = strstr(source, "static M11_GameInputResult m11_process_v1_c080_click(");
    clickRouteEnd = clickRoute ? strstr(clickRoute,
        "static int m11_front_cell_mirror_ordinal(") : NULL;
    ok = hitTest && hitTestEnd && hitTestEnd > hitTest && clickRoute && clickRouteEnd &&
         clickRouteEnd > clickRoute &&
         has_between(hitTest, hitTestEnd,
             "m11_build_dm1_front_champion_portrait_receipt(") &&
         has_between(hitTest, hitTestEnd,
             "viewportX >= receipt.dstX + receipt.width") &&
         has_between(hitTest, hitTestEnd,
             "viewportY >= receipt.dstY + receipt.height") &&
         has_between(hitTest, hitTestEnd,
             "m11_front_mirror_host_material_ready(state)") &&
         has_between(clickRoute, clickRouteEnd,
             "m11_front_mirror_hit_test(state, localX, localY, &mirrorOrdinal)") &&
         has_between(clickRoute, clickRouteEnd,
             "m11_select_mirror_candidate_by_ordinal(state, mirrorOrdinal)") &&
         !has_between(clickRoute, clickRouteEnd,
             "localX >= 96 && localX <= 127") &&
         strstr(source,
             "DM1_V1_HocMirrorCandidateClickAdmission_BuildFromRuntimeDecisionPc34(");
    free(source);
    return ok ? 0 : 1;
}
