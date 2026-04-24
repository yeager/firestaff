/*
 * Pass 38 bounded probe — animating door states (1..3) via compat
 * timeline ownership.
 *
 * Verifies the Pass-38 owners introduced in
 * `memory_door_action_pc34_compat.c` and the TIMELINE_EVENT_DOOR_ANIMATE
 * branch of `F0887_ORCH_DispatchTimelineEvents_Compat`:
 *
 *   - F0714_DOOR_ResolveAnimationEffect_Compat resolves SET/CLEAR/TOGGLE
 *     against the current door state, mirroring the TOGGLE branch of
 *     F0244_TIMELINE_ProcessEvent10_Square_Door in TIMELINE.C.
 *   - F0713_DOOR_BuildAnimationEvent_Compat emits a well-formed
 *     TIMELINE_EVENT_DOOR_ANIMATE event with fireAtTick = startTick
 *     and aux1 carrying the effect.
 *   - F0712_DOOR_StepAnimation_Compat walks a door square through
 *     states 4 -> 3 -> 2 -> 1 -> 0 (SET) and 0 -> 1 -> 2 -> 3 -> 4
 *     (CLEAR), mirroring the ` += -1 / += +1` branches of
 *     F0241_TIMELINE_ProcessEvent1_DoorAnimation.  DESTROYED (state 5)
 *     never animates.
 *   - End-to-end: once an animation event is scheduled, repeated
 *     F0884_ORCH_AdvanceOneTick_Compat calls drive the square through
 *     every intermediate state with a DOOR_STATE emission per step,
 *     reschedule internally, and stop at the final state with no
 *     further DOOR_ANIMATE events in the queue.
 *
 * Mirrors the Pass-31 probe harness pattern: build a tiny in-memory
 * dungeon fixture, call the functions under test, and record PASS/FAIL
 * lines with a summary count.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_door_action_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#define MAP_W 4
#define MAP_H 4

static int g_pass = 0;
static int g_fail = 0;

static unsigned char sqb(int elementType, int attribs) {
    return (unsigned char)(((elementType & 7) << 5) | (attribs & 0x1F));
}

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static void build_fixture(struct DungeonDatState_Compat* dungeon,
                          unsigned char* squareData) {
    int c, r;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(squareData, 0, (size_t)MAP_W * MAP_H);
    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(struct DungeonMapDesc_Compat));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(struct DungeonMapTiles_Compat));
    dungeon->maps[0].width = MAP_W;
    dungeon->maps[0].height = MAP_H;
    dungeon->tiles[0].squareData = squareData;
    dungeon->tiles[0].squareCount = MAP_W * MAP_H;
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;

    for (c = 0; c < MAP_W; ++c) {
        for (r = 0; r < MAP_H; ++r) {
            squareData[c * MAP_H + r] = sqb(DUNGEON_ELEMENT_WALL, 0);
        }
    }

    /* Door squares, initial low-3-bit state in brackets:
     *   (0,1) DOOR open           [0]
     *   (1,1) DOOR closed         [4]
     *   (2,1) DOOR destroyed      [5]
     *   (3,1) DOOR vertical half  [2] (animating; vertical bit 0x08 set)
     */
    squareData[0 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 0);
    squareData[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 4);
    squareData[2 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 5);
    squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 0x0A /* vert | state 2 */);
    squareData[0 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
}

static void free_fixture(struct DungeonDatState_Compat* dungeon) {
    free(dungeon->maps);
    free(dungeon->tiles);
    memset(dungeon, 0, sizeof(*dungeon));
}

static int square_state(const struct DungeonDatState_Compat* d, int map, int mx, int my) {
    return d->tiles[map].squareData[mx * d->maps[map].height + my] & 0x07;
}

