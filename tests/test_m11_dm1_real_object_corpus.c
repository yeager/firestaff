#include "m11_game_view.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned short find_slot_resident(const M11_GameViewState *state,
                                         unsigned short excluded,
                                         unsigned int mask)
{
    static const int types[] = { THING_TYPE_WEAPON, THING_TYPE_ARMOUR,
        THING_TYPE_SCROLL, THING_TYPE_POTION, THING_TYPE_CONTAINER,
        THING_TYPE_JUNK };
    size_t t;
    for (t = 0; t < sizeof(types) / sizeof(types[0]); ++t) {
        int i;
        for (i = 0; i < state->world.things->thingCounts[types[t]]; ++i) {
            unsigned short candidate = (unsigned short)((types[t] << 10) | i);
            if (candidate != excluded &&
                (dm1_v1_dungeon_get_object_allowed_slots_pc34(
                    state->world.things, candidate) & mask)) return candidate;
        }
    }
    return THING_NONE;
}

static int check_original_chests(M11_GameViewState *state)
{
    int c, mode, checked = 0;
    for (c = 0; c < state->world.things->containerCount; ++c) {
        unsigned short contents[8], next = state->world.things->containers[c].slot;
        unsigned short chest = (unsigned short)((THING_TYPE_CONTAINER << 10) | c);
        int count = 0;
        while (next != THING_ENDOFLIST && next != THING_NONE && count < 8) {
            contents[count++] = next;
            next = F0512_DUNGEON_GetThingNext_Compat(state->world.things, next);
        }
        /* Do not silently truncate an unexpected original chain. */
        if (next != THING_ENDOFLIST) {
            fprintf(stderr, "chest=%d original chain count=%d tail=%04x\n", c, count, next);
            return 0;
        }
        for (mode = 0; mode < 2; ++mode) {
            int i;
            state->presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED :
                                            M12_PRESENTATION_V1_ORIGINAL;
            state->inventoryPanelActive = 1;
            state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chest;
            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
            if (!DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(state)) {
                fprintf(stderr, "chest=%d open rejected\n", c); return 0;
            }
            for (i = 0; i < count; ++i) {
                int x, y, w, h, step;
                if (!M11_GameView_GetV1ChestSlotBoxZone(i, &x, &y, &w, &h)) {
                    fprintf(stderr, "chest=%d zone=%d rejected\n", c, i); return 0;
                }
                /* CHEST.C F0333/F0334 and CHAMPION.C F0302: take each
                 * original resident and return it to the same visible slot. */
                for (step = 0; step < 2; ++step) {
                    /* CHEST.C F0333:30-33: refreshing the same owner
                     * must preserve the visual hole after pickup. */
                    if (step && !DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(state)) return 0;
                    unsigned short expected = step ? THING_NONE : contents[i];
                    (void)M11_GameView_HandlePointer(state, x + w / 2, 33 + y + h / 2, 1);
                    if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != expected) {
                        fprintf(stderr, "chest=%d slot=%d step=%d got=%04x expected=%04x\n",
                            c, i, step, DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state), expected);
                        return 0;
                    }
                    (void)M11_GameView_HandlePointerButtonRelease(state,
                        x + w / 2, 33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                    if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != expected) {
                        fprintf(stderr, "chest=%d slot=%d release step=%d got=%04x expected=%04x\n",
                            c, i, step, DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state), expected);
                        return 0;
                    }
                }
            }
            DM1_V1_M11Runtime_CloseOpenChestPc34Compat(state);
            next = state->world.things->containers[c].slot;
            for (i = 0; i < count; ++i) {
                if (next != contents[i]) {
                    fprintf(stderr, "chest=%d closed slot=%d got=%04x expected=%04x\n", c, i, next, contents[i]);
                    return 0;
                }
                next = F0512_DUNGEON_GetThingNext_Compat(state->world.things, next);
            }
            if (next != THING_ENDOFLIST) return 0;
            if (count && state->world.things->containerCount > 1) {
                int otherIndex = (c + 1) % state->world.things->containerCount;
                unsigned short other = (unsigned short)((THING_TYPE_CONTAINER << 10) | otherIndex);
                unsigned short otherHead = state->world.things->containers[otherIndex].slot;
                int x, y, w, h;
                if (!DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(state) ||
                    !M11_GameView_GetV1ChestSlotBoxZone(0, &x, &y, &w, &h)) return 0;
                (void)M11_GameView_HandlePointer(state, x + w / 2, 33 + y + h / 2, 1);
                (void)M11_GameView_HandlePointerButtonRelease(state,
                    x + w / 2, 33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = other;
                /* F0333:34-41 closes G0426 before changing owner. */
                if (!DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(state) ||
                    DM1_V1_M11Runtime_GetOpenChestThingPc34Compat(state) != other ||
                    DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != contents[0]) return 0;
                next = state->world.things->containers[c].slot;
                for (i = 1; i < count; ++i) {
                    if (next != contents[i]) return 0;
                    next = F0512_DUNGEON_GetThingNext_Compat(state->world.things, next);
                }
                if (next != THING_ENDOFLIST) return 0;
                DM1_V1_M11Runtime_CloseOpenChestPc34Compat(state);
                if (state->world.things->containers[otherIndex].slot != otherHead) return 0;
                /* Restore the disposable test placement through G0425's
                 * close transaction; no original media is modified. */
                state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chest;
                if (!DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(state)) return 0;
                for (i = 0; i < 8; ++i)
                    state->v1OpenChestSlots[i] = i < count ? contents[i] : THING_NONE;
                DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(state);
                DM1_V1_M11Runtime_CloseOpenChestPc34Compat(state);
            }
        }
        checked += count;
    }
    state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = THING_NONE;
    printf("ok: original chest contents roundtripped (%d residents)\n", checked);
    return checked > 0;
}

