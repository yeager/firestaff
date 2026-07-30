/*
 * DM1 V1 champion panel runtime pixel probe.
 *
 * This is Firestaff-side evidence only.  It opens the hash-verified DM1 V1
 * runtime when local assets are available, installs a deterministic four
 * champion party, renders one V1 frame, and checks that the party HUD/status
 * boxes are populated through the real M11 draw stack and GRAPHICS.DAT-backed
 * slot-box assets.  It does not claim original DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 draws the C151..C154 status boxes;
 *   ReDMCSB CHAMDRAW.C F0287 draws bottom-anchored HP/stamina/mana bars;
 *   ReDMCSB CHAMDRAW.C F0291 draws C033/C034/C035 hand slot boxes;
 *   ReDMCSB CHAMDRAW.C F0622 lines 41-58 prepares the C113..C116
 *   19x14 champion icon composite from the C028 icon strip.
 *   ReDMCSB COORD.C/layout-696 anchors C113..C116 champion icon zones.
 *   ReDMCSB COORD.C/layout-696 keeps C151..C154 on a 69px stride with
 *   67x29 status boxes, leaving two black pixels between adjacent boxes.
 *   ReDMCSB CHAMDRAW.C F0292 lines 879-884 clears the live status-box
 *   name strip after the full 67x29 box has already been refreshed.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_panel_disabled_icon_state_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_layout_zones_pc34_compat.h"
#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_CHAMPION_COUNT = 4,
    PROBE_STALE_PIXEL = M11_FB_ENCODE(0, 15),
    PROBE_ACTION_EMPTY_HAND_ICON = 201,
    PROBE_ACTION_CELL_CYAN = 4
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int probe_dm1_action_icon_cell_zone(int slot,
                                           int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    DM1_V1_ActionAreaRectPc34 rect;
    if (!dm1_v1_action_icon_cell_zone_id_pc34(slot)) return 0;
    rect = dm1_v1_action_icon_cell_rect_pc34(slot);
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

static int probe_dm1_action_icon_inner_zone(int slot,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH) {
    DM1_V1_ActionAreaRectPc34 rect;
    if (!dm1_v1_action_icon_inner_zone_id_pc34(slot)) return 0;
    rect = dm1_v1_action_icon_inner_rect_pc34(slot);
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

static int probe_dm1_action_icon_cell_backdrop_color(
    const M11_GameViewState* game,
    int slot) {
    const struct ChampionState_Compat* champ;
    if (!game || slot < 0 || slot >= CHAMPION_MAX_PARTY ||
        slot >= game->world.party.championCount) {
        return -1;
    }
    champ = &game->world.party.champions[slot];
    return dm1_v1_champion_panel_action_icon_cell_backdrop_color_pc34(
        champ->present != 0, champ->hp.current <= 0);
}

static int probe_dm1_status_rect_xywh(const DM1_V1_ChampionStatusRectPc34* rect,
                                      int* outX,
                                      int* outY,
                                      int* outW,
                                      int* outH) {
    if (!rect) return 0;
    if (outX) *outX = rect->x;
    if (outY) *outY = rect->y;
    if (outW) *outW = rect->w;
    if (outH) *outH = rect->h;
    return 1;
}

static int probe_dm1_layout_rect_xywh(DM1_V1_LayoutZoneRectPc34 rect,
                                      int* outX,
                                      int* outY,
                                      int* outW,
                                      int* outH) {
    if (rect.w <= 0 || rect.h <= 0) return 0;
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

/* This probe keeps independent source-locked oracles for the status-panel
 * helpers.  M11 now exports the same API names, so give the local oracle
 * copies probe-private names without changing their assertions or M11. */
#define probe_M11_GameView_GetV1StatusBoxFillColor probe_v1_status_box_fill_color
#define probe_M11_GameView_GetV1StatusNameClearColor probe_v1_status_name_clear_color
#define probe_M11_GameView_GetV1StatusNameColor probe_v1_status_name_color
#define probe_M11_GameView_GetV1StatusBoxZone probe_v1_status_box_zone
#define probe_M11_GameView_GetV1StatusNameZone probe_v1_status_name_zone
#define probe_M11_GameView_GetV1StatusNameTextZone probe_v1_status_name_text_zone
#define probe_M11_GameView_GetV1StatusBarZone probe_v1_status_bar_zone
#define probe_M11_GameView_GetV1StatusHandSlotBoxZone probe_v1_status_hand_slot_box_zone
#define M11_GameView_GetV1StatusHandIconZone probe_v1_status_hand_icon_zone
#define probe_M11_GameView_GetV1ChampionBarColor probe_v1_champion_bar_color
#define probe_M11_GameView_GetV1StatusBarBlankColor probe_v1_status_bar_blank_color
#define M11_GameView_GetV1ObjectIconSourceZone probe_v1_object_icon_source_zone

