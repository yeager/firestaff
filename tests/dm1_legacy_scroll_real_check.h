#ifndef DM1_LEGACY_SCROLL_REAL_CHECK_H
#define DM1_LEGACY_SCROLL_REAL_CHECK_H

#include "asset_loader_m11.h"
#include "dm1_v1_sound_pc34_compat.h"
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
                        int weight, otherWeight;
                        unsigned short other = THING_NONE;
                        /* CHAMPION.C F0297:264/F0298:293 charges the held
                         * object only to the leader. A second champion's
                         * independently carried original object must survive. */
                        for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_WEAPON]; ++r) {
                            unsigned short candidate = (unsigned short)((THING_TYPE_WEAPON << 10) | r);
                            const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                            if (candidate != thing && bytes && !(bytes[0] == 0xff && bytes[1] == 0xff)) {
                                other = candidate;
                                break;
                            }
                        }
                        if (other == THING_NONE ||
                            !dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, other, &otherWeight) ||
                            otherWeight <= 0) return 0;
                        F0600_CHAMPION_InitEmpty_Compat(&state->world.party.champions[1]);
                        state->world.party.championCount = 2;
                        state->world.party.champions[1].present = 1;
                        state->world.party.champions[1].hp.current = 100;
                        state->world.party.champions[1].hp.maximum = 100;
                        state->world.party.champions[1].inventory[19] = other;
                        {
                            char expected[4096], actual[4096];
                            state->dm1InventoryChampionOrdinal = 2;
                            for (int scroll = 0; scroll < state->world.things->thingCounts[THING_TYPE_SCROLL]; ++scroll) {
                                state->world.party.champions[1].inventory[20] =
                                    (unsigned short)((THING_TYPE_SCROLL << 10) | scroll);
                                if (F0509_DUNGEON_DecodeScrollText_Compat(state->world.things,
                                        scroll, expected, sizeof(expected)) < 0 ||
                                    !DM1_V1_M11Runtime_DecodeInventoryActionHandScrollTextPc34Compat(
                                        state, actual, sizeof(actual)) || strcmp(expected, actual) ||
                                    state->world.party.activeChampionIndex != 0) return 0;
                            }
                            state->world.party.champions[1].inventory[20] = THING_NONE;
                            state->dm1InventoryChampionOrdinal = 0;
                        }
                        state->world.party.champions[1].load = (unsigned short)otherWeight;
                        /* Exercise the explicit owner accessor before input
                         * migration: only champion 1 has an action-hand item. */
                        state->world.party.champions[1].inventory[19] = THING_NONE;
                        state->world.party.champions[1].inventory[20] = other;
                        state->dm1InventoryChampionOrdinal = 2;
                        if (DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat(state, 20) < 0 ||
                            state->world.party.activeChampionIndex != 0) return 0;
                        {
                            unsigned char selected[320 * 200], baseline[320 * 200];
                            memset(selected, 0, sizeof(selected));
                            memset(baseline, 0, sizeof(baseline));
                            state->championDamageTimer[0] = 0;
                            state->championDamageTimer[1] = 5;
                            M11_GameView_Draw(state, selected, 320, 200);
                            state->world.party.activeChampionIndex = 1;
                            state->dm1InventoryChampionOrdinal = 0;
                            M11_GameView_Draw(state, baseline, 320, 200);
                            state->world.party.activeChampionIndex = 0;
                            state->dm1InventoryChampionOrdinal = 2;
                            /* Same inventory owner, different leader. Compare
                             * C017 viewport only, excluding leader HUD markers. */
                            for (int row = 33; row < 169; ++row) {
                                if (memcmp(selected + row * 320, baseline + row * 320, 224)) {
                                    fprintf(stderr, "FAIL: inventory owner raster row=%d\n", row);
                                    return 0;
                                }
                            }
                            state->championDamageTimer[1] = 0;
                        }
                        state->dm1InventoryChampionOrdinal = CHAMPION_MAX_PARTY + 1;
                        if (DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat(state, 20) != -1) return 0;
                        state->dm1InventoryChampionOrdinal = 0;
                        if (DM1_V1_M11Runtime_GetInventorySlotIconIndexPc34Compat(state, 20) != -1) return 0;
                        state->world.party.champions[1].inventory[20] = THING_NONE;
                        state->world.party.champions[1].inventory[19] = other;
                        {
                            unsigned short chest = THING_NONE;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_CONTAINER]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_CONTAINER << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff)) {
                                    chest = candidate;
                                    break;
                                }
                            }
                            if (chest == THING_NONE) return 0;
                            state->dm1InventoryChampionOrdinal = 2;
                            state->world.party.champions[1].inventory[20] = chest;
                            if (!DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(state) ||
                                DM1_V1_M11Runtime_GetOpenChestThingPc34Compat(state) != chest ||
                                state->world.party.activeChampionIndex != 0) return 0;
                            /* Opening only: discard the read-only cache without
                             * invoking close-time chain rewriting in this probe. */
                            state->v1OpenChestThing = THING_NONE;
                            state->v1OpenChestSlotsValid = 0;
                            state->world.party.champions[1].inventory[20] = THING_NONE;
                            state->dm1InventoryChampionOrdinal = 0;
                        }
                        (void)M11_GameView_HandlePointerButtonRelease(state, x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        state->dm1InventoryChampionOrdinal = 2;
                        (void)M11_GameView_HandlePointer(state, 20, 54, 1);
                        if (!state->v1ChampionStatsPanelActive || state->world.party.activeChampionIndex != 0) return 0;
                        (void)M11_GameView_HandlePointerButtonRelease(state, 20, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        if (state->v1ChampionStatsPanelActive) return 0;
                        state->dm1InventoryChampionOrdinal = CHAMPION_MAX_PARTY + 1;
                        (void)M11_GameView_HandlePointer(state, 20, 54, 1);
                        if (state->v1ChampionStatsPanelActive) return 0;
                        (void)M11_GameView_HandlePointerButtonRelease(state, 20, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        state->dm1InventoryChampionOrdinal = 0;
                        if (!dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, thing, &weight)) return 0;
                        if (weight <= 0) return 0;
                        state->world.party.champions[0].load = (unsigned short)weight;
                        (void)M11_GameView_HandlePointerButtonRelease(state, x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        (void)M11_GameView_HandlePointer(state, x+w/2, 33+y+h/2, 1);
                        (void)M11_GameView_HandlePointerButtonRelease(state, x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != thing ||
                            state->world.party.champions[0].inventory[slots[slot]] != THING_NONE ||
                            state->world.party.champions[0].load != weight) return 0;
                        /* Explicit-owner migration gate: exchange the held
                         * original object with champion 1's original weapon. */
                        state->dm1InventoryChampionOrdinal = 2;
                        for (int exchange = 0; exchange < 2; ++exchange) {
                            unsigned short expectedHand = exchange ? thing : other;
                            unsigned short expectedSlot = exchange ? other : thing;
                            for (int release = 0; release < 2; ++release) {
                                if (release) (void)M11_GameView_HandlePointerButtonRelease(state,
                                    x+w/2, 33+y+h/2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                                else (void)M11_GameView_HandlePointer(state, x+w/2, 33+y+h/2, 1);
                                if (state->world.party.activeChampionIndex != 0 ||
                                    state->world.party.champions[0].inventory[19] != THING_NONE ||
                                    state->world.party.champions[1].inventory[19] != expectedSlot ||
                                    DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != expectedHand ||
                                    state->world.party.champions[0].load != (exchange ? weight : otherWeight) ||
                                    state->world.party.champions[1].load != (exchange ? otherWeight : weight)) return 0;
                            }
                        }
                        state->dm1InventoryChampionOrdinal = 0;
                        state->inventoryPanelActive = 0;
                        {
                            /* PANEL.C:2363 changes G0423, not G0411. */
                            static const int owners[] = {1,0,0,1,1};
                            static const int ordinals[] = {2,1,0,2,0};
                            for (int visit = 0; visit < 5; ++visit) {
                                int px = owners[visit] * 69 + 45;
                                for (int release = 0; release < 2; ++release) {
                                    if (release) (void)M11_GameView_HandlePointerButtonRelease(state,
                                        px, 10, DM1_V1_MOUSE_MASK_LEFT_PC34);
                                    else (void)M11_GameView_HandlePointer(state, px, 10, 1);
                                    if (state->inventoryPanelActive != (ordinals[visit] != 0) ||
                                        state->dm1InventoryChampionOrdinal != ordinals[visit] ||
                                        state->world.party.activeChampionIndex != 0 ||
                                        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != thing) return 0;
                                }
                            }
                        }
                        /* CLIKCHAM.C F0368:55-76 transfers only held weight
                         * when the leader changes, then transfers it back. */
                        for (int pointer = 0; pointer < 2; ++pointer)
                        for (int leader = 1; leader >= 0; --leader) {
                            /* CLIKCHAM.C F0368:67 aligns the new leader. */
                            state->world.party.champions[leader].direction =
                                (unsigned char)((state->world.party.direction + 1) & 3);
                            if (pointer) {
                                (void)M11_GameView_HandlePointer(state, leader * 69 + 10, 2, 1);
                                if (state->inventoryPanelActive ||
                                    state->world.party.activeChampionIndex != leader ||
                                    state->world.party.champions[0].load != (leader ? 0 : weight) ||
                                    state->world.party.champions[1].load != otherWeight + (leader ? weight : 0)) return 0;
                                (void)M11_GameView_HandlePointerButtonRelease(state,
                                    leader * 69 + 10, 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            } else {
                                (void)M11_GameView_HandleInput(state, M12_MENU_INPUT_CYCLE_CHAMPION);
                            }
                            if (state->world.party.champions[leader].direction != state->world.party.direction ||
                                state->world.party.activeChampionIndex != leader ||
                                state->world.party.champions[0].load != (leader ? 0 : weight) ||
                                state->world.party.champions[1].load != otherWeight + (leader ? weight : 0)) {
                                fprintf(stderr, "FAIL: leader switch %d pointer=%d loads=%u/%u expected=%d/%d\n",
                                    leader, pointer, state->world.party.champions[0].load,
                                    state->world.party.champions[1].load,
                                    leader ? 0 : weight, otherWeight + (leader ? weight : 0));
                                return 0;
                            }
                        }
                        /* CLIKCHAM.C F0368:55 rejects a zero-health leader.
                         * Controlled health setup does not fabricate media. */
                        state->world.party.champions[1].hp.current = 0;
                        for (int pointer = 0; pointer < 2; ++pointer) {
                            if (pointer) {
                                (void)M11_GameView_HandlePointer(state, 79, 2, 1);
                                (void)M11_GameView_HandlePointerButtonRelease(state, 79, 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            } else {
                                (void)M11_GameView_HandleInput(state, M12_MENU_INPUT_CYCLE_CHAMPION);
                            }
                            if (state->world.party.activeChampionIndex != 0 ||
                                state->world.party.champions[0].load != weight ||
                                state->world.party.champions[1].load != otherWeight) {
                                fprintf(stderr, "FAIL: dead leader admitted pointer=%d\n", pointer);
                                return 0;
                            }
                        }
                        state->world.party.champions[1].hp.current = 100;
                        (void)M11_GameView_HandlePointer(state, 64, 158, 1);
                        if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != THING_NONE) return 0;
                        fprintf(stderr, "legacy floor-drop load thing=%04x before=%d after=%u expected=0\n",
                            thing, weight, state->world.party.champions[0].load);
                        if (state->world.party.champions[0].load != 0) return 0;
                        (void)M11_GameView_HandlePointerButtonRelease(state, 64, 158, DM1_V1_MOUSE_MASK_LEFT_PC34);
                        {
                            unsigned short food = THING_NONE;
                            int amount = 0;
                            static const int amounts[8] = {500,600,650,820,550,350,990,1400};
                            /* DUNGEON.C G0242; original allocated food only. */
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_JUNK]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_JUNK << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff) &&
                                    (bytes[2] & 127) >= 29 && (bytes[2] & 127) <= 36) {
                                    food = candidate;
                                    amount = amounts[(bytes[2] & 127) - 29];
                                    break;
                                }
                            }
                            if (food == THING_NONE) return 0;
                            state->audioState.lastSoundIndex = -1;
                            state->inventoryPanelActive = 1;
                            state->dm1InventoryChampionOrdinal = 2;
                            state->world.party.champions[0].food = 0;
                            state->world.party.champions[1].food = 0;
                            if (getenv("FIRESTAFF_VERIFY_LIVING_CASTER")) {
                                unsigned short savedRight = state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT];
                                state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = food;
                                state->world.party.activeChampionIndex = 1;
                                if (!M11_GameView_UseItem(state) ||
                                    state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] != THING_NONE) { fprintf(stderr, "FAIL: alternate food consume/slot\n"); return 0; }
                                const unsigned char *eaten = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, food);
                                if (!eaten || eaten[0] != 255 || eaten[1] != 255) { fprintf(stderr, "FAIL: alternate food raw release\n"); return 0; }
                                state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = savedRight;
                                state->world.party.champions[1].load = otherWeight;
                                state->world.party.activeChampionIndex = 0;
                            } else {
                                if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, food)) return 0;
                                (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                                (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            }
                            if (state->world.party.champions[0].food != 0 ||
                                state->audioState.lastSoundIndex != DM1_SND_SWALLOW ||
                                state->world.party.champions[1].food != amount ||
                                state->world.party.activeChampionIndex != 0) return 0;
                        }
                        if (!(state->world.party.champions[1].inventory[19] == other &&
                            state->world.party.champions[1].load == otherWeight &&
                            state->world.party.champions[0].load == 0 &&
                            DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) == THING_NONE)) return 0;
                        /* Drive the real death handler after a controlled
                         * zero-health transition of the inventory owner. */
                        {
                            unsigned short water = THING_NONE;
                            int charges = 0;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_JUNK]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_JUNK << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff) &&
                                    (bytes[2] & 127) == 1 && (bytes[3] >> 6) > 0) {
                                    water = candidate;
                                    charges = bytes[3] >> 6;
                                    break;
                                }
                            }
                            if (water == THING_NONE) { fprintf(stderr, "FAIL: no original charged waterskin\n"); return 0; }
                            if (getenv("FIRESTAFF_VERIFY_LIVING_CASTER")) {
                                unsigned short savedRight = state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT];
                                state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = water;
                                state->world.party.activeChampionIndex = 1;
                                for (int drink = 0; drink <= charges; ++drink) {
                                    int before = drink == charges ? -1024 : (drink & 1) ? -1024 : 1800;
                                    int expected = drink == charges ? before : before + 800;
                                    if (expected > 2048) expected = 2048;
                                    state->world.party.champions[0].water = 0;
                                    state->world.party.champions[1].water = (short)before;
                                    state->audioState.lastSoundIndex = -1;
                                    int used = M11_GameView_UseItem(state);
                                    const unsigned char *raw = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, water);
                                    int remaining = charges > drink ? charges - drink - 1 : 0;
                                    if (used != (drink < charges) || !raw || (raw[3] >> 6) != remaining ||
                                        state->audioState.lastSoundIndex != (drink < charges ? DM1_SND_SWALLOW : -1) ||
                                        state->world.party.champions[1].water != expected ||
                                        state->world.party.champions[0].water != 0 ||
                                        state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] != water) {
                                        fprintf(stderr, "FAIL: alternate original waterskin\n"); return 0;
                                    }
                                }
                                state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = savedRight;
                                state->world.party.champions[1].load = otherWeight;
                                state->world.party.activeChampionIndex = 0;
                                /* The following mouth check now verifies the same empty skin. */
                                charges = 0;
                            }
                            state->world.party.champions[0].water = 0;
                            state->world.party.champions[1].water = 1800;
                            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, water)) return 0;
                            /* Direct fixture placement does not run the pickup/load handler. */
                            int placedWeight;
                            if (!dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, water, &placedWeight)) return 0;
                            state->world.party.champions[0].load = placedWeight;
                            /* PANEL.C F0346:1832-1838 caps water at 2048,
                             * decrements charges and retains the leader hand. */
                            for (int drink = 0; drink <= charges; ++drink) {
                                int heldWeight;
                                int waterBefore = getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? -1024 : 1800;
                                int expectedWater = drink < charges ? waterBefore + 800 : waterBefore;
                                if (expectedWater > 2048) expectedWater = 2048;
                                /* An empty skin must not grant water even
                                 * when the recipient is below the cap. */
                                if (drink == charges) waterBefore = expectedWater = -1024;
                                state->world.party.champions[1].water = (short)waterBefore;
                                (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                                (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, water);
                                int remaining = charges - drink - 1;
                                if (remaining < 0) remaining = 0;
                                if (!bytes || (bytes[3] >> 6) != remaining ||
                                    state->world.party.champions[0].water != 0 ||
                                    state->world.party.champions[1].water != expectedWater ||
                                    DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != water ||
                                    !dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, water, &heldWeight) ||
                                    state->world.party.champions[0].load != heldWeight ||
                                    state->world.party.champions[1].load != otherWeight) {
                                    fprintf(stderr, "FAIL: cross-owner waterskin charges/water/load\n");
                                    return 0;
                                }
                            }
                            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                        }
                        {
                            unsigned short flask = THING_NONE;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_POTION]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_POTION << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff) &&
                                    (bytes[3] & 127) == 15) { flask = candidate; break; }
                            }
                            if (flask == THING_NONE) { fprintf(stderr, "FAIL: no original water flask\n"); return 0; }
                            state->world.party.champions[0].water = 0;
                            state->world.party.champions[1].water = -1024;
                            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, flask)) return 0;
                            (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                            (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, flask);
                            int flaskWeight;
                            /* PANEL.C F0346:1912-1916: +1600, same Thing,
                             * transformed into C20 empty flask. */
                            if (!bytes || (bytes[3] & 127) != 20 ||
                                state->world.party.champions[1].water != 576 ||
                                state->world.party.champions[0].water != 0 ||
                                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != flask ||
                                !dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, flask, &flaskWeight) ||
                                state->world.party.champions[0].load != flaskWeight ||
                                state->world.party.champions[1].load != otherWeight) {
                                fprintf(stderr, "FAIL: cross-owner water flask\n"); return 0;
                            }
                            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                        }
                        {
                            unsigned short antidote = THING_NONE;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_POTION]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_POTION << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff) &&
                                    (bytes[3] & 127) == 10) { antidote = candidate; break; }
                            }
                            if (antidote == THING_NONE) { fprintf(stderr, "FAIL: no original antivenin\n"); return 0; }
                            struct SkillState_Compat skillsBefore[LIFECYCLE_SKILL_COUNT];
                            memcpy(skillsBefore, state->world.lifecycle.champions[1].skills20, sizeof(skillsBefore));
                            int markersBefore = state->audioState.playedMarkerCount;
                            state->audioState.lastSoundIndex = -1;
                            struct TimelineEvent_Compat poison = {0};
                            poison.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
                            poison.fireAtTick = state->world.gameTick + 100;
                            poison.aux0 = LIFECYCLE_STATUS_POISON;
                            poison.aux1 = 128;
                            poison.aux4 = 1;
                            if (!F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &poison)) return 0;
                            state->world.party.champions[1].poisonDose = 128;
                            state->world.lifecycle.champions[1].poisonEventCount = 1;
                            poison.aux4 = 0;
                            if (!F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &poison)) return 0;
                            state->world.party.champions[0].poisonDose = 128;
                            state->world.lifecycle.champions[0].poisonEventCount = 1;
                            if (getenv("FIRESTAFF_VERIFY_LIVING_CASTER")) {
                                unsigned short savedRight = state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT];
                                int savedLeader = state->world.party.activeChampionIndex;
                                state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = antidote;
                                state->world.party.activeChampionIndex = 1;
                                if (!M11_GameView_UseItem(state)) return 0;
                                state->world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = savedRight;
                                state->world.party.activeChampionIndex = savedLeader;
                            } else {
                                if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, antidote)) return 0;
                                (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                                (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            }
                            if (state->world.party.champions[1].poisonDose != 0 ||
                                state->world.lifecycle.champions[1].poisonEventCount != 0) {
                                fprintf(stderr, "FAIL: antivenin retains poison counter\n"); return 0;
                            }
                            if (memcmp(skillsBefore, state->world.lifecycle.champions[1].skills20, sizeof(skillsBefore))) {
                                fprintf(stderr, "FAIL: drinking antivenin awards skill experience\n"); return 0;
                            }
                            if (state->audioState.lastSoundIndex != DM1_SND_SWALLOW ||
                                state->audioState.playedMarkerCount != markersBefore) {
                                fprintf(stderr, "FAIL: antivenin original swallow transport\n"); return 0;
                            }
                            int leaderPoison = 0;
                            for (int e = 0; e < state->world.timeline.count; ++e) {
                                const struct TimelineEvent_Compat *pending = &state->world.timeline.events[e];
                                if (pending->kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
                                    pending->aux0 == LIFECYCLE_STATUS_POISON && pending->aux4 == 1) return 0;
                                if (!memcmp(pending, &poison, sizeof(poison))) ++leaderPoison;
                            }
                            if (leaderPoison != 1 || state->world.party.champions[0].poisonDose != 128 ||
                                state->world.lifecycle.champions[0].poisonEventCount != 1) return 0;
                            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                        }
                        {
                            unsigned short shield = THING_NONE;
                            int power = 0;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_POTION]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_POTION << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff) &&
                                    (bytes[3] & 127) == 12) { shield = candidate; power = bytes[2]; break; }
                            }
                            if (shield == THING_NONE) { fprintf(stderr, "FAIL: no original YA potion\n"); return 0; }
                            int baseline = getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? 51 : 0;
                            int delta = power / 25 + 8;
                            delta += delta >> 1;
                            if (baseline > 50) delta >>= 2;
                            state->world.lifecycle.champions[1].shieldDefense = (short)baseline;
                            state->world.lifecycle.champions[0].shieldDefense = 7;
                            state->world.magic.partyShieldDefense = 11;
                            state->world.lifecycle.status.partyShieldDefense = 11;
                            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, shield)) return 0;
                            (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                            (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            if (state->world.lifecycle.champions[1].shieldDefense != baseline + delta ||
                                state->world.lifecycle.champions[0].shieldDefense != 7 ||
                                state->world.magic.partyShieldDefense != 11 ||
                                state->world.lifecycle.status.partyShieldDefense != 11) {
                                fprintf(stderr, "FAIL: YA potion shield owner\n"); return 0;
                            }
                            int found = 0;
                            for (int e = 0; e < state->world.timeline.count; ++e) {
                                const struct TimelineEvent_Compat *pending = &state->world.timeline.events[e];
                                if (pending->kind != TIMELINE_EVENT_STATUS_TIMEOUT ||
                                    pending->aux0 != LIFECYCLE_STATUS_CHAMPION_SHIELD) continue;
                                if (pending->aux4 != 1 || pending->aux1 != delta ||
                                    pending->fireAtTick != state->world.gameTick + (unsigned int)(delta * delta)) return 0;
                                ++found;
                            }
                            if (found != 1) return 0;
                            {
                                unsigned int expires = state->world.gameTick + (unsigned int)(delta * delta);
                                while (state->world.gameTick <= expires) {
                                    unsigned int before = state->world.gameTick;
                                    (void)M11_GameView_AdvanceIdleTick(state);
                                    if (state->world.gameTick <= before) return 0;
                                    int expectedShield = before < expires ? baseline + delta : baseline;
                                    if (state->world.lifecycle.champions[1].shieldDefense != expectedShield ||
                                        state->world.lifecycle.champions[0].shieldDefense != 7 ||
                                        state->world.magic.partyShieldDefense != 11 ||
                                        state->world.lifecycle.status.partyShieldDefense != 11) {
                                        fprintf(stderr, "FAIL: YA expiry tick/owner\n"); return 0;
                                    }
                                }
                            }
                            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                        }
                        /* ReDMCSB PANEL.C F0348:1695-1741: S1.2+/Amiga gains
                           above 150 are quartered, incremented, then capped at 170.
                           Only party statistics are staged; potion power is original. */
                        for (int type = 6; type <= 9; ++type) {
                            const int attrs[] = {CHAMPION_ATTR_DEXTERITY, CHAMPION_ATTR_STRENGTH,
                                                 CHAMPION_ATTR_WISDOM, CHAMPION_ATTR_VITALITY};
                            unsigned short potion = THING_NONE;
                            int power = 0, attr = attrs[type - 6];
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_POTION]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_POTION << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (bytes && !(bytes[0] == 0xff && bytes[1] == 0xff) &&
                                    (bytes[3] & 127) == type) { potion = candidate; power = bytes[2]; break; }
                            }
                            if (potion == THING_NONE) { fprintf(stderr, "FAIL: missing original stat potion %d\n", type); return 0; }
                            int baseline = getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? 169 : 151;
                            int delta = type == 7 ? power / 35 + 5 : power / 25 + 8;
                            int expected = baseline + (delta >> 2) + 1;
                            if (expected > 170) expected = 170;
                            unsigned short leaderStat = state->world.party.champions[0].attributes[attr];
                            unsigned short oldStat = state->world.party.champions[1].attributes[attr];
                            state->world.party.champions[1].attributes[attr] = (unsigned short)baseline;
                            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, potion)) return 0;
                            (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                            (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, potion);
                            if (!bytes || (bytes[3] & 127) != 20 ||
                                state->world.party.champions[1].attributes[attr] != expected ||
                                state->world.party.champions[0].attributes[attr] != leaderStat ||
                                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != potion) {
                                fprintf(stderr, "FAIL: original stat potion owner/threshold %d\n", type); return 0;
                            }
                            state->world.party.champions[1].attributes[attr] = oldStat;
                            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                        }
                        unsigned short deathExtra = THING_NONE;
                        for (int restorative = 11; restorative <= 14; restorative += restorative == 11 ? 2 : 1)
                        for (int powerRune = 0; powerRune < 6; ++powerRune) {
                            unsigned short flask = THING_NONE;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_POTION]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_POTION << 10) | r);
                                const unsigned char *raw = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (raw && !(raw[0] == 255 && raw[1] == 255) && (raw[3] & 127) == 20) { flask = candidate; break; }
                            }
                            if (flask == THING_NONE) return 0;
                            struct ChampionState_Compat saved = state->world.party.champions[1];
                            struct ChampionLifecycleState_Compat savedLife = state->world.lifecycle.champions[1];
                            state->world.party.champions[1].inventory[CHAMPION_SLOT_ACTION_HAND] = flask;
                            state->world.party.champions[1].mana.current = 100;
                            state->world.party.champions[1].mana.maximum = 100;
                            for (int s = 0; s < LIFECYCLE_SKILL_COUNT; ++s)
                                state->world.lifecycle.champions[1].skills20[s].experience = 1000000;
                            state->dm1SpellCasting.magicCasterIndex = 1;
                            /* ReDMCSB MENU.C:68,71,74: Ya / Vi / Zo Bro Ra.
                             * No potion bytes/power are assigned by the fixture. */
                            if (!M11_GameView_OpenSpellPanel(state) ||
                                !M11_GameView_EnterRune(state, powerRune) ||
                                !M11_GameView_EnterRune(state, restorative == 11 ? 0 : restorative == 14 ? 1 : 5) ||
                                (restorative == 13 && (!M11_GameView_EnterRune(state, 4) || !M11_GameView_EnterRune(state, 4))) ||
                                !M11_GameView_CastSpell(state)) {
                                fprintf(stderr, "FAIL: original flask restorative cast\n"); return 0;
                            }
                            const unsigned char *raw = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, flask);
                            if (!raw || (raw[3] & 127) != restorative) { fprintf(stderr, "FAIL: restorative subtype %d\n", restorative); return 0; }
                            /* MENU.C F0412:1824,1853: RANDOM(16) + ordinal*40.
                             * Independent bound rejects wrong power-rune scaling;
                             * it does not prove which RNG sample was consumed. */
                            if (raw[2] < (powerRune + 1) * 40 || raw[2] > (powerRune + 1) * 40 + 15) {
                                fprintf(stderr, "FAIL: restorative source power range\n"); return 0;
                            }
                            int counter = ((511 - raw[2]) / (32 + (raw[2] + 1) / 8)) >> 1;
                            int before = getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? 990 : 100;
                            int expected = before + 1000 / counter;
                            if (expected > 1000) expected = 1000;
                            int manaBefore = getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? 895 : 95;
                            if (restorative == 13) {
                                expected = manaBefore + 2 * (raw[2] / 25 + 8) - 8;
                                if (expected > 900) expected = 900;
                                if (expected > 100) expected -= (expected - (manaBefore > 100 ? manaBefore : 100)) >> 1;
                            }
                            state->world.party.champions[1].inventory[CHAMPION_SLOT_ACTION_HAND] = THING_NONE;
                            state->world.party.champions[1].stamina.current = before;
                            state->world.party.champions[1].stamina.maximum = 1000;
                            state->world.party.champions[1].mana.current = manaBefore;
                            state->world.party.champions[1].mana.maximum = 100;
                            /* PANEL.C F0349:1900 and 1925-1926: VI restores
                             * maximum/counter health, capped at maximum.
                             * PANEL.C:1901-1910 and BASE.C F0027:1688 onward
                             * independently determine wounds and final RNG seed. */
                            state->world.party.champions[1].hp.current = before;
                            state->world.party.champions[1].hp.maximum = 1000;
                            state->world.party.champions[1].wounds = restorative == 14 ? 63 : 0;
                            uint32_t expectedSeed = state->world.masterRng.seed;
                            unsigned short expectedWounds = state->world.party.champions[1].wounds;
                            if (restorative == 14) {
                                int iterations = raw[2] / 42;
                                int tries = 10;
                                if (iterations < 1) iterations = 1;
                                do {
                                    for (int i = 0; i < iterations; ++i) {
                                        expectedSeed = expectedSeed * UINT32_C(0xBB40E62D) + 11u;
                                        expectedWounds &= (unsigned short)(expectedSeed >> 8);
                                    }
                                    iterations = 1;
                                } while (expectedWounds == 63 && --tries);
                            }
                            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, flask)) return 0;
                            (void)M11_GameView_HandlePointer(state, 60, 54, 1);
                            (void)M11_GameView_HandlePointerButtonRelease(state, 60, 54, DM1_V1_MOUSE_MASK_LEFT_PC34);
                            raw = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, flask);
                            if (state->world.masterRng.seed != expectedSeed || state->world.party.champions[1].wounds != expectedWounds) {
                                fprintf(stderr, "FAIL: restorative wound RNG ordering\n"); return 0;
                            }
                            if (!raw || (raw[3] & 127) != 20 ||
                                state->world.party.champions[1].stamina.current != (restorative == 11 ? expected : before) ||
                                state->world.party.champions[1].mana.current != (restorative == 13 ? expected : manaBefore) ||
                                state->world.party.champions[1].hp.current != (restorative == 14 ? expected : before)) {
                                fprintf(stderr, "FAIL: spell-created restorative consumption\n"); return 0;
                            }
                            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                            state->world.party.champions[1] = saved;
                            state->world.lifecycle.champions[1] = savedLife;
                        }
                        for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_WEAPON]; ++r) {
                            unsigned short candidate = (unsigned short)((THING_TYPE_WEAPON << 10) | r);
                            const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                            if (candidate != other && candidate != thing && bytes &&
                                !(bytes[0] == 0xff && bytes[1] == 0xff)) {
                                deathExtra = candidate;
                                break;
                            }
                        }
                        if (deathExtra == THING_NONE) return 0;
                        /* G0057 drops source backpack 13 (host 11) before
                         * source ready hand 0 (host 19); F0163 appends. */
                        state->world.party.champions[1].inventory[11] = deathExtra;
                        {
                            int extraWeight;
                            if (!dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, deathExtra, &extraWeight)) return 0;
                            state->world.party.champions[1].load = (unsigned short)(otherWeight + extraWeight);
                        }
                        {
                            uint32_t tickBefore = state->world.gameTick;
                            unsigned short hpBefore = state->world.party.champions[1].hp.current;
                            unsigned short leaderHp = state->world.party.champions[0].hp.current;
                            struct TimelineEvent_Compat poison = {0};
                            /* F0322:1947 uses max(1, Attack >> 6): 128 -> 2. */
                            poison.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
                            poison.fireAtTick = tickBefore;
                            poison.mapIndex = state->world.party.mapIndex;
                            poison.aux0 = LIFECYCLE_STATUS_POISON;
                            poison.aux1 = 128;
                            poison.aux2 = LIFECYCLE_STATUS_POISON;
                            poison.aux4 = 1;
                            state->world.lifecycle.champions[1].poisonEventCount = 1;
                            if (!F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &poison)) return 0;
                            (void)M11_GameView_AdvanceIdleTick(state);
                            if (state->world.gameTick <= tickBefore ||
                                !state->inventoryPanelActive ||
                                state->dm1InventoryChampionOrdinal != 2) {
                                fprintf(stderr, "FAIL: inventory pauses source simulation\n");
                                return 0;
                            }
                            if (state->world.party.champions[1].hp.current != hpBefore - 2 ||
                                state->world.party.champions[0].hp.current != leaderHp) {
                                fprintf(stderr, "FAIL: inventory poison tick damage/owner\n");
                                return 0;
                            }
                            {
                                int chains = 0;
                                for (int e = 0; e < state->world.timeline.count; ++e) {
                                    const struct TimelineEvent_Compat *next = &state->world.timeline.events[e];
                                    if (next->kind != TIMELINE_EVENT_STATUS_TIMEOUT ||
                                        next->aux0 != LIFECYCLE_STATUS_POISON || next->aux4 != 1) continue;
                                    if (next->aux1 != 127 || next->aux2 != LIFECYCLE_STATUS_POISON ||
                                        next->fireAtTick != tickBefore + 36 ||
                                        next->mapIndex != state->world.party.mapIndex) {
                                        fprintf(stderr, "FAIL: source poison reschedule attack=%d tag=%d due=%u now=%u map=%d party=%d\n",
                                            next->aux1, next->aux2, next->fireAtTick, state->world.gameTick,
                                            next->mapIndex, state->world.party.mapIndex);
                                        return 0;
                                    }
                                    ++chains;
                                }
                                if (chains != 1) return 0;
                            }
                        }
                        state->world.party.champions[1].hp.current = 0;
                        unsigned short inheritedHand = THING_NONE;
                        int inheritedWeight = 0;
                        if (getenv("FIRESTAFF_VERIFY_LEADER_DEATH")) {
                            state->world.party.activeChampionIndex = 1;
                            for (int r = 0; r < state->world.things->thingCounts[THING_TYPE_WEAPON]; ++r) {
                                unsigned short candidate = (unsigned short)((THING_TYPE_WEAPON << 10) | r);
                                const unsigned char *bytes = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, candidate);
                                if (candidate != thing && candidate != other && candidate != deathExtra && bytes &&
                                    !(bytes[0] == 0xff && bytes[1] == 0xff)) { inheritedHand = candidate; break; }
                            }
                            if (inheritedHand == THING_NONE ||
                                !dm1_v1_dungeon_get_object_weight_f0140_pc34(state->world.things, inheritedHand, &inheritedWeight) ||
                                !DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, inheritedHand)) return 0;
                            state->world.party.champions[1].load += (unsigned short)inheritedWeight;
                            state->world.party.champions[0].direction =
                                (unsigned char)((state->world.party.direction + 1) & 3);
                        }
                        state->dm1SpellCasting.magicCasterIndex = 1;
                        state->dm1SpellCasting.input[1].symbols[0] = 96;
                        state->dm1SpellCasting.input[1].symbolStep = 1;
                        state->spellBuffer.runes[0] = 96;
                        state->spellBuffer.runeCount = 1;
                        state->spellRuneRow = 1;
                        memset(&state->dm1SpellCasting.input[0], 0,
                               sizeof(state->dm1SpellCasting.input[0]));
                        state->dm1SpellCasting.input[0].symbols[0] = 97;
                        state->dm1SpellCasting.input[0].symbolStep = 1;
                        if (getenv("FIRESTAFF_VERIFY_LIVING_CASTER")) {
                            /* F0319 leaves a different living caster selected;
                             * distinguish the live UI from its cached input. */
                            state->dm1SpellCasting.magicCasterIndex = 0;
                            state->spellBuffer.runes[0] = 98;
                            state->spellRuneRow = 2;
                        }
                        /* Real dungeon, controlled pending C75 events in RAM.
                         * Different owners and event kinds must survive. */
                        struct TimelineQueue_Compat beforeDeathTimeline;
                        for (int poisonCase = 0; poisonCase < 5; ++poisonCase) {
                            struct TimelineEvent_Compat event = {0};
                            event.kind = poisonCase == 4 ? TIMELINE_EVENT_PLAY_SOUND : TIMELINE_EVENT_STATUS_TIMEOUT;
                            event.fireAtTick = state->world.gameTick + 100;
                            event.aux0 = poisonCase == 2 ? LIFECYCLE_STATUS_INVISIBILITY : LIFECYCLE_STATUS_POISON;
                            event.aux1 = 20;
                            event.aux4 = poisonCase == 1 ? 0 : 1;
                            if (!F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &event)) return 0;
                        }
                        state->world.lifecycle.champions[1].poisonEventCount = 2;
                        beforeDeathTimeline = state->world.timeline;
                        M11_GameView_ProbeCheckPartyDeath(state);
                        {
                            int retained = 0;
                            for (int e = 0; e < beforeDeathTimeline.count; ++e) {
                                const struct TimelineEvent_Compat *event = &beforeDeathTimeline.events[e];
                                if (event->kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
                                    event->aux0 == LIFECYCLE_STATUS_POISON && event->aux4 == 1) continue;
                                if (retained >= state->world.timeline.count ||
                                    memcmp(event, &state->world.timeline.events[retained], sizeof(*event))) {
                                    fprintf(stderr, "FAIL: death poison queue ownership/order\n");
                                    return 0;
                                }
                                ++retained;
                            }
                            if (retained != state->world.timeline.count ||
                                state->world.timeline.nowTick != beforeDeathTimeline.nowTick ||
                                state->world.lifecycle.champions[1].poisonEventCount != 0) return 0;
                        }
                        if (state->dm1SpellCasting.magicCasterIndex != 0 ||
                            state->dm1SpellCasting.input[1].symbols[0] != 0 ||
                            state->dm1SpellCasting.input[1].symbolStep != 0 ||
                            state->spellBuffer.runeCount != 1 ||
                            state->spellBuffer.runes[0] !=
                                (getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? 98 : 97) ||
                            state->spellRuneRow !=
                                (getenv("FIRESTAFF_VERIFY_LIVING_CASTER") ? 2 : 1) ||
                            state->dm1SpellCasting.input[0].symbols[0] != 97 ||
                            state->dm1SpellCasting.input[0].symbolStep != 1) {
                            fprintf(stderr, "FAIL: death caster/rune handoff\n");
                            return 0;
                        }
                        if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != inheritedHand ||
                            state->world.party.champions[0].load != inheritedWeight) return 0;
                        if (getenv("FIRESTAFF_VERIFY_LEADER_DEATH") &&
                            state->world.party.champions[0].direction != state->world.party.direction) {
                            fprintf(stderr, "FAIL: surviving leader direction not aligned\n");
                            return 0;
                        }
                        if (state->world.party.champions[1].load != 0) {
                            fprintf(stderr, "FAIL: death retained load=%u\n", state->world.party.champions[1].load);
                            return 0;
                        }
                        if (state->world.party.champions[1].inventory[19] != THING_NONE) return 0;
                        {
                            unsigned short cursor = F0511_DUNGEON_GetSquareFirstThing_Compat(
                                state->world.dungeon, state->world.things,
                                state->world.party.mapIndex, state->world.party.mapX,
                                state->world.party.mapY);
                            int count = 0, steps = 0, extraCount = 0, extraPosition = -1, handPosition = -1;
                            while (cursor != THING_ENDOFLIST && cursor != THING_NONE && steps++ < 4096) {
                                if ((cursor & 0x3fff) == (other & 0x3fff)) {
                                    if ((cursor >> 14) != (state->world.party.champions[1].cell & 3)) return 0;
                                    ++count;
                                    handPosition = steps;
                                }
                                if ((cursor & 0x3fff) == deathExtra) { ++extraCount; extraPosition = steps; }
                                cursor = F0512_DUNGEON_GetThingNext_Compat(state->world.things, cursor);
                            }
                            if (count != 1 || extraCount != 1 || extraPosition >= handPosition || steps >= 4096) {
                                fprintf(stderr, "FAIL: death floor object count=%d steps=%d\n", count, steps);
                                return 0;
                            }
                        }
                        {
                            uint32_t before, after;
                            const unsigned char *dropped = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, other);
                            unsigned short next;
                            if (!F0891_ORCH_WorldHash_Compat(&state->world, &before)) return 0;
                            if (!dropped) return 0;
                            next = (unsigned short)(dropped[0] | ((unsigned int)dropped[1] << 8));
                            M11_GameView_ProbeCheckPartyDeath(state);
                            dropped = dm1_v1_dungeon_get_thing_data_pc34(state->world.things, other);
                            if (!dropped || !F0891_ORCH_WorldHash_Compat(&state->world, &after) || before != after ||
                                next != (unsigned short)(dropped[0] | ((unsigned int)dropped[1] << 8))) return 0;
                        }
                        /* F0319's final death sets PartyDead and does not
                         * run the surviving-caster selection branch. */
                        state->world.party.champions[0].hp.current = 1;
                        state->inventoryPanelActive = 1;
                        state->dm1InventoryChampionOrdinal = 1;
                        {
                            struct TimelineEvent_Compat lethal = {0};
                            lethal.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
                            lethal.fireAtTick = state->world.gameTick;
                            lethal.mapIndex = state->world.party.mapIndex;
                            lethal.aux0 = LIFECYCLE_STATUS_POISON;
                            lethal.aux1 = 128;
                            lethal.aux2 = LIFECYCLE_STATUS_POISON;
                            lethal.aux4 = 0;
                            state->world.lifecycle.champions[0].poisonEventCount = 2;
                            if (!F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &lethal)) return 0;
                            (void)M11_GameView_AdvanceIdleTick(state);
                            if (state->world.party.champions[0].hp.current != 0 ||
                                state->world.lifecycle.champions[0].poisonEventCount != 0) return 0;
                            for (int e = 0; e < state->world.timeline.count; ++e) {
                                const struct TimelineEvent_Compat *pending = &state->world.timeline.events[e];
                                if (pending->kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
                                    pending->aux0 == LIFECYCLE_STATUS_POISON && pending->aux4 == 0) {
                                    fprintf(stderr, "FAIL: lethal poison retains pending chain\n");
                                    return 0;
                                }
                            }
                        }
                        if (!state->partyDead || !state->world.partyDead ||
                            state->dm1SpellCasting.magicCasterIndex != 0 ||
                            state->dm1SpellCasting.input[0].symbols[0] != 0 ||
                            state->dm1SpellCasting.input[0].symbolStep != 0 ||
                            state->spellBuffer.runeCount != 0 ||
                            state->spellRuneRow != 0) {
                            fprintf(stderr, "FAIL: final death spell state\n");
                            return 0;
                        }
                        fprintf(stderr, "death owner panel=%d ordinal=%d handled=%u\n",
                            state->inventoryPanelActive, state->dm1InventoryChampionOrdinal,
                            state->championDeathHandledMask);
                        return !state->inventoryPanelActive &&
                            state->dm1InventoryChampionOrdinal == 0 &&
                            (state->championDeathHandledMask & 2) &&
                            state->world.party.activeChampionIndex == 0;
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
