#ifndef DM1_LEGACY_SCROLL_REAL_CHECK_H
#define DM1_LEGACY_SCROLL_REAL_CHECK_H

#include "asset_loader_m11.h"
#include "dm1_v1_text_message_pc34_compat.h"
#include "font_m11.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_atari_st_graphics_dat.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "dm1_v1_throw_shoot_pc34_compat.h"

static int check_legacy_object_transfers(M11_GameViewState *state)
{
    /* DATA.C G0038:320-350 and CHAMPION.C F0302:662-707.
     * Source slot masks are independent of the runtime admission helper. */
    static const unsigned int slotMasks[30] = {
        0xffff,0xffff,2,8,16,32,256,128,128,128,4,256,64,
        0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
        0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
    };
    static const int slots[30] = {19,20,0,2,3,4,6,9,8,10,1,5,7,
        11,12,13,14,15,16,17,18,21,22,23,24,25,26,27,28,29};
    int checked = 0;
    int swaps = 0;
    unsigned char objectData[65535];
    size_t objectBytes = 0, objectOffset = 0;
    int matches = 0;
    if (state->assetLoader.atariStDm1) {
        DM1_V1_AtariStGraphicsDat dat;
        int bytes;
        if (!dm1_v1_atari_st_graphics_open(state->assetLoader.atariStData,
                (size_t)state->assetLoader.atariStDataSize, &dat)) return 0;
        bytes = dm1_v1_atari_st_graphics_read(&dat, 559, objectData, sizeof(objectData));
        if (bytes <= 0) return 0;
        objectBytes = (size_t)bytes;
    } else if (!dm1_v1_legacy_graphics_read_raw(state->assetLoader.legacyData,
            (size_t)state->assetLoader.legacyDataSize, 1, 559,
            objectData, sizeof(objectData), &objectBytes)) return 0;
    /* DUNGEON.C G0237:79-84: identify the unique Scroll/Chest/Mon Potion
     * record prefix, then compare masks from the selected original media.
     * DEFS.H:1683-1688: six-byte OBJECT_INFO has a 16-bit Type,
     * two byte fields, then a 16-bit AllowedSlots mask. */
    for (size_t offset = 0; offset + 180 * 6 <= objectBytes; ++offset) {
        const unsigned char *p = objectData + offset;
        if (p[0] == 0 && p[1] == 30 && p[2] == 1 && p[3] == 0 && p[4] == 5 && p[5] == 0 &&
            p[6] == 0 && p[7] == 144 && p[8] == 0 && p[9] == 0 && p[10] == 2 && p[11] == 0 &&
            p[12] == 0 && p[13] == 148 && p[14] == 67 && p[15] == 0 && p[16] == 5 && p[17] == 0) {
            objectOffset = offset;
            ++matches;
        }
    }
    if (matches != 1) {
        fprintf(stderr, "FAIL: original G0237 location ambiguous/missing (%d, bytes=%zu)\n", matches, objectBytes);
        return 0;
    }
    for (int type = THING_TYPE_WEAPON; type <= THING_TYPE_JUNK; ++type) {
        for (int i = 0; i < state->world.things->thingCounts[type]; ++i) {
            unsigned short thing = (unsigned short)((type << 10) | i);
            unsigned int mask;
            const unsigned char *raw = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, thing);
            if (!raw) return 0;
            if (raw[0] == 0xff && raw[1] == 0xff) continue;
            {
                /* F0141 DUNGEON.C:1136-1163. Runtime dungeon records
                 * are normalized little-endian; original G0237 remains
                 * big-endian. Decode independently of the subtype/index
                 * helpers whose result this oracle is checking. */
                unsigned int word = raw[2] | ((unsigned int)raw[3] << 8);
                int info = type == THING_TYPE_SCROLL ? 0 :
                    type == THING_TYPE_CONTAINER ? 1 + ((raw[4] >> 1) & 3) :
                    type == THING_TYPE_POTION ? 2 + ((word >> 8) & 127) :
                    type == THING_TYPE_WEAPON ? 23 + (word & 127) :
                    type == THING_TYPE_ARMOUR ? 69 + (word & 127) : 127 + (word & 127);
                const unsigned char *entry;
                if (info < 0 || info >= 180) return 0;
                if (dm1_v1_dungeon_get_object_info_index_pc34(state->world.things, thing) != info) {
                    fprintf(stderr, "FAIL: original F0141 index thing=%04x expected=%d\n", thing, info);
                    return 0;
                }
                entry = objectData + objectOffset + (size_t)info * 6;
                mask = ((unsigned int)entry[4] << 8) | entry[5];
                if (dm1_v1_dungeon_get_object_allowed_slots_pc34(state->world.things, thing) != mask) {
                    fprintf(stderr, "FAIL: original G0237 mask thing=%04x info=%d expected=%04x\n", thing, info, mask);
                    return 0;
                }
            }
            for (int mode = 0; mode < 2; ++mode) for (int slot = 0; slot < 30; ++slot) {
                int x, y, w, h;
                int admitted = (mask & slotMasks[slot]) != 0;
                state->presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED : M12_PRESENTATION_V1_ORIGINAL;
                state->inventoryPanelActive = 1;
                if (state->world.party.champions[0].inventory[slots[slot]] != THING_NONE ||
                    !M11_GameView_GetV1InventorySourceSlotBoxZone(slot + 8, &x, &y, &w, &h) ||
                    !DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing)) return 0;
                for (int step = 0; step < (admitted ? 2 : 1); ++step) {
                    unsigned short hand = (step || !admitted) ? thing : THING_NONE;
                    unsigned short resident = (step || !admitted) ? THING_NONE : thing;
                    (void)M11_GameView_HandlePointer(state, x+w/2, 33+y+h/2, 1);
                    /* Opt-in diagnostic: initialize a consistent one-item
                     * load, then use normal pickup/drop input. Direct hand
                     * setup alone is not evidence of correct initial load. */
                    if (getenv("FIRESTAFF_VERIFY_LEGACY_LOAD") &&
                        type == THING_TYPE_WEAPON && slot == 0 && mode == 0 && step == 0) {
                        int weight;
                        if (!dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, thing, &weight)) return 0;
                        if (weight <= 0) return 0;
                        state->world.party.champions[0].load = (unsigned short)weight;
                        (void)M11_GameView_HandlePointerButtonRelease(state, x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        (void)M11_GameView_HandlePointer(state, x+w/2, 33+y+h/2, 1);
                        (void)M11_GameView_HandlePointerButtonRelease(state, x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != thing ||
                            state->world.party.champions[0].inventory[slots[slot]] != THING_NONE ||
                            state->world.party.champions[0].load != weight) return 0;
                        state->inventoryPanelActive = 0;
                        (void)M11_GameView_HandlePointer(state, 64, 158, 1);
                        if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != THING_NONE) return 0;
                        fprintf(stderr, "legacy floor-drop load thing=%04x before=%d after=%u expected=0\n",
                            thing, weight, state->world.party.champions[0].load);
                        if (state->world.party.champions[0].load != 0) return 0;
                        (void)M11_GameView_HandlePointerButtonRelease(state, 64, 158, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        return state->world.party.champions[0].load == 0 &&
                            DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) == THING_NONE;
                    }
                    for (int release = 0; release < 2; ++release) {
                        if (release) (void)M11_GameView_HandlePointerButtonRelease(state,
                            x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                            state->world.party.champions[0].inventory[slots[slot]] != resident) {
                            fprintf(stderr, "FAIL: legacy object %04x mode %d slot %d step %d release %d\n",
                                thing, mode, slots[slot], step, release);
                            return 0;
                        }
                    }
                    ++checked;
                }
            }
            {
                unsigned short other = THING_NONE;
                int x, y, w, h;
                /* CHAMPION.C F0302:697-707 exchanges both owners. Use a
                 * distinct allocated original weapon, never a fabricated
                 * resident or the same Thing on both sides of the swap. */
                for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_WEAPON]; ++r) {
                    unsigned short candidate = (unsigned short)((THING_TYPE_WEAPON << 10) | r);
                    const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                    if (candidate != thing && bytes && !(bytes[0] == 0xff && bytes[1] == 0xff)) {
                        other = candidate;
                        break;
                    }
                }
                if (other == THING_NONE ||
                    !M11_GameView_GetV1InventorySourceSlotBoxZone(9, &x, &y, &w, &h)) return 0;
                for (int mode = 0; mode < 2; ++mode) {
                    state->presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED : M12_PRESENTATION_V1_ORIGINAL;
                    state->world.party.champions[0].inventory[20] = other;
                    if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing)) return 0;
                    for (int step = 0; step < 2; ++step) {
                        unsigned short hand = step ? thing : other;
                        unsigned short resident = step ? other : thing;
                        (void)M11_GameView_HandlePointer(state, x+w/2, 33+y+h/2, 1);
                        for (int release = 0; release < 2; ++release) {
                            if (release) (void)M11_GameView_HandlePointerButtonRelease(state,
                                x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                                state->world.party.champions[0].inventory[20] != resident) {
                                fprintf(stderr, "FAIL: legacy occupied action hand %04x/%04x mode %d step %d release %d\n",
                                    thing, other, mode, step, release);
                                return 0;
                            }
                        }
                        ++swaps;
                    }
                    state->world.party.champions[0].inventory[20] = THING_NONE;
                }
            }
        }
    }
    printf("PASS: %d original legacy object placement/pickup/rejection checks and releases\n", checked);
    printf("PASS: %d original legacy occupied action-hand swaps and releases\n", swaps);
    return checked > 0 && swaps > 0;
}