static int probe_M11_GameView_GetV1StatusBoxFillColor(void) {
    return dm1_v1_champion_status_box_fill_color_pc34();
}

static int probe_M11_GameView_GetV1StatusNameClearColor(void) {
    return dm1_v1_champion_status_name_clear_color_pc34();
}

static int probe_M11_GameView_GetV1StatusNameColor(const M11_GameViewState* game,
                                             int slot) {
    const struct ChampionState_Compat* champ;
    if (!game || slot < 0 || slot >= CHAMPION_MAX_PARTY ||
        slot >= game->world.party.championCount) {
        return -1;
    }
    champ = &game->world.party.champions[slot];
    return dm1_v1_champion_status_name_color_pc34(
        champ->present,
        champ->hp.current,
        slot == game->world.party.activeChampionIndex);
}

static int probe_M11_GameView_GetV1StatusBoxZone(int slot,
                                           int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!dm1_v1_champion_status_box_rect_pc34(slot, &rect)) return 0;
    return probe_dm1_status_rect_xywh(&rect, outX, outY, outW, outH);
}

static int probe_M11_GameView_GetV1StatusNameZone(int slot,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!dm1_v1_champion_status_name_rect_pc34(slot, &rect)) return 0;
    return probe_dm1_status_rect_xywh(&rect, outX, outY, outW, outH);
}

static int probe_M11_GameView_GetV1StatusNameTextZone(int slot,
                                                int* outX,
                                                int* outY,
                                                int* outW,
                                                int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!dm1_v1_champion_status_name_text_rect_pc34(slot, &rect)) return 0;
    return probe_dm1_status_rect_xywh(&rect, outX, outY, outW, outH);
}

static int probe_M11_GameView_GetV1StatusBarZone(int slot,
                                           int stat,
                                           int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!dm1_v1_champion_status_bar_rect_pc34(slot, stat, &rect)) return 0;
    return probe_dm1_status_rect_xywh(&rect, outX, outY, outW, outH);
}

static int probe_M11_GameView_GetV1StatusHandSlotBoxZone(int slot,
                                                   int hand,
                                                   int* outX,
                                                   int* outY,
                                                   int* outW,
                                                   int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!dm1_v1_champion_status_hand_slot_box_rect_pc34(slot, hand, &rect)) {
        return 0;
    }
    return probe_dm1_status_rect_xywh(&rect, outX, outY, outW, outH);
}

static int M11_GameView_GetV1StatusHandIconZone(int slot,
                                                int hand,
                                                int* outX,
                                                int* outY,
                                                int* outW,
                                                int* outH) {
    DM1_V1_ChampionStatusRectPc34 rect;
    if (!dm1_v1_champion_status_hand_icon_rect_pc34(slot, hand, &rect)) {
        return 0;
    }
    return probe_dm1_status_rect_xywh(&rect, outX, outY, outW, outH);
}

static int probe_M11_GameView_GetV1ChampionIconZone(int slot,
                                              int* outX,
                                              int* outY,
                                              int* outW,
                                              int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect;
    if (!dm1_v1_champion_icon_rect_pc34(slot, &rect)) return 0;
    return probe_dm1_layout_rect_xywh(rect, outX, outY, outW, outH);
}

static int M11_GameView_GetV1ChampionIconSourceIndex(
    const M11_GameViewState* game,
    int slot) {
    const struct ChampionState_Compat* champ;
    if (!game || slot < 0 || slot >= CHAMPION_MAX_PARTY ||
        slot >= game->world.party.championCount) {
        return -1;
    }
    champ = &game->world.party.champions[slot];
    if (!champ->present) return -1;
    return ((int)champ->direction - ((int)game->world.party.direction & 3) + 4) & 3;
}

static int probe_ChampionIconGraphicId(void) {
    return dm1_v1_graphic_champion_icons_pc34();
}

