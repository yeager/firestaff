/*
 * DM1 V1 all-map readable wall-inscription runtime capture probe.
 *
 * The probe finds every real visible wall TextString with a legal D1C party
 * pose, renders it through M11_GameView_Draw, and compares every opaque M648
 * glyph pixel against hash-verified GRAPHICS.DAT. This proves the original
 * DUNVIEW/TEXT route's decode, palette index, unscaled 8x8 placement, and
 * color-10 transparency boundary without a synthetic replacement.
 *
 * Source-locked to ReDMCSB:
 *   DUNGEON.C:2329 stores 0x80 line separators for inscriptions.
 *   DUNVIEW.C:3679-3682 patches the D1C wall with M712_NEGGRAPHIC_/C735.
 *   DUNVIEW.C:3697-3706 draws M648_GRAPHIC_INSCRIPTION_FONT glyphs.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_Y = 33,
    COLOR_BLACK = 0,
    COLOR_DARK_GRAY = 12,
    INSCRIPTION_MAX_TEXT = 128
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, fmt, ...) \
    do { \
        if (cond) { \
            printf("PASS " fmt "\n", ##__VA_ARGS__); \
            ++g_pass; \
        } else { \
            printf("FAIL " fmt "\n", ##__VA_ARGS__); \
            ++g_fail; \
        } \
    } while (0)

static int dir_dx(int dir) {
    return (dir & 3) == 1 ? 1 : ((dir & 3) == 3 ? -1 : 0);
}

static int dir_dy(int dir) {
    return (dir & 3) == 2 ? 1 : ((dir & 3) == 0 ? -1 : 0);
}

static int square_index_for(const M11_GameViewState* state,
                            int map_index,
                            int x,
                            int y) {
    const struct DungeonMapDesc_Compat* map;
    if (!state || !state->world.dungeon ||
        map_index < 0 ||
        map_index >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[map_index];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    return x * (int)map->height + y;
}

static int square_element_for(const M11_GameViewState* state,
                              int map_index,
                              int x,
                              int y) {
    int idx = square_index_for(state, map_index, x, y);
    unsigned char square;
    if (idx < 0 || !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[map_index].squareData) {
        return -1;
    }
    square = state->world.dungeon->tiles[map_index].squareData[idx];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int raw_next_thing(const M11_GameViewState* state, unsigned short thing) {
    static const unsigned char kThingDataByteCount[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    int type = (int)THING_GET_TYPE(thing);
    int index = (int)THING_GET_INDEX(thing);
    const unsigned char* raw;
    if (!state || !state->world.things || type < 0 || type >= 16 ||
        !state->world.things->rawThingData[type] ||
        index < 0 || index >= state->world.things->thingCounts[type] ||
        kThingDataByteCount[type] == 0) {
        return THING_ENDOFLIST;
    }
    raw = state->world.things->rawThingData[type] +
          index * (int)kThingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static void normalize_inscription_text(char* text) {
    char* p;
    if (!text) {
        return;
    }
    for (p = text; *p; ++p) {
        if (*p == (char)0x80) {
            *p = '\n';
        }
    }
}

static int row_color_count(const unsigned char* fb,
                           int y,
                           int x0,
                           int x1,
                           unsigned char color) {
    int x;
    int count = 0;
    if (!fb || y < 0 || y >= FB_H) {
        return 0;
    }
    if (x0 < 0) x0 = 0;
    if (x1 > FB_W) x1 = FB_W;
    for (x = x0; x < x1; ++x) {
        if ((fb[y * FB_W + x] & 0x0F) == color) {
            ++count;
        }
    }
    return count;
}

static int glyph_pixel_count(const unsigned char* fb,
                             int x,
                             int y,
                             int w,
                             int h) {
    int yy;
    int xx;
    int count = 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= FB_H) {
            continue;
        }
        for (xx = x; xx < x + w; ++xx) {
            unsigned char idx;
            if (xx < 0 || xx >= FB_W) {
                continue;
            }
            idx = fb[yy * FB_W + xx] & 0x0F;
            if (idx != COLOR_BLACK && idx != COLOR_DARK_GRAY) {
                ++count;
            }
        }
    }
    return count;
}

static int check_rendered_lines(const unsigned char* fb, char* decoded) {
    static const int kLineBottomY[4] = {48, 59, 75, 86};
    char* cursor = decoded;
    int line = 0;
    int ok = 1;
    while (cursor && *cursor && line < 4) {
        char* next = strchr(cursor, '\n');
        char saved = '\0';
        if (next) {
            saved = *next;
            *next = '\0';
        }
        if (*cursor) {
            int charCount = (int)strlen(cursor);
            int textW = DM1_V1_InscriptionTextWidth(charCount);
            int textX = DM1_V1_InscriptionTextX(charCount);
            int textY = VIEWPORT_Y + kLineBottomY[line] - 7;
            int glyphPixels = glyph_pixel_count(fb, textX, textY, textW,
                                                 DM1_V1_INSCRIPTION_GLYPH_HEIGHT);
            int blackRow = row_color_count(fb, textY - 1, textX, textX + textW,
                                           COLOR_BLACK);
            int darkRow = row_color_count(fb, textY - 1, textX, textX + textW,
                                          COLOR_DARK_GRAY);
            CHECK(blackRow < textW - 2,
                  "line %d has no synthetic black patch row count=%d width=%d text=\"%s\"",
                  line, blackRow, textW, cursor);
            CHECK(darkRow < textW - 4,
                  "line %d has no synthetic dark-gray patch row count=%d width=%d",
                  line, darkRow, textW);
            CHECK(glyphPixels >= charCount * 2,
                  "line %d glyph pixels visible count=%d chars=%d",
                  line, glyphPixels, charCount);
            ok = ok && blackRow < textW - 2 &&
                 darkRow < textW - 4 &&
                 glyphPixels >= charCount * 2;
        }
        if (!next) {
            break;
        }
        *next = saved;
        cursor = next + 1;
        ++line;
    }
    return ok;
}

static int check_source_glyph_capture(const unsigned char* fb,
                                      const unsigned char* without_text_fb,
                                      const M11_AssetSlot* font,
                                      const unsigned short* text_data,
                                      int text_data_word_count,
                                      int text_offset) {
    unsigned char glyphs[128];
    int cursor = 0;
    int line = 0;
    int opaque_pixels = 0;
    int mismatches = 0;
    int transparent_pixels = 0;
    int transparency_mismatches = 0;

    if (!fb || !without_text_fb || !font || !font->pixels || font->width < 288 ||
        font->height < DM1_V1_INSCRIPTION_GLYPH_HEIGHT || !text_data) {
        return 0;
    }
    if (DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
            text_data, text_data_word_count, text_offset,
            glyphs, (int)sizeof(glyphs)) <= 1) {
        return 0;
    }
    while (cursor < (int)sizeof(glyphs) && line < DM1_V1_INSCRIPTION_MAX_LINES) {
        DM1_V1_InscriptionLinePlanPc34 plan;
        int glyph_index;
        if (!DM1_V1_InscriptionLinePlanFromRawGlyphsPc34(
                glyphs, (int)sizeof(glyphs), cursor, line, &plan)) {
            return 0;
        }
        for (glyph_index = 0; glyph_index < plan.glyphCount; ++glyph_index) {
            int glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(
                glyphs[plan.glyphStart + glyph_index]);
            int yy;
            int xx;
            if (glyph < 0) {
                return 0;
            }
            for (yy = 0; yy < DM1_V1_INSCRIPTION_GLYPH_HEIGHT; ++yy) {
                for (xx = 0; xx < DM1_V1_INSCRIPTION_GLYPH_WIDTH; ++xx) {
                    unsigned char source = font->pixels[
                        yy * font->width + glyph * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx];
                    if (source != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR) {
                        unsigned char rendered = fb[(VIEWPORT_Y + plan.textY + yy) * FB_W +
                                                    plan.textX + glyph_index * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx];
                        ++opaque_pixels;
                        if (rendered != source) {
                            ++mismatches;
                        }
                    } else {
                        int framebuffer_index =
                            (VIEWPORT_Y + plan.textY + yy) * FB_W +
                            plan.textX + glyph_index * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx;
                        ++transparent_pixels;
                        if (fb[framebuffer_index] != without_text_fb[framebuffer_index]) {
                            ++transparency_mismatches;
                        }
                    }
                }
            }
        }
        if (plan.done) {
            break;
        }
        cursor = plan.nextCursor;
        ++line;
    }
    CHECK(opaque_pixels > 0,
          "M648 capture has source glyph pixels");
    CHECK(mismatches == 0,
          "M648 source pixels match unscaled rendered capture mismatches=%d opaque=%d",
          mismatches, opaque_pixels);
    CHECK(transparent_pixels > 0,
          "M648 capture has C10 transparent pixels");
    CHECK(transparency_mismatches == 0,
          "M648 C10 pixels preserve active M11 wall capture mismatches=%d transparent=%d",
          transparency_mismatches, transparent_pixels);
    return opaque_pixels > 0 && mismatches == 0 &&
        transparent_pixels > 0 && transparency_mismatches == 0;
}

static int capture_active_m648_inscription(M11_GameViewState* state,
                                           int text_index,
                                           const char* decoded,
                                           const M11_AssetSlot* font,
                                           unsigned char* framebuffer,
                                           unsigned char* without_text_fb)
{
    if (!state || !decoded || !font || !framebuffer || !without_text_fb ||
        !state->world.things || text_index < 0 ||
        text_index >= state->world.things->textStringCount) {
        return 0;
    }
    /* The reference frame is the original wall route without the same
     * TextString. The next frame is the active M11 M648 route. */
    state->world.things->textStrings[text_index].visible = 0;
    memset(without_text_fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, without_text_fb, FB_W, FB_H);
    state->world.things->textStrings[text_index].visible = 1;
    memset(framebuffer, 0, FB_W * FB_H);
    M11_GameView_Draw(state, framebuffer, FB_W, FB_H);
    return check_rendered_lines(framebuffer, (char*)decoded) &&
        check_source_glyph_capture(framebuffer, without_text_fb, font,
                                   state->world.things->textData,
                                   state->world.things->textDataWordCount,
                                   state->world.things->textStrings[text_index]
                                       .textDataWordOffset);
}

