/*
 * Source-lock gate for the DM1 V1 TITLE step → special-palette mapping.
 *
 * ReDMCSB TITLE.C F0437 PC/F20 source-lock:
 *   - TITLE.C:319-324 blits "PRESENTS" from source y=137 to 0,90..105
 *     with F1012_PALETTE_SetCurtain(C0_BLACK_PALETTE) and
 *     F0694_SetMultipleColorsInPalette(C12_PRESENTS), which sets
 *     only 0x0F to white.  ReDMCSB VIDEODRV.C C25_VGA G8159_PRESENTS is
 *     the source of truth.
 *   - TITLE.C:340-402 builds 18 shrinked 320x80 title bitmaps and
 *     blits them in reverse order, then waits two VBlanks and blits
 *     MASTER / STRIKES BACK at y=118..174.  The DUNGEON MASTER zoom
 *     and the STRIKES BACK reveal both use the merged
 *     C13_DUNGEON + C14_MASTER palette from VIDEODRV.C C25_VGA G8160
 *     and G8161.  In RGB8 that is VGA_PALETTE_PC34_SPECIAL_TITLE.
 *
 * v2.7.4 release regression: the previous runtime applied
 * VGA_PALETTE_PC34_SPECIAL_TITLE for every title step, so the
 * "PRESENTS" word rendered red (color 4 = (255,0,0) in the wrong
 * palette) instead of plain white.  V1_TitleFrontend_GetStepPalette
 * is the single source of truth for the step → palette mapping and
 * must never collapse to a single palette for all steps.
 */

#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ_INT(actual, expected, msg) do { \
    int _a = (int)(actual); \
    int _e = (int)(expected); \
    if (_a == _e) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: expected %d, got %d\n", (msg), _e, _a); } \
} while (0)

static int step_palette(V1_TitleFrontendSourceEventKind kind) {
    int palette = -1;
    if (!V1_TitleFrontend_GetStepPalette(kind, &palette)) return -1;
    return palette;
}

static void check_step_kind_palette(V1_TitleFrontendSourceEventKind kind,
                                    int expectedPalette,
                                    const char* label) {
    int actual = step_palette(kind);
    ASSERT_EQ_INT(actual, expectedPalette, label);
}

static void check_rgb(unsigned int palette,
                      unsigned int color,
                      unsigned char r,
                      unsigned char g,
                      unsigned char b,
                      const char* label) {
    const unsigned char* actual = F9011_VGA_GetSpecialColorRgb_Compat((unsigned char)color, palette);
    if (!actual) {
        ++g_fail;
        fprintf(stderr, "FAIL: %s: color %u did not resolve\n", label, color);
        return;
    }
    if (actual[0] == r && actual[1] == g && actual[2] == b) {
        ++g_pass;
    } else {
        ++g_fail;
        fprintf(stderr,
                "FAIL: %s: color %u expected (%u,%u,%u), got (%u,%u,%u)\n",
                label,
                color,
                (unsigned int)r,
                (unsigned int)g,
                (unsigned int)b,
                (unsigned int)actual[0],
                (unsigned int)actual[1],
                (unsigned int)actual[2]);
    }
}

