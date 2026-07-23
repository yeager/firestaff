/* Source gate for ReDMCSB PANEL.C F0344/F0351 champion statistic rows. */

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
    char *panel;
    char *panel_end;
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

    panel = strstr(source,
        "static int m11_draw_v1_inventory_champion_stats_panel(");
    panel_end = panel ? strstr(panel,
        "/* ── ReDMCSB PANEL.C F0352:") : NULL;
    ok = panel && panel_end && panel_end > panel &&
         contains_between(panel, panel_end,
            "PANEL.C F0344/F0351 may only publish the champion health/stamina") &&
         contains_between(panel, panel_end,
            "if (!m11_dm1_pc34_hud_font_is_source_bound(state))") &&
         contains_between(panel, panel_end,
            "dm1_v1_graphic_panel_empty_pc34()") &&
         !contains_between(panel, panel_end,
            "if (!g_activeOriginalFont || !M11_Font_IsLoaded(g_activeOriginalFont))");
    free(source);
    return ok ? 0 : 1;
}
