#!/usr/bin/env python3
"""Re-base the leave_and_reenter portrait probes (rond 2).

Seed pose (1,2) NORTH -> real (7,9) NORTH in front of the (7,8)
S-face HALK sensor (seeded to the probe's target ordinal, unchanged
helper).  Leave target (1,3) SOUTH -> real (7,13) SOUTH in front of
the (7,14) N-face ZED sensor (sensorData=10, matches the shipped
ZED_ORDINAL=10 expectation).  The single forward step becomes four
forward steps through the open hall column x=7.
"""
import sys
from pathlib import Path

ROOT = Path("/Volumes/Extern-disk/firestaff-work/probes/m11")
FILES = [
    "firestaff_dm1_v1_hall_of_champions_portrait_02_leave_and_reenter_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_06_leave_and_reenter_portrait_rect_position_runtime_probe.c",
    "firestaff_dm1_v1_hall_of_champions_portrait_19_leave_and_reenter_portrait_rect_position_runtime_probe.c",
]

ENUM_OLD = """    LEAVE_TARGET_MAPX = 1,
    LEAVE_TARGET_MAPY = 3,"""
ENUM_NEW = """    LEAVE_TARGET_MAPX = 7,
    LEAVE_TARGET_MAPY = 13,
    /* Forward steps from the (7,9) seed pose to the (7,13) leave
     * target through the open hall column x=7: (7,10) -> (7,11) ->
     * (7,12) -> (7,13). */
    LEAVE_STEP_COUNT = 4,"""

LEAVE_LEG_OLD = """    stepResLeave = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "leave step UP (forward SOUTH) result=%d, party at "
                 "(%d, %d) dir=%d (expected (%d, %d) dir=%d, the "
                 "(1,3) corridor cell)",
                 (int)stepResLeave,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction,
                 LEAVE_TARGET_MAPX, LEAVE_TARGET_MAPY, DIR_SOUTH);
        CHECK(stepResLeave == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == LEAVE_TARGET_MAPX &&
              state.world.party.mapY == LEAVE_TARGET_MAPY &&
              state.world.party.direction == DIR_SOUTH, msg);
    }"""
LEAVE_LEG_NEW = """    for (leaveStep = 0; leaveStep < (int)LEAVE_STEP_COUNT; ++leaveStep) {
        stepResLeave = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "leave step %d/%d UP (forward SOUTH) result=%d, "
                     "party at (%d, %d) dir=%d",
                     leaveStep + 1, (int)LEAVE_STEP_COUNT,
                     (int)stepResLeave,
                     state.world.party.mapX, state.world.party.mapY,
                     state.world.party.direction);
            CHECK(stepResLeave == M11_GAME_INPUT_REDRAW, msg);
        }
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after %d leave steps party at (%d, %d) dir=%d "
                 "(expected (%d, %d) dir=%d, the (7,13) hall cell in "
                 "front of the (7,14) N-face ZED mirror)",
                 (int)LEAVE_STEP_COUNT,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction,
                 LEAVE_TARGET_MAPX, LEAVE_TARGET_MAPY, DIR_SOUTH);
        CHECK(state.world.party.mapX == LEAVE_TARGET_MAPX &&
              state.world.party.mapY == LEAVE_TARGET_MAPY &&
              state.world.party.direction == DIR_SOUTH, msg);
    }"""

RETURN_LEG_OLD = """    stepResReturn = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "return step UP (forward NORTH) result=%d, party at "
                 "(%d, %d) dir=%d (expected (%d, %d) dir=%d, the "
                 "seeded (1,2) NORTH cell)",
                 (int)stepResReturn,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction,
                 SEED_POSE_MAPX, SEED_POSE_MAPY, DIR_NORTH);
        /* The (1,3) -> (1,2) NORTH step may be blocked by the
         * DM1 V1 one-way corridor layout.  We accept either
         * outcome here: if walkable, the party returns to
         * (1,2) NORTH; if blocked, the party stays at (1,3)
         * NORTH and we fall back to a direct field-set
         * teleport for the reentry framebuffer.  The
         * invariants below test the C127 sensor state, not
         * the movement path. */
        CHECK(stepResReturn == M11_GAME_INPUT_REDRAW, msg);
    }"""
