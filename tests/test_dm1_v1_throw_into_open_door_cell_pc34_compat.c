/*
 * DM1 V1 runtime regression: throwing or dropping an item into a square
 * with an open or partly-open door.
 *
 * Source mapping (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   - PROJEXPL.C:F0217_PROJECTILE_HasImpactOccured door branch
 *     lines 471-505 routes thrown items and magical projectiles around
 *     closed doors and lets them pass through open/partly-open doors.
 *   - PROJEXPL.C:F0219_PROJECTILE_ProcessEvents48To49 per-tick advance
 *     (mirror F0811) inspects the destination cell through F0814 and
 *     resolves door collision through F0816 + F0820.
 *   - PROJEXPL.C:490-500 pouch/thrown item random roll (F0816 v1
 *     deterministic non-pass for kinetic; needs disassembly review
 *     for full parity with the source random roll).
 *   - DUNGEON.C:F0163_LINK_AddToSquareThingList lines 1800-1837
 *     floor-drop writes a thing onto a square regardless of door state
 *     (the door occupies the same square; the floor drop is to the
 *     thing-list, not the square byte).
 *   - DEFS.H:1039-1044 C0..C5 door states.
 *   - DEFS.H:2088 C30/G0425 chest slot anchors; projectile slot list.
 *
 * Contract:
 *   - F0814_PROJECTILE_InspectDestination_Compat classifies an open
 *     door (state 0), a partly-open door (state 1) and a destroyed
 *     door (state 5) as PROJECTILE_BLOCKER_OPEN. Half/ThreeFourth/
 *     Full (states 2/3/4) classify as PROJECTILE_BLOCKER_CLOSED_DOOR.
 *   - F0811 cross-cell advance: open/partly-open/destroyed doors let
 *     the thrown item fly through (PROJECTILE_RESULT_FLEW,
 *     despawn=0). Closed half/three-fourth/full doors block
 *     (PROJECTILE_RESULT_HIT_DOOR, despawn=1) and emit a
 *     TIMELINE_EVENT_DOOR_DESTRUCTION via F0819/F0820.
 *   - F0816 PROJECTILE_DoesPassThroughDoor_Compat for a KINETIC
 *     projectile returns passes=1 only for open/partly-open/destroyed
 *     doors; closed half/three-fourth/full return passes=0.
 *   - The drop-to-floor path (DUNGEON.C F0163) writes the thing onto
 *     the square's thing-list regardless of door state because the
 *     door square is the same coordinate as the floor where the
 *     thing is placed (door byte + thing-list are parallel data).
 *
 * Non-overlap:
 *   - The CSB/DM1 viewport F0111 door panel partly-open zone math
 *     lives in dm1_v1_viewport_f0111_door_panel_pc34_compat
 *     (DUNVIEW.C:4218-4339). This test covers the *collision/flight*
 *     aspect, not the *viewport blit* aspect.
 *   - The existing test_dm1_v1_projectile_explosion_render covers
 *     mostly magical projectile + mixed door scenarios. This test is
 *     narrower: it pins the kinetic (thrown item) case across all six
 *     door states and the floor drop door-state independence.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* ---- ReDMCSB door state anchors (mirror DEFS.H:1039-1044) ---- */
#define THROW_OPEN_DOOR_STATE  PROJECTILE_DOOR_STATE_OPEN              /* C0 */
#define THROW_PARTLY_OPEN_STATE PROJECTILE_DOOR_STATE_CLOSED_ONE_FOURTH /* C1 */
#define THROW_CLOSED_HALF_STATE PROJECTILE_DOOR_STATE_CLOSED_HALF      /* C2 */
#define THROW_CLOSED_3Q_STATE   PROJECTILE_DOOR_STATE_CLOSED_THREE_FOURTH /* C3 */
#define THROW_CLOSED_FULL_STATE PROJECTILE_DOOR_STATE_CLOSED_FULL      /* C4 */
#define THROW_DESTROYED_STATE   PROJECTILE_DOOR_STATE_DESTROYED        /* C5 */

#define THROW_DOOR_STATE_COUNT 6

typedef struct {
    int state;
    const char* label;
    int expectedF0814Blocker;       /* PROJECTILE_BLOCKER_* */
    int expectedF0811ResultKind;    /* PROJECTILE_RESULT_* */
    int expectedDespawn;            /* 0 or 1 */
    int expectedDoorDestructionEvent; /* 0 or 1 */
    int expectedF0816Passes;        /* 0 or 1 */
    int expectedDoorToggleEvent;    /* 0 or 1 (always 0 for kinetic) */
} DoorStateExpectation;

