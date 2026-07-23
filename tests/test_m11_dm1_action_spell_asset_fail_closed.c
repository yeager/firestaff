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
    char *action_name_lookup;
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
                          "dm1_v1_box_action_area_x_pc34()") ||
        !contains_between(action_menu, action_menu_end,
                          "dm1_v1_box_action_area_w_pc34()") ||
        !contains_between(action_menu, action_menu_end,
                          "slot && slot->loaded && slot->pixels") ||
        !contains_between(action_menu, action_menu_end,
                          "slot->width == renderPlan->graphic_rect.w") ||
        !contains_between(action_menu, action_menu_end,
                          "slot->height == renderPlan->graphic_rect.h") ||
        contains_between(action_menu, action_menu_end,
                         "slot->width == renderPlan->clear_rect.w") ||
        !contains_between(action_menu, action_menu_end,
                          "Do not draw") ||
        !contains_between(action_menu, action_menu_end,
                          "return 1;") ||
        contains_between(action_menu, action_menu_end,
                         "M11_ACTION_NAMES") ||
        contains_between(action_menu, action_menu_end,
                         "m11_blit_panel_asset_native(state,")) {
        ok = 0;
    }
    if (!strstr(s, "render->clearRect.x, render->clearRect.y")) {
        ok = 0;
    }
    action_name_lookup = strstr(s, "const char* M11_GameView_GetActionName(");
    if (!action_name_lookup ||
        !strstr(action_name_lookup,
                "dm1_v1_action_name_f0384_pc34(actionIndex)") ||
        strstr(action_name_lookup, "M11_ACTION_NAMES")) {
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
    if (!strstr(s, "M11_DM1_M653_BASELINE_TO_TOP_PC34 = 4") ||
        !strstr(s, "dm1_v1_champion_panel_spell_area_overlay_material_receipt_pc34") ||
        !strstr(s, "!receipt.drawable") ||
        !strstr(s, "m11_panel_asset_source_loaded") ||
        !strstr(s, "m11_draw_dm1_m653_cell_at_baseline") ||
        !strstr(s, "m11_draw_dm1_m653_cell_clipped_at_baseline") ||
        !strstr(s, "clipRect->x, clipRect->y, clipRect->w, clipRect->h") ||
        !strstr(s, "m11_build_dm1_spell_area_overlay_plan") ||
        !strstr(s, "m11_invert_dm1_pc34_indexed_box") ||
        !strstr(s, "framebuffer[y * framebufferWidth + x] ^= 0x0fU") ||
        !strstr(s, "plan.line1[i].highlighted") ||
        !strstr(s, "DM1_V1_CPSAO_LINE2_X0_PC34") ||
        !strstr(s, "DM1_V1_CPSAO_LINE3_X0_PC34") ||
        !contains_between(action_menu, action_menu_end,
                          "m11_draw_dm1_ui_text_trailing_spaces") ||
        !strstr(s, "plan.line2[i].screen_y") ||
        !strstr(s, "plan.line3[i].screen_y") ||
        !strstr(s, "const int topY = baselineY - M11_DM1_M653_BASELINE_TO_TOP_PC34") ||
        !strstr(s, "x += 6;")) {
        ok = 0;
    }
    free(s); return ok ? 0 : 1;
}