/* ReDMCSB CHAMPION.C F0302:689-711 exchanges G4055 and the selected
 * slot. A mouse release must not undo the press-owned exchange. Use each
 * original decoded Thing, including scrolls and containers, in both modes. */
static int check_inventory_roundtrip(M11_GameViewState *state,
                                     unsigned short thing)
{
    int modes[] = { M12_PRESENTATION_V1_ORIGINAL,
                    M12_PRESENTATION_V21_UPSCALED };
    /* DATA.C G0038:1049-1079, independent source slot-mask oracle. */
    static const unsigned int slotMasks[30] = {
        0xffff, 0xffff, 2, 8, 16, 32, 256, 128, 128, 128, 4, 256, 64,
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
        0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
    };
    static const int hostSlots[30] = {
        19, 20, 0, 2, 3, 4, 6, 9, 8, 10, 1, 5, 7,
        11, 12, 13, 14, 15, 16, 17, 18, 21, 22, 23, 24, 25, 26, 27, 28, 29
    };
    unsigned int allowed = dm1_v1_dungeon_get_object_allowed_slots_pc34(
        state->world.things, thing);
    int x, y, w, h;
    int mode, inventorySlot;
    for (mode = 0; mode < 2; ++mode) {
      for (inventorySlot = 0; inventorySlot < 30; ++inventorySlot) {
        int step;
        int hostSlot = hostSlots[inventorySlot];
        int admitted = (allowed & slotMasks[inventorySlot]) != 0;
        if (!M11_GameView_GetV1InventorySourceSlotBoxZone(
                inventorySlot + 8, &x, &y, &w, &h)) return 0;
        state->presentationMode = modes[mode];
        state->inventoryPanelActive = 1;
        state->world.party.champions[0].inventory[hostSlot] =
            THING_NONE;
        if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing))
            return 0;
        for (step = 0; step < (admitted ? 2 : 1); ++step) {
            unsigned short hand = (step || !admitted) ? thing : THING_NONE;
            unsigned short slot = (step || !admitted) ? THING_NONE : thing;
            (void)M11_GameView_HandlePointer(state, x + w / 2,
                    33 + y + h / 2, 1);
            if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                state->world.party.champions[0].inventory[hostSlot] != slot) {
                fprintf(stderr, "slot=%d mode=%d admitted=%d step=%d\n",
                        inventorySlot, modes[mode], admitted, step);
                return 0;
            }
            (void)M11_GameView_HandlePointerButtonRelease(state, x + w / 2,
                33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
            if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                state->world.party.champions[0].inventory[hostSlot] != slot)
                return 0;
        }
        /* F0302 tests the incoming object's mask before removing either
         * Thing. Seed a distinct admissible original resident to check both
         * swap directions and rejection without manufacturing object data. */
        {
            unsigned short resident = find_slot_resident(state, thing,
                                                         slotMasks[inventorySlot]);
            if (resident == THING_NONE) return 0;
            state->world.party.champions[0].inventory[hostSlot] = resident;
            for (step = 0; step < (admitted ? 2 : 1); ++step) {
                unsigned short hand = (step || !admitted) ? thing : resident;
                unsigned short slot = (step || !admitted) ? resident : thing;
                (void)M11_GameView_HandlePointer(state, x + w / 2,
                                                33 + y + h / 2, 1);
                if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                    state->world.party.champions[0].inventory[hostSlot] != slot)
                    return 0;
                (void)M11_GameView_HandlePointerButtonRelease(state, x + w / 2,
                    33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
                if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                    state->world.party.champions[0].inventory[hostSlot] != slot)
                    return 0;
            }
            state->world.party.champions[0].inventory[hostSlot] = THING_NONE;
        }
      }
    }
    return 1;
}

