/* Original PC 3.4 HoC-to-first-monster action regression. */
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "menu_input_m12.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned int group_health_total(const struct DungeonGroup_Compat *group)
{
    unsigned int total = 0;
    unsigned int i;
    if (!group) return 0;
    for (i = 0; i <= group->count && i < 4u; ++i)
        total += group->health[i];
    return total;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    const char *originalSave = getenv("FIRESTAFF_DM1_PC34_ORIGINAL_SAVE");
    const char *shieldText = getenv("FIRESTAFF_TEST_PARTY_SHIELD");
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
    int clickX, clickY, clickW, clickH;
    uint32_t attackBeforeDoor;
    uint32_t attackAfterFirstEncounter;
    uint32_t encounterTickStart;
    unsigned short initialChampionHp;
    unsigned short hpAfterFirstEncounter;
    unsigned int naturalDamage;
    unsigned int monsterHpBeforePunch;
    unsigned int monsterHpAfterPunch;
    int repeatedGroup169Attacks = 0;
    long partyShieldDefense = shieldText ? strtol(shieldText, NULL, 10) : 0;

    if (partyShieldDefense < 0 || partyShieldDefense > 100) {
        fputs("FAIL: test party shield must be between 0 and 100\n", stderr);
        return 2;
    }

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
    /* When a real PC34 save is available, verify F0435 restores the saved
     * four-champion formation cells from each 319-byte C2 champion record.
     * The source writes Direction at byte 28 and Cell at byte 29; losing the
     * latter collapses the whole party into cell zero and breaks melee reach. */
    if (originalSave && originalSave[0]) {
        M11_GameViewState resumed;
        unsigned int formationCells = 0u;
        int slot;
        FILE *save = fopen(originalSave, "rb");
        if (!save) {
            fprintf(stderr, "FAIL: authentic PC34 save is unavailable: %s\n",
                    originalSave);
            return 1;
        }
        fclose(save);
        M11_GameView_Init(&resumed);
        if (!M11_GameView_StartDm1(&resumed, archive) ||
            !M11_GameView_LoadDm1SavePath(&resumed, originalSave, NULL) ||
            resumed.world.party.championCount != 4) {
            fprintf(stderr,
                    "FAIL: authentic PC34 save did not restore four champions (%s %s)\n",
                    resumed.lastAction, resumed.lastOutcome);
            M11_GameView_Shutdown(&resumed);
            return 1;
        }
        for (slot = 0; slot < resumed.world.party.championCount; ++slot) {
            const unsigned int cell = resumed.world.party.champions[slot].cell;
            if (cell > 3u || (formationCells & (1u << cell)) != 0u) {
                fprintf(stderr,
                        "FAIL: authentic PC34 save has invalid/duplicate champion cell %u at slot %d\n",
                        cell, slot);
                M11_GameView_Shutdown(&resumed);
                return 1;
            }
            formationCells |= 1u << cell;
        }
        if (formationCells != 0x0fu) {
            fprintf(stderr,
                    "FAIL: authentic PC34 save restored incomplete formation mask 0x%x\n",
                    formationCells);
            M11_GameView_Shutdown(&resumed);
            return 1;
        }
        M11_GameView_Shutdown(&resumed);
        puts("PASS: authentic PC34 save restores all four source champion cells");
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, archive) || !state.world.dungeon ||
        !state.world.things || state.world.dungeon->header.mapCount < 1 ||
        state.world.things->scrollCount != 35 ||
        state.world.pc34F0306FirstOperandFirst != 0 ||
        !state.vblankTiming.pc34PitInputWait) {
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
    initialChampionHp = state.world.party.champions[0].hp.current;
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
        /* The authentic PC34 Hall route steps on the C003 floor-party plate
         * at (6,9), which opens the door in (5,9). Reaching the next cell
         * through production movement verifies both F0276 activation and
         * the door transition without assigning the party pose in the test. */
        if (i == 13u &&
            (state.world.party.mapIndex != 0 ||
             state.world.party.mapX != 6 || state.world.party.mapY != 9)) {
            fputs("FAIL: authentic route did not activate the Hall floor plate at (6,9)\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (i == 14u &&
            (state.world.party.mapIndex != 0 ||
             state.world.party.mapX != 5 || state.world.party.mapY != 9)) {
            fputs("FAIL: authentic route did not pass through the opened Hall door at (5,9)\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
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
    /* Controlled in-memory spell state lets the same original group attack
     * exercise the live M11 body-shield snapshot layer. F0446 uses the same
     * 0..100 range for party ShieldDefense (ReDMCSB ENDGAME.C:805-812). */
    state.world.magic.partyShieldDefense =
        (unsigned short)partyShieldDefense;
    attackBeforeDoor = state.world.lifecycle.lastCreatureAttackTime;
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
    if (state.world.party.mapIndex != 1 || state.world.party.mapX != 6 ||
        state.world.party.mapY < 0 || state.world.party.mapY >=
            (int)state.world.dungeon->maps[1].height ||
        state.world.party.direction != 2 ||
        (partyShieldDefense == 0 &&
         state.world.party.champions[0].hp.current >= initialChampionHp) ||
        state.world.lifecycle.lastCreatureAttackTime == attackBeforeDoor ||
        state.world.things->groupCount <= 169 ||
        state.world.things->groups[169].health[0] == 0) {
        fputs("FAIL: authentic map-1 group did not naturally attack after arrival\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    naturalDamage = state.world.party.champions[0].hp.current < initialChampionHp
        ? (unsigned int)(initialChampionHp -
                         state.world.party.champions[0].hp.current)
        : 0u;
    {
        int group169Active = 0;
        int group169StateKind = -1;
        int group169X = -1;
        int group169Y = -1;
        for (i = 0; i < (size_t)state.world.creatureAICount; ++i) {
            const struct CreatureAIState_Compat* ai = &state.world.creatureAI[i];
            if (ai->reserved0 == 169 && ai->groupMapIndex == 1) {
                group169Active = 1;
                group169StateKind = ai->stateKind;
                group169X = ai->groupMapX;
                group169Y = ai->groupMapY;
                break;
            }
        }
        if (!group169Active || state.world.pc34ActiveGroupSourceCount <= 0 ||
            group169StateKind != AI_STATE_ATTACK ||
            state.world.things->groups[169].behavior != DM1_BEHAVIOR_ATTACK) {
            fputs("FAIL: original group #169 did not enter natural attack behavior\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        first = F0511_DUNGEON_GetSquareFirstThing_Compat(state.world.dungeon,
            state.world.things, 1, group169X, group169Y);
        if (first != 0x10a9u) {
            fputs("FAIL: active original group #169 lost its map square\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (abs(group169X - state.world.party.mapX) +
                abs(group169Y - state.world.party.mapY) > 1) {
            fputs("FAIL: original group #169 did not approach the party\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (group169X != state.world.party.mapX ||
            group169Y != state.world.party.mapY + 1) {
            fputs("FAIL: original group #169 is not in the party's forward cell\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        const int32_t ninjaXpBefore =
            state.world.lifecycle.champions[0].skills20[1].experience;
        const int32_t fightXpBefore =
            state.world.lifecycle.champions[0].skills20[9].experience;
        monsterHpBeforePunch = group_health_total(
            &state.world.things->groups[169]);
        /* ReDMCSB MENU.C:803-840, F0391 selects an action-list row and
         * dispatches F0407 before clearing the acting champion. */
        if (!M11_GameView_GetV1ActionIconCellZone(0,
                &clickX, &clickY, &clickW, &clickH) ||
            M11_GameView_HandlePointerButton(&state,
                clickX + clickW / 2, clickY + clickH / 2,
                M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED ||
            state.actingChampionOrdinal != 1 ||
            !M11_GameView_GetActingActionIndices(&state, actions) ||
            actions[0] != 6 || actions[1] != 7 || actions[2] != 8) {
            fputs("FAIL: pointer click did not open first champion action list\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (!M11_GameView_GetV1ActionMenuRowZone(0,
                &clickX, &clickY, &clickW, &clickH) ||
            M11_GameView_HandlePointerButton(&state,
                clickX + clickW / 2, clickY + clickH / 2,
                M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED) {
            fputs("FAIL: pointer click did not dispatch first-monster action\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 5; ++tick)
            (void)M11_GameView_AdvanceIdleTick(&state);
        monsterHpAfterPunch = group_health_total(
            &state.world.things->groups[169]);
        /* ReDMCSB MENU.C:382-390,427-440 maps the visible first action to
         * PUNCH (index 6), 8 base experience, skill 9 (FIGHT). CHAMPION.C:
         * 874-895 mirrors that award to base skill 1 (NINJA). */
        if (state.world.lifecycle.champions[0].skills20[1].experience <=
                ninjaXpBefore ||
            state.world.lifecycle.champions[0].skills20[9].experience <=
                fightXpBefore ||
            state.world.lifecycle.champions[0].skills20[1].experience -
                ninjaXpBefore !=
            state.world.lifecycle.champions[0].skills20[9].experience -
                fightXpBefore) {
            fputs("FAIL: live PUNCH did not award matching Ninja/Fight XP\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (monsterHpAfterPunch >= monsterHpBeforePunch) {
            fprintf(stderr,
                    "FAIL: authentic PC34 PUNCH awarded XP but did not "
                    "reduce group #169 HP (%u -> %u)\n",
                    monsterHpBeforePunch, monsterHpAfterPunch);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        struct ChampionState_Compat saved = state.world.party.champions[0];
        struct ChampionState_Compat *champ =
            &state.world.party.champions[0];
        int feetIcon;
        int expectedMaxLoad;
        int withStaleCache;
        int withFreshCache;

        /* F0309 feeds F0312 from live stamina, wounds and footwear. Exercise
         * the real loaded HoC champion after changing those source fields,
         * and ensure neither the M11 cache nor an F0312 call relies on its
         * previous maxLoad value. */
        champ->stamina.current = champ->stamina.maximum / 4u;
        champ->wounds = (unsigned short)(champ->wounds | (1u << 4));
        feetIcon = dm1_v1_dungeon_get_object_icon_index_pc34(
            state.world.things, champ->inventory[CHAMPION_SLOT_FEET],
            state.world.party.direction);
        champ->maxLoad = 1u;
        withStaleCache = F0312_CHAMPION_HandStrengthBaseline_Compat(
            &state.world, champ, 0, 80);
        expectedMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
            champ, feetIcon, state.world.pc34F0306FirstOperandFirst);
        champ->maxLoad = (unsigned short)expectedMaxLoad;
        withFreshCache = F0312_CHAMPION_HandStrengthBaseline_Compat(
            &state.world, champ, 0, 80);
        if (withStaleCache != withFreshCache) {
            fputs("FAIL: F0312 used stale maxLoad after live stat changes\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        champ->maxLoad = 1u;
        (void)M11_GameView_AdvanceIdleTick(&state);
        expectedMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
            champ, dm1_v1_dungeon_get_object_icon_index_pc34(
                state.world.things, champ->inventory[CHAMPION_SLOT_FEET],
                state.world.party.direction),
            state.world.pc34F0306FirstOperandFirst);
        if ((int)champ->maxLoad != expectedMaxLoad) {
            fputs("FAIL: DM1 PC34 did not refresh cached F0309 maxLoad\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        *champ = saved;
    }
    /* F0207 attacks once when the source creature enters its attack
     * animation. ReDMCSB GROUP.C:2344 then clears the C38 attack latch so the
     * per-creature timeline can attack again. Prove the recurrence while the
     * original group is still alive, after exercising the player's action
     * path. A full shield keeps this long source-time probe from ending the
     * run through champion death. */
    if (state.world.things->groups[169].health[0] == 0) {
        fputs("FAIL: first monster died before the repeated-attack probe\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    attackAfterFirstEncounter = state.world.lifecycle.lastCreatureAttackTime;
    encounterTickStart = state.world.gameTick;
    hpAfterFirstEncounter = state.world.party.champions[0].hp.current;
    state.world.magic.partyShieldDefense = 100u;
    for (tick = 0; tick < 1500; ++tick) {
        int emissionIndex;
        if (state.partyDead || state.world.partyDead) break;
        (void)M11_GameView_AdvanceIdleTick(&state);
        for (emissionIndex = 0;
             emissionIndex < state.lastTickResult.emissionCount;
             ++emissionIndex) {
            const struct TickEmission_Compat *emission =
                &state.lastTickResult.emissions[emissionIndex];
            if (emission->kind == EMIT_CREATURE_ATTACK &&
                emission->payload[0] == 169) {
                ++repeatedGroup169Attacks;
            }
        }
    }
    if (state.world.lifecycle.lastCreatureAttackTime <=
            attackAfterFirstEncounter || repeatedGroup169Attacks == 0) {
        fprintf(stderr,
            "FAIL: source group #169 did not attack again in 21 seconds "
            "(attacks=%d group=%u hp=%u->%u gameTick=%u->%u "
            "attackTick=%u->%u)\n",
            repeatedGroup169Attacks,
            (unsigned int)state.world.things->groups[169].health[0],
            (unsigned int)hpAfterFirstEncounter,
            (unsigned int)state.world.party.champions[0].hp.current,
            (unsigned int)encounterTickStart,
            (unsigned int)state.world.gameTick,
            (unsigned int)attackAfterFirstEncounter,
            (unsigned int)state.world.lifecycle.lastCreatureAttackTime);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    printf("PASS: authentic PC34 first-door repeated natural monster attacks and combat input naturalDamage=%u partyShield=%ld\n",
           naturalDamage, partyShieldDefense);
    return 0;
}
