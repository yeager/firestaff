/* Real-media guard for the Hall of Champions scroll used by the original
 * DOS capture route.  It deliberately checks source ownership only; it does
 * not claim that an emulator route reached or picked up the object. */
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState state;
    unsigned short thing;
    int steps = 0;

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
    M11_GameView_Shutdown(&state);
    puts("PASS: canonical PC34 HoC (4,15) owns closed scroll 0 -> text 33");
    return 0;
}