static int probe_M11_GameView_GetV1ChampionBarColor(int slot) {
    if (slot < 0 || slot >= DM1_CHAMPION_COUNT) {
        return DM1_COLOR_LIGHTEST_GRAY;
    }
    return DM1_ChampionColor[slot];
}

static int probe_M11_GameView_GetV1StatusBarBlankColor(void) {
    return DM1_COLOR_DARKEST_GRAY;
}

static int probe_M11_GameView_GetV1SlotBoxNormalGraphicId(void) {
    return dm1_v1_graphic_slot_box_normal_pc34();
}

static int probe_SlotBoxWoundedGraphicId(void) {
    return dm1_v1_graphic_slot_box_wounded_pc34();
}

static int probe_M11_GameView_GetV1SlotBoxActingHandGraphicId(void) {
    return dm1_v1_graphic_slot_box_acting_hand_pc34();
}

static int probe_M11_GameView_GetV1StatusHandSlotGraphic(
    const M11_GameViewState* game,
    int slot,
    int hand) {
    const struct ChampionState_Compat* champ;
    if (!game || slot < 0 || slot >= CHAMPION_MAX_PARTY ||
        hand < 0 || hand > 1 ||
        slot >= game->world.party.championCount) {
        return 0;
    }
    champ = &game->world.party.champions[slot];
    if (!champ->present || champ->hp.current == 0) return 0;
    return dm1_v1_champion_status_hand_slot_graphic_pc34(
        hand,
        (uint16_t)champ->wounds,
        hand == 1 && game->actingChampionOrdinal == (unsigned int)(slot + 1));
}

/* Local pixel-oracle helper; keep it distinct from the public M11 API. */
static int probe_v1_status_hand_icon_index(
    const M11_GameViewState* game,
    int slot,
    int hand) {
    const struct ChampionState_Compat* champ;
    if (!game || slot < 0 || slot >= CHAMPION_MAX_PARTY ||
        hand < 0 || hand > 1 ||
        slot >= game->world.party.championCount) {
        return -1;
    }
    champ = &game->world.party.champions[slot];
    if (!champ->present || champ->hp.current == 0) return -1;
    return DM1_ChampionPanel_EmptyHandIconIndex(hand, (uint16_t)champ->wounds);
}

static int M11_GameView_GetV1ObjectIconSourceZone(int iconIndex,
                                                  int* outGraphic,
                                                  int* outX,
                                                  int* outY,
                                                  int* outW,
                                                  int* outH) {
    DM1_V1_ObjectIconSourceZonePc34 zone;
    if (!dm1_v1_object_icon_source_zone_pc34(iconIndex, &zone)) return 0;
    if (outGraphic) *outGraphic = zone.graphic_index;
    if (outX) *outX = zone.x;
    if (outY) *outY = zone.y;
    if (outW) *outW = zone.w;
    if (outH) *outH = zone.h;
    return 1;
}

static int M11_GameView_MapV1ActionIconPaletteColor(int colorIndex,
                                                    int applyActionPalette) {
    if (applyActionPalette && ((colorIndex & 0x0F) == 12)) return 4;
    return colorIndex;
}

static int count_color(const unsigned char* fb,
                       int width,
                       int x,
                       int y,
                       int w,
                       int h,
                       int color) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if ((int)px_index(fb, width, x + xx, y + yy) == color) {
                ++count;
            }
        }
    }
    return count;
}

static int count_raw_pixel(const unsigned char* fb,
                           int width,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char rawPixel) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * width + x + xx] == rawPixel) {
                ++count;
            }
        }
    }
    return count;
}

static int expected_fill_height(int current, int maximum, int fullHeight) {
    long scaled;
    if (current <= 0 || maximum <= 0 || fullHeight <= 0) {
        return 0;
    }
    scaled = (long)fullHeight * (long)current / (long)maximum;
    if (scaled < 1) {
        scaled = 1;
    }
    if (scaled > fullHeight) {
        scaled = fullHeight;
    }
    return (int)scaled;
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          int direction,
                          int hp,
                          int hpMax,
                          int stamina,
                          int staminaMax,
                          int mana,
                          int manaMax,
                          unsigned short wounds) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = (unsigned short)hpMax;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = (unsigned short)staminaMax;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = (unsigned short)manaMax;
    champ->wounds = wounds;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 2; /* slot 1 action hand uses C035 */
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 100, 80, 80, 60, 60, 0);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 50, 100, 40, 80, 30, 60, 0x0001u);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 25, 100, 20, 80, 15, 60, 0x0002u);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 1, 100, 1, 80, 1, 60, 0);
}

