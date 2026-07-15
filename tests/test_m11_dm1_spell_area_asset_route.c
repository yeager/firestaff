#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH must name the repository root"
#endif

static char *read_file(const char *path)
{
    FILE *file;
    long length;
    char *text;

    file = fopen(path, "rb");
    if (file == NULL || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file != NULL) {
            fclose(file);
        }
        return NULL;
    }
    text = (char *)malloc((size_t)length + 1U);
    if (text == NULL || fread(text, 1U, (size_t)length, file) !=
                            (size_t)length) {
        free(text);
        fclose(file);
        return NULL;
    }
    text[length] = '\0';
    fclose(file);
    return text;
}

static int require_text(const char *source, const char *needle)
{
    if (strstr(source, needle) != NULL) {
        return 1;
    }
    fprintf(stderr, "missing source route: %s\n", needle);
    return 0;
}

int main(void)
{
    const char *path = FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c";
    char *source = read_file(path);
    const char *late_route;
    const char *party_panel;
    int ok = 1;

    if (source == NULL) {
        fprintf(stderr, "cannot read %s\n", path);
        return 1;
    }

    ok &= require_text(source,
                       "if (state->spellPanelOpen && !m11_v1_chrome_mode_enabled() &&");
    ok &= require_text(source,
                       "!m11_is_dm1_source_kind(state->sourceKind))");
    ok &= require_text(source,
                       "m11_draw_v1_spell_area_overlay(state, framebuffer, framebufferWidth,");
    ok &= require_text(source,
                       "DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34");
    ok &= require_text(source,
                       "DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34");
    ok &= require_text(source, "!state || !state->assetsAvailable");
    ok &= require_text(source, "facts.c011_loaded_pixels");
    ok &= require_text(source, "M11_Font_IsLoaded(g_activeOriginalFont)");
    ok &= require_text(source,
                       "dm1_v1_champion_panel_spell_area_overlay_material_receipt_pc34");
    ok &= require_text(source, "m11_panel_asset_source_loaded");
    ok &= require_text(source, "!receipt.drawable");
    ok &= require_text(source, "DM1_V1_SPELL_AREA_LINES_WIDTH_PC34");
    ok &= require_text(source, "DM1_V1_SPELL_AREA_LINES_HEIGHT_PC34");
    ok &= require_text(source, "DM1_V1_SPELL_AREA_LINES_AVAILABLE_Y_PC34");
    ok &= require_text(source, "DM1_V1_SPELL_AREA_LINES_SELECTED_Y_PC34");
    ok &= require_text(source, "224, 50");
    ok &= require_text(source, "224, 62");

    party_panel = strstr(source, "m11_draw_party_panel(state, framebuffer");
    late_route = party_panel ? strstr(party_panel,
                        "m11_draw_v1_spell_area_overlay(state, framebuffer, framebufferWidth,")
                             : NULL;
    if (party_panel == NULL || late_route == NULL || late_route <= party_panel) {
        fprintf(stderr, "spell asset route is not late in the V1 paint order\n");
        ok = 0;
    }

    free(source);
    return ok ? 0 : 1;
}
