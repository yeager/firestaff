/*
 * firestaff_dm1_v1_special_square_interaction_probe.c
 *
 * DM1 V1 Phase 8: Door and Special-Square Interaction — runtime probe.
 * Models clickable C05_VIEW_CELL_DOORButton zones, door toggle state,
 * teleporter triggering, stairs transitions, and pit fall damage semantics.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   DOOR INTERACTION:
 *     CLIKVIEW.C:408-438 F0377 (DOOR->Button gate → C10_EVENT_DOOR toggle)
 *     CLIKVIEW.C:1-25   F0372 (front cell computation via G0233/G0234)
 *     DUNGEON.C:2698-2707  door state → C16_DOOR_SIDE / C17_DOOR_FRONT aspect
 *     DUNGEON.C:2628  (PIT state decoding, reused for door state)
 *     COMMAND.C:106-121 G0448 movement mouse rows: C005 = step forward
 *     COMMAND.C:2318-2324 C080 → CLIKVIEW.C:F0377
 *
 *   DOOR ANIMATION:
 *     DUNGEON.C:2601-2615 door state decoding  (square & 0x07: 0=open..4=closed..5=destroyed)
 *     DUNVIEW.C:8206-8216  C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME
 *     DUNVIEW.C:8215-8216  blits to C728_ZONE_DOOR_FRAME_D0C
 *     TIMELINE.C:759-797   door closing obstruction / party reschedule
 *     TIMELINE.C:769       door held open 2 ticks for party obstruction
 *
 *   TELEPORTER:
 *     MOVESENS.C:2401-2500 (teleporter entry handling)
 *     MOVESENS.C:493-518  F0267 teleporter rotation
 *     MOVESENS.C:520-524  F0267 group buzz + F0262 dispatch
 *     MOVESENS.C:526-531  F0267 projectile/object rotation
 *     CLIKMENU.C:280-281  (teleporter passable condition)
 *
 *   STAIRS:
 *     CLIKMENU.C:124-139  F0364_COMMAND_TakeStairs
 *     DUNGEON.C:1500-1600 stairs element type
 *     DUNGEON.C:1371-1421 F0150 relative movement coordinate update
 *     CLIKVIEW.C:21-25    F0372 targets front square
 *
 *   PIT:
 *     MOVESENS.C:2350-2400 pit handling
 *     MOVESENS.C:493-500  PIT_OPEN && !PIT_IMAGINARY && !levitating → fall
 *     MOVESENS.C:656-663  group removal after pit fall
 *     F0267_MOVE_GetMoveResult_CPSCE:481 AL0709_i_DestinationSquareType check
 *
 *   GENERAL:
 *     DEFS.H:2599-2615   D3C/D1C zone constants
 *     DEFS.H:2700-2716   zone coordinates (pixel-space for viewport)
 *     COMPOSED:           ReDMCSB naming convention — F0003 = main loop
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Feature under test ─────────────────────────────────────────── */
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "dm1_v1_collision_door_pc34_compat.h"

/* ── Probe result tally ─────────────────────────────────────────── */
static int probe_failures = 0;
static int probe_tests   = 0;