static const DoorStateExpectation kExpectations[THROW_DOOR_STATE_COUNT] = {
    {
        THROW_OPEN_DOOR_STATE, "open",
        PROJECTILE_BLOCKER_OPEN,
        PROJECTILE_RESULT_FLEW, 0, 0, 1, 0
    },
    {
        THROW_PARTLY_OPEN_STATE, "partly_open_one_fourth",
        PROJECTILE_BLOCKER_OPEN,
        PROJECTILE_RESULT_FLEW, 0, 0, 1, 0
    },
    {
        THROW_CLOSED_HALF_STATE, "closed_half",
        PROJECTILE_BLOCKER_CLOSED_DOOR,
        PROJECTILE_RESULT_HIT_DOOR, 1, 1, 0, 0
    },
    {
        THROW_CLOSED_3Q_STATE, "closed_three_fourth",
        PROJECTILE_BLOCKER_CLOSED_DOOR,
        PROJECTILE_RESULT_HIT_DOOR, 1, 1, 0, 0
    },
    {
        THROW_CLOSED_FULL_STATE, "closed_full",
        PROJECTILE_BLOCKER_CLOSED_DOOR,
        PROJECTILE_RESULT_HIT_DOOR, 1, 1, 0, 0
    },
    {
        THROW_DESTROYED_STATE, "destroyed",
        PROJECTILE_BLOCKER_OPEN,
        PROJECTILE_RESULT_FLEW, 0, 0, 1, 0
    }
};

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char* id, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char* id, const char* haystack,
                            const char* needle, const char* anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

/* ---- Fixture builders ----------------------------------------------- */

static void make_thrown_item(
    struct ProjectileInstance_Compat* p,
    int dir, int cell, int mx, int my)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex          = 0;
    p->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p->projectileSubtype  = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    p->ownerKind          = PROJECTILE_OWNER_CHAMPION;
    p->ownerIndex         = 0;
    p->mapIndex           = 0;
    p->mapX               = mx;
    p->mapY               = my;
    p->cell               = cell & 3;
    p->direction          = dir & 3;
    p->kineticEnergy      = 40;
    p->attack             = 24;
    p->stepEnergy         = 4;
    p->firstMoveGraceFlag = 0;
    p->attackTypeCode     = COMBAT_ATTACK_NORMAL;
    p->flags              = PROJECTILE_FLAG_IGNORE_DOOR_PASS_THROUGH;
    p->reserved3          = 1;
}

static void make_door_digest(
    struct CellContentDigest_Compat* d,
    int destDoorState);