int main(void) {
    struct DungeonDatState_Compat dungeon;
    unsigned char squareData[MAP_W * MAP_H];
    int effect, curState;
    int rc;
    struct TimelineEvent_Compat ev;
    struct DoorAnimationStep_Compat step;

    build_fixture(&dungeon, squareData);

    /* ================================================================
     *  F0714 — effect resolution from SET/CLEAR/TOGGLE
     * ================================================================ */

    effect = -1; curState = -1;
    rc = F0714_DOOR_ResolveAnimationEffect_Compat(&dungeon, 0, 1, 1,
             DOOR_EFFECT_TOGGLE, &effect, &curState);
    record("P38_F0714_TOGGLE_CLOSED",
           rc == 1 && effect == DOOR_EFFECT_SET && curState == 4,
           "closed door (state=4) TOGGLE resolves to SET (opening)");

    effect = -1; curState = -1;
    rc = F0714_DOOR_ResolveAnimationEffect_Compat(&dungeon, 0, 0, 1,
             DOOR_EFFECT_TOGGLE, &effect, &curState);
    record("P38_F0714_TOGGLE_OPEN",
           rc == 1 && effect == DOOR_EFFECT_CLEAR && curState == 0,
           "open door (state=0) TOGGLE resolves to CLEAR (closing)");

    effect = -1; curState = -1;
    rc = F0714_DOOR_ResolveAnimationEffect_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_TOGGLE, &effect, &curState);
    record("P38_F0714_TOGGLE_ANIMATING",
           rc == 1 && effect == DOOR_EFFECT_SET && curState == 2,
           "animating mid-state door TOGGLE resolves to SET (opening) and reports current state");

    effect = -1; curState = -1;
    rc = F0714_DOOR_ResolveAnimationEffect_Compat(&dungeon, 0, 2, 1,
             DOOR_EFFECT_TOGGLE, &effect, &curState);
    record("P38_F0714_DESTROYED",
           rc == 0 && curState == 5,
           "destroyed door (state=5) returns 0 and does not animate");

    /* Already-at-target on explicit SET / CLEAR -> return 0 (no animation). */
    effect = -1;
    rc = F0714_DOOR_ResolveAnimationEffect_Compat(&dungeon, 0, 0, 1,
             DOOR_EFFECT_SET, &effect, NULL);
    record("P38_F0714_SET_ALREADY_OPEN",
           rc == 0 && effect == DOOR_EFFECT_SET,
           "explicit SET on an already-OPEN door returns 0 (no animation needed)");
    effect = -1;
    rc = F0714_DOOR_ResolveAnimationEffect_Compat(&dungeon, 0, 1, 1,
             DOOR_EFFECT_CLEAR, &effect, NULL);
    record("P38_F0714_CLEAR_ALREADY_CLOSED",
           rc == 0 && effect == DOOR_EFFECT_CLEAR,
           "explicit CLEAR on an already-CLOSED door returns 0 (no animation needed)");

    /* ================================================================
     *  F0713 — event builder
     * ================================================================ */

    memset(&ev, 0, sizeof(ev));
    rc = F0713_DOOR_BuildAnimationEvent_Compat(0, 1, 1, DOOR_EFFECT_SET,
                                               42u, &ev);
    record("P38_F0713_BUILD_SET",
           rc == 1 &&
               ev.kind == TIMELINE_EVENT_DOOR_ANIMATE &&
               ev.fireAtTick == 42u &&
               ev.mapIndex == 0 && ev.mapX == 1 && ev.mapY == 1 &&
               ev.aux0 == -1 && ev.aux1 == DOOR_EFFECT_SET,
           "F0713 builds a well-formed DOOR_ANIMATE event for SET at fireAtTick=startTick");

    memset(&ev, 0, sizeof(ev));
    rc = F0713_DOOR_BuildAnimationEvent_Compat(0, 0, 1, DOOR_EFFECT_CLEAR,
                                               7u, &ev);
    record("P38_F0713_BUILD_CLEAR",
           rc == 1 &&
               ev.aux1 == DOOR_EFFECT_CLEAR &&
               ev.fireAtTick == 7u,
           "F0713 builds a DOOR_ANIMATE event for CLEAR at fireAtTick=startTick");

    /* Invalid effect / coords rejected. */
    rc = F0713_DOOR_BuildAnimationEvent_Compat(0, 0, 1, DOOR_EFFECT_TOGGLE,
                                               0u, &ev);
    record("P38_F0713_REJECT_TOGGLE",
           rc == 0,
           "F0713 rejects TOGGLE effect (caller must resolve it via F0714 first)");

    rc = F0713_DOOR_BuildAnimationEvent_Compat(-1, 0, 0, DOOR_EFFECT_SET,
                                               0u, &ev);
    record("P38_F0713_REJECT_OOB",
           rc == 0,
           "F0713 rejects negative mapIndex");

    /* ================================================================
     *  F0712 — stepper walks (pure / mutating)
     * ================================================================ */

    /* SET on closed door: 4 -> 3 (ADVANCED). */
    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 1, 1,
             DOOR_EFFECT_SET, 0 /* do not mutate */, &step);
    record("P38_F0712_SET_4TO3_PURE",
           rc == 1 &&
               step.kind == DOOR_ANIM_STEP_ADVANCED &&
               step.oldDoorState == 4 && step.newDoorState == 3 &&
               square_state(&dungeon, 0, 1, 1) == 4 /* unchanged */,
           "SET step from 4 computes newState=3 without mutating the square when mutateSquare=0");

    /* SET on animating state (2): 2 -> 1 (ADVANCED) and apply. */
    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_SET, 1 /* mutate */, &step);
    record("P38_F0712_SET_2TO1_MUTATING",
           rc == 1 &&
               step.kind == DOOR_ANIM_STEP_ADVANCED &&
               step.oldDoorState == 2 && step.newDoorState == 1 &&
               step.doorVertical == 1 &&
               square_state(&dungeon, 0, 3, 1) == 1,
           "SET step from 2 mutates square to state 1 and reports doorVertical");

    /* SET step from 1 -> 0 (REACHED_TARGET). */
    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_SET, 1 /* mutate */, &step);
    record("P38_F0712_SET_1TO0_TARGET",
           rc == 1 &&
               step.kind == DOOR_ANIM_STEP_REACHED_TARGET &&
               step.oldDoorState == 1 && step.newDoorState == 0 &&
               square_state(&dungeon, 0, 3, 1) == 0,
           "SET step from 1 reaches OPEN target; kind=REACHED_TARGET and square ends at state 0");

    /* Further SET at target is a no-op with REACHED_TARGET. */
    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_SET, 1, &step);
    record("P38_F0712_SET_AT_TARGET_NOOP",
           rc == 1 &&
               step.kind == DOOR_ANIM_STEP_REACHED_TARGET &&
               step.oldDoorState == 0 && step.newDoorState == 0,
           "further SET on an OPEN door is a no-op with kind=REACHED_TARGET");

    /* Re-seed a closed door and walk CLEAR from 0 -> 4 step by step. */
    squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 0x08 /* vert | state 0 */);

    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_CLEAR, 1, &step);
    record("P38_F0712_CLEAR_0TO1",
           rc == 1 &&
               step.kind == DOOR_ANIM_STEP_ADVANCED &&
               step.oldDoorState == 0 && step.newDoorState == 1 &&
               square_state(&dungeon, 0, 3, 1) == 1,
           "CLEAR step from 0 walks to state 1 (closing)");

    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_CLEAR, 1, &step);
    record("P38_F0712_CLEAR_1TO2",
           rc == 1 && step.kind == DOOR_ANIM_STEP_ADVANCED &&
               step.newDoorState == 2 &&
               square_state(&dungeon, 0, 3, 1) == 2,
           "CLEAR step from 1 walks to state 2 (closing)");

    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_CLEAR, 1, &step);
    record("P38_F0712_CLEAR_2TO3",
           rc == 1 && step.kind == DOOR_ANIM_STEP_ADVANCED &&
               step.newDoorState == 3 &&
               square_state(&dungeon, 0, 3, 1) == 3,
           "CLEAR step from 2 walks to state 3 (closing)");

    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 3, 1,
             DOOR_EFFECT_CLEAR, 1, &step);
    record("P38_F0712_CLEAR_3TO4_TARGET",
           rc == 1 && step.kind == DOOR_ANIM_STEP_REACHED_TARGET &&
               step.newDoorState == 4 &&
               square_state(&dungeon, 0, 3, 1) == 4,
           "CLEAR step from 3 reaches CLOSED target (4)");

    /* DESTROYED never animates. */
    memset(&step, 0, sizeof(step));
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 2, 1,
             DOOR_EFFECT_SET, 1, &step);
    record("P38_F0712_DESTROYED_NOOP",
           rc == 1 && step.kind == DOOR_ANIM_STEP_NO_CHANGE &&
               step.oldDoorState == 5 && step.newDoorState == 5 &&
               square_state(&dungeon, 0, 2, 1) == 5,
           "DESTROYED door never animates (NO_CHANGE, square unchanged)");

    /* Null out / invalid effect rejected. */
    rc = F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 1, 1,
             DOOR_EFFECT_TOGGLE, 0, &step);
    record("P38_F0712_REJECT_TOGGLE",
           rc == 0,
           "F0712 rejects unresolved TOGGLE effect");
    record("P38_F0712_NULL_OUT",
           F0712_DOOR_StepAnimation_Compat(&dungeon, 0, 1, 1,
             DOOR_EFFECT_SET, 0, NULL) == 0,
           "F0712 rejects NULL outStep");

    /* ================================================================
     *  End-to-end through F0884_ORCH_AdvanceOneTick_Compat
     *
     *  Scheduled DOOR_ANIMATE event should walk the door through
     *  intermediate states 1..3 across four ticks from CLOSED -> OPEN
     *  with a DOOR_STATE emission per step and a rattle SOUND on every
     *  non-final step.
     * ================================================================ */
    {
        struct GameWorld_Compat* world = F0880_WORLD_AllocDefault_Compat();
        struct TimelineEvent_Compat animEv;
        struct TickInput_Compat input;
        struct TickResult_Compat tick;
        int i;
        int doorStateEmissions = 0;
        int rattleEmissions = 0;
        int lastStateSeen = -1;

        F0881_WORLD_InitDefault_Compat(world, 1u);

        /* Re-seed the fixture's door to CLOSED (state 4) and wire the
         * world's dungeon pointer to it.  We deliberately do not copy
         * the fixture; the world is stand-alone to keep this a clean
         * runtime probe.
         */
        squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 4);
        world->dungeon = &dungeon;

        /* Schedule an animation event to fire on this exact tick. */
        F0713_DOOR_BuildAnimationEvent_Compat(0, 3, 1, DOOR_EFFECT_SET,
                                              world->gameTick, &animEv);
        F0721_TIMELINE_Schedule_Compat(&world->timeline, &animEv);

        /* Run four ticks; each tick should advance one state
         *   t0: 4 -> 3 (ADVANCED, reschedule)
         *   t1: 3 -> 2 (ADVANCED, reschedule)
         *   t2: 2 -> 1 (ADVANCED, reschedule)
         *   t3: 1 -> 0 (REACHED_TARGET, no reschedule)
         */
        for (i = 0; i < 4; ++i) {
            memset(&input, 0, sizeof(input));
            input.command = CMD_NONE;
            input.tick = world->gameTick;
            F0884_ORCH_AdvanceOneTick_Compat(world, &input, &tick);
            {
                int j;
                for (j = 0; j < tick.emissionCount; ++j) {
                    const struct TickEmission_Compat* e = &tick.emissions[j];
                    if (e->kind == EMIT_DOOR_STATE) {
                        doorStateEmissions++;
                        lastStateSeen = e->payload[2];
                    }
                    if (e->kind == EMIT_SOUND_REQUEST && e->payload[0] == 2) {
                        rattleEmissions++;
                    }
                }
            }
        }

        record("P38_E2E_STATE_PROGRESSION",
               square_state(&dungeon, 0, 3, 1) == 0,
               "four orchestrator ticks walk CLOSED door through 3,2,1,0 via the dispatcher");

        record("P38_E2E_DOOR_STATE_EMISSION_COUNT",
               doorStateEmissions == 4,
               "four DOOR_STATE emissions observed (one per intermediate step plus the final)");

        record("P38_E2E_RATTLE_COUNT",
               rattleEmissions == 3,
               "three rattle SOUND_REQUEST emissions observed (one per non-final step)");

        record("P38_E2E_FINAL_EMISSION_IS_OPEN",
               lastStateSeen == 0,
               "final DOOR_STATE emission carries state 0 (OPEN)");

        /* After the walk completes, no further DOOR_ANIMATE event
         * should linger in the queue. */
        {
            struct TimelineEvent_Compat peek;
            int peekRc = F0722_TIMELINE_Peek_Compat(&world->timeline, &peek);
            record("P38_E2E_NO_LINGERING_EVENT",
                   peekRc == 0 ||
                       (peek.kind != TIMELINE_EVENT_DOOR_ANIMATE),
                   "no TIMELINE_EVENT_DOOR_ANIMATE remains after final step");
        }

        /* One more tick should not mutate the door (idempotent at target). */
        memset(&input, 0, sizeof(input));
        input.command = CMD_NONE;
        F0884_ORCH_AdvanceOneTick_Compat(world, &input, &tick);
        record("P38_E2E_IDEMPOTENT_AT_TARGET",
               square_state(&dungeon, 0, 3, 1) == 0,
               "further tick after target leaves the door at OPEN (no regress)");

        /* Detach the in-fixture dungeon before freeing the world so
         * F0883 does not double-free our stack-ish fixture. */
        world->dungeon = NULL;
        world->ownsDungeon = 0;
        F0883_WORLD_Free_Compat(world);
        free(world);
    }

    /* ================================================================
     *  End-to-end CLEAR walk: OPEN -> CLOSED across four ticks.
     * ================================================================ */
    {
        struct GameWorld_Compat* world = F0880_WORLD_AllocDefault_Compat();
        struct TimelineEvent_Compat animEv;
        struct TickInput_Compat input;
        struct TickResult_Compat tick;
        int i;
        int stateSequence[4] = { -1, -1, -1, -1 };
        int seqLen = 0;

        F0881_WORLD_InitDefault_Compat(world, 2u);

        squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 0);
        world->dungeon = &dungeon;

        F0713_DOOR_BuildAnimationEvent_Compat(0, 3, 1, DOOR_EFFECT_CLEAR,
                                              world->gameTick, &animEv);
        F0721_TIMELINE_Schedule_Compat(&world->timeline, &animEv);

        for (i = 0; i < 4; ++i) {
            memset(&input, 0, sizeof(input));
            input.tick = world->gameTick;
            F0884_ORCH_AdvanceOneTick_Compat(world, &input, &tick);
            {
                int j;
                for (j = 0; j < tick.emissionCount; ++j) {
                    const struct TickEmission_Compat* e = &tick.emissions[j];
                    if (e->kind == EMIT_DOOR_STATE && seqLen < 4) {
                        stateSequence[seqLen++] = e->payload[2];
                    }
                }
            }
        }

        record("P38_E2E_CLEAR_SEQUENCE",
               seqLen == 4 &&
                   stateSequence[0] == 1 &&
                   stateSequence[1] == 2 &&
                   stateSequence[2] == 3 &&
                   stateSequence[3] == 4,
               "closing walk emits states in strict 1,2,3,4 order (mirror of F0241 SET 0 -> CLOSED)");

        record("P38_E2E_CLEAR_FINAL_STATE",
               square_state(&dungeon, 0, 3, 1) == 4,
               "closing walk lands the square at state 4 (CLOSED)");

        world->dungeon = NULL;
        world->ownsDungeon = 0;
        F0883_WORLD_Free_Compat(world);
        free(world);
    }

    free_fixture(&dungeon);

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
