/* Source gate for ReDMCSB DUNVIEW.C:3913-3928 C346 -> C026 composition. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int contains_between(const char *begin, const char *end,
                            const char *needle)
{
    size_t needle_length;
    const char *cursor;

    if (!begin || !end || end < begin || !needle) return 0;
    needle_length = strlen(needle);
    for (cursor = begin; cursor + needle_length <= end; ++cursor) {
        if (memcmp(cursor, needle, needle_length) == 0) return 1;
    }
    return 0;
}

int main(void)
{
    FILE *f = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char *source;
    char *route;
    char *route_end;
    char *ornaments;
    char *ornaments_end;
    long size;
    int ok;

    if (!f || fseek(f, 0, SEEK_END) || (size = ftell(f)) < 0 ||
        fseek(f, 0, SEEK_SET) ||
        !(source = malloc((size_t)size + 1U)) ||
        fread(source, 1, (size_t)size, f) != (size_t)size) {
        return 1;
    }
    source[size] = '\0';
    fclose(f);

    route = strstr(source, "static void m11_draw_dm1_front_mirror_route(");
    route_end = route ? strstr(route,
        "static int m11_draw_dm1_wall_ornament_host_material_receipt(") : NULL;
    ornaments = strstr(source, "static void m11_draw_dm1_wall_ornaments(");
    ornaments_end = ornaments ? strstr(ornaments,
        "static int m11_dm1_side_wall_blit_for_rel(") : NULL;
    ok = route && route_end && route_end > route &&
         strstr(source, "static int m11_draw_dm1_front_mirror_backing_host_receipt(") &&
         strstr(source, "static int m11_draw_dm1_front_champion_portrait_host_receipt(") &&
         contains_between(route, route_end,
            "if (!m11_draw_dm1_front_mirror_backing_host_receipt(") &&
         contains_between(route, route_end,
            "return;\n    }\n    if (m11_draw_dm1_front_champion_portrait_host_receipt(") &&
         contains_between(route, route_end,
            "if (drawReceipt.candidatePanelOwnsCell)") &&
         strstr(source, "receipt->portraitSourceX + receipt->portraitWidth >") &&
         strstr(source, "receipt->portraitSourceY + receipt->portraitHeight >") &&
         strstr(source, "dm1_v1_graphic_validate_champion_portrait_atlas_pc34(") &&
         strstr(source, "dm1_v1_graphic_champion_portrait_source_zone_pc34(") &&
         strstr(source, "receipt->portraitSourceX != sourceZone.x") &&
         strstr(source, "C026 retains\n     * its raw indices") &&
         strstr(source, "receipt->backingSourceX + receipt->backingSourceWidth >") &&
         strstr(source, "receipt->backingSourceY + receipt->backingSourceHeight >") &&
         strstr(source, "m11_blit_scaled_palette_map_region(") &&
         strstr(source, "receipt->backingSourceWidth") &&
         ornaments && ornaments_end && ornaments_end > ornaments &&
         contains_between(ornaments, ornaments_end,
            "if (cell.championPortraitOrdinal >= 0)") &&
         contains_between(ornaments, ornaments_end,
            "The dedicated D1C route below is the sole C127 consumer") &&
         contains_between(ornaments, ornaments_end, "continue;");
    free(source);
    return ok ? 0 : 1;
}