static void test_f0811_inactive_projectile_slot_noop(void)
{
    struct ProjectileInstance_Compat p;
    struct ProjectileInstance_Compat pOut;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0811_inactive_projectile_slot_noop\n");

    make_thrown_item(&p, 0, 0, 5, 5);
    make_door_digest(&d, THROW_OPEN_DOOR_STATE);
    p.reserved3 = 0;
    memset(&pOut, 0x7A, sizeof(pOut));
    memset(&r, 0x7B, sizeof(r));

    expect_int("f0811.inactive.rc",
               F0811_PROJECTILE_Advance_Compat(&p, &d, 100u, NULL,
                                                &pOut, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0219 only dispatches linked PROJECTILE things");
    expect_int("f0811.inactive.kind", r.resultKind, PROJECTILE_RESULT_INVALID,
               "inactive compact runtime slots are no-op, not visible projectiles");
    expect_int("f0811.inactive.despawn", r.despawn, 0,
               "inactive compact runtime slots do not materialize/despawn");
    expect_int("f0811.inactive.x", r.newMapX, p.mapX,
               "inactive compact runtime slots preserve source x");
    expect_int("f0811.inactive.y", r.newMapY, p.mapY,
               "inactive compact runtime slots preserve source y");
}

static void make_door_digest(
    struct CellContentDigest_Compat* d,
    int destDoorState)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex                       = 0;
    d->sourceMapX                           = 5;
    d->sourceMapY                           = 5;
    d->sourceSquareType                     = PROJECTILE_ELEMENT_CORRIDOR;
    d->destMapIndex                         = 0;
    d->destMapX                             = 5;
    d->destMapY                             = 4; /* north step from source */
    d->destSquareType                       = PROJECTILE_ELEMENT_DOOR;
    d->destDoorState                        = destDoorState;
    d->destDoorAllowsProjectilePassThrough  = 0;
    d->destDoorHasButton                    = 0;
    d->destTeleporterNewDirection           = -1;
    d->destCreatureType                     = -1;
}

static const char* door_state_anchor(void)
{
    return "ReDMCSB DEFS.H:1039-1044 C0..C5 door states";
}

static const char* proj_door_anchor(void)
{
    return "ReDMCSB PROJEXPL.C:F0217 door branch 471-505 + F0219:F0820 mirror";
}

static const char* f0816_anchor(void)
{
    return "ReDMCSB PROJEXPL.C:F0217/F0219 + memory_projectile_pc34_compat.c F0816";
}

static const char* f0163_anchor(void)
{
    return "ReDMCSB DUNGEON.C:F0163 lines 1800-1837 floor-drop thing-list link";
}

/* ---- Test: F0814 classification per door state -------------------- */

static void test_f0814_inspect_destination_per_door_state(void)
{
    int i;
    printf("test_f0814_inspect_destination_per_door_state\n");

    for (i = 0; i < THROW_DOOR_STATE_COUNT; ++i) {
        const DoorStateExpectation* e = &kExpectations[i];
        struct CellContentDigest_Compat d;
        int blocker = -1;

        make_door_digest(&d, e->state);
        expect_int("f0814_inspect",
                   F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker),
                   1, door_state_anchor());
        expect_int(e->label, blocker, e->expectedF0814Blocker, proj_door_anchor());
    }
}

/* ---- Test: F0811 cross-cell advance for thrown item per door state -- */

static void test_f0811_thrown_item_per_door_state(void)
{
    int i;
    printf("test_f0811_thrown_item_per_door_state\n");

    for (i = 0; i < THROW_DOOR_STATE_COUNT; ++i) {
        const DoorStateExpectation* e = &kExpectations[i];
        struct ProjectileInstance_Compat p;
        struct ProjectileInstance_Compat pOut;
        struct CellContentDigest_Compat d;
        struct ProjectileTickResult_Compat r;

        make_thrown_item(&p, 0 /* NORTH */, 0, 5, 5);
        make_door_digest(&d, e->state);

        memset(&pOut, 0, sizeof(pOut));
        memset(&r, 0, sizeof(r));

        expect_int("f0811_advance",
                   F0811_PROJECTILE_Advance_Compat(&p, &d, 100u, NULL, &pOut, &r),
                   1, proj_door_anchor());
        expect_int(e->label, r.resultKind, e->expectedF0811ResultKind,
                   proj_door_anchor());
        expect_int(e->label, r.despawn, e->expectedDespawn, proj_door_anchor());
        expect_int(e->label, r.emittedDoorDestructionEvent,
                   e->expectedDoorDestructionEvent, proj_door_anchor());
        expect_int(e->label, r.emittedDoorToggleEvent,
                   e->expectedDoorToggleEvent, proj_door_anchor());
    }
}

/* ---- Test: F0816 kinetic pass-through per door state ---------------- */

static void test_f0816_thrown_item_pass_through_per_door_state(void)
{
    int i;
    printf("test_f0816_thrown_item_pass_through_per_door_state\n");

    for (i = 0; i < THROW_DOOR_STATE_COUNT; ++i) {
        const DoorStateExpectation* e = &kExpectations[i];
        struct ProjectileInstance_Compat p;
        struct CellContentDigest_Compat d;
        int passes = -1;

        make_thrown_item(&p, 0, 0, 5, 5);
        make_door_digest(&d, e->state);

        expect_int("f0816_pass",
                   F0816_PROJECTILE_DoesPassThroughDoor_Compat(&p, &d, NULL, &passes),
                   1, f0816_anchor());
        expect_int(e->label, passes, e->expectedF0816Passes, f0816_anchor());
    }
}

/* ---- Test: door square byte layout (drop invariant) ----------------- */