int main(void) {
    static const unsigned char expectedPresents[16][3] = {
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 255, 255}
    };
    static const unsigned char expectedDungeonMaster[16][3] = {
        {0, 0, 0}, {109, 109, 109}, {146, 146, 146}, {188, 156, 60},
        {156, 92, 60}, {220, 188, 60}, {188, 92, 60}, {220, 220, 92},
        {255, 255, 60}, {255, 182, 0}, {219, 146, 109}, {124, 60, 28},
        {255, 0, 0}, {182, 182, 182}, {0, 0, 255}, {255, 255, 255}
    };

    /* The helper must always succeed and never write a negative or
     * out-of-range special palette index.  Out-of-range output here
     * would crash M11_Render_PresentIndexedWithSpecialPalette. */
    ASSERT_TRUE(V1_TitleFrontend_GetStepPalette(
                    V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS,
                    ((int*)0)) == 0,
                "GetStepPalette rejects null outSpecialPalette");

    /* ReDMCSB TITLE.C F0437 PC/F20 step → palette contract. */
    check_step_kind_palette(V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS,
                            VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS,
                            "PRESENTS step uses C12_PRESENTS palette");
    check_step_kind_palette(V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT,
                            VGA_PALETTE_PC34_SPECIAL_TITLE,
                            "ZOOM_BLIT step uses merged C13_DUNGEON+C14_MASTER palette");
    check_step_kind_palette(V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT,
                            VGA_PALETTE_PC34_SPECIAL_TITLE,
                            "MASTER_STRIKES_BACK_BLIT step uses merged C13_DUNGEON+C14_MASTER palette");
    check_step_kind_palette(V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK,
                            VGA_PALETTE_PC34_SPECIAL_TITLE,
                            "POST_ZOOM_VBLANK step defaults to DUNGEON+MASTER palette");
    check_step_kind_palette(V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK,
                            VGA_PALETTE_PC34_SPECIAL_TITLE,
                            "FINAL_GUARD_VBLANK step defaults to DUNGEON+MASTER palette");
    check_step_kind_palette(V1_TITLE_FRONTEND_SOURCE_EVENT_MENU_ELIGIBLE,
                            VGA_PALETTE_PC34_SPECIAL_TITLE,
                            "MENU_ELIGIBLE step defaults to DUNGEON+MASTER palette");
    ASSERT_TRUE(V1_TitleFrontend_GetFallbackFramePalette(1u, ((int*)0)) == 0,
                "GetFallbackFramePalette rejects null outSpecialPalette");
    {
        int palette = -1;
        ASSERT_TRUE(V1_TitleFrontend_GetFallbackFramePalette(1u, &palette),
                    "fallback TITLE paletteOrdinal=1 maps successfully");
        ASSERT_EQ_INT(palette,
                      VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS,
                      "fallback TITLE paletteOrdinal=1 uses C12_PRESENTS");
        ASSERT_TRUE(V1_TitleFrontend_GetFallbackFramePalette(2u, &palette),
                    "fallback TITLE paletteOrdinal=2 maps successfully");
        ASSERT_EQ_INT(palette,
                      VGA_PALETTE_PC34_SPECIAL_TITLE,
                      "fallback TITLE paletteOrdinal=2 uses C13_DUNGEON+C14_MASTER");
        ASSERT_TRUE(V1_TitleFrontend_GetFallbackFramePalette(0u, &palette),
                    "fallback TITLE unknown palette ordinals map safely");
        ASSERT_EQ_INT(palette,
                      VGA_PALETTE_PC34_SPECIAL_TITLE,
                      "fallback TITLE unknown palette ordinal defaults to C13_DUNGEON+C14_MASTER");
    }

    /* End-to-end: every step in the full TITLE animation schedule
     * must resolve to either PRESENTS-once or DUNGEON+MASTER-everywhere,
     * and PRESENTS must appear exactly once.  This is the regression
     * guard the v2.7.4 release needed: no step in the schedule is
     * allowed to silently use the merged DUNGEON+MASTER palette where
     * the source asks for the PRESENTS palette. */
    int presentsCount = 0;
    int dungeonMasterCount = 0;
    int otherCount = 0;
    unsigned int stepCount = V1_TitleFrontend_GetSourceAnimationStepCount();
    ASSERT_TRUE(stepCount > 0u, "title animation step count is non-zero");
    for (unsigned int i = 1u; i <= stepCount; ++i) {
        V1_TitleFrontendSourceAnimationStep step;
        if (!V1_TitleFrontend_GetSourceAnimationStep(i, &step)) {
            ++g_fail;
            fprintf(stderr, "FAIL: source step %u missing\n", i);
            continue;
        }
        int palette = step_palette(step.kind);
        if (palette < 0 || palette >= VGA_PALETTE_PC34_SPECIAL_PALETTE_COUNT) {
            ++g_fail;
            fprintf(stderr, "FAIL: source step %u palette out of range: %d\n", i, palette);
            continue;
        }
        if (palette == VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS) {
            ++presentsCount;
        } else if (palette == VGA_PALETTE_PC34_SPECIAL_TITLE) {
            ++dungeonMasterCount;
        } else {
            ++otherCount;
        }
    }
    ASSERT_EQ_INT(presentsCount, 1, "PRESENTS palette appears exactly once in title schedule");
    ASSERT_TRUE(dungeonMasterCount > 0,
                "DUNGEON+MASTER palette appears at least once in title schedule");
    ASSERT_EQ_INT(otherCount, 0, "no title step resolves to a palette other than PRESENTS or DUNGEON+MASTER");

    /* Palette must never be the same for both phases.  This is the
     * single-shot regression assertion: collapsing both phases to a
     * single palette index is exactly the v2.7.4 bug. */
    ASSERT_TRUE(VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS
                    != VGA_PALETTE_PC34_SPECIAL_TITLE,
                "PRESENTS and DUNGEON+MASTER palette slots are distinct");

    /* The two palettes must be visually distinct: the PRESENTS phase
     * blanks indices 0..14 while DUNGEON+MASTER overrides colors 3..8,
     * 0x0B, 0x0C, 0x0F.  Sample color 4 to prove the palettes are not the
     * same row. */
    const unsigned char* presentsColor4 =
        F9011_VGA_GetSpecialColorRgb_Compat(4, VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS);
    const unsigned char* titleColor4 =
        F9011_VGA_GetSpecialColorRgb_Compat(4, VGA_PALETTE_PC34_SPECIAL_TITLE);
    ASSERT_TRUE(presentsColor4 && titleColor4,
                "color 4 must resolve in both PRESENTS and DUNGEON+MASTER palettes");
    ASSERT_TRUE(presentsColor4[0] == 0u && presentsColor4[1] == 0u && presentsColor4[2] == 0u,
                "PRESENTS color 4 is black in the PC34 VGA C12_PRESENTS row");
    ASSERT_TRUE(!(titleColor4[0] == 0u && titleColor4[1] == 0u && titleColor4[2] == 0u),
                "DUNGEON+MASTER color 4 is not black (C13_DUNGEON lights 0x03..0x08, 0x0B, 0x0C)");

    /* Sample color 0x0C to prove the master "MASTER" / "STRIKES BACK"
     * red is wired into the DUNGEON+MASTER palette and is not the
     * v2.7.4 wrong-palette gold (170, 119, 0). */
    const unsigned char* titleColor12 =
        F9011_VGA_GetSpecialColorRgb_Compat(12, VGA_PALETTE_PC34_SPECIAL_TITLE);
    const unsigned char* titleColor15 =
        F9011_VGA_GetSpecialColorRgb_Compat(15, VGA_PALETTE_PC34_SPECIAL_TITLE);
    ASSERT_TRUE(titleColor12 && titleColor12[0] == 255u && titleColor12[1] == 0u && titleColor12[2] == 0u,
                "DUNGEON+MASTER color 0x0C is bright red (C14_MASTER 0x3F,0x00,0x00)");
    ASSERT_TRUE(titleColor15 && titleColor15[0] == 255u && titleColor15[1] == 255u && titleColor15[2] == 255u,
                "DUNGEON+MASTER color 0x0F remains white in the PC34 VGA C13_DUNGEON row");

    /* Full ReDMCSB VIDEODRV.C C25_VGA palette lock:
     *   VIDEODRV.C G8159_PRESENTS rows 549-566: only 0x0F is white.
     *   VIDEODRV.C G8160_DUNGEON rows 568-593: colors 0x03..0x08,
     *     0x0B, 0x0C, and 0x0F.
     *   VIDEODRV.C G8161_MASTER rows 600-603: color 0x0C bright red.
     * The constants are VGA 6-bit DAC values converted with
     * rgb8 = (vga6 << 2) | (vga6 >> 4), except C12 0x3F white keeps
     * the existing PC34 compatibility row's 255 endpoint. */
    for (unsigned int color = 0u; color < 16u; ++color) {
        check_rgb(VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS,
                  color,
                  expectedPresents[color][0],
                  expectedPresents[color][1],
                  expectedPresents[color][2],
                  "VIDEODRV.C G8159_PRESENTS full palette");
        check_rgb(VGA_PALETTE_PC34_SPECIAL_TITLE,
                  color,
                  expectedDungeonMaster[color][0],
                  expectedDungeonMaster[color][1],
                  expectedDungeonMaster[color][2],
                  "VIDEODRV.C G8160_DUNGEON + G8161_MASTER full palette");
    }

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
