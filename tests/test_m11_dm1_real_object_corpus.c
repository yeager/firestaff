#include "m11_game_view.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ReDMCSB CHAMPION.C F0302:689-711 exchanges G4055 and the selected
 * slot. A mouse release must not undo the press-owned exchange. Use each
 * original decoded Thing, including scrolls and containers, in both modes. */
static int check_inventory_roundtrip(M11_GameViewState *state,
                                     unsigned short thing)
{
    int modes[] = { M12_PRESENTATION_V1_ORIGINAL,
                    M12_PRESENTATION_V21_UPSCALED };
    int x, y, w, h;
    int mode;
    if (!M11_GameView_GetV1InventorySourceSlotBoxZone(9, &x, &y, &w, &h))
        return 0;
    for (mode = 0; mode < 2; ++mode) {
        int step;
        state->presentationMode = modes[mode];
        state->inventoryPanelActive = 1;
        state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
            THING_NONE;
        if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing))
            return 0;
        for (step = 0; step < 2; ++step) {
            unsigned short hand = step ? thing : THING_NONE;
            unsigned short slot = step ? THING_NONE : thing;
            if (M11_GameView_HandlePointer(state, x + w / 2,
                    33 + y + h / 2, 1) != M11_GAME_INPUT_REDRAW ||
                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] != slot)
                return 0;
            (void)M11_GameView_HandlePointerButtonRelease(state, x + w / 2,
                33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
            if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != hand ||
                state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] != slot)
                return 0;
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
