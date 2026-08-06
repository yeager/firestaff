#include "m11_game_view.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        } else {
            ++*outChecked;
        }
    }
    return failures;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    int checked = 0;
    int failures = 0;

    if (!dataDir || !dataDir[0]) {
        puts("skip: FIRESTAFF_DM1_DATA_DIR is not set");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir) || !state.world.things ||
        !state.dm1ObjectNameTableValid) {
        fprintf(stderr, "DM1 real object corpus failed to start\n");
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
    printf("ok: real DM1 object corpus names/icons verified (%d records)\n",
           checked);
    return 0;
}