static int check_status_box_pixels(const M11_GameViewState* game,
                                   const unsigned char* fb,
                                   int slot) {
    int ok = 1;
    int x, y, w, h;
    int nx, ny, nw, nh;
    int nameTextX, nameTextY, nameTextW, nameTextH;
    int fillColor = probe_M11_GameView_GetV1StatusBoxFillColor();
    int nameClearColor = probe_M11_GameView_GetV1StatusNameClearColor();
    int nameColor = probe_M11_GameView_GetV1StatusNameColor(game, slot);
    char label[128];

    snprintf(label, sizeof(label), "slot%d status box zone", slot);
    ok &= expect_true(label, probe_M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                             w == 67 && h == 29);

    snprintf(label, sizeof(label), "slot%d status box fill visible", slot);
    ok &= expect_true(label, count_color(fb, PROBE_FB_W, x, y, w, h, fillColor) > 300);

    /* ReDMCSB: CHAMDRAW.C F0292 lines 879-905 refreshes live status
     * boxes before drawing name text and bar graphs.  Render over a
     * nonzero framebuffer-level sentinel and prove the full 67x29
     * party-HUD/status-box rectangle was touched by the M11 V1 draw
     * stack.  This is Firestaff runtime evidence, not DOS parity. */
    snprintf(label, sizeof(label), "slot%d status box overwrites stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);

    snprintf(label, sizeof(label), "slot%d name clear zone", slot);
    ok &= expect_true(label, probe_M11_GameView_GetV1StatusNameZone(slot, &nx, &ny, &nw, &nh) &&
                             count_color(fb, PROBE_FB_W, nx, ny, nw, nh, nameClearColor) > 90);

    snprintf(label, sizeof(label), "slot%d name text color", slot);
    ok &= expect_true(label, probe_M11_GameView_GetV1StatusNameTextZone(slot,
                                                                  &nameTextX,
                                                                  &nameTextY,
                                                                  &nameTextW,
                                                                  &nameTextH) &&
                             count_color(fb, PROBE_FB_W,
                                         nameTextX, nameTextY,
                                         nameTextW, nameTextH,
                                         nameColor) > 0);
    return ok;
}

static int check_status_box_gutter_pixels(const unsigned char* fb) {
    int ok = 1;
    int slot;
    char label[128];

    for (slot = 0; slot < PROBE_CHAMPION_COUNT - 1; ++slot) {
        int x, y, w, h;
        int nextX, nextY, nextW, nextH;
        int gutterX;
        int gutterW;
        snprintf(label, sizeof(label), "slot%d status box zone for gutter", slot);
        ok &= expect_true(label, probe_M11_GameView_GetV1StatusBoxZone(slot,
                                                                 &x, &y,
                                                                 &w, &h) &&
                                 w == 67 && h == 29);
        snprintf(label, sizeof(label), "slot%d next status box zone for gutter", slot);
        ok &= expect_true(label, probe_M11_GameView_GetV1StatusBoxZone(slot + 1,
                                                                 &nextX, &nextY,
                                                                 &nextW, &nextH) &&
                                 nextY == y && nextW == 67 && nextH == 29);
        if (!ok) {
            return 0;
        }
        gutterX = x + w;
        gutterW = nextX - gutterX;
        snprintf(label, sizeof(label), "slot%d status box two-pixel gutter width",
                 slot);
        ok &= expect_int(label, gutterW, 2);
        snprintf(label, sizeof(label), "slot%d status box gutter remains black",
                 slot);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     gutterX, y,
                                     gutterW, h,
                                     0),
                         gutterW * h);
    }
    return ok;
}