/* PANEL.C F0340/F0341: original Atari/Amiga center X=162,
 * baseline=92-floor(7*n/2). TEXT.C F0040:413,714 subtracts four
 * before copying the original font. Real scrolls only; placement is in RAM. */
static int check_legacy_scroll_raster(M11_GameViewState *state)
{
    int checked = 0;
    if (!state->world.things || !M11_Font_IsLoaded(&state->originalFont)) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&state->world.party.champions[0]);
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (int i = 0; i < state->world.things->thingCounts[THING_TYPE_SCROLL]; ++i) {
        unsigned short thing = (unsigned short)((THING_TYPE_SCROLL << 10) | i);
        for (int mode = 0; mode < 2; ++mode) {
            unsigned char frame[320 * 200], expected[320 * 200];
            char text[4096];
            DM1_V1_ScrollLayout layout;
            const M11_AssetSlot *panel;
            int ink = 0;
            state->presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED : M12_PRESENTATION_V1_ORIGINAL;
            state->inventoryPanelActive = 1;
            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing)) return 0;
            {
                int sx, sy, sw, sh;
                /* CHAMPION.C F0302:662-707: dropping in the action hand
                 * and picking back up are distinct press transactions.
                 * Releases must not repeat either ownership exchange. */
                if (!M11_GameView_GetV1InventorySourceSlotBoxZone(9, &sx, &sy, &sw, &sh)) return 0;
                for (int step = 0; step < 2; ++step) {
                    (void)M11_GameView_HandlePointer(state, sx + 1, 33 + sy + 1, 1);
                    if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != (step ? thing : THING_NONE) ||
                        state->world.party.champions[0].inventory[20] != (step ? THING_NONE : thing)) return 0;
                    (void)M11_GameView_HandlePointerButtonRelease(state, sx + 1, 33 + sy + 1, DM1_V1_MOUSE_MASK_LEFT_PC34);
                    if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != (step ? thing : THING_NONE) ||
                        state->world.party.champions[0].inventory[20] != (step ? THING_NONE : thing)) {
                        fprintf(stderr, "FAIL: legacy scroll %d action-hand transfer %d\n", i, step);
                        return 0;
                    }
                }
            }
            if (M11_GameView_HandlePointer(state, 20, 54, 1) != M11_GAME_INPUT_REDRAW ||
                !state->v1ScrollPanelActive || state->v1ScrollPanelThing != thing ||
                !DM1_V1_M11Runtime_DecodeInventoryActionHandScrollTextPc34Compat(state, text, sizeof(text))) return 0;
            memset(frame, 0, sizeof(frame));
            M11_GameView_Draw(state, frame, 320, 200);
            {
                const M11_AssetSlot *slot = M11_AssetLoader_Load(&state->assetLoader, 33);
                int borderPixels = 0;
                if (!slot || !slot->loaded || !slot->pixels ||
                    slot->width != 32 || slot->height != 18) return 0;
                /* CHAMDRAW.C F0291:558-559,655-658 uses the first 18
                 * columns of each 32-pixel row and C12 transparency.
                 * Check all 30 borders, outside the 16x16 object icons.
                 * Coordinates still share the source-slot resolver. */
                for (int box = 8; box <= 37; ++box) {
                    int bx, by, bw, bh;
                    if (!M11_GameView_GetV1InventorySourceSlotBoxZone(
                            box, &bx, &by, &bw, &bh)) return 0;
                    for (int y = 0; y < 18; ++y) for (int x = 0; x < 18; ++x) {
                        unsigned char color = slot->pixels[y * 32 + x];
                        if ((x > 0 && x < 17 && y > 0 && y < 17) || color == 12) continue;
                        if (frame[(33 + by - 1 + y) * 320 + bx - 1 + x] != color) {
                            fprintf(stderr, "FAIL: original legacy slot border %d (%d,%d)\n", box, x, y);
                            return 0;
                        }
                        ++borderPixels;
                    }
                }
                if (!borderPixels) return 0;
            }
            panel = M11_AssetLoader_Load(&state->assetLoader, 23);
            if (!panel || !panel->loaded || !panel->pixels || panel->width != 144 || panel->height != 73) return 0;
            memcpy(expected, frame, sizeof(expected));
            for (int y = 0; y < 73; ++y) for (int x = 0; x < 144; ++x)
                if (panel->pixels[y * 144 + x] != 8)
                    expected[(85 + y) * 320 + 80 + x] = panel->pixels[y * 144 + x];
            dm1_v1_text_scroll_measure_layout(text, &layout);
            for (int line = 0; line < layout.storedLineCount; ++line) {
                int length = (int)strlen(layout.lines[line]);
                for (int c = 0; c < length; ++c) {
                    unsigned char ch = (unsigned char)layout.lines[line][c];
                    if (ch >= 'A' && ch <= 'Z') ch -= 64;
                    else if (ch >= '{') ch -= 96;
                    for (int y = 0; y < 6; ++y) for (int x = 0; x < 6; ++x) {
                        int dx = 162 - 3 * length + 6 * c + x;
                        int dy = 33 + 92 - (7 * layout.lineCount / 2) - 4 + 7 * line + y;
                        int bit = M11_Font_GetPixel(&state->originalFont, ch * 8 + 3 + x, y);
                        if (dx >= 0 && dx < 320 && dy >= 0 && dy < 200) expected[dy * 320 + dx] = bit ? 0 : 15;
                        ink += bit != 0;
                    }
                }
            }
            if (!ink) return 0;
            for (int y = 0; y < 73; ++y) for (int x = 0; x < 144; ++x)
                if (frame[(85 + y) * 320 + 80 + x] != expected[(85 + y) * 320 + 80 + x]) {
                    fprintf(stderr, "FAIL: original legacy scroll %d mode %d pixel (%d,%d) actual=%u expected=%u debug=%d candidate=%d assets=%d\n", i, mode, 80+x, 85+y, frame[(85+y)*320+80+x], expected[(85+y)*320+80+x], state->showDebugHUD, state->candidateMirrorPanelActive, state->assetsAvailable);
                    return 0;
                }
            (void)M11_GameView_HandlePointer(state, 20, 54, 0);
            ++checked;
        }
    }
    printf("PASS: %d original legacy scroll/mode raster checks\n", checked);
    return checked > 0 && check_legacy_object_transfers(state);
}
#endif