RETURN_LEG_NEW = """    for (returnStep = 0; returnStep < (int)LEAVE_STEP_COUNT; ++returnStep) {
        stepResReturn = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "return step %d/%d UP (forward NORTH) result=%d, "
                     "party at (%d, %d) dir=%d (heading for (%d, %d) "
                     "dir=%d, the seeded (7,9) NORTH cell)",
                     returnStep + 1, (int)LEAVE_STEP_COUNT,
                     (int)stepResReturn,
                     state.world.party.mapX, state.world.party.mapY,
                     state.world.party.direction,
                     SEED_POSE_MAPX, SEED_POSE_MAPY, DIR_NORTH);
            /* The (7,13) -> (7,9) NORTH steps cross the open hall;
             * if any step is blocked the party stays put and the
             * field-set teleport fallback below restores the seed
             * pose.  The invariants below test the C127 sensor
             * state, not the movement path. */
            CHECK(stepResReturn == M11_GAME_INPUT_REDRAW, msg);
        }
        if (state.world.party.mapX == SEED_POSE_MAPX &&
            state.world.party.mapY == SEED_POSE_MAPY) {
            break;
        }
    }"""

DECL_OLD = "    M11_GameInputResult stepResLeave, stepResReturn;"
DECL_NEW = ("    M11_GameInputResult stepResLeave, stepResReturn;\n"
            "    int leaveStep;\n    int returnStep;")

# count-asserted code replacements (exactly once per file)
CODE_SUBS = [
    ("    SEED_POSE_MAPX = 1,\n    SEED_POSE_MAPY = 2,",
     "    SEED_POSE_MAPX = 7,\n    SEED_POSE_MAPY = 9,"),
    (ENUM_OLD, ENUM_NEW),
    (LEAVE_LEG_OLD, LEAVE_LEG_NEW),
    (RETURN_LEG_OLD, RETURN_LEG_NEW),
    (DECL_OLD, DECL_NEW),
]