static int check_bar_pixels(const M11_GameViewState* game,
                            const unsigned char* fb,
                            int slot,
                            int stat) {
    const struct ChampionState_Compat* champ = &game->world.party.champions[slot];
    int current[3];
    int maximum[3];
    int x, y, w, h;
    int fillHeight;
    int blankHeight;
    int fillColor = probe_M11_GameView_GetV1ChampionBarColor(slot);
    int blankColor = probe_M11_GameView_GetV1StatusBarBlankColor();
    int ok = 1;
    char label[128];

    current[0] = champ->hp.current;
    current[1] = champ->stamina.current;
    current[2] = champ->mana.current;
    maximum[0] = champ->hp.maximum;
    maximum[1] = champ->stamina.maximum;
    maximum[2] = champ->mana.maximum;

    snprintf(label, sizeof(label), "slot%d stat%d bar zone", slot, stat);
    ok &= expect_true(label, probe_M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
                             w == 4 && h == 25);
    fillHeight = expected_fill_height(current[stat], maximum[stat], h);
    blankHeight = h - fillHeight;
    if (blankHeight > 0) {
        snprintf(label, sizeof(label), "slot%d stat%d blank top", slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, x, y, w, blankHeight, blankColor),
                         w * blankHeight);
    }
    if (fillHeight > 0) {
        snprintf(label, sizeof(label), "slot%d stat%d colored bottom", slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     x, y + blankHeight,
                                     w, fillHeight,
                                     fillColor),
                         w * fillHeight);
    }
    return ok;
}

static int check_hand_slot_asset_pixels(const M11_GameViewState* game,
                                        const unsigned char* fb,
                                        int slot,
                                        int hand) {
    int gfx = probe_M11_GameView_GetV1StatusHandSlotGraphic(game, slot, hand);
    const M11_AssetSlot* asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                                     (unsigned int)gfx);
    int x, y, w, h;
    int expected = 0;
    int matched = 0;
    int yy;
    char label[128];
    int ok = 1;

    snprintf(label, sizeof(label), "slot%d hand%d expected graphic", slot, hand);
    ok &= expect_true(label, gfx == probe_M11_GameView_GetV1SlotBoxNormalGraphicId() ||
                             gfx == probe_SlotBoxWoundedGraphicId() ||
                             gfx == probe_M11_GameView_GetV1SlotBoxActingHandGraphicId());
    if (!ok) {
        return 0;
    }
    snprintf(label, sizeof(label), "slot%d hand%d slot-box asset", slot, hand);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 18 && asset->height == 18);
    snprintf(label, sizeof(label), "slot%d hand%d slot-box zone", slot, hand);
    ok &= expect_true(label, probe_M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                                     &x, &y, &w, &h) &&
                             w == 18 && h == 18);
    if (!ok) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            if (xx > 0 && xx < 17 && yy > 0 && yy < 17) {
                continue; /* object/empty-hand icon inset may overdraw */
            }
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) {
                continue;
            }
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d hand%d GRAPHICS.DAT perimeter match", slot, hand);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 85);
    printf("slot%d hand%d gfx=%d perimeter=%d/%d\n", slot, hand, gfx, matched, expected);
    return ok;
}

/* ReDMCSB: CHAMDRAW.C F0291 lines 632-651 draws the C033/C034/C035
 * status-hand slot box, then draws the selected 16x16 icon one pixel
 * inside that box.  For empty ready/action hands the source icon
 * ordinals are C212/C213 and C214/C215 respectively.  This check
 * pixel-matches the on-screen inset against the GRAPHICS.DAT object
 * icon sheet for every live party hand slot. */
static int check_hand_slot_icon_pixels(const M11_GameViewState* game,
                                       const unsigned char* fb,
                                       int slot,
                                       int hand) {
    int iconIndex = probe_v1_status_hand_icon_index(game, slot, hand);
    int graphicIndex;
    int srcX, srcY, srcW, srcH;
    const M11_AssetSlot* asset;
    int x, y, w, h;
    int matched = 0;
    int total = 0;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d hand%d empty-hand icon index", slot, hand);
    ok &= expect_true(label,
                      iconIndex == 212 || iconIndex == 213 ||
                      iconIndex == 214 || iconIndex == 215);
    snprintf(label, sizeof(label), "slot%d hand%d icon source zone", slot, hand);
    ok &= expect_true(label,
                      M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                             &graphicIndex,
                                                             &srcX, &srcY,
                                                             &srcW, &srcH) &&
                      srcW == 16 && srcH == 16);
    snprintf(label, sizeof(label), "slot%d hand%d icon screen zone", slot, hand);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusHandIconZone(slot, hand,
                                                           &x, &y, &w, &h) &&
                      w == 16 && h == 16);
    if (!ok) {
        return 0;
    }

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)graphicIndex);
    snprintf(label, sizeof(label), "slot%d hand%d icon sheet asset", slot, hand);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             srcX + srcW <= (int)asset->width &&
                             srcY + srcH <= (int)asset->height);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src = (unsigned char)
                (asset->pixels[(srcY + yy) * (int)asset->width + srcX + xx] & 0x0F);
            unsigned char dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++total;
            if (dst == src) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d hand%d GRAPHICS.DAT icon inset match",
             slot, hand);
    ok &= expect_true(label, total == 256 && matched == total);
    printf("slot%d hand%d icon=%d gfx=%d pixels=%d/%d\n",
           slot, hand, iconIndex, graphicIndex, matched, total);
    return ok;
}

