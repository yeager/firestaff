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

int main(void) {
    FILE *f = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    long n;
    char *s;
    int ok;
    if (!f || fseek(f, 0, SEEK_END) || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) ||
        !(s = malloc((size_t)n + 1U)) || fread(s, 1, (size_t)n, f) != (size_t)n) return 1;
    s[n] = '\0'; fclose(f);
    char *action_menu;
    char *action_menu_end;
    char *hud_font_gate;
    char *hud_font_gate_end;

    ok = strstr(s, "if (actionAsset && spellAsset && actionAsset->loaded") &&
         strstr(s, "Missing real data leaves this source-owned strip black.") &&
         strstr(s, "if (!drewAuthenticFrames && !m11_v1_chrome_mode_enabled())") &&
         strstr(s, "if (!slot || !slot->loaded || !slot->pixels ||");
    action_menu = strstr(s, "static int m11_draw_dm_action_menu(");
    action_menu_end = action_menu
        ? strstr(action_menu, "/* Draw the four DM1 action-hand icon cells")
        : NULL;
    if (!action_menu || !action_menu_end || action_menu_end <= action_menu ||
        !contains_between(action_menu, action_menu_end,
                          "slot && slot->loaded && slot->pixels") ||
        contains_between(action_menu, action_menu_end,
                         "m11_blit_panel_asset_native(state,")) {
        ok = 0;
    }
    hud_font_gate = strstr(s, "static int m11_dm1_pc34_hud_font_is_source_bound(");
    hud_font_gate_end = hud_font_gate
        ? strstr(hud_font_gate, "static int g_m11_font_scale_override")
        : NULL;
    if (!hud_font_gate || !hud_font_gate_end || hud_font_gate_end <= hud_font_gate ||
        !contains_between(hud_font_gate, hud_font_gate_end,
                          "M11_FONT_GRAPHIC_INDEX_PC34") ||
        !contains_between(hud_font_gate, hud_font_gate_end,
                          "M11_FONT_GRAPHIC_INDEX_LEGACY") ||
        !contains_between(hud_font_gate, hud_font_gate_end,
                          "M11_Font_ResolvedGraphicIndex") ||
        !contains_between(hud_font_gate, hud_font_gate_end,
                          "g_activeOriginalFont != &state->originalFont") ||
        contains_between(hud_font_gate, hud_font_gate_end,
                         "M11_FONT_GRAPHIC_INDEX_FALLBACK") ||
        !strstr(s, "m11_dm1_pc34_hud_font_is_source_bound(state)")) {
        ok = 0;
    }
    free(s); return ok ? 0 : 1;
}
