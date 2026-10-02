/* Original PC 3.4 HoC-to-first-monster action regression. */
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "menu_input_m12.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState state;
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
    unsigned char actions[3];
    unsigned short first;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 77;
    }
    {
        FILE *media = fopen(archive, "rb");
        if (!media) {
            puts("SKIP: original PC 3.4 archive is unavailable");
            return 77;
        }
        fclose(media);
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

    /* Continue the source route through the first pressure/door passage.
     * ReDMCSB COMMAND.C:2155 dispatches movement to CLIKMENU.C F0366;
     * CLIKVIEW.C:437 dispatches the visible front-cell C080 click. */
    {
        static const M12_MenuInput to_first_door[] = {
            M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_UP,
            M12_MENU_INPUT_STRAFE_RIGHT, M12_MENU_INPUT_STRAFE_RIGHT,
            M12_MENU_INPUT_STRAFE_RIGHT, M12_MENU_INPUT_TURN_RIGHT,
            M12_MENU_INPUT_TURN_RIGHT
        };
        for (i = 0; i < sizeof(to_first_door) / sizeof(to_first_door[0]); ++i) {
            if (M11_GameView_HandleInput(&state, to_first_door[i]) ==
                M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic first-door route input ignored\n", stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                (void)M11_GameView_AdvanceIdleTick(&state);
        }
    }
    if (M11_GameView_HandlePointerButton(&state, 167, 81,
            M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED) {
        fputs("FAIL: authentic first-door button click ignored\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (tick = 0; tick < 30; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&state);
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP) ==
        M11_GAME_INPUT_IGNORED) {
        fputs("FAIL: first-door passage input ignored\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (tick = 0; tick < 5; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&state);
    first = F0511_DUNGEON_GetSquareFirstThing_Compat(state.world.dungeon,
        state.world.things, 1, 6, 2);
    if (state.world.party.mapIndex != 1 || state.world.party.mapX != 6 ||
        state.world.party.mapY != 1 || state.world.party.direction != 2 ||
        state.world.party.champions[0].hp.current != 47 ||
        first != 0x10a9u || state.world.things->groupCount <= 169 ||
        state.world.things->groups[169].slot != THING_ENDOFLIST) {
        fputs("FAIL: first-door passage lost original pose or group #169\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    /* ReDMCSB CLIKMENU.C:300-323, F0366: a live group in the front square
     * blocks forward movement and schedules its adjacent-party reaction. */
    (void)M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    for (tick = 0; tick < 5; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&state);
    if (state.world.party.mapX != 6 || state.world.party.mapY != 1 ||
        F0511_DUNGEON_GetSquareFirstThing_Compat(state.world.dungeon,
            state.world.things, 1, 6, 2) != 0x10a9u) {
        fputs("FAIL: original group did not block forward movement\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (int round = 0; round < 4; ++round) {
        /* ReDMCSB MENU.C:803-840, F0391 selects an action-list row and
         * dispatches F0407 before clearing the acting champion. */
        if (!M11_GameView_SetActingChampion(&state, 0) ||
            !M11_GameView_GetActingActionIndices(&state, actions) ||
            actions[0] != 6 || actions[1] != 7 || actions[2] != 8) {
            fputs("FAIL: first-monster action list unavailable\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (!M11_GameView_TriggerActionRow(&state, 0)) {
            fputs("FAIL: first-monster action did not commit\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 30; ++tick)
            (void)M11_GameView_AdvanceIdleTick(&state);
        first = F0511_DUNGEON_GetSquareFirstThing_Compat(state.world.dungeon,
            state.world.things, 1, 6, 2);
        if (first != (round < 3 ? 0x10a9u : THING_ENDOFLIST)) {
            fprintf(stderr, "FAIL: source group #169 round %d left thing %04x\n",
                round + 1, first);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    for (int step = 0; step < 2; ++step) {
        if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP) ==
            M11_GAME_INPUT_IGNORED) {
            fputs("FAIL: cleared monster passage input ignored\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 5; ++tick)
            (void)M11_GameView_AdvanceIdleTick(&state);
        if (state.world.party.mapIndex != 1 || state.world.party.mapX != 6 ||
            state.world.party.mapY != step + 2 ||
            state.world.party.direction != 2) {
            fputs("FAIL: cleared monster passage did not admit forward movement\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    (void)M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    for (tick = 0; tick < 5; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&state);
    if (state.world.party.mapIndex != 1 || state.world.party.mapX != 6 ||
        state.world.party.mapY != 3 || state.world.party.direction != 2) {
        fputs("FAIL: original wall after monster passage did not block movement\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("PASS: authentic PC34 first-door monster combat and passage");
    return 0;
}