static int check_champion_icon_pixels(const M11_GameViewState* game,
                                      const unsigned char* fb,
                                      int slot) {
    int x, y, w, h;
    int iconIndex;
    int gfxId;
    int iconStripCellW;
    int expected = 0;
    int matched = 0;
    int transparentMatch = 0;
    int transparentPixels = 0;
    const M11_AssetSlot* asset;
    int baseColor = probe_M11_GameView_GetV1ChampionBarColor(slot);
    int xx;
    int yy;
    const int transparentColor = 12; /* M11_COLOR_DARK_GRAY */
    char label[128];
    int ok = 1;
    snprintf(label, sizeof(label), "slot%d champion icon source", slot);
    iconIndex = M11_GameView_GetV1ChampionIconSourceIndex(game, slot);
    ok &= expect_true(label, iconIndex >= 0);
    snprintf(label, sizeof(label), "slot%d champion icon zone", slot);
    ok &= expect_true(label, probe_M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
                              w == 19 && h == 14);
    gfxId = probe_ChampionIconGraphicId();
    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)gfxId);
    snprintf(label, sizeof(label), "slot%d champion icon strip asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->height >= 14 && asset->width >= 76 &&
                             asset->width % 4 == 0 &&
                             asset->height == (unsigned short)h);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    iconStripCellW = (int)asset->width / 4;

    /* ReDMCSB: CHAMDRAW.C F0622 lines 41-58 fills a 19x14 temporary
     * bitmap with the champion base color, then blends the C028
     * GRAPHIC_CHAMPION_ICONS strip over it with C12 dark-gray
     * transparency.  Reproduce that composite rule across the full
     * C113..C116 cell so the right-edge columns cannot retain stale
     * pixels from a prior HUD frame. */
    for (yy = 0; yy < h; ++yy) {
        int srcX0 = iconIndex * iconStripCellW;
        for (xx = 0; xx < w; ++xx) {
            int srcX = srcX0 + xx;
            unsigned char src = (unsigned char)(asset->pixels[yy * (int)asset->width + srcX] & 0x0F);
            unsigned char dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            if (src == transparentColor) {
                ++transparentPixels;
                if ((int)dst == baseColor) {
                    ++transparentMatch;
                }
            } else {
                ++expected;
                if ((int)dst == (int)src) {
                    ++matched;
                }
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d champion icon opaque pixel match", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 95);
    snprintf(label, sizeof(label), "slot%d champion icon no stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);
    if (transparentPixels > 0) {
        snprintf(label, sizeof(label), "slot%d champion icon transparent fill match", slot);
        ok &= expect_true(label,
                          transparentMatch * 100 >= transparentPixels * 95);
    } else {
        snprintf(label, sizeof(label), "slot%d champion icon has transparent pixels", slot);
        ok &= expect_true(label, 0);
    }
    printf("slot%d icon src=%d gfx=%d opaque=%d/%d transparent=%d/%d base=%d\n",
           slot, iconIndex, gfxId, matched, expected,
           transparentMatch, transparentPixels, baseColor);
    return ok;
}

/* ReDMCSB: MENUS.C F0386 draws action-hand cells C089..C092 for live
 * champions, fills the action icon bitmap with C04 cyan for an empty hand,
 * then blits the empty-hand object icon through the action-area palette
 * remap.  This is a Firestaff runtime pixel gate for the champion panel's
 * action icon row; it does not claim original DOS screenshot parity. */
static int check_action_icon_cell_pixels(const M11_GameViewState* game,
                                         const unsigned char* fb,
                                         int slot) {
    int cellX, cellY, cellW, cellH;
    int innerX, innerY, innerW, innerH;
    int graphicIndex;
    int srcX, srcY, srcW, srcH;
    const M11_AssetSlot* asset;
    int matched = 0;
    int total = 0;
    int cyanPixels;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d action icon cell id", slot);
    ok &= expect_int(label,
                     dm1_v1_action_icon_cell_zone_id_pc34(slot),
                     89 + slot);
    snprintf(label, sizeof(label), "slot%d action icon inner id", slot);
    ok &= expect_int(label,
                     dm1_v1_action_icon_inner_zone_id_pc34(slot),
                     93 + slot);
    snprintf(label, sizeof(label), "slot%d action icon cell zone", slot);
    ok &= expect_true(label,
                      probe_dm1_action_icon_cell_zone(slot,
                                                           &cellX,
                                                           &cellY,
                                                           &cellW,
                                                           &cellH) &&
                      cellW == 20 && cellH == 35);
    snprintf(label, sizeof(label), "slot%d action icon inner zone", slot);
    ok &= expect_true(label,
                      probe_dm1_action_icon_inner_zone(slot,
                                                            &innerX,
                                                            &innerY,
                                                            &innerW,
                                                            &innerH) &&
                      innerW == 16 && innerH == 16);
    snprintf(label, sizeof(label), "slot%d action icon backdrop color", slot);
    ok &= expect_int(label,
                     probe_dm1_action_icon_cell_backdrop_color(game, slot),
                     PROBE_ACTION_CELL_CYAN);
    snprintf(label, sizeof(label), "slot%d action empty-hand source zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1ObjectIconSourceZone(
                          PROBE_ACTION_EMPTY_HAND_ICON,
                          &graphicIndex,
                          &srcX,
                          &srcY,
                          &srcW,
                          &srcH) &&
                      srcW == 16 && srcH == 16);
    if (!ok) {
        return 0;
    }

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)graphicIndex);
    snprintf(label, sizeof(label), "slot%d action empty-hand icon asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             srcX + srcW <= (int)asset->width &&
                             srcY + srcH <= (int)asset->height);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    cyanPixels = count_color(fb, PROBE_FB_W,
                             cellX, cellY, cellW, cellH,
                             PROBE_ACTION_CELL_CYAN);
    snprintf(label, sizeof(label), "slot%d action icon cyan backdrop visible",
             slot);
    ok &= expect_true(label, cyanPixels > 350);

    for (yy = 0; yy < innerH; ++yy) {
        int xx;
        for (xx = 0; xx < innerW; ++xx) {
            unsigned char src = (unsigned char)
                (asset->pixels[(srcY + yy) * (int)asset->width + srcX + xx] & 0x0F);
            unsigned char expected = (unsigned char)
                M11_GameView_MapV1ActionIconPaletteColor(src, 1);
            unsigned char dst = px_index(fb, PROBE_FB_W, innerX + xx, innerY + yy);
            ++total;
            if (dst == expected) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label),
             "slot%d action empty-hand GRAPHICS.DAT icon match", slot);
    ok &= expect_true(label, total == 256 && matched == total);
    printf("slot%d action cell C%d inner C%d icon=%d gfx=%d cyan=%d pixels=%d/%d\n",
           slot,
           dm1_v1_action_icon_cell_zone_id_pc34(slot),
           dm1_v1_action_icon_inner_zone_id_pc34(slot),
           PROBE_ACTION_EMPTY_HAND_ICON,
           graphicIndex,
           cyanPixels,
           matched,
           total);
    return ok;
}

