/* Real-media Hall of Champions interaction guard for original PC 3.4. */
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "menu_input_m12.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int panel_pixel_delta(const unsigned char *a, const unsigned char *b,
                             int x, int y, int w, int h)
{
    int changed = 0;
    for (int row = 0; row < h; ++row)
        for (int col = 0; col < w; ++col)
            changed += a[(y + row) * 320 + x + col] !=
                       b[(y + row) * 320 + x + col];
    return changed;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState state;
    unsigned short thing;
    int steps = 0;
    char panelText[512];
    char sourceText[512];
    unsigned char framebuffer[320 * 200];
    unsigned char beforeEyeFrame[320 * 200];
    unsigned char heldEyeFrame[320 * 200];
    const M11_AssetSlot *scrollPanel;
    int panelX, panelY, panelW, panelH;
    int sourceMatches = 0, sourceOpaque = 0, textPixels = 0;
    M11_Dm1FloorItemHostPresentationReceipt floorReceipt;
    static const M12_MenuInput route_to_c127[] = {
        M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
        M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_TURN_RIGHT, M12_MENU_INPUT_TURN_RIGHT
    };
    static const M12_MenuInput route_to_scroll[] = {
        M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN
    };
    size_t i;
    int tick;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 77;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, archive) || !state.world.dungeon ||
        !state.world.things || state.world.dungeon->header.mapCount < 1 ||
        state.world.things->scrollCount != 35) {
        fputs("FAIL: canonical PC34 dungeon did not load\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Replay the same 28-command PC-3.4 keypad route as hoc_route in the
     * native CLI regression, using production runtime inputs. The direct M11
     * test advances five idle ticks after each input so one command completes
     * before the next; this is deterministic runtime pacing, not a claim that
     * the CLI script has identical tick timing. Start from the source
     * bootstrap pose; no save or pose is fabricated. */
    for (tick = 0; tick < 5; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&state);
    for (i = 0u; i < sizeof(route_to_c127) / sizeof(route_to_c127[0]); ++i) {
        M11_GameInputResult result =
            M11_GameView_HandleInput(&state, route_to_c127[i]);
        if (result == M11_GAME_INPUT_IGNORED) {
            fputs("FAIL: authentic PC34 Hall route input was ignored\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 5; ++tick)
            (void)M11_GameView_AdvanceIdleTick(&state);
    }
    if (M11_GameView_HandlePointerButton(&state, 112, 83,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        !state.candidateMirrorPanelActive ||
        state.candidateMirrorOrdinal != 5) {
        fputs("FAIL: source route did not open C127 ordinal 5\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_HandlePointerButton(&state, 130, 115,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        state.world.party.championCount != 1) {
        fputs("FAIL: C040 did not recruit the authentic C127 candidate\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (i = 0u; i < sizeof(route_to_scroll) / sizeof(route_to_scroll[0]); ++i) {
        M11_GameInputResult result =
            M11_GameView_HandleInput(&state, route_to_scroll[i]);
        if (result == M11_GAME_INPUT_IGNORED) {
            fputs("FAIL: authentic Hall route to scroll was ignored\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 5; ++tick)
            (void)M11_GameView_AdvanceIdleTick(&state);
    }
    if (state.world.party.mapIndex != 0 || state.world.party.mapX != 4 ||
        state.world.party.mapY != 15 || state.world.party.direction != 0) {
        fputs("FAIL: production movement did not reach authentic HoC (4,15)\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(state.world.dungeon,
        state.world.things, 0, 4, 15);
    while (thing != THING_NONE && thing != THING_ENDOFLIST && steps++ < 32) {
        unsigned int type = THING_GET_TYPE(thing);
        unsigned int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_SCROLL && index == 0u) break;
        if (type >= DUNGEON_THING_TYPE_COUNT ||
            index >= (unsigned int)state.world.things->thingCounts[type]) {
            thing = THING_NONE;
            break;
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(state.world.things, thing);
    }
    if (thing == THING_NONE || thing == THING_ENDOFLIST || steps > 32 ||
        state.world.things->scrolls[0].closed == 0 ||
        state.world.things->scrolls[0].textStringThingIndex != 33u) {
        fputs("FAIL: PC34 HoC square (4,15) lost closed scroll 0/text 33\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* DUNVIEW.C F0127:8294/F0115 draws D0C's C2500 cells 0/1 before
     * CLIKVIEW.C F0373 can pick up the visible object through C080.
     * The source pile's near object is WATER (0x280b); C007 and a normal
     * slot click must then place it in the backpack. */
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&floorReceipt);
    if (!floorReceipt.valid || !floorReceipt.floorItemLane ||
        !floorReceipt.destinationPixelsChanged ||
        M11_GameView_HandlePointerButton(&state, 66, 160,
            M11_DM1_MOUSE_MASK_LEFT) ==
            M11_GAME_INPUT_IGNORED ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != 0x280bu) {
        fputs("FAIL: PC34 HoC C080 did not collect rendered source WATER\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_HandlePointerButton(&state, 54, 14,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        !state.inventoryPanelActive ||
        M11_GameView_HandlePointerButton(&state, 74, 74,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_1] !=
            0x280bu ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != THING_NONE) {
        fputs("FAIL: PC34 HoC WATER did not transfer from hand to backpack\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    /* Once WATER is stored, redraw the exposed scroll and click its source
     * pile box. C071 is a held control: inspect the text while pressed,
     * then verify F0353-equivalent release cleanup. */
    if (M11_GameView_HandlePointerButton(&state, 54, 14,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        state.inventoryPanelActive) {
        fputs("FAIL: PC34 HoC inventory did not close after WATER transfer\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1FloorItemHostPresentationReceipt(&floorReceipt);
    if (!floorReceipt.valid || !floorReceipt.floorItemLane ||
        !floorReceipt.destinationPixelsChanged ||
        M11_GameView_HandlePointerButton(&state, 150, 160,
            M11_DM1_MOUSE_MASK_LEFT) ==
            M11_GAME_INPUT_IGNORED ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing ||
        THING_GET_TYPE(thing) != THING_TYPE_SCROLL ||
        THING_GET_INDEX(thing) != 0u ||
        state.world.things->scrolls[0].textStringThingIndex != 33u ||
        M11_GameView_HandlePointerButton(&state, 54, 14,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        !state.inventoryPanelActive) {
        fputs("FAIL: authentic HoC scroll did not reach inventory\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(beforeEyeFrame, 0, sizeof(beforeEyeFrame));
    M11_GameView_Draw(&state, beforeEyeFrame, 320, 200);
    if (!state.v1FoodWaterPanelActive) {
        fputs("FAIL: empty action hand lost F0345 before Eye press\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_HandlePointerButton(&state, 20, 53,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        !state.v1EyePressActive || !state.v1ScrollPanelActive ||
        state.v1ScrollPanelThing != thing ||
        DM1_V1_M11Runtime_DecodeInventoryActionHandScrollTextPc34Compat(
            &state, panelText, (int)sizeof(panelText)) <= 0 ||
        F0509_DUNGEON_DecodeScrollText_Compat(state.world.things, 0,
            sourceText, (int)sizeof(sourceText)) <= 0 ||
        strcmp(panelText, sourceText) != 0) {
        fputs("FAIL: Eye press did not select and decode authentic scroll 0/text 33\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(heldEyeFrame, 0, sizeof(heldEyeFrame));
    M11_GameView_Draw(&state, heldEyeFrame, 320, 200);
    scrollPanel = M11_AssetLoader_Load(&state.assetLoader,
        (unsigned int)M11_GameView_GetV1OpenScrollPanelGraphicId());
    if (!M11_GameView_GetV1InventoryPanelZone(
            &panelX, &panelY, &panelW, &panelH) ||
        !scrollPanel || !scrollPanel->loaded || !scrollPanel->pixels ||
        scrollPanel->width != panelW || scrollPanel->height != panelH ||
        panelX < 0 || panelY < 0 || panelX + panelW > 320 ||
        panelY + 33 + panelH > 200) {
        fputs("FAIL: authentic C023 scroll panel material/zone missing\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    /* PANEL.C F0341 blits C023 at C101 (viewport y + 33), using red 8
     * as transparency. Text from the original M653 glyph bank then writes
     * inside that same C101 rectangle. Compare indexed source pixels around
     * the text, and require the text ink to change some C023 pixels. */
    for (int row = 0; row < panelH; ++row) {
        for (int col = 0; col < panelW; ++col) {
            unsigned char source = scrollPanel->pixels[row * panelW + col];
            unsigned char actual =
                heldEyeFrame[(panelY + 33 + row) * 320 + panelX + col];
            if (source == 8) continue;
            ++sourceOpaque;
            if (actual == source) ++sourceMatches;
            else if (actual == 0 && row >= 15 && row < panelH - 10)
                ++textPixels;
        }
    }
    if (panel_pixel_delta(beforeEyeFrame, heldEyeFrame,
                          panelX, panelY + 33, panelW, panelH) < 1000 ||
        sourceOpaque < 1000 || sourceMatches < sourceOpaque * 3 / 4 ||
        textPixels < 10) {
        fprintf(stderr, "FAIL: held Eye C101 pixels: source %d/%d, ink %d\n",
                sourceMatches, sourceOpaque, textPixels);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_HandlePointerButtonRelease(&state, 20, 53,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
        state.v1EyePressActive || state.v1ScrollPanelActive ||
        !state.v1FoodWaterPanelActive ||
        state.inventoryPanelActive == 0 ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing) {
        fputs("FAIL: Eye release did not restore inventory and retain scroll\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (panel_pixel_delta(beforeEyeFrame, framebuffer,
                          panelX, panelY + 33, panelW, panelH) != 0) {
        fprintf(stderr, "FAIL: Eye release changed %d C101 pixels\n",
                panel_pixel_delta(beforeEyeFrame, framebuffer,
                                  panelX, panelY + 33, panelW, panelH));
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("PASS: authentic PC34 HoC C080 WATER/scroll, held C023 Eye frame, F0345 release panel");
    return 0;
}