# comment / message replacements (expected once; warn only)
TEXT_SUBS = [
    ("(1,2) NORTH-route front square (1,1)",
     "(7,9) NORTH-route front square (7,8)"),
    ("(1,2) NORTH seed pose is the",
     "(7,9) NORTH seed pose is the"),
    ("""The walkable leave target.  (1,3) is the canonical
     * walkaround cell (savegame start per
     * firestaff_m11_hall_walkaround_runtime_probe); from (1,2)
     * NORTH the canonical leave is TURN_RIGHT (face EAST) +
     * TURN_RIGHT (face SOUTH) + UP (forward SOUTH) -> (1,3)
     * SOUTH.  The (1,3) SOUTH pose has a C127 sensor with
     * sensorData=10 (ZED) per actual_pose_runtime_probe, so the""",
     """The walkable leave target.  (7,13) SOUTH is the real
     * front-mirror pose for the (7,14) N-face C127 sensor on the
     * verified PC34 layout; from (7,9)
     * NORTH the leave is TURN_RIGHT (face EAST) +
     * TURN_RIGHT (face SOUTH) + 4x UP (forward SOUTH) -> (7,13)
     * SOUTH through the open hall column x=7.  The (7,13) SOUTH
     * pose faces a C127 sensor with
     * sensorData=10 (ZED) on the real PC34 layout, so the"""),
    ("""/* The ZED ordinal is what DM1 V1 DUNGEON.DAT ships at the
     * (1,3) SOUTH pose (front=(1,4) C127 sensor data=10).
     * Expected at the leave target as a sanity check that the
     * C127 sensor on the south wall is alive and the
     * (1,2)->(1,3) step really moved the party. */""",
     """/* The ZED ordinal is what the real PC34 DUNGEON.DAT ships at
     * the (7,13) SOUTH pose (front=(7,14) N-face C127 sensor
     * data=10).  Expected at the leave target as a sanity check
     * that the C127 sensor on the hall wall is alive and the
     * (7,9)->(7,13) steps really moved the party. */"""),
    ("""/* C2.1: walk the party to the (1,3) corridor cell.  From
     * (1,2) facing NORTH, the canonical walkable leave is:
     *   TURN_RIGHT (face EAST) -> TURN_RIGHT (face SOUTH) -> UP.
     * The forward step from SOUTH goes to (1,3) SOUTH.  Per
     * the walkaround_runtime_probe the (1,3) cell is reachable
     * from the savegame start and the (1,3) -> (1,4) SOUTH step
     * proves the (1,y) corridor is walkable south.  The reverse
     * (1,2) -> (1,3) step is the input interleave that proves
     * the leave_and_reenter movement path. */""",
     """/* C2.1: walk the party to the (7,13) hall cell.  From
     * (7,9) facing NORTH, the leave is:
     *   TURN_RIGHT (face EAST) -> TURN_RIGHT (face SOUTH) -> 4x UP.
     * The forward steps from SOUTH go (7,10) -> (7,11) -> (7,12)
     * -> (7,13) SOUTH through the open hall column x=7.  (7,13)
     * SOUTH faces the (7,14) N-face C127 ZED sensor on the real
     * PC34 layout.  The (7,9) -> (7,13) steps are the input
     * interleave that proves the leave_and_reenter movement path. */"""),
    ("""Render at the leave target.  The (1,3) SOUTH pose in real
     * DM1 V1 DUNGEON.DAT has the (1,4) front square with a C127
     * sensor (sensorData=10, ZED per actual_pose_runtime_probe).""",
     """Render at the leave target.  The (7,13) SOUTH pose in the
     * real PC34 DUNGEON.DAT has the (7,14) front square with a
     * C127 sensor (sensorData=10, ZED)."""),
    ("alive - the (1,2)->(1,3) step really moved the party)",
     "alive - the (7,9)->(7,13) steps really moved the party)"),
    ("the (1,4) front square", "the (7,14) front square"),
    ("""/* C2.2: walk the party back to (1,2) facing NORTH.  The
     * symmetric leave is:
     *   TURN_RIGHT (face WEST) -> TURN_RIGHT (face NORTH) -> UP.
     * The UP step from (1,3) NORTH goes to (1,2) NORTH, which
     * is the seeded C127 sensor pose.  Note: this assumes the
     * (1,3) -> (1,2) NORTH step is walkable; if the DM1 V1
     * corridor is one-way (some canonical corridor cells are
     * enter-only from the start) the probe degrades to a
     * direct field-set teleport and reports the
     * leave_and_reenter invariants on the reentry framebuffer. */""",
     """/* C2.2: walk the party back to (7,9) facing NORTH.  The
     * symmetric return is:
     *   TURN_RIGHT (face WEST) -> TURN_RIGHT (face NORTH) -> 4x UP.
     * The UP steps from (7,13) NORTH go (7,12) -> (7,11) -> (7,10)
     * -> (7,9) NORTH, which is the seeded C127 sensor pose.  If a
     * return step is blocked the probe degrades to a direct
     * field-set teleport and reports the leave_and_reenter
     * invariants on the reentry framebuffer. */"""),
    ("""/* (1,3) -> (1,2) NORTH is blocked (one-way corridor):
         * teleport back via direct field-set.""",
     """/* (7,13) -> (7,9) NORTH return was blocked:
         * teleport back via direct field-set."""),
    ("teleport back to (1,2) NORTH to exercise the ",
     "teleport back to (7,9) NORTH to exercise the "),
    ("on the same seeded (1,2) NORTH pose without movement",
     "on the same seeded (7,9) NORTH pose without movement"),
]

def main():
    ok = True
    for name in FILES:
        path = ROOT / name
        text = path.read_text()
        for old, new in CODE_SUBS:
            c = text.count(old)
            if c != 1:
                print(f"ERROR {name}: code pattern {old[:50]!r} count={c}")
                ok = False
                continue
            text = text.replace(old, new)
        for old, new in TEXT_SUBS:
            c = text.count(old)
            if c != 1:
                print(f"WARN {name}: text pattern {old[:50]!r} count={c}")
            text = text.replace(old, new)
        path.write_text(text)
        rest12 = text.count("(1,2)")
        print(f"updated {name} (remaining '(1,2)': {rest12})")
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main())