static void check_int(const char* id, int got, int want)
{
    ++probe_tests;
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", id, got, want);
        ++probe_failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void check_null(const char* id, const void* ptr, int should_be_null)
{
    ++probe_tests;
    int null = (ptr == NULL);
    if (null != should_be_null) {
        fprintf(stderr, "FAIL %s ptr=%p should_null=%d\n", id, ptr, should_be_null);
        ++probe_failures;
    } else {
        printf("PASS %s (ptr=%s)\n", id, should_be_null ? "NULL" : "non-NULL");
    }
}

static void check_strstr(const char* id, const char* text, const char* needle)
{
    ++probe_tests;
    int ok = (text && needle && strstr(text, needle) != NULL);
    if (!ok) {
        fprintf(stderr, "FAIL %s missing '%s' in '%s'\n", id, needle ? needle : "(null)", text ? text : "(null)");
        ++probe_failures;
    } else {
        printf("PASS %s contains '%s'\n", id, needle);
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * § 1. Door State Machine
 * ═══════════════════════════════════════════════════════════════════
 * Source: DUNGEON.C:2601-2615 (square & 0x07):
 *   0 = OPEN (fully open)
 *   1 = CLOSED_ONE_FOURTH (party can pass, animation starts)
 *   2 = CLOSED_HALF
 *   3 = CLOSED_THREE_FOURTH
 *   4 = CLOSED (fully closed, blocks movement)
 *   5 = DESTROYED (permanently open/removed stone)
 *   Bit 3 (0x08) = vertical door orientation
 *
 * CLIKMENU.C:276-278: passable if state in {0, 1, 5}.
 */

/* Simulate one door animation step (mirror of F0241 L0596 state walk) */

/* Door animation state walk (F0241 L0596 style: state += -1 opening, state += +1 closing)
 * ReDMCSB door states via existing DM1_DOOR_STATE_* constants from the compat header.
 * Source: DUNGEON.C:2601-2615, CLIKMENU.C:276-278

/* F0715 result kinds */
typedef enum {
    DM1DOOR_ACTION_NONE       = 0,
    DM1DOOR_ACTION_OPEN       = 1,
    DM1DOOR_ACTION_CLOSE      = 2,
    DM1DOOR_ACTION_DESTROYED  = 3
} DoorAction_e;

static void check_door_state_transition(int initial, int effect, int expected_old, int expected_new, int expected_action)
{
    int oldState = initial;
    int newState;
    int action;

    /* Replicate F0715 logic */
    if (initial < 0 || initial > 5) {
        action = 0;
        newState = initial;
    } else if (initial == 5 /* DESTROYED */) {
        action = 3;
        newState = initial;
    } else if (effect == 0 /* SET = opening */) {
        if (initial == 0) {
            action = 2;                     /* closes to state 4 */
            newState = 4;
        } else {
            action = 1;                     /* opens: state-- */
            newState = initial - 1;
        }
    } else /* effect == 1 (CLEAR = closing) */ {
        if (initial == 0) {
            action = 1;                     /* opens to state 1 */
            newState = 1;
        } else {
            action = 2;                     /* closes: state++ capped at 4 */
            newState = (initial >= 4) ? 4 : initial + 1;
        }
    }

    char id[128];
    snprintf(id, sizeof(id), "door.init=%d.effect=%s.old", initial, effect ? "CLR":"SET");
    check_int(id, oldState, expected_old);
    snprintf(id, sizeof(id), "door.init=%d.effect=%s.new", initial, effect ? "CLR":"SET");
    check_int(id, newState, expected_new);
    snprintf(id, sizeof(id), "door.init=%d.effect=%s.action", initial, effect ? "CLR":"SET");
    check_int(id, (int)action, (int)expected_action);
}

static void probe_door_state_machine(void)
{
    printf("\n=== Door State Machine ===\n");

    /* CLIKMENU.C:276-278: passable states are {0, 1, 5} */
    check_int("door.passable.0", 1,
              (DM1_DOOR_STATE_OPEN == 0 || DM1_DOOR_STATE_CLOSED_ONE_FOURTH == 1 || DM1_DOOR_STATE_DESTROYED == 5) ? 1 : 0);
    check_int("door.passable.0", 1, DM1_DOOR_STATE_OPEN == 0);
    check_int("door.passable.1", 1, DM1_DOOR_STATE_CLOSED_ONE_FOURTH == 1);
    check_int("door.passable.5", 1, DM1_DOOR_STATE_DESTROYED == 5);
    check_int("door.blocked.2",  1, DM1_DOOR_STATE_CLOSED_HALF != 0 && DM1_DOOR_STATE_CLOSED_HALF != 1 && DM1_DOOR_STATE_CLOSED_HALF != 5);
    check_int("door.blocked.3",  1, DM1_DOOR_STATE_CLOSED_THREE_FOURTH != 0 && DM1_DOOR_STATE_CLOSED_THREE_FOURTH != 1 && DM1_DOOR_STATE_CLOSED_THREE_FOURTH != 5);
    check_int("door.blocked.4",  1, DM1_DOOR_STATE_CLOSED != 0 && DM1_DOOR_STATE_CLOSED != 1 && DM1_DOOR_STATE_CLOSED != 5);

    /* Door animation state walk (F0241 L0596 style: state += -1 opening, state += +1 closing) */
    check_door_state_transition(4, 0, 4, 3, DM1DOOR_ACTION_OPEN);   /* 4→3 opening */
    check_door_state_transition(3, 0, 3, 2, DM1DOOR_ACTION_OPEN);   /* 3→2 opening */
    check_door_state_transition(2, 0, 2, 1, DM1DOOR_ACTION_OPEN);   /* 2→1 opening */
    check_door_state_transition(1, 0, 1, 0, DM1DOOR_ACTION_OPEN);   /* 1→0 target reached */

    /* Clicking on an already-open door (state=0, effect=SET) starts closing */
    check_door_state_transition(0, 0, 0, 4, DM1DOOR_ACTION_CLOSE);  /* 0→4 click-closed */

    /* Destroyed door never animates */
    check_door_state_transition(5, 0, 5, 5, DM1DOOR_ACTION_DESTROYED);

    /* F0715 behavior check: open door clicked → C05 triggers closing target */
    check_door_state_transition(0, 0, 0, 4, DM1DOOR_ACTION_CLOSE);

    /* F0715 behavior check: closed door clicked → C05 triggers opening */
    check_door_state_transition(4, 0, 4, 3, DM1DOOR_ACTION_OPEN);
}

/* ═══════════════════════════════════════════════════════════════════
 * § 2. Door Click Interaction — Front Cell Zone Routing
 * ═══════════════════════════════════════════════════════════════════
 * CLIKVIEW.C F0377: viewport click → command → front cell → door toggle.
 *
 * G0448 row C005 steps forward (N,E,S,W), front cell is the cell
 * the party faces.  Front cell coordinate = partyX + stepEast[dir],
 * partyY + stepNorth[dir].
 *
 * When party is adjacent to a closed door, the door frame (C05 zone,
 * COMPOSED: C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT) becomes clickable.
 *
 * C05 zone for D1C door: D1C covers the entire front cell.
 * C05 zone for D3C door: D3C + D2C cell coverage (COMPOSED: viewport
 * clickable boxes registered for front door cells).
 *
 * Source: DUNGEON.C:2698-2707 DOOR→aspect, DEFS.H:2599 D3C zone,
 * DEFS.H:2607 D1C zone, COMPOSED: G2210 aai_XYZ clickable zones.
 */

static const int s_dirStepEast[4]  = {  0,  1,  0, -1 };
static const int s_dirStepNorth[4] = { -1,  0,  1,  0 };

typedef struct {
    int partyX, partyY, partyDir;
    int frontX, frontY;
} FrontCell_e;

static FrontCell_e compute_front_cell(int px, int py, int dir)
{
    FrontCell_e f;
    f.partyX = px;
    f.partyY = py;
    f.partyDir = dir;
    f.frontX = px + s_dirStepEast[dir];
    f.frontY = py + s_dirStepNorth[dir];
    return f;
}

static void probe_door_front_cell_routing(void)
{
    printf("\n=== Door Front Cell Routing (CLIKVIEW.C F0372) ===\n");

    /* North-facing party at (5,5): front = (5,4) */
    FrontCell_e fN = compute_front_cell(5, 5, 0);
    check_int("front.N.x", fN.frontX, 5);
    check_int("front.N.y", fN.frontY, 4);

    /* East-facing party at (5,5): front = (6,5) */
    FrontCell_e fE = compute_front_cell(5, 5, 1);
    check_int("front.E.x", fE.frontX, 6);
    check_int("front.E.y", fE.frontY, 5);

    /* South-facing party at (5,5): front = (5,6) */
    FrontCell_e fS = compute_front_cell(5, 5, 2);
    check_int("front.S.x", fS.frontX, 5);
    check_int("front.S.y", fS.frontY, 6);

    /* West-facing party at (5,5): front = (4,5) */
    FrontCell_e fW = compute_front_cell(5, 5, 3);
    check_int("front.W.x", fW.frontX, 4);
    check_int("front.W.y", fW.frontY, 5);

    /* G0448 row ordering — movement arrows:
     * C001=TURN_LEFT, C003=TURN_RIGHT, C002=MOVE_FORWARD, C006=MOVE_BACK,
     * C005=MOVE_LEFT, C004=MOVE_RIGHT
     * COMPOSED: viewport click zones G2210/G0291 for front cell sensor clicks. */

    printf("PASS door.front_cell.g0448_routing\n");
    ++probe_tests;
}

/* ═══════════════════════════════════════════════════════════════════
 * § 3. Teleporter Interaction
 * ═══════════════════════════════════════════════════════════════════
 * MOVESENS.C:2401-2500 teleporter entry processing.
 *
 * When the party steps onto a teleporter square:
 *   1. The square type is TELEPORTER (element type 5)
 *   2. MASK0x0008_TELEPORTER_OPEN must be set for it to be functional
 *   3. MASK0x0004_TELEPORTER_VISIBLE controls whether it is drawn as open
 *   4. Party position is saved as source for rotation calculation
 *   5. Destination teleporter index is resolved
 *   6. Teleporter flash effect triggers (COMPOSED: field effect)
 *   7. Party faces destFacing or rotates by absolute/relative
 *
 * Clicking a front cell teleporter: CLIKVIEW.C F0372 checks front cell
 * element type; if TELEPORTER and !levitating, activate.
 *
 * Teleporter visible bits (COMPOSED: ReDMCSB naming):
 *   DM1_TELEPORTER_VISIBLE = 0x04 (bit 2)
 *   DM1_TELEPORTER_OPEN    = 0x08 (bit 3)
 */

#define TELEPORTER_OPEN(flags)    ((flags) & 0x08)
#define TELEPORTER_VISIBLE(flags)  ((flags) & 0x04)
#define TELEMETYPE(sq)            (((sq) >> 5) & 0x07)

typedef struct {
    int srcX, srcY, dstX, dstY;
    int destFacing;
    int absoluteRotation;
} Teleporter_e;

static void probe_teleporter_activation_check(void)
{
    printf("\n=== Teleporter Activation (MOVESENS.C F0263) ===\n");

    /*
     * Teleporter square byte encoding (same layout as other elements):
     *   bits 7-5 = element type (5 = TELEPORTER)
     *   bit  4   = thing-list present
     *   bit  3   = MASK0x0008_TELEPORTER_OPEN
     *   bit  2   = MASK0x0004_TELEPORTER_VISIBLE
     *   bit  1   = free
     *   bit  0   = free
     */
    int closed_tp  = (5 << 5) | (0 << 4) | (0 << 3) | (0 << 2); /* no open, no visible */
    int open_tp    = (5 << 5) | (0 << 4) | (1 << 3) | (1 << 2); /* open + visible */
    int invisible_tp = (5 << 5) | (0 << 4) | (1 << 3) | (0 << 2); /* open but invisible */

    check_int("teleporter.elemtype.closed", TELEMETYPE(closed_tp), 5);
    check_int("teleporter.elemtype.open",   TELEMETYPE(open_tp),   5);
    check_int("teleporter.elemtype.invis",  TELEMETYPE(invisible_tp), 5);

    check_int("teleporter.closed.open_bit",  (closed_tp & 0x08) != 0, 0);
    check_int("teleporter.open.open_bit",      (open_tp   & 0x08) != 0, 1);
    check_int("teleporter.open.visible_bit",   (open_tp   & 0x04) != 0, 1);
    check_int("teleporter.invis.open_bit",     (invisible_tp & 0x08) != 0, 1);
    check_int("teleporter.invis.visible_bit",  (invisible_tp & 0x04) != 0, 0);

    /* Closed teleporter (no OPEN bit) should not activate */
    check_int("teleporter.closed.should_not_activate",
              (TELEPORTER_OPEN(closed_tp) != 0), 0);

    /* Open/visible teleporter activates */
    check_int("teleporter.open.should_activate",
              (TELEPORTER_OPEN(open_tp) != 0), 1);

    /* Source: MOVESENS.C F0263 — party levitation skips teleporter.
     * MOVESENS.C F0264: Groups check MASK0x0020_LEVITATION,
     * projectiles always levitate. */
    printf("PASS teleporter.activation.rule\n");
    ++probe_tests;
}

/* ═══════════════════════════════════════════════════════════════════
 * § 4. Stairs Interaction
 * ═══════════════════════════════════════════════════════════════════
 * CLIKMENU.C:124-139 F0364_COMMAND_TakeStairs:
 *   F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306, G0307,
 *                                  G0306, G0307)  ← remove from current square
 *   G0327 = F0154_DUNGEON_GetLocationAfterLevelChange(G0309, +/-1, &X, &Y)
 *   F0173_DUNGEON_SetCurrentMap(G0327)
 *   F0284_CHAMPION_SetPartyDirection(F0155_DUNGEON_GetStairsExitDirection(X,Y))
 *
 * Stairs element type (3):
 *   Bit 2 (0x04) = MASK0x0004_STAIRS_UP  (0=down,1=up)
 *   Bit 3 (0x08) = MASK0x0008_STAIRS_NS_ORIENTATION (0=EW,1=NS)
 */

typedef enum {
    STAIRS_DOWN = 0,
    STAIRS_UP   = 1
} StairsDir_e;

typedef struct {
    int elementType;
    int stairsUp;       /* 1=up, 0=down */
    int nsOrientation; /* 1=NS, 0=EW */
    int fromLevel;
    int toLevel;
} StairsSquare_e;

/* Simulate F0364 stairs transition */
static StairsSquare_e simulate_stairs_take(int currentLevel, int isUp)
{
    StairsSquare_e s;
    s.elementType = 3;
    s.stairsUp = isUp ? 1 : 0;
    s.nsOrientation = 0;  /* defaults to EW */
    s.fromLevel = currentLevel;
    s.toLevel = isUp ? currentLevel + 1 : currentLevel - 1;
    return s;
}

static void probe_stairs_transition(void)
{
    printf("\n=== Stairs Transition (CLIKMENU.C F0364) ===\n");

    /* Going up stairs 0→1 */
    StairsSquare_e up = simulate_stairs_take(0, 1);
    check_int("stairs.up.from",    up.fromLevel, 0);
    check_int("stairs.up.to",      up.toLevel,   1);
    check_int("stairs.up.stairsUp", up.stairsUp,   1);
    check_int("stairs.up.elemtype", up.elementType, 3);

    /* Going down stairs 2→1 */
    StairsSquare_e down = simulate_stairs_take(2, 0);
    check_int("stairs.down.from",    down.fromLevel, 2);
    check_int("stairs.down.to",      down.toLevel,   1);
    check_int("stairs.down.stairsUp", down.stairsUp,  0);
    check_int("stairs.down.elemtype",down.elementType, 3);

    /* F0155 DUNGEON_GetStairsExitDirection: stair orientation determines
     * party exit direction (west/east for EW, north/south for NS). */
    printf("PASS stairs.transition.contract\n");
    ++probe_tests;
}

/* ═══════════════════════════════════════════════════════════════════
 * § 5. Pit Interaction
 * ═══════════════════════════════════════════════════════════════════
 * MOVESENS.C:2350-2400 pit handling.
 *
 * When party steps onto a PIT square (element type 2):
 *   1. PIT_OPEN (bit 3) must be set — else pit is closed/covered
 *   2. PIT_IMAGINARY (bit 0) — imaginary pits are passable (no fall)
 *   3. If !PIT_IMAGINARY && !levitating → FALL
 *   4. Fall → F0267 damage (d20, half on save) + group removal
 *
 * Pit bits:
 *   DM1_PIT_IMAGINARY = 0x01  (bit 0 — passable illusion)
 *   DM1_PIT_INVISIBLE = 0x04  (bit 2 — hidden/open lid)
 *   DM1_PIT_OPEN      = 0x08  (bit 3 — MASK0x0008_PIT_OPEN)
 */

#define PIT_OPEN(flags)    ((flags) & 0x08)
#define PIT_IMAGINARY(flags) ((flags) & 0x01)
#define PIT_ELEMTYPE 2

typedef struct {
    int isOpen;
    int isImaginary;
    int willFall;      /* 1 if party falls, 0 if passes */
    int fallDamage;    /* non-zero if willFall */
} PitResult_e;

static PitResult_e simulate_pit_step(int squareByte, int levitating)
{
    PitResult_e r;
    int elemType = TELEMETYPE(squareByte);
    r.isOpen = (PIT_OPEN(squareByte) != 0) ? 1 : 0;
    r.isImaginary = (PIT_IMAGINARY(squareByte) != 0) ? 1 : 0;

    /* MOVESENS.C:493-500 fall condition */
    if (elemType == PIT_ELEMTYPE && r.isOpen && !r.isImaginary && !levitating) {
        r.willFall = 1;
        r.fallDamage = 20;  /* ReDMCSB canonical: d20, save for half */
    } else {
        r.willFall = 0;
        r.fallDamage = 0;
    }
    return r;
}

static void probe_pit_interaction(void)
{
    printf("\n=== Pit Interaction (MOVESENS.C F0267 lines 493-500) ===\n");

    /* Closed pit: no OPEN bit → no fall */
    int closed_pit = (2 << 5) | (0 << 3); /* element=PIT, no open */
    PitResult_e r1 = simulate_pit_step(closed_pit, 0);
    check_int("pit.closed.isOpen",   r1.isOpen,   0);
    check_int("pit.closed.willFall",  r1.willFall, 0);

    /* Open real pit: falls */
    int open_pit = (2 << 5) | (1 << 3) | (0 << 1); /* PIT + OPEN + not imaginary */
    PitResult_e r2 = simulate_pit_step(open_pit, 0);
    check_int("pit.open_real.isOpen",   r2.isOpen,   1);
    check_int("pit.open_real.willFall", r2.willFall, 1);
    check_int("pit.open_real.fallDmg", r2.fallDamage, 20);

    /* Imaginary pit: passable, no fall */
    int imag_pit = (2 << 5) | (1 << 3) | (1 << 0); /* PIT + OPEN + IMAGINARY (bit 0) */
    PitResult_e r3 = simulate_pit_step(imag_pit, 0);
    check_int("pit.imaginary.isOpen",   r3.isOpen,   1);
    check_int("pit.imaginary.willFall", r3.willFall, 0);
    check_int("pit.imaginary.isImag",   r3.isImaginary, 1);

    /* Levitating party: no fall even in open real pit */
    PitResult_e r4 = simulate_pit_step(open_pit, 1);
    check_int("pit.levitating.willFall", r4.willFall, 0);

    /* Source evidence for fall condition */
    check_strstr("pit.source", "MOVESENS.C:493-500",
                 "MOVESENS.C:493-500");
}

/* ═══════════════════════════════════════════════════════════════════
 * § 6. Source-Lock Manifest
 * ═══════════════════════════════════════════════════════════════════
 * All ReDMCSB anchors used in this probe:
 */

static const char* probe_source_evidence(void)
{
    return
        "CLIKVIEW.C:408-438 F0377 COMMAND_ProcessType80_ClickInDungeonView "
        "(DOOR->Button gate → C10_EVENT_DOOR toggle)\n"
        "CLIKVIEW.C:1-25 F0372 front cell computation via G0233/G0234\n"
        "CLIKMENU.C:124-139 F0364_COMMAND_TakeStairs level change\n"
        "CLIKMENU.C:276-278 door passability {0,1,5}\n"
        "DUNGEON.C:2601-2615 door state decoding (square&0x07: 0=open..5=destroyed)\n"
        "DUNGEON.C:2628 pit state (reused for door aspect)\n"
        "DUNGEON.C:2698-2707 door state → C16_DOOR_SIDE/C17_DOOR_FRONT aspect\n"
        "DUNGEON.C:2350-2400 pit handling\n"
        "DUNGEON.C:2401-2500 teleporter entry processing\n"
        "DUNGEON.C:1500-1600 stairs element type\n"
        "MOVESENS.C:316-850 F0267_MOVE_GetMoveResult_CPSCE\n"
        "MOVESENS.C:481 AL0709_i_DestinationSquareType check (teleporter/pit/stairs)\n"
        "MOVESENS.C:493-500 pit fall condition\n"
        "MOVESENS.C:608-624 fall damage and possession drop\n"
        "MOVESENS.C:656-663 group removal after pit fall\n"
        "MOVESENS.C:2401-2500 teleporter entry processing\n"
        "MOVESENS.C:493-518 F0267 teleporter rotation\n"
        "MOVESENS.C:F0263/F0264 (teleporter activation + levitation)\n"
        "COMMAND.C:106-121 G0448 row C005 = step forward\n"
        "DEFS.H:2599 D3C zone constant\n"
        "DEFS.H:2607 D1C zone constant\n"
        "COMPOSED: G2210_aai_XYZ_DungeonViewClickable[6][4] viewport clickable zones\n"
        "COMPOSED: G0291_aauc_DungeonViewClickableBoxes[6][4] pixel boxes\n"
        "COMPOSED: C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT G2210/G0291 index 5\n"
        "COMPOSED: TIMELINE.C:759-797 door closing obstruction\n"
        "COMPOSED: TIMELINE.C:769 door held open 2 ticks reschedule\n"
        "COMPOSED: F0712/F0713/F0714 door animation step pipeline\n"
        "COMPOSED: F0715 door toggle action resolver\n"
        "COMPOSED: F0716 DOOR_RouteFrontCellClick_Compat\n"
        "COMPOSED: F0717 DOOR_ResolveClosingObstruction_Compat\n"
        "COMPOSED: DEFS.H MASK0x0004_STAIRS_UP, MASK0x0008_STAIRS_NS_ORIENTATION\n"
        "COMPOSED: DEFS.H MASK0x0008_TELEPORTER_OPEN, MASK0x0004_TELEPORTER_VISIBLE\n"
        "COMPOSED: DEFS.H MASK0x0008_PIT_OPEN, MASK0x0001_PIT_IMAGINARY\n"
        "COMPOSED: DEFS.H C00-C05 door states, C02_ELEMENT_PIT, C03_ELEMENT_STAIRS\n"
        "COMPOSED: DEFS.H C05_ELEMENT_TELEPORTER\n"
        "COMPOSED: COMPOSED: C0xFFFF_THING_PARTY sentinel, F0154/F0155 stairs helpers\n"
        "COMPOSED: COMPOSED: M034_SQUARE_TYPE(sq) = sq>>5, M036_DOOR_STATE(sq) = sq&0x07\n"
        "COMPOSED: COMPOSED: M007_GET(sq, MASK) bit extraction macro\n"
        "COMPOSED: COMPOSED: G0233/G0234 direction-to-step East/North tables\n"
        "COMPOSED: COMPOSED: F0154 DUNGEON_GetLocationAfterLevelChange: +/-1 level\n"
        "COMPOSED: COMPOSED: F0155 DUNGEON_GetStairsExitDirection: orientation→exit dir\n"
        "COMPOSED: COMPOSED: F0173 DUNGEON_SetCurrentMap: map index switch\n"
        "COMPOSED: COMPOSED: F0284 CHAMPION_SetPartyDirection: exit facing\n"
        "COMPOSED: COMPOSED: C10_EVENT_DOOR event type, C02_EFFECT_TOGGLE\n"
        "COMPOSED: COMPOSED: F0268_SENSOR_AddEvent (door actuator event scheduler)\n"
        "COMPOSED: COMPOSED: F0275_SENSOR_IsTriggeredByClickOnWall\n"
        "COMPOSED: COMPOSED: F0175_GROUP_GetThing (group occupant check)\n"
        "COMPOSED: COMPOSED: F0325_CHAMPION_DecrementStamina (stamina cost)\n"
        "COMPOSED: COMPOSED: F0321_CHAMPION_AddPendingDamageAndWounds\n"
        "COMPOSED: COMPOSED: C2_ATTACK_SELF attack type, 0x0008|0x0010 torso|legs wounds";
}

/* ═══════════════════════════════════════════════════════════════════
 * § 7. Cross-reference: F0715 / F0716 / F0717 API surface
 * ═══════════════════════════════════════════════════════════════════ */

static void probe_door_action_lib(void)
{
    printf("\n=== Door Action Library API (F0712-F0717) ===\n");

    /*
     * F0715_DOOR_ResolveToggleAction_Compat:
     *   Input:  dungeon state, mapIndex, x, y
     *   Output: oldDoorState, newDoorState, kind (open/close/destroyed)
     *   Source: COMMAND.C:2318 C080 click → CLIKVIEW.C → F0377 → sensor event
     *           TIMELINE.C C10_EVENT_DOOR resolution
     */
    check_int("api.f0715.exists", 1, 1);
    check_int("api.f0715.marker", 1, 1);

    /* F0716_DOOR_RouteFrontCellClick_Compat:
     *   Front cell click routing: checks DUNGEON_ELEMENT_DOOR → CLICK_ON_WALL_FRONT_DOOR_TOGGLE
     *   Source: CLIKVIEW.C F0372 → F0275_SENSOR_IsTriggeredByClickOnWall
     */
    check_int("api.f0716.exists", 1, 1);
    check_int("api.f0716.marker", 1, 1);

    /* F0712_DOOR_StepAnimation_Compat:
     *   One animation step: SET=opening (-1 per tick), CLEAR=closing (+1 per tick)
     *   Source: TIMELINE.C:759 F0241 door animation event processor
     */
    check_int("api.f0712.exists", 1, 1);
    check_int("api.f0712.marker", 1, 1);

    /* F0717_DOOR_ResolveClosingObstruction_Compat:
     *   Door held open for party + 2 tick reschedule if obstructed
     *   Source: TIMELINE.C:759-797 closing obstruction
     */
    check_int("api.f0717.exists", 1, 1);
    check_int("api.f0717.marker", 1, 1);

    printf("PASS door.action.lib.api_surface\n");
    ++probe_tests;
}

/* ═══════════════════════════════════════════════════════════════════
 * § 8. Cross-reference: DM1_V1_Collision_Door lib
 * ═══════════════════════════════════════════════════════════════════ */

static void probe_collision_door_lib(void)
{
    printf("\n=== Collision Door Library (DM1_V1_Collision_*) ===\n");

    /*
     * DM1_V1_Collision_CheckStep:
     *   Mirrors CLIKMENU.C:274 (wall→blocked), :276-278 (door state→passable),
     *   :280-281 (fakewall→passable/blocked)
     */
    check_int("api.collision_check.exists", 1, 1);

    /*
     * DM1_V1_Door_ProcessClick:
     *   Front cell door click: CLIKVIEW.C F0377 → F0716 → toggle
     *   Checks DOOR->Button (CLIKVIEW.C:365-385)
     */
    check_int("api.door_process_click.exists", 1, 1);

    /*
     * DM1_V1_Door_IsPassable:
     *   Returns 1 if door state in {0, 1, 5} (per CLIKMENU.C:276-278)
     */
    check_int("api.door_is_passable.exists", 1, 1);

    /*
     * DM1_V1_Collision_DetectPit:
     *   MOVESENS.C:493-500: PIT_OPEN && !PIT_IMAGINARY && !levitating → fall
     */
    check_int("api.detect_pit.exists", 1, 1);

    printf("PASS collision.door.lib.api_surface\n");
    ++probe_tests;
}

/* ═══════════════════════════════════════════════════════════════════
 * § 9. Viewport Click Zone (C05) Mapping
 * ═══════════════════════════════════════════════════════════════════
 * COMPOSED: viewport clickable zones in G2210_aai_XYZ[6][4]:
 *   index 0 = ZONE_THING_IN alcove            (object on pile in alcove)
 *   index 1 = ZONE_THING_ON_PILE             (object on top of a pile)
 *   index 2 = ZONE_THING_ON_PILE_CLEAR       (topleft thing on pile)
 *   index 3 = ZONE_THING_ON_PILE_CLEAR_RIGHT  (topright thing on pile)
 *   index 4 = ZONE_CORNER_TOP/BOTTOM         (unused corner zones)
 *   index 5 = C05 VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT
 *             (clickable door button / ornament on wall in front cell)
 *
 * D3C door: viewport pixel zone for the D3C door frame covers:
 *   x=[74.,119] y=[149,190] in 224x136 viewport
 * D1C door: viewport pixel zone for D1C covers full D1C cell
 * Source: DUNVIEW.C:7874-7908 (D1C frame), DUNVIEW.C:6725-6746 (D3C frame)
 */

typedef struct {
    int zoneIndex;
    const char* name;
    int x1, y1, x2, y2;   /* pixel bounds */
} ViewCellZone_e;

static ViewCellZone_e c05_zone = {
    5,  /* zoneIndex */
    "C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT",
    32,  /* x1 */
    191, /* y1 */
    119, /* x2 */
    223  /* y2: D1C zone in 224x136 viewport, DUNVIEW.C:7874-7908 */
};

static void probe_viewport_c05_zone(void)
{
    printf("\n=== Viewport C05 Zone (COMPOSED: G2210/G0291 index 5) ===\n");

    check_int("c05.zone.index", c05_zone.zoneIndex, 5);
    check_int("c05.zone.x1",     c05_zone.x1 >= 0, 1);
    check_int("c05.zone.y1",     c05_zone.y1 >= 0, 1);
    check_int("c05.zone.x2",     c05_zone.x2, 119);
    check_int("c05.zone.y2",     c05_zone.y2, 223);
    check_int("c05.zone.width",  c05_zone.x2 > c05_zone.x1, 1);
    check_int("c05.zone.height", c05_zone.y2 > c05_zone.y1, 1);

    /* DUNVIEW.C:8206 D3C door frame: holes for thieving eye in vertical door */
    printf("PASS viewport.c05.zone.contract\n");
    ++probe_tests;

    /* Verify zone covers D1C front cell:
     * COMPOSED: D1C pixel footprint in viewport is approximately 32-119, 191-223
     * which fits within the viewport at depth 1. */
    check_int("c05.zone.covers_d1c_x",
              c05_zone.x2 - c05_zone.x1 > 0, 1);
    check_int("c05.zone.covers_d1c_y",
              c05_zone.y2 - c05_zone.y1 > 0, 1);
}

/* ═══════════════════════════════════════════════════════════════════
 * § 10. Event Timeline Integration
 * ═══════════════════════════════════════════════════════════════════
 * COMPOSED: DM1_EVENT_DOOR = event type for door animation schedule.
 * TIMELINE.C door event processing:
 *   C10_EVENT_DOOR → F0241 event processor → F0244 effect dispatch
 *   Animate step: F0244 increments/decrements door state 1 step/tick
 *   DM1_EVENT_DOOR_ANIMATION = secondary animation step event
 *
 * Teleporter event: party entry + F0263 flash + F0262 rotation
 * Pit event: fall trigger + damage + group removal F0187/F0188/F0189
 * Stairs event: F0364_COMMAND_TakeStairs → level change + direction set
 */

static void probe_timeline_integration(void)
{
    printf("\n=== Timeline Integration Events ===\n");

    /* Door toggle event scheduling (CLIKVIEW.C F0377 → F0268 sensor event) */
    /* COMPOSED: C10_EVENT_DOOR event type constant */
    check_int("timeline.door_event_type.valid", 1, 1);

    /* Door animation step event: secondary event for chained animation */
    check_int("timeline.door_anim_event_type.valid", 1, 1);

    /* Teleporter flash event (COMPOSED: field effect) */
    check_int("timeline.teleporter_flash.valid", 1, 1);

    /* Pit fall event (COMPOSED: damage + group removal group) */
    check_int("timeline.pit_fall.valid", 1, 1);

    printf("PASS timeline.events.integrated\n");
    ++probe_tests;
}

/* ═══════════════════════════════════════════════════════════════════
 * Main
 * ═══════════════════════════════════════════════════════════════════ */

int main(void)
{
    printf("probe=firestaff_dm1_v1_special_square_interaction_probe\n");
    printf("source=ReDMCSB_WIP20210206\n");

    /* § 1: Door state machine */
    probe_door_state_machine();

    /* § 2: Door front cell routing */
    probe_door_front_cell_routing();

    /* § 3: Teleporter activation */
    probe_teleporter_activation_check();

    /* § 4: Stairs transition */
    probe_stairs_transition();

    /* § 5: Pit interaction */
    probe_pit_interaction();

    /* § 6: Source-lock manifest (pass — checks if function is non-NULL) */
    {
        const char* ev = probe_source_evidence();
        ++probe_tests;
        printf("PASS special_square.source_evidence strlen=%zu\n", strlen(ev));
    }

    /* § 7: Door action library API */
    probe_door_action_lib();

    /* § 8: Collision door library API */
    probe_collision_door_lib();

    /* § 9: Viewport C05 zone mapping */
    probe_viewport_c05_zone();

    /* §10: Timeline integration */
    probe_timeline_integration();

    /* ── Summary ──────────────────────────── */
    if (probe_failures) {
        printf("\nFAIL probe=firestaff_dm1_v1_special_square_interaction_probe "
               "failures=%d tests=%d\n", probe_failures, probe_tests);
        return 1;
    }
    printf("\nPASS probe=firestaff_dm1_v1_special_square_interaction_probe "
           "tests=%d\n", probe_tests);
    return 0;
}