static int check_type(M11_GameViewState *state, int thingType,
                      const char *label, int *outChecked)
{
    const struct DungeonThings_Compat *things = state->world.things;
    int count = things->thingCounts[thingType];
    int i;
    int failures = 0;

    for (i = 0; i < count; ++i) {
        unsigned short thing = (unsigned short)((thingType << 10) | i);
        const unsigned char *raw =
            dm1_v1_dungeon_get_thing_data_pc34(things, thing);
        DM1_V1_ObjectIconSourceZonePc34 zone;
        char name[96];
        int subtype;
        int info;
        int icon;

        if (!raw) {
            fprintf(stderr, "missing raw %s record %d\n", label, i);
            ++failures;
            continue;
        }
        subtype = dm1_v1_dungeon_get_object_subtype_pc34(things, thing);
        info = dm1_v1_dungeon_get_object_info_index_pc34(things, thing);
        icon = dm1_v1_dungeon_get_object_icon_index_pc34(
            things, thing, state->world.party.direction);
        memset(name, 0, sizeof(name));
        if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing) ||
            !DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(
                state, name, (int)sizeof(name)) || name[0] == '\0' ||
            strncmp(name, "WEAPON ", 7) == 0 ||
            strncmp(name, "ARMOUR ", 7) == 0 ||
            strncmp(name, "POTION ", 7) == 0 ||
            strncmp(name, "JUNK ", 5) == 0 ||
            subtype < 0 || info < 0 || icon < 0 ||
            !dm1_v1_object_icon_source_zone_pc34(icon, &zone) ||
            zone.graphic_index < 0 || zone.w != 16 || zone.h != 16) {
            fprintf(stderr,
                    "invalid real %s[%d]: subtype=%d info=%d icon=%d name='%s'\n",
                    label, i, subtype, info, icon, name);
            ++failures;
        } else if (!check_inventory_roundtrip(state, thing)) {
            fprintf(stderr, "real %s[%d] inventory roundtrip failed\n", label, i);
            ++failures;
        } else if (thingType == THING_TYPE_SCROLL) {
            int mode;
            int scrollOk = 1;
            /* PANEL.C F0342:1126-1131 routes original scrolls to F0341,
             * not the generic object-description dialog. Reading is not
             * an inventory transfer. Test real scroll records in both modes. */
            for (mode = 0; mode < 2; ++mode) {
                unsigned char framebuffer[320 * 200];
                state->presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED :
                                                M12_PRESENTATION_V1_ORIGINAL;
                state->inventoryPanelActive = 1;
                if (M11_GameView_HandlePointer(state, 20, 54, 1) !=
                        M11_GAME_INPUT_REDRAW ||
                    !state->v1ScrollPanelActive || state->v1ScrollPanelThing != thing ||
                    state->v1ObjectDescriptionPanelActive ||
                    M11_GameView_IsDialogOverlayActive(state)) scrollOk = 0;
                memset(framebuffer, 0, sizeof(framebuffer));
                M11_GameView_Draw(state, framebuffer, 320, 200);
                (void)M11_GameView_HandlePointerButtonRelease(state, 20, 54,
                    DM1_V1_MOUSE_MASK_LEFT_PC34);
                if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != thing)
                    scrollOk = 0;
            }
            if (!scrollOk) {
                fprintf(stderr, "real scroll[%d] eye route failed\n", i);
                ++failures;
            } else ++*outChecked;
        } else {
            ++*outChecked;
        }
    }
    return failures;
}