/* ReDMCSB CASTER.C F0394 clears G0000 when there is no caster. When a
 * caster opens the panel it blits C009 at the 96px source box (224,42) and
 * copies rows 1 and 2 of C011 to (224,50)/(224,62) before TEXT.C writes the
 * cyan rune glyphs. This locks the real GRAPHICS.DAT composition instead of
 * accepting the old 87px click-zone crop or a generated spell frame. */
static int check_spell_area_asset_composition(const M11_GameViewState* game,
                                              const unsigned char* fb,
                                              int spellOpen) {
    /* G0000 owns the physical 96x33 source box. C009 is only the
     * 87x25 bitmap placed inside it, so this probe must not use the
     * graphic rectangle when it verifies the clear extent. */
    DM1_V1_SpellAreaRectPc34 rect = dm1_v1_spell_area_source_box_rect_pc34();
    const M11_AssetSlot* background;
    const M11_AssetSlot* lines;
    int ok = 1;
    int x;
    int y;
    int matched = 0;
    int total = 0;
    char label[128];

    snprintf(label, sizeof(label), "spell area physical source rectangle");
    ok &= expect_true(label,
                      rect.x == 224 && rect.y == 42 &&
                      rect.w == 96 && rect.h == 33);
    if (!spellOpen) {
        snprintf(label, sizeof(label), "no-caster spell box is black");
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, rect.x, rect.y,
                                     rect.w, rect.h, 0),
                         rect.w * rect.h);
        return ok;
    }

    background = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                      DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34);
    lines = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34);
    snprintf(label, sizeof(label), "C009 real spell background dimensions");
    ok &= expect_true(label, background && background->loaded && background->pixels &&
                             background->width == 87 && background->height == 25);
    snprintf(label, sizeof(label), "C011 real spell line dimensions");
    ok &= expect_true(label, lines && lines->loaded && lines->pixels &&
                             lines->width == 14 && lines->height == 39);
    snprintf(label, sizeof(label), "PC34 font loaded for spell glyphs");
    ok &= expect_true(label, game->originalFontAvailable != 0);
    if (!ok) return 0;

    /* CASTER.C F0394 copies the 11 interior scanlines of its 12-row
     * line bitmap at y=50..61; y=62 belongs to the next source line. */
    for (y = 0; y < 12; ++y) {
        for (x = 0; x < 14; ++x) {
            unsigned char expected = (unsigned char)(lines->pixels[(13 + y) * 14 + x] & 0x0F);
            unsigned char actual = px_index(fb, PROBE_FB_W, 224 + x, 50 + y);
            ++total;
            if (actual == expected) ++matched;
        }
    }
    snprintf(label, sizeof(label), "C011 line 2 GRAPHICS.DAT pixel retention");
    ok &= expect_true(label, total == 168 && matched == total);

    matched = 0;
    total = 0;
    for (y = 0; y < 12; ++y) {
        for (x = 0; x < 14; ++x) {
            unsigned char expected = (unsigned char)(lines->pixels[(26 + y) * 14 + x] & 0x0F);
            unsigned char actual = px_index(fb, PROBE_FB_W, 224 + x, 62 + y);
            ++total;
            if (actual == expected) ++matched;
        }
    }
    snprintf(label, sizeof(label), "C011 line 3 GRAPHICS.DAT pixel retention");
    ok &= expect_true(label, total == 168 && matched == total);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M12_StartupMenuInitOptions menuOptions;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int slot;
    int stat;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    memset(&menuOptions, 0, sizeof(menuOptions));
    /* The pixel probe opens a selected DM1 entry directly.  Gallery I/O is
     * unrelated to the real PC34 HUD frame and can block headless runs on
     * an unavailable verification-screenshot volume. */
    menuOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, dataDir, NULL, &menuOptions);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr, "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    seed_party(&game);
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_status_box_pixels(&game, fb, slot);
        ok &= check_champion_icon_pixels(&game, fb, slot);
        for (stat = 0; stat < 3; ++stat) {
            ok &= check_bar_pixels(&game, fb, slot, stat);
        }
        ok &= check_hand_slot_asset_pixels(&game, fb, slot, 0);
        ok &= check_hand_slot_asset_pixels(&game, fb, slot, 1);
        ok &= check_hand_slot_icon_pixels(&game, fb, slot, 0);
        ok &= check_hand_slot_icon_pixels(&game, fb, slot, 1);
    }
    ok &= check_status_box_gutter_pixels(fb);

    /* ReDMCSB: MENUS.C F0387 draws either the acting champion action
     * menu or the four C089..C092 action icon cells.  The first pass keeps
     * actingChampionOrdinal=2 to cover C035 in the status hand slot; this
     * second pass clears the acting champion so the idle icon-mode branch
     * is pixel-checked with the same real assets and seeded party. */
    game.actingChampionOrdinal = 0;
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_action_icon_cell_pixels(&game, fb, slot);
    }
    ok &= check_spell_area_asset_composition(&game, fb, 0);

    /* Enter through C100/F0394's public route. Merely flipping
     * spellPanelOpen skips its selected-caster record, which correctly
     * causes the live painter to retain the source black clear. */
    ok &= expect_true("spell panel source open", M11_GameView_OpenSpellPanel(&game));
    ok &= expect_true("spell panel source rune 0", M11_GameView_EnterRune(&game, 0));
    ok &= expect_true("spell panel source rune 1", M11_GameView_EnterRune(&game, 1));
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_spell_area_asset_composition(&game, fb, 1);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel runtime pixel probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