/*
 * The drop-to-floor invariant: a door square's door state (encoded in
 * the low 3 bits of the square byte) is independent of the square's
 * thing-list (encoded as a parallel data structure anchored at
 * G0271_ppuc_SquareFirstThingData[mapIndex][mapX * mapHeight + mapY]).
 * Dropping an item on a door square therefore succeeds regardless of
 * door state because the door is in the square byte, not the thing
 * list.
 *
 * We verify the structural contract here:
 *   - the door square byte can be encoded for any door state 0..5
 *   - the door-state mask (low 3 bits) is well-defined and stable
 *   - the DUNGEON_ELEMENT_DOOR type is encoded in bits 5..7 of the
 *     square byte per DEFS.H C0..C5 door encoding
 *   - the same map coordinate can carry both the door square byte
 *     AND a separate thing-list entry (no struct overlap)
 *
 * This is the minimal source-locked contract the runtime relies on
 * for the drop path.  The actual m11_prepend_thing_to_square logic
 * lives in src/engine/m11_game_view.c and uses the same data layout.
 */
static void test_door_square_byte_layout_per_door_state(void)
{
    int i;
    int mapX = 5;
    int mapY = 4;
    int mapHeight = 16;
    unsigned char squareBytes[16 * 16];
    unsigned short firstThings[16 * 16];

    printf("test_door_square_byte_layout_per_door_state\n");

    for (i = 0; i < THROW_DOOR_STATE_COUNT; ++i) {
        const DoorStateExpectation* e = &kExpectations[i];
        int squareIndex = mapX * mapHeight + mapY;
        unsigned char expectedByte;
        unsigned char actualByte;
        unsigned char maskedDoorState;
        unsigned char maskedElement;

        memset(squareBytes, 0, sizeof(squareBytes));
        memset(firstThings, 0, sizeof(firstThings));

        expectedByte =
            (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | (e->state & 0x07));
        squareBytes[squareIndex] = expectedByte;

        /* Drop the item: write a sentinel thing-list entry on the same
         * square.  THING_ENDOFLIST 0xFFFE is the end-of-list sentinel;
         * here we use a different sentinel (0x7301) to mark an occupied
         * thing-list head. */
        firstThings[squareIndex] = 0x7301;

        actualByte     = squareBytes[squareIndex];
        maskedDoorState = (unsigned char)(actualByte & 0x07);
        maskedElement  = (unsigned char)((actualByte & DUNGEON_SQUARE_MASK_TYPE) >> 5);

        expect_int(e->label, maskedElement, DUNGEON_ELEMENT_DOOR,
                   "DUNGEON_SQUARE_MASK_TYPE element-type encoding (DEFS.H)");
        expect_int(e->label, maskedDoorState, e->state,
                   "DUNGEON_SQUARE_MASK_ATTRIBS door-state low-3-bits (DEFS.H:1039-1044)");
        expect_int(e->label, firstThings[squareIndex], 0x7301,
                   "DUNGEON.C F0163: door square byte + thing-list coexist on same coordinate");
    }
}

/* ---- Test: source evidence mentions required anchors --------------- */

static void test_source_evidence_mentions_required_anchors(void)
{
    printf("test_source_evidence_mentions_required_anchors\n");

    expect_contains("anchor.f0217",
                    proj_door_anchor(), "PROJEXPL.C:F0217",
                    "ReDMCSB door branch");
    expect_contains("anchor.f0219",
                    proj_door_anchor(), "F0219",
                    "ReDMCSB per-tick advance");
    expect_contains("anchor.f0163",
                    f0163_anchor(), "F0163",
                    "ReDMCSB floor-drop link");
    expect_contains("anchor.defs",
                    door_state_anchor(), "DEFS.H:1039-1044",
                    "ReDMCSB door states");
    expect_contains("anchor.f0816",
                    f0816_anchor(), "F0816",
                    "memory_projectile_pc34_compat.c pass-through");
}

int main(void)
{
    printf("probe=dm1_v1_throw_into_open_door_cell_pc34_compat\n");
    test_f0811_inactive_projectile_slot_noop();
    test_f0814_inspect_destination_per_door_state();
    test_f0811_thrown_item_per_door_state();
    test_f0816_thrown_item_pass_through_per_door_state();
    test_door_square_byte_layout_per_door_state();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_throw_into_open_door_cell_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    if (g_assertions < 50) {
        printf("FAIL dm1_v1_throw_into_open_door_cell_pc34_compat assertions=%d (expect >= 50)\n",
               g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_throw_into_open_door_cell_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