int main(int argc, char** argv) {
    const char* dataDir = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    M11_GameViewState state;
    unsigned char fb[FB_W * FB_H];
    unsigned char without_text_fb[FB_W * FB_H];
    const M11_AssetSlot* font;
    int checked = 0;
    int palette_variant_poses = 0;
    int palette_variant_captures = 0;
    int palette_seen[6] = {0};
    int original_magical_light;
    int ok = 1;
    int x;
    int y;

    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 all-map inscription source capture probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M11_GameView_Init(&state);
    /* This probe owns one direct DM1 runtime only. It must reach the active
     * M11 draw pass instead of treating launcher availability as rendering
     * evidence or scanning unrelated games before its first capture. */
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        fprintf(stderr, "SKIP could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 0;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;
    original_magical_light = state.world.magic.magicalLightAmount;

    font = M11_AssetLoader_Load(&state.assetLoader,
                                DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) {
        printf("FAIL hash-verified GRAPHICS.DAT did not provide M648 288x8\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ReDMCSB DUNGEON.C:2573-2593 maps each visible TextString's CELL to
     * one visible wall side, then DUNVIEW.C:3592/3619 renders only the D1C
     * front-selected inscription through M648.  The MacBook Pro report hit
     * only some HoC inscriptions, so this checks every real front-readable
     * HoC text pose instead of stopping at the first one. */
    for (int map_index = 0;
         map_index < (int)state.world.dungeon->header.mapCount;
         ++map_index) {
        const struct DungeonMapDesc_Compat* map = &state.world.dungeon->maps[map_index];
        for (y = 0; y < (int)map->height; ++y) {
            for (x = 0; x < (int)map->width; ++x) {
            int squareIndex = square_index_for(&state, map_index, x, y);
            unsigned short thing;
            if (squareIndex < 0 ||
                square_element_for(&state, map_index, x, y) != DUNGEON_ELEMENT_WALL) {
                continue;
            }
            /* Thing-list slots are packed across all maps.  A map-local tile
             * index is valid only for map 0; use the DUNGEON.C F0511 lookup
             * that applies the preceding-map thing-list base. */
            thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state.world.dungeon, state.world.things, map_index, x, y);
            while (thing != THING_ENDOFLIST && thing != THING_NONE) {
                int type = (int)THING_GET_TYPE(thing);
                int index = (int)THING_GET_INDEX(thing);
                int cell = (int)THING_GET_CELL(thing);
                if (type == THING_TYPE_TEXTSTRING &&
                    index >= 0 &&
                    index < state.world.things->textStringCount &&
                    state.world.things->textStrings[index].visible) {
                    int dir;
                    for (dir = 0; dir < 4; ++dir) {
                        char decoded[INSCRIPTION_MAX_TEXT];
                        int partyX;
                        int partyY;
                        if (((dir + 2) & 3) != cell) {
                            continue;
                        }
                        partyX = x - dir_dx(dir);
                        partyY = y - dir_dy(dir);
                        if (square_element_for(&state, map_index, partyX, partyY) !=
                            DUNGEON_ELEMENT_CORRIDOR) {
                            continue;
                        }
                        if (F0507_DUNGEON_DecodeTextAtOffset_Compat(
                                state.world.things->textData,
                                state.world.things->textDataWordCount,
                                state.world.things->textStrings[index].textDataWordOffset,
                                decoded,
                                (int)sizeof(decoded)) < 0 ||
                            !decoded[0]) {
                            continue;
                        }
                        normalize_inscription_text(decoded);
                        printf("pose %d map=%d wall=(%d,%d) party=(%d,%d) dir=%d text=\"%s\"\n",
                               checked, map_index, x, y, partyX, partyY, dir, decoded);
                        state.world.party.mapIndex = map_index;
                        state.world.party.mapX = partyX;
                        state.world.party.mapY = partyY;
                        state.world.party.direction = dir;
                        state.world.magic.magicalLightAmount = original_magical_light;
                        ok = capture_active_m648_inscription(
                            &state, index, decoded, font, fb, without_text_fb) && ok;
                        if (palette_variant_poses < 3) {
                            static const int kMagicLightAmounts[] = {0, 25, 100};
                            int variant;
                            for (variant = 0;
                                 variant < (int)(sizeof(kMagicLightAmounts) /
                                                 sizeof(kMagicLightAmounts[0]));
                                 ++variant) {
                                int palette_index;
                                state.world.magic.magicalLightAmount =
                                    kMagicLightAmounts[variant];
                                palette_index =
                                    M11_GameView_GetDungeonPaletteIndex(&state);
                                CHECK(palette_index >= 0 && palette_index <= 5,
                                      "F0337 palette variant pose=%d light=%d index=%d",
                                      checked, kMagicLightAmounts[variant],
                                      palette_index);
                                if (palette_index >= 0 && palette_index <= 5) {
                                    palette_seen[palette_index] = 1;
                                }
                                ok = capture_active_m648_inscription(
                                    &state, index, decoded, font, fb,
                                    without_text_fb) && ok;
                                ++palette_variant_captures;
                            }
                            state.world.magic.magicalLightAmount =
                                original_magical_light;
                            ++palette_variant_poses;
                        }
                        ++checked;
                    }
                }
                thing = (unsigned short)raw_next_thing(&state, thing);
            }
        }
        }
    }

    if (checked <= 0) {
        printf("FAIL no front-readable DM1 TextString pose found\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    CHECK(palette_variant_poses == 3,
          "captured M648 after F0337 palette variants across three real walls poses=%d",
          palette_variant_poses);
    CHECK(palette_variant_captures == 9,
          "captured each M648 palette variant pose=%d",
          palette_variant_captures);
    {
        int palette_count = 0;
        int palette;
        for (palette = 0; palette < 6; ++palette) {
            palette_count += palette_seen[palette];
        }
        CHECK(palette_count >= 2,
              "M648 capture observed multiple source F0337 palette levels count=%d",
              palette_count);
    }
    M11_GameView_Shutdown(&state);

    printf("checked=%d summary=%d passed %d failed\n", checked, g_pass, g_fail);
    return (ok && g_fail == 0) ? 0 : 1;
}