int main(void)
{
    /* Every expectation in this corpus comes from the canonical DOS PC 3.4
     * tables.  Require its archive rather than selecting an arbitrary DM1
     * directory that may contain a different language or revision. */
    const char *dataDir = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState state;
    int checked = 0;
    int failures = 0;

    if (!dataDir || !dataDir[0]) {
        puts("skip: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 77;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir) || !state.world.things ||
        !state.dm1ObjectNameTableValid || !state.originalFontAvailable) {
        fprintf(stderr, "DM1 real object corpus failed to start\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    F0600_CHAMPION_InitEmpty_Compat(&state.world.party.champions[0]);
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    if (!check_original_chests(&state)) {
        fprintf(stderr, "original chest content/ownership roundtrip failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    /* ReDMCSB G0237 has 180 rows.  The key/junk tail is easy to shift when
     * hand-copying the aspect table: a missing row silently selects a later
     * object's art while C still accepts the initializer. */
    {
        static const int expectedJunkAspects[32] = {
            62, 62, 62, 62,
            76, 3, 60, 61, 27, 28, 25, 26,
            71, 70, 5, 66, 15, 15, 58, 59,
            59, 79, 63, 64, 72, 73, 74, 75,
            77, 78, 74, 41
        };
        int i;
        for (i = 0; i < 32; ++i) {
            int subtype = 21 + i;
            if (dm1_item_aspect_index(THING_TYPE_JUNK, subtype) !=
                expectedJunkAspects[i]) {
                fprintf(stderr,
                        "invalid G0237 junk aspect: subtype %d aspect=%d expected=%d\n",
                        subtype,
                        dm1_item_aspect_index(THING_TYPE_JUNK, subtype),
                        expectedJunkAspects[i]);
                ++failures;
            }
        }
    }
    failures += check_type(&state, THING_TYPE_WEAPON, "weapon", &checked);
    failures += check_type(&state, THING_TYPE_ARMOUR, "armour", &checked);
    failures += check_type(&state, THING_TYPE_SCROLL, "scroll", &checked);
    failures += check_type(&state, THING_TYPE_POTION, "potion", &checked);
    failures += check_type(&state, THING_TYPE_CONTAINER, "container", &checked);
    failures += check_type(&state, THING_TYPE_JUNK, "junk", &checked);
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);
    M11_GameView_Shutdown(&state);

    if (failures != 0) {
        fprintf(stderr, "FAIL: %d real DM1 object records\n", failures);
        return 1;
    }
    printf("ok: real DM1 names/icons and Original/V2.1 inventory roundtrips verified (%d records)\n",
           checked);
    return 0;
}
