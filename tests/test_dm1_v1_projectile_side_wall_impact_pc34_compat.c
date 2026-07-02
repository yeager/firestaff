/*
 * DM1 V1 runtime regression: projectile side-wall (and any-cardinal
 * wall) impact route. Focused, contract-only synthetic regression
 * pinning the F0811 wall-impact dispatch through F0820 resolve.
 *
 * Source mapping (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   - PROJEXPL.C:F0217_PROJECTILE_HasImpactOccured wall branch
 *     lines 689-727 routes the wall case (C00_ELEMENT_WALL,
 *     closed-fakewall, stairs-onto-stairs) into a projectile impact.
 *   - PROJEXPL.C:F0219_PROJECTILE_ProcessEvents48To49 per-tick
 *     advance (mirror F0811) inspects the destination cell via the
 *     F0814 BLOCKER_WALL branch and dispatches HIT_WALL on
 *     cross-cell travel.
 *   - PROJEXPL.C:583-602 non-explosion impact sound path
 *     (C00_SOUND_METALLIC_THUD for C05_THING_TYPE_WEAPON otherwise
 *     C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM) is
 *     the contract the v1 model pins for the non-explosion arm
 *     of F0217/F0820. NEEDS DISASSEMBLY REVIEW for the weapon vs
 *     non-weapon mapping on the Firestaff projectile struct
 *     (no associated-thing field; attackTypeCode carries the
 *     closest analogue).
 *   - PROJEXPL.C:F0218_PROJECTILE_GetImpactCount lines 621-638
 *     scans projectiles in a target cell, calls F0217 for each
 *     projectile impact, deletes the impacted projectile event,
 *     and increments the impact count.
 *   - DUNGEON.C:F0151_DUNGEON_GetSquare + DEFS.H C00..C5 square
 *     type encoding; DUNGEON_ELEMENT_WALL in the high 3 bits of
 *     the square byte.
 *   - memory_projectile_pc34_compat.c F0814 wall branch
 *     (PROJECTILE_ELEMENT_WALL → PROJECTILE_BLOCKER_WALL) and
 *     F0820 HIT_WALL case (no emittedExplosion for kinetic,
 *     despawn=1, source-locked non-explosion impact sound code
 *     selection).
 *
 * Contract pinned by this test:
 *   - F0814 classifies a C00_ELEMENT_WALL destination square as
 *     PROJECTILE_BLOCKER_WALL (and likewise for closed fakewall
 *     and stairs-onto-stairs).
 *   - F0811 cross-cell flight into a wall reports
 *     PROJECTILE_RESULT_HIT_WALL, despawn=1, and the projectile's
 *     outNewState mapIndex/X/Y is the source square (the
 *     projectile is NOT committed to the wall square).
 *   - F0820 HIT_WALL arm emits no combat action, no door event,
 *     and uses the ReDMCSB non-explosion impact sound branch:
 *     C00 metallic thud for the local kinetic-arrow/weapon analogue,
 *     C04 wooden thud for a non-weapon, non-explosion Open Door
 *     projectile hitting a wall.
 *   - For a magical projectile whose subtype maps to
 *     SUBTYPE_CREATES_EXPLOSION, F0820 HIT_WALL sets
 *     emittedExplosion=1 and populates outExplosion at the wall
 *     coordinates.
 *   - Lightning/poison-bolt wall impacts whose subtype-adjusted
 *     explosion attack becomes zero follow PROJEXPL.C:F0217's
 *     T0217044 path: projectile deleted, no explosion, and no
 *     fallback non-explosion thud.
 *   - Side-lane blockers set the F0219 cross-square gate before the
 *     impact return, but resolve before the destination square is
 *     committed; this test pins that boundary for all four cardinal
 *     directions, including map-boundary blockers, without broadening
 *     projectile motion.
 *
 * Non-overlap:
 *   - test_dm1_v1_projectile_explosion_render covers the
 *     magical/closed-fakewall/stairs/door travel blockers in
 *     a single F0811 sweep. This test is narrower: it pins
 *     the wall-impact dispatch + final object location + per-
 *     direction parity across 4 cardinals.
 *   - test_dm1_v1_throw_into_open_door_cell covers the door
 *     collision / pass-through / square byte layout. This test
 *     covers walls, not doors.
 *   - test_dm1_v1_viewport_3d covers the wall-zone viewport
 *     rendering. This test covers the runtime collision path,
 *     not the viewport blit.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* ---- ReDMCSB anchors used by the source-locked comments ----- */
#define SIDEWALL_REDMCSB_F0217_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0217 lines 689-727 wall travel branch"
#define SIDEWALL_REDMCSB_F0219_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0219 lines 644-755 F0811 mirror"
#define SIDEWALL_REDMCSB_F0820_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0217 lines 583-602 non-explosion impact sound path"
#define SIDEWALL_REDMCSB_F0218_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0218 lines 621-638 projectile-vs-projectile impact scan"
#define SIDEWALL_REDMCSB_SOUND_ANCHOR \
    "ReDMCSB PROJEXPL.C:587-600 C00_SOUND_METALLIC_THUD vs C04_SOUND_WOODEN_THUD"
#define SIDEWALL_REDMCSB_PARITY_ANCHOR \
    "ReDMCSB PROJEXPL.C:714-725 cell cross + parity rotation"
#define SIDEWALL_REDMCSB_F0212_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0212 lines 43-92 projectile create + C48/C49 first move"
#define SIDEWALL_REDMCSB_GRACE_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0219 lines 670-689 first-move grace flag skip"
#define SIDEWALL_DEFS_ANCHOR \
    "ReDMCSB DEFS.H C00_ELEMENT_WALL (DUNGEON_ELEMENT_WALL = 0 high-3-bits)"

/* ---- Test fixture: directional wall-impact envelope ---------- */
typedef struct {
    int direction;             /* 0=N 1=E 2=S 3=W */
    int cell;                   /* 0..3 */
    int sourceMapX, sourceMapY;
    int wallMapX,   wallMapY;
    const char* label;
} SideWallCase;

static const SideWallCase kSideWallCases[4] = {
    { 0, 0, 5, 5, 5, 4, "north" },
    { 1, 1, 5, 5, 6, 5, "east"  },
    { 2, 2, 5, 5, 5, 6, "south" },
    { 3, 3, 5, 5, 4, 5, "west"  },
};

typedef struct {
    const char* label;
    int sourceSquareType;
    int destSquareType;
    int destFakeWallIsImaginaryOrOpen;
    int destDoorState;
    int destHasFluxcage;
    int destHasOtherProjectile;
    int destIsMapBoundary;
    int expectedBlocker;
    int expectedResultKind;
    int expectedDoorDestructionEvent;
} SideCellBlockerCase;

static const SideCellBlockerCase kSideCellBlockers[] = {
    {
        "side_lane_wall",
        PROJECTILE_ELEMENT_CORRIDOR,
        PROJECTILE_ELEMENT_WALL,
        0,
        PROJECTILE_DOOR_STATE_NONE,
        0,
        0,
        0,
        PROJECTILE_BLOCKER_WALL,
        PROJECTILE_RESULT_HIT_WALL,
        0
    },
    {
        "side_lane_closed_fakewall",
        PROJECTILE_ELEMENT_CORRIDOR,
        PROJECTILE_ELEMENT_FAKEWALL,
        0,
        PROJECTILE_DOOR_STATE_NONE,
        0,
        0,
        0,
        PROJECTILE_BLOCKER_WALL,
        PROJECTILE_RESULT_HIT_WALL,
        0
    },
    {
        "side_lane_stairs_from_stairs",
        PROJECTILE_ELEMENT_STAIRS,
        PROJECTILE_ELEMENT_STAIRS,
        0,
        PROJECTILE_DOOR_STATE_NONE,
        0,
        0,
        0,
        PROJECTILE_BLOCKER_STAIRS,
        PROJECTILE_RESULT_HIT_WALL,
        0
    },
    {
        "side_lane_closed_door",
        PROJECTILE_ELEMENT_CORRIDOR,
        PROJECTILE_ELEMENT_DOOR,
        0,
        PROJECTILE_DOOR_STATE_CLOSED_FULL,
        0,
        0,
        0,
        PROJECTILE_BLOCKER_CLOSED_DOOR,
        PROJECTILE_RESULT_HIT_DOOR,
        1
    },
    {
        "side_lane_fluxcage",
        PROJECTILE_ELEMENT_CORRIDOR,
        PROJECTILE_ELEMENT_CORRIDOR,
        0,
        PROJECTILE_DOOR_STATE_NONE,
        1,
        0,
        0,
        PROJECTILE_BLOCKER_FLUXCAGE,
        PROJECTILE_RESULT_HIT_FLUXCAGE,
        0
    },
    {
        "side_lane_other_projectile",
        PROJECTILE_ELEMENT_CORRIDOR,
        PROJECTILE_ELEMENT_CORRIDOR,
        0,
        PROJECTILE_DOOR_STATE_NONE,
        0,
        1,
        0,
        PROJECTILE_BLOCKER_OTHER_PROJECTILE,
        PROJECTILE_RESULT_HIT_OTHER_PROJECTILE,
        0
    },
    {
        "side_lane_boundary",
        PROJECTILE_ELEMENT_CORRIDOR,
        PROJECTILE_ELEMENT_CORRIDOR,
        0,
        PROJECTILE_DOOR_STATE_NONE,
        0,
        0,
        1,
        PROJECTILE_BLOCKER_BOUNDARY,
        PROJECTILE_RESULT_HIT_WALL,
        0
    }
};

#define SIDE_CELL_BLOCKER_COUNT \
    ((int)(sizeof(kSideCellBlockers) / sizeof(kSideCellBlockers[0])))

typedef struct {
    int direction;
    int sideCell;              /* M017_NEXT(direction): crossing side lane */
    int sourceMapX, sourceMapY;
    int blockerMapX, blockerMapY;
    int expectedImpactCell;
    const char* label;
} SideCellMotionCase;

static const SideCellMotionCase kSideCellMotionCases[4] = {
    { 0, 1, 5, 5, 5, 4, 1, "north_east_side_lane" },
    { 1, 2, 5, 5, 6, 5, 2, "east_south_side_lane" },
    { 2, 3, 5, 5, 5, 6, 3, "south_west_side_lane" },
    { 3, 0, 5, 5, 4, 5, 0, "west_north_side_lane" },
};

/* ---- Static test-state counters ----------------------------- */
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

/* ---- Fixture builders ---------------------------------------- */

static void make_kinetic_arrow(
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
    p->flags              = 0;
}

static void make_magical_fireball(
    struct ProjectileInstance_Compat* p,
    int dir, int cell, int mx, int my)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex          = 0;
    p->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    p->projectileSubtype  = PROJECTILE_SUBTYPE_FIREBALL;
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
    p->attackTypeCode     = COMBAT_ATTACK_FIRE;
    p->flags              = PROJECTILE_FLAG_CREATES_EXPLOSION;
}

static void make_magical_open_door_projectile(
    struct ProjectileInstance_Compat* p,
    int dir, int cell, int mx, int my)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex          = 0;
    p->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    p->projectileSubtype  = PROJECTILE_SUBTYPE_OPEN_DOOR;
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
    p->attackTypeCode     = COMBAT_ATTACK_MAGIC;
    p->flags              = 0;
}

static void make_wall_digest(
    struct CellContentDigest_Compat* d,
    int sourceX, int sourceY, int wallX, int wallY)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex          = 0;
    d->sourceMapX              = sourceX;
    d->sourceMapY              = sourceY;
    d->sourceSquareType        = PROJECTILE_ELEMENT_CORRIDOR;
    d->destMapIndex            = 0;
    d->destMapX                = wallX;
    d->destMapY                = wallY;
    d->destSquareType          = PROJECTILE_ELEMENT_WALL;
    d->destFakeWallIsImaginaryOrOpen = 0;
    d->destDoorState           = PROJECTILE_DOOR_STATE_NONE;
    d->destTeleporterNewDirection = -1;
    d->destCreatureType        = -1;
    d->destIsMapBoundary       = 0;
}

static void make_side_cell_blocker_digest(
    struct CellContentDigest_Compat* d,
    const SideCellBlockerCase* c,
    const SideCellMotionCase* m)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex          = 0;
    d->sourceMapX              = m->sourceMapX;
    d->sourceMapY              = m->sourceMapY;
    d->sourceSquareType        = c->sourceSquareType;
    d->destMapIndex            = 0;
    d->destMapX                = m->blockerMapX;
    d->destMapY                = m->blockerMapY;
    d->destSquareType          = c->destSquareType;
    d->destFakeWallIsImaginaryOrOpen = c->destFakeWallIsImaginaryOrOpen;
    d->destDoorState           = c->destDoorState;
    d->destHasFluxcage         = c->destHasFluxcage;
    d->destHasOtherProjectile  = c->destHasOtherProjectile;
    d->destIsMapBoundary       = c->destIsMapBoundary;
    d->destTeleporterNewDirection = -1;
    d->destCreatureType        = -1;
}

/* ---- Test 1: F0814 wall classification ---------------------- */
static void test_f0814_wall_blocker_is_wall(void)
{
    struct CellContentDigest_Compat d;
    int blocker = -1;
    int rc = 0;

    printf("test_f0814_wall_blocker_is_wall\n");

    make_wall_digest(&d, 5, 5, 5, 4);
    rc = F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker);
    expect_int("f0814.wall.rc",  rc,                            1, SIDEWALL_REDMCSB_F0217_ANCHOR);
    expect_int("f0814.wall.blocker", blocker,                  PROJECTILE_BLOCKER_WALL,
               SIDEWALL_REDMCSB_F0217_ANCHOR);
}

/* ---- Test 2: F0811 wall-impact dispatch (4 cardinals) -------- */
static void test_f0811_side_wall_impact_kinetic_per_direction(void)
{
    int i;
    printf("test_f0811_side_wall_impact_kinetic_per_direction\n");

    for (i = 0; i < 4; ++i) {
        const SideWallCase* c = &kSideWallCases[i];
        struct ProjectileInstance_Compat p;
        struct ProjectileInstance_Compat pOut;
        struct CellContentDigest_Compat  d;
        struct ProjectileTickResult_Compat r;
        int rc = 0;

        memset(&pOut, 0, sizeof(pOut));
        memset(&r,    0, sizeof(r));

        make_kinetic_arrow(&p, c->direction, c->cell,
                           c->sourceMapX, c->sourceMapY);
        make_wall_digest(&d, c->sourceMapX, c->sourceMapY,
                              c->wallMapX,   c->wallMapY);

        rc = F0811_PROJECTILE_Advance_Compat(&p, &d, 200u + (uint32_t)i,
                                             NULL, &pOut, &r);

        expect_int("f0811.wall.rc",  rc,                        1, SIDEWALL_REDMCSB_F0219_ANCHOR);
        expect_int(c->label,         r.resultKind,              PROJECTILE_RESULT_HIT_WALL,
                   SIDEWALL_REDMCSB_F0219_ANCHOR);
        expect_int(c->label,         r.despawn,                 1, SIDEWALL_REDMCSB_F0219_ANCHOR);
        expect_int(c->label,         r.emittedExplosion,        0,
                   "F0820 HIT_WALL: kinetic subtype is not in SUBTYPE_CREATES_EXPLOSION");
        expect_int(c->label,         r.emittedCombatAction,     0,
                   "F0820 HIT_WALL: wall impact has no combat target");
        expect_int(c->label,         r.emittedDoorDestructionEvent, 0,
                   "F0820 HIT_WALL: wall impact has no door event");
        expect_int(c->label,         r.emittedDoorToggleEvent,  0,
                   "F0820 HIT_WALL: wall impact has no door toggle event");
        expect_int(c->label,         r.emittedSoundCode,        0,
                   "ReDMCSB PROJEXPL.C:F0217 lines 587-600 selects "
                   "C00_SOUND_METALLIC_THUD for weapon-backed non-explosion impacts.");
        expect_int(c->label,         r.newMapIndex,             p.mapIndex,
                   "F0811 wall impact does not commit projectile to wall mapIndex");
        expect_int(c->label,         r.newMapX,                 p.mapX,
                   "F0811 wall impact does not commit projectile to wall X");
        expect_int(c->label,         r.newMapY,                 p.mapY,
                   "F0811 wall impact does not commit projectile to wall Y");
        expect_int(c->label,         r.newDirection,            p.direction,
                   "F0811 wall impact preserves projectile direction");
        expect_int(c->label,         pOut.mapIndex,             p.mapIndex,
                   "F0811 wall-impact outNewState: source mapIndex retained");
        expect_int(c->label,         pOut.mapX,                 p.mapX,
                   "F0811 wall-impact outNewState: source X retained");
        expect_int(c->label,         pOut.mapY,                 p.mapY,
                   "F0811 wall-impact outNewState: source Y retained");
        expect_int(c->label,         pOut.kineticEnergy,        p.kineticEnergy - p.stepEnergy,
                   "F0811 wall impact decrements kinetic energy by stepEnergy");
        expect_int(c->label,         pOut.attack,               p.attack - p.stepEnergy,
                   "F0811 wall impact decrements attack by stepEnergy");
    }
}

/* ---- Test 3: F0811 thrown item side-cell blocker impacts ---- */
static void test_f0811_thrown_item_side_cell_blockers(void)
{
    int i, j;
    printf("test_f0811_thrown_item_side_cell_blockers\n");

    for (i = 0; i < SIDE_CELL_BLOCKER_COUNT; ++i) {
        const SideCellBlockerCase* c = &kSideCellBlockers[i];
        for (j = 0; j < 4; ++j) {
            const SideCellMotionCase* m = &kSideCellMotionCases[j];
            struct ProjectileInstance_Compat p;
            struct ProjectileInstance_Compat pOut;
            struct CellContentDigest_Compat d;
            struct ProjectileTickResult_Compat r;
            int blocker = -1;

            memset(&pOut, 0, sizeof(pOut));
            memset(&r, 0, sizeof(r));

            /* ReDMCSB PROJEXPL.C:F0219 lines 717-725: a projectile
             * crosses into the next square from either the forward lane
             * or side lane M017_NEXT(direction). These fixtures cover
             * the side-lane arm for all four cardinal directions. */
            make_kinetic_arrow(&p, m->direction, m->sideCell,
                               m->sourceMapX, m->sourceMapY);
            make_side_cell_blocker_digest(&d, c, m);

            expect_int("f0814.side_cell_blocker",
                       F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker),
                       1, SIDEWALL_REDMCSB_F0219_ANCHOR);
            expect_int(c->label, blocker, c->expectedBlocker,
                       "ReDMCSB PROJEXPL.C:F0219 lines 721-725 side-lane blocker dispatch");

            expect_int("f0811.side_cell.rc",
                       F0811_PROJECTILE_Advance_Compat(&p, &d,
                                                       320u + (uint32_t)(i * 4 + j),
                                                       NULL, &pOut, &r),
                       1, SIDEWALL_REDMCSB_F0219_ANCHOR);
            expect_int(c->label, r.resultKind, c->expectedResultKind,
                       "ReDMCSB PROJEXPL.C:F0219 lines 717-725 + F0217 impact resolution");
            if (c->expectedResultKind == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE) {
                expect_int(c->label, r.resultKind,
                           PROJECTILE_RESULT_HIT_OTHER_PROJECTILE,
                           SIDEWALL_REDMCSB_F0218_ANCHOR);
            }
            expect_int(c->label, r.despawn, 1,
                       "ReDMCSB PROJEXPL.C:F0217 lines 607-608 unlinks/deletes projectile after impact");
            expect_int(c->label, r.emittedDoorDestructionEvent,
                       c->expectedDoorDestructionEvent,
                       "ReDMCSB PROJEXPL.C:F0217 lines 506-508 door impact attack event");
            if (c->expectedDoorDestructionEvent) {
                expect_int(c->label, r.outNextTick.kind,
                           TIMELINE_EVENT_DOOR_DESTRUCTION,
                           "ReDMCSB PROJEXPL.C:F0217 lines 506-508 schedules door destruction at impact cell");
                expect_int(c->label, (int)r.outNextTick.fireAtTick,
                           321 + (i * 4 + j),
                           "F0819 door destruction event fires one tick after side-cell impact");
                expect_int(c->label, r.outNextTick.mapIndex, d.destMapIndex,
                           "F0819 door destruction event uses blocked side-cell mapIndex");
                expect_int(c->label, r.outNextTick.mapX, d.destMapX,
                           "F0819 door destruction event uses blocked side-cell X");
                expect_int(c->label, r.outNextTick.mapY, d.destMapY,
                           "F0819 door destruction event uses blocked side-cell Y");
                expect_int(c->label, r.outNextTick.cell, p.cell,
                           "F0819 door destruction event keeps the projectile impact lane");
                expect_int(c->label, r.outNextTick.aux0,
                           (p.attack - p.stepEnergy) + 1,
                           "F0219 decrements attack before F0217 line 506 adds one for the deterministic door attack");
                expect_int(c->label, r.outNextTick.aux1, p.projectileSubtype,
                           "F0819 door destruction event records projectile subtype");
                expect_int(c->label, r.outNextTick.aux2, p.ownerIndex,
                           "F0819 door destruction event records owner index");
                expect_int(c->label, r.outNextTick.aux3, p.ownerKind,
                           "F0819 door destruction event records owner kind");
            } else {
                expect_int(c->label, r.outNextTick.kind, 0,
                           "Side-cell wall/fluxcage/projectile blockers do not emit door destruction events");
            }
            expect_int(c->label, r.emittedCombatAction, 0,
                       "F0217 side-cell wall/door/fluxcage/projectile blockers have no champion or creature target");
            expect_int(c->label, r.emittedExplosion, 0,
                       "Kinetic thrown item side-cell blockers do not create explosions");
            expect_int(c->label, r.crossedCell, 1,
                       "F0219 lines 717-725 publish the cross-square gate before the impact return");
            expect_int(c->label,
                       r.outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
                       0,
                       "F0217 impact return may schedule impact side effects, but not a C49 projectile move");
            expect_int(c->label, pOut.mapIndex, p.mapIndex,
                       "F0811 side-cell blocker impact retains source mapIndex");
            expect_int(c->label, pOut.mapX, p.mapX,
                       "F0811 side-cell blocker impact retains source X");
            expect_int(c->label, pOut.mapY, p.mapY,
                       "F0811 side-cell blocker impact retains source Y");
            expect_int(c->label, r.newMapIndex, p.mapIndex,
                       "F0811 side-cell impact result reports source mapIndex");
            expect_int(c->label, r.newMapX, p.mapX,
                       "F0811 side-cell impact result reports source X, not blocked side-cell X");
            expect_int(c->label, r.newMapY, p.mapY,
                       "F0811 side-cell impact result reports source Y, not blocked side-cell Y");
            expect_int(m->label, pOut.cell, m->expectedImpactCell,
                       "ReDMCSB PROJEXPL.C:F0219 lines 717-727 impact returns before "
                       "the side-lane cell parity commit");
            expect_int(m->label, r.newCell, pOut.cell,
                       "F0811 side-cell impact result cell mirrors outNewState");
            expect_int(c->label, pOut.kineticEnergy, p.kineticEnergy - p.stepEnergy,
                       "F0811 side-cell blocker impact decrements kinetic energy by stepEnergy");
            expect_int(c->label, pOut.attack, p.attack - p.stepEnergy,
                       "F0811 side-cell blocker impact decrements attack by stepEnergy");
            expect_int(c->label, r.newKineticEnergy, pOut.kineticEnergy,
                       "F0811 side-cell impact result reports decremented kinetic energy");
            expect_int(c->label, r.newAttack, pOut.attack,
                       "F0811 side-cell impact result reports decremented attack");
        }
    }
}

/* ---- Test 4: created thrown item despawns on side-cell blockers ---- */
static void test_f0810_f0811_f0813_created_thrown_item_side_cell_blockers(void)
{
    int i, j;
    printf("test_f0810_f0811_f0813_created_thrown_item_side_cell_blockers\n");

    for (i = 0; i < SIDE_CELL_BLOCKER_COUNT; ++i) {
        const SideCellBlockerCase* c = &kSideCellBlockers[i];
        for (j = 0; j < 4; ++j) {
            const SideCellMotionCase* m = &kSideCellMotionCases[j];
            struct ProjectileCreateInput_Compat createIn;
            struct ProjectileList_Compat list;
            struct TimelineEvent_Compat firstMoveEvent;
            struct ProjectileInstance_Compat pOut;
            struct CellContentDigest_Compat d;
            struct ProjectileTickResult_Compat r;
            uint32_t createTick = 400u + (uint32_t)(i * 4 + j);
            uint32_t moveTick = 500u + (uint32_t)(i * 4 + j);
            int slot = -1;

            memset(&createIn, 0, sizeof(createIn));
            memset(&list, 0, sizeof(list));
            memset(&firstMoveEvent, 0, sizeof(firstMoveEvent));
            memset(&pOut, 0, sizeof(pOut));
            memset(&r, 0, sizeof(r));

            /* ReDMCSB PROJEXPL.C:F0212 lines 43-92 creates the
             * projectile thing on the source square and schedules
             * C48/C49 movement.  F0219 lines 717-725 then resolves
             * a post-grace side-lane blocker before F0267 can commit
             * the projectile into the next square. */
            createIn.category = PROJECTILE_CATEGORY_KINETIC;
            createIn.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
            createIn.ownerKind = PROJECTILE_OWNER_CHAMPION;
            createIn.ownerIndex = 0;
            createIn.mapIndex = 0;
            createIn.mapX = m->sourceMapX;
            createIn.mapY = m->sourceMapY;
            createIn.cell = m->sideCell;
            createIn.direction = m->direction;
            createIn.kineticEnergy = 40;
            createIn.attack = 24;
            createIn.stepEnergy = 4;
            createIn.currentTick = (int)createTick;
            createIn.attackTypeCode = COMBAT_ATTACK_NORMAL;
            createIn.firstMoveGraceFlag = 0;

            make_side_cell_blocker_digest(&d, c, m);

            expect_int("f0810.side_cell_create",
                       F0810_PROJECTILE_Create_Compat(&createIn, &list, &slot, &firstMoveEvent),
                       1,
                       "ReDMCSB PROJEXPL.C:F0212 lines 43-92 projectile create + C48/C49 schedule");
            expect_int(c->label, list.count, 1,
                       "F0810 projectile list owns the thrown item before impact");
            expect_int(c->label, slot, 0,
                       "F0810 first available projectile slot");
            expect_int(c->label, list.entries[slot].reserved3, 1,
                       "F0810 occupied projectile sentinel");
            expect_int(m->label, list.entries[slot].mapX, m->sourceMapX,
                       "F0212 links the created thrown item to the source square X");
            expect_int(m->label, list.entries[slot].mapY, m->sourceMapY,
                       "F0212 links the created thrown item to the source square Y");
            expect_int(m->label, list.entries[slot].cell, m->sideCell,
                       "F0212 stores the side-lane cell on the projectile thing");
            expect_int(m->label, firstMoveEvent.kind, TIMELINE_EVENT_PROJECTILE_MOVE,
                       "F0212 schedules the first projectile movement event");
            expect_int(m->label, (int)firstMoveEvent.fireAtTick, (int)createTick + 1,
                       "F0212 schedules first movement one tick after throw");
            expect_int(m->label, firstMoveEvent.mapX, m->sourceMapX,
                       "F0212 first movement event starts from source X");
            expect_int(m->label, firstMoveEvent.mapY, m->sourceMapY,
                       "F0212 first movement event starts from source Y");
            expect_int(m->label, firstMoveEvent.cell, m->sideCell,
                       "F0212 first movement event preserves side-lane cell");
            expect_int(m->label, firstMoveEvent.aux0, slot,
                       "F0212 first movement event addresses the projectile slot");

            expect_int("f0811.created_side_cell.rc",
                       F0811_PROJECTILE_Advance_Compat(&list.entries[slot], &d,
                                                       moveTick, NULL, &pOut, &r),
                       1, SIDEWALL_REDMCSB_F0219_ANCHOR);
            expect_int(c->label, r.resultKind, c->expectedResultKind,
                       "ReDMCSB PROJEXPL.C:F0219 lines 717-725 side-cell blocker impact");
            expect_int(c->label, r.despawn, 1,
                       "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile after impact");
            expect_int(c->label, r.newMapX, m->sourceMapX,
                       "F0219 blocker impact retains source X before F0267 move commit");
            expect_int(c->label, r.newMapY, m->sourceMapY,
                       "F0219 blocker impact retains source Y before F0267 move commit");
            expect_int(c->label, r.newMapX == m->blockerMapX && r.newMapY == m->blockerMapY,
                       0,
                       "F0219 side-cell impact does not materialize the thrown item in the blocked cell");
            expect_int(c->label, r.outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
                       0,
                       "F0217 impact return stops projectile rescheduling");
            if (c->expectedDoorDestructionEvent) {
                expect_int(c->label, r.outNextTick.mapX, m->blockerMapX,
                           "F0217 closed-door impact side effect targets blocked side-cell X");
                expect_int(c->label, r.outNextTick.mapY, m->blockerMapY,
                           "F0217 closed-door impact side effect targets blocked side-cell Y");
            }

            expect_int("f0813.side_cell_despawn",
                       F0813_PROJECTILE_Despawn_Compat(&list, slot),
                       1,
                       "ReDMCSB PROJEXPL.C:F0217 lines 607-608 unlink/delete projectile");
            expect_int(c->label, list.count, 0,
                       "F0813 projectile list releases impacted thrown item");
            expect_int(c->label, list.entries[slot].reserved3, 0,
                       "F0813 clears occupied projectile sentinel after side-cell impact");
            expect_int(c->label, list.entries[slot].slotIndex, -1,
                       "F0813 marks projectile slot unused after side-cell impact");
        }
    }
}

/* ---- Test 5: F0811 magical fireball wall-impact creates explosion - */
static void test_f0811_magical_fireball_wall_impact_creates_explosion(void)
{
    struct ProjectileInstance_Compat p;
    struct ProjectileInstance_Compat pOut;
    struct CellContentDigest_Compat  d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0811_magical_fireball_wall_impact_creates_explosion\n");

    memset(&pOut, 0, sizeof(pOut));
    memset(&r,    0, sizeof(r));

    make_magical_fireball(&p, 0 /* N */, 0, 5, 5);
    make_wall_digest(&d, 5, 5, 5, 4);

    expect_int("f0811.fireball.rc",
               F0811_PROJECTILE_Advance_Compat(&p, &d, 250u, NULL, &pOut, &r),
               1, SIDEWALL_REDMCSB_F0219_ANCHOR);
    expect_int("f0811.fireball.kind",         r.resultKind,        PROJECTILE_RESULT_HIT_WALL,
               SIDEWALL_REDMCSB_F0219_ANCHOR);
    expect_int("f0811.fireball.despawn",      r.despawn,           1, SIDEWALL_REDMCSB_F0219_ANCHOR);
    expect_int("f0811.fireball.explosion",    r.emittedExplosion,  1,
               "F0820 HIT_WALL: magical + CREATES_EXPLOSION emits explosion");
    expect_int("f0811.fireball.sound",        r.emittedSoundCode,  0,
               "F0820 HIT_WALL: explosion branch consumes the projectile, "
               "no non-explosion impact sound (PROJEXPL.C:583-586 takes the "
               "explosion path, skipping the 587-600 sound branch).");
    expect_int("f0811.fireball.combat",       r.emittedCombatAction, 0,
               "F0820 HIT_WALL: explosion is queued separately, not as a combat action.");
    expect_int("f0811.fireball.explosion_type", r.outExplosion.explosionType,
               C000_EXPLOSION_FIREBALL,
               "F0820 HIT_WALL: explosion type is C000_EXPLOSION_FIREBALL for FIREBALL");
    expect_int("f0811.fireball.explosion_x",  r.outExplosion.mapX, d.destMapX,
               "F0820 HIT_WALL: explosion mapX is wall X (impact cell)");
    expect_int("f0811.fireball.explosion_y",  r.outExplosion.mapY, d.destMapY,
               "F0820 HIT_WALL: explosion mapY is wall Y (impact cell)");
    expect_int("f0811.fireball.explosion_attack",
               r.outExplosion.attack, p.kineticEnergy,
               "F0820 HIT_WALL: explosion attack uses projectile kinetic energy");
}

/* ---- Test 5a: F0217 zero-adjusted lightning wall impact -------- */
static void test_f0820_lightning_zero_adjusted_wall_impact_skips_explosion_and_sound(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_lightning_zero_adjusted_wall_impact_skips_explosion_and_sound\n");

    make_magical_fireball(&p, 0 /* N */, 0, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    p.kineticEnergy = 1;
    p.attack = 1;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.lightning_zero.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 301u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 lines 574-584 jumps to T0217044 when Lightning / 2 is zero");
    expect_int("f0820.lightning_zero.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "zero-adjusted lightning still resolves as a wall impact");
    expect_int("f0820.lightning_zero.despawn", r.despawn, 1,
               "F0217 deletes the projectile after the T0217044 no-explosion branch");
    expect_int("f0820.lightning_zero.explosion", r.emittedExplosion, 0,
               "zero-adjusted lightning creates no explosion");
    expect_int("f0820.lightning_zero.sound_request", r.emittedSoundRequest, 0,
               "T0217044 skips the fallback non-explosion impact sound branch");
    expect_int("f0820.lightning_zero.sound", r.emittedSoundCode, 0,
               "zero-adjusted lightning emits no thud or spell sound");
    expect_int("f0820.lightning_zero.combat", r.emittedCombatAction, 0,
               "wall impact has no combat target");
    expect_int("f0820.lightning_zero.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "wall impact has no door side effect");
}

static void test_f0820_lightning_wall_impact_creates_cell_explosion(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_lightning_wall_impact_creates_cell_explosion\n");

    make_magical_fireball(&p, 0 /* N */, 3, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    p.kineticEnergy = 6;
    p.attack = 6;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.lightning.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 304u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 lines 565-585 creates Lightning impact explosion with attack / 2");
    expect_int("f0820.lightning.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "lightning resolves as a wall impact");
    expect_int("f0820.lightning.despawn", r.despawn, 1,
               "F0217 deletes the projectile after the explosion branch");
    expect_int("f0820.lightning.explosion", r.emittedExplosion, 1,
               "nonzero adjusted lightning creates a lightning explosion");
    expect_int("f0820.lightning.type", r.outExplosion.explosionType,
               C002_EXPLOSION_LIGHTNING_BOLT,
               "Lightning impact maps to C002 lightning explosion");
    expect_int("f0820.lightning.attack", r.outExplosion.attack, 3,
               "Lightning impact attack is KineticEnergy >> 1");
    expect_int("f0820.lightning.cell", r.outExplosion.cell, 3,
               "PROJEXPL.C:F0217 line 585 preserves P0456_i_Cell for non-poison-cloud explosions");
    expect_int("f0820.lightning.centered", r.outExplosion.centered, 0,
               "Lightning impact does not use the poison-cloud centered-cell sentinel");
    expect_int("f0820.lightning.sound_request", r.emittedSoundRequest, 0,
               "explosion branch skips the fallback non-explosion impact sound");
    expect_int("f0820.lightning.sound", r.emittedSoundCode, 0,
               "nonzero Lightning impact emits no thud or spell sound");
    expect_int("f0820.lightning.combat", r.emittedCombatAction, 0,
               "wall impact has no combat target");
    expect_int("f0820.lightning.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "wall impact has no door side effect");
}

static void test_f0820_harm_non_material_wall_impact_creates_cell_explosion(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_harm_non_material_wall_impact_creates_cell_explosion\n");

    make_magical_fireball(&p, 0 /* N */, 1, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    p.kineticEnergy = 11;
    p.attack = 11;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.harm_non_material.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 305u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 lines 565-585 creates Harm Non Material impact explosion from KineticEnergy");
    expect_int("f0820.harm_non_material.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "Harm Non Material resolves as a wall impact");
    expect_int("f0820.harm_non_material.despawn", r.despawn, 1,
               "F0217 deletes the projectile after the explosion branch");
    expect_int("f0820.harm_non_material.explosion", r.emittedExplosion, 1,
               "Harm Non Material creates an impact explosion");
    expect_int("f0820.harm_non_material.type", r.outExplosion.explosionType,
               C003_EXPLOSION_HARM_NON_MATERIAL,
               "Harm Non Material impact maps to C003 explosion");
    expect_int("f0820.harm_non_material.attack", r.outExplosion.attack, 11,
               "Harm Non Material impact attack uses unshifted KineticEnergy");
    expect_int("f0820.harm_non_material.cell", r.outExplosion.cell, 1,
               "PROJEXPL.C:F0217 line 585 preserves P0456_i_Cell for non-poison-cloud explosions");
    expect_int("f0820.harm_non_material.centered", r.outExplosion.centered, 0,
               "Harm Non Material impact does not use the poison-cloud centered-cell sentinel");
    expect_int("f0820.harm_non_material.sound_request", r.emittedSoundRequest, 0,
               "explosion branch skips the fallback non-explosion impact sound");
    expect_int("f0820.harm_non_material.sound", r.emittedSoundCode, 0,
               "Harm Non Material impact emits no thud or spell sound");
    expect_int("f0820.harm_non_material.combat", r.emittedCombatAction, 0,
               "wall impact has no combat target");
    expect_int("f0820.harm_non_material.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "wall impact has no door side effect");
}

static void test_f0820_slime_wall_impact_emits_wooden_thud_without_explosion(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_slime_wall_impact_emits_wooden_thud_without_explosion\n");

    make_magical_fireball(&p, 0 /* N */, 0, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_SLIME;
    p.kineticEnergy = 31;
    p.attack = 31;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.slime.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 306u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 line 459 excludes Slime from CreateExplosionOnImpact");
    expect_int("f0820.slime.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "Slime resolves as a wall impact");
    expect_int("f0820.slime.despawn", r.despawn, 1,
               "F0217 deletes the projectile after the non-explosion branch");
    expect_int("f0820.slime.explosion", r.emittedExplosion, 0,
               "Slime impact does not create C001 slime explosion");
    expect_int("f0820.slime.sound_request", r.emittedSoundRequest, 1,
               "Slime falls through to the non-explosion impact sound branch");
    expect_int("f0820.slime.sound", r.emittedSoundCode, 4,
               "ReDMCSB PROJEXPL.C:F0217 lines 587-600 selects wooden thud for non-weapon impacts");
    expect_int("f0820.slime.combat", r.emittedCombatAction, 0,
               "wall impact has no combat target");
    expect_int("f0820.slime.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "wall impact has no door side effect");
}

static void test_f0820_poison_bolt_zero_adjusted_wall_impact_skips_explosion_and_sound(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_poison_bolt_zero_adjusted_wall_impact_skips_explosion_and_sound\n");

    make_magical_fireball(&p, 0 /* N */, 0, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    p.kineticEnergy = 3;
    p.attack = 3;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.poison_bolt_zero.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 302u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 lines 574-576 jumps to T0217044 when Poison Bolt / 4 is zero");
    expect_int("f0820.poison_bolt_zero.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "zero-adjusted poison bolt still resolves as a wall impact");
    expect_int("f0820.poison_bolt_zero.despawn", r.despawn, 1,
               "F0217 deletes the projectile after the T0217044 no-explosion branch");
    expect_int("f0820.poison_bolt_zero.explosion", r.emittedExplosion, 0,
               "zero-adjusted poison bolt creates no poison-cloud explosion");
    expect_int("f0820.poison_bolt_zero.sound_request", r.emittedSoundRequest, 0,
               "T0217044 skips the fallback spell-sound impact branch");
    expect_int("f0820.poison_bolt_zero.sound", r.emittedSoundCode, 0,
               "zero-adjusted poison bolt emits no spell sound or thud");
    expect_int("f0820.poison_bolt_zero.combat", r.emittedCombatAction, 0,
               "wall impact has no combat target");
    expect_int("f0820.poison_bolt_zero.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "wall impact has no door side effect");
}

static void test_f0820_poison_bolt_wall_impact_creates_centered_cloud(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_poison_bolt_wall_impact_creates_centered_cloud\n");

    make_magical_fireball(&p, 0 /* N */, 2, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    p.kineticEnergy = 8;
    p.attack = 8;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.poison_bolt.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 303u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 lines 565-585 creates Poison Bolt impact explosion with attack / 4");
    expect_int("f0820.poison_bolt.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "poison bolt resolves as a wall impact");
    expect_int("f0820.poison_bolt.despawn", r.despawn, 1,
               "F0217 deletes the projectile after the explosion branch");
    expect_int("f0820.poison_bolt.explosion", r.emittedExplosion, 1,
               "nonzero adjusted poison bolt creates a poison-cloud explosion");
    expect_int("f0820.poison_bolt.type", r.outExplosion.explosionType,
               C007_EXPLOSION_POISON_CLOUD,
               "Poison Bolt impact maps to C007 poison cloud");
    expect_int("f0820.poison_bolt.attack", r.outExplosion.attack, 2,
               "Poison Bolt impact attack is KineticEnergy >> 2");
    expect_int("f0820.poison_bolt.cell", r.outExplosion.cell,
               EXPLOSION_CELL_CENTERED,
               "PROJEXPL.C:F0217 line 585 centers poison-cloud explosions");
    expect_int("f0820.poison_bolt.centered", r.outExplosion.centered, 1,
               "Poison Bolt impact marks the poison cloud centered");
    expect_int("f0820.poison_bolt.sound_request", r.emittedSoundRequest, 0,
               "explosion branch skips the fallback non-explosion impact sound");
    expect_int("f0820.poison_bolt.sound", r.emittedSoundCode, 0,
               "nonzero Poison Bolt impact emits no thud or spell sound");
    expect_int("f0820.poison_bolt.combat", r.emittedCombatAction, 0,
               "wall impact has no combat target");
    expect_int("f0820.poison_bolt.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "wall impact has no door side effect");
}

/* ---- Test 5b: F0217 thrown potion wall-impact explosion -------- */
static void test_f0820_thrown_poison_potion_wall_impact_creates_centered_cloud(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat  d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_thrown_poison_potion_wall_impact_creates_centered_cloud\n");

    make_kinetic_arrow(&p, 0 /* N */, 0, 5, 5);
    p.projectileSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
    p.associatedPotionPower = 77;
    p.flags |= PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT;
    p.kineticEnergy = 12;
    p.attack = 5;
    make_wall_digest(&d, 5, 5, 5, 4);

    memset(&r, 0, sizeof(r));
    expect_int("f0820.poison_potion.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 300u, NULL, &r),
               1,
               "ReDMCSB PROJEXPL.C:F0217 lines 444-455 and 565-585 potion impact explosion");
    expect_int("f0820.poison_potion.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "thrown poison potion resolves as a wall impact");
    expect_int("f0820.poison_potion.explosion", r.emittedExplosion, 1,
               "F0217 RemovePotion branch creates an explosion even for a non-magical projectile");
    expect_int("f0820.poison_potion.type", r.outExplosion.explosionType,
               C007_EXPLOSION_POISON_CLOUD,
               "Ven potion impact maps to poison cloud");
    expect_int("f0820.poison_potion.attack", r.outExplosion.attack, 77,
               "Ven potion impact explosion attack uses potion power");
    expect_int("f0820.poison_potion.cell", r.outExplosion.cell,
               EXPLOSION_CELL_CENTERED,
               "Poison cloud impact uses the centered-cell sentinel");
    expect_int("f0820.poison_potion.centered", r.outExplosion.centered, 1,
               "Poison cloud impact marks the explosion centered");
}

/* ---- Test 6: F0820 wall-impact dispatch in isolation -------- */
static void test_f0820_wall_impact_kinetic_dispatch(void)
{
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat  d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_wall_impact_kinetic_dispatch\n");

    memset(&r, 0, sizeof(r));

    make_kinetic_arrow(&p, 1 /* E */, 1, 5, 5);
    make_wall_digest(&d, 5, 5, 6, 5);

    expect_int("f0820.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &p, &d, PROJECTILE_RESULT_HIT_WALL, 300u, NULL, &r),
               1, SIDEWALL_REDMCSB_F0820_ANCHOR);
    expect_int("f0820.kind",    r.resultKind,        PROJECTILE_RESULT_HIT_WALL,
               SIDEWALL_REDMCSB_F0820_ANCHOR);
    expect_int("f0820.despawn", r.despawn,           1, SIDEWALL_REDMCSB_F0820_ANCHOR);
    expect_int("f0820.explosion", r.emittedExplosion, 0,
               "F0820 HIT_WALL arm does not emit an explosion for a kinetic arrow");
    expect_int("f0820.combat",  r.emittedCombatAction, 0,
               "F0820 HIT_WALL arm does not emit a combat action (no target on wall)");
    expect_int("f0820.door_destruction", r.emittedDoorDestructionEvent, 0,
               "F0820 HIT_WALL arm does not emit a door destruction event");
    expect_int("f0820.door_toggle", r.emittedDoorToggleEvent, 0,
               "F0820 HIT_WALL arm does not emit a door toggle event");
    expect_int("f0820.sound",   r.emittedSoundCode,  0,
               "ReDMCSB PROJEXPL.C:F0217 lines 587-600 selects "
               "C00_SOUND_METALLIC_THUD for weapon-backed non-explosion impacts.");
}

/* ---- Test 7: F0811/F0820 non-explosion wall impact sound ---- */
static void test_f0811_open_door_wall_impact_emits_wooden_thud(void)
{
    struct ProjectileInstance_Compat p;
    struct ProjectileInstance_Compat pOut;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0811_open_door_wall_impact_emits_wooden_thud\n");

    memset(&pOut, 0, sizeof(pOut));
    memset(&r, 0, sizeof(r));

    make_magical_open_door_projectile(&p, 0 /* N */, 0, 5, 5);
    make_wall_digest(&d, 5, 5, 5, 4);

    expect_int("f0811.open_door_wall.rc",
               F0811_PROJECTILE_Advance_Compat(&p, &d, 360u, NULL, &pOut, &r),
               1, SIDEWALL_REDMCSB_F0219_ANCHOR);
    expect_int("f0811.open_door_wall.kind", r.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               SIDEWALL_REDMCSB_F0219_ANCHOR);
    expect_int("f0811.open_door_wall.despawn", r.despawn, 1,
               "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile after impact");
    expect_int("f0811.open_door_wall.explosion", r.emittedExplosion, 0,
               "PROJECTILE_SUBTYPE_OPEN_DOOR is excluded from the ReDMCSB "
               "PROJEXPL.C:F0217 CreateExplosionOnImpact predicate");
    expect_int("f0811.open_door_wall.sound", r.emittedSoundCode, 4,
               SIDEWALL_REDMCSB_SOUND_ANCHOR);
    expect_int("f0811.open_door_wall.combat", r.emittedCombatAction, 0,
               "F0820 HIT_WALL arm has no champion or creature target");
    expect_int("f0811.open_door_wall.door_event",
               r.emittedDoorDestructionEvent || r.emittedDoorToggleEvent, 0,
               "F0820 HIT_WALL arm has no door side effect");
    expect_int("f0811.open_door_wall.new_x", r.newMapX, p.mapX,
               "F0811 wall impact reports the retained source square X");
    expect_int("f0811.open_door_wall.new_y", r.newMapY, p.mapY,
               "F0811 wall impact reports the retained source square Y");
    expect_int("f0811.open_door_wall.out_x", pOut.mapX, p.mapX,
               "F0811 wall impact outNewState retains source square X");
    expect_int("f0811.open_door_wall.out_y", pOut.mapY, p.mapY,
               "F0811 wall impact outNewState retains source square Y");
}

/* ---- Test 8: fresh thrown item first-move grace crosses side-cell blocker ----
 *
 * ReDMCSB PROJEXPL.C F0212 lines 43-92 creates the projectile thing and
 * schedules a C48/C49 first move with `firstMoveGraceFlag=1` so the
 * thrown item does not bounce off the source champion lane. F0219
 * lines 670-689 honour the grace flag by jumping straight to
 * MOTION_STEP on the first tick, then F0219 lines 714-749 still apply
 * the cross-square gate and the wall/door/fluxcage/projectile
 * side-cell impact resolution for the very first move. This test
 * exercises that F0810 → F0811 grace-tick path so a freshly thrown
 * item with a wall, closed door, fluxcage, or other projectile
 * directly in the side-cell cross direction still consumes the
 * projectile on the destination square and never materialises it
 * there.
 */
static void test_f0810_f0811_first_move_grace_thrown_item_side_cell_blockers(void)
{
    int i, j;
    printf("test_f0810_f0811_first_move_grace_thrown_item_side_cell_blockers\n");

    for (i = 0; i < SIDE_CELL_BLOCKER_COUNT; ++i) {
        const SideCellBlockerCase* c = &kSideCellBlockers[i];
        for (j = 0; j < 4; ++j) {
            const SideCellMotionCase* m = &kSideCellMotionCases[j];
            struct ProjectileCreateInput_Compat createIn;
            struct ProjectileList_Compat list;
            struct TimelineEvent_Compat firstMoveEvent;
            struct ProjectileInstance_Compat pOut;
            struct CellContentDigest_Compat d;
            struct ProjectileTickResult_Compat r;
            uint32_t createTick = 600u + (uint32_t)(i * 4 + j);
            uint32_t moveTick   = 601u + (uint32_t)(i * 4 + j);
            int slot = -1;

            memset(&createIn,        0, sizeof(createIn));
            memset(&list,            0, sizeof(list));
            memset(&firstMoveEvent,  0, sizeof(firstMoveEvent));
            memset(&pOut,            0, sizeof(pOut));
            memset(&d,               0, sizeof(d));
            memset(&r,               0, sizeof(r));

            /* ReDMCSB PROJEXPL.C F0212 lines 43-92 throws a fresh
             * item into the side lane with the grace flag set, so
             * the F0811 first-move tick (moveTick = createTick + 1)
             * must skip the source-cell champion/creature check
             * (F0219 lines 670-689) and still apply the side-cell
             * impact path (F0219 lines 714-749). */
            createIn.category           = PROJECTILE_CATEGORY_KINETIC;
            createIn.subtype            = PROJECTILE_SUBTYPE_KINETIC_ARROW;
            createIn.ownerKind          = PROJECTILE_OWNER_CHAMPION;
            createIn.ownerIndex         = 0;
            createIn.mapIndex           = 0;
            createIn.mapX               = m->sourceMapX;
            createIn.mapY               = m->sourceMapY;
            createIn.cell               = m->sideCell;
            createIn.direction          = m->direction;
            createIn.kineticEnergy      = 40;
            createIn.attack             = 24;
            createIn.stepEnergy         = 4;
            createIn.currentTick        = (int)createTick;
            createIn.attackTypeCode     = COMBAT_ATTACK_NORMAL;
            createIn.firstMoveGraceFlag = 1;

            make_side_cell_blocker_digest(&d, c, m);

            /* F0212 lines 43-92 link the projectile thing onto the
             * source square and schedule the C48/C49 first move with
             * the grace flag set. F0810 must reflect that. */
            expect_int("f0810.first_move_grace_create",
                       F0810_PROJECTILE_Create_Compat(&createIn, &list, &slot,
                                                       &firstMoveEvent),
                       1, SIDEWALL_REDMCSB_F0212_ANCHOR);
            expect_int(c->label, slot, 0,
                       "F0810 fresh-throw side-lane item occupies slot 0");
            expect_int(c->label, list.count, 1,
                       "F0810 fresh-throw side-lane item is in the projectile list");
            expect_int(m->label, list.entries[slot].firstMoveGraceFlag, 1,
                       "F0212 first-move grace flag survives F0810 link step");
            expect_int(m->label, list.entries[slot].cell, m->sideCell,
                       "F0212 first-move grace projectile keeps the side-lane cell");
            expect_int(m->label, firstMoveEvent.kind, TIMELINE_EVENT_PROJECTILE_MOVE,
                       "F0212 first move is C48/C49 PROJECTILE_MOVE");
            expect_int(m->label, (int)firstMoveEvent.fireAtTick, (int)createTick + 1,
                       "F0212 first-move tick is createTick + 1");
            expect_int(m->label, firstMoveEvent.cell, m->sideCell,
                       "F0212 first-move event keeps the side-lane cell");

            /* F0219 lines 670-689: grace tick skips the source-cell
             * impact check and jumps to MOTION_STEP. The F0811 cross
             * gate then publishes crossedCell=1 for a side-cell
             * projectile, and the side-cell blocker dispatch yields
             * the same final result as the post-grace Test 3 path. */
            expect_int("f0811.first_move_grace_advance",
                       F0811_PROJECTILE_Advance_Compat(&list.entries[slot], &d,
                                                       moveTick, NULL, &pOut, &r),
                       1, SIDEWALL_REDMCSB_F0219_ANCHOR);
            expect_int(c->label, r.newFirstMoveGraceFlag, 0,
                       "F0219 lines 670-689 grace tick clears the grace flag");
            expect_int(m->label, list.entries[slot].firstMoveGraceFlag, 1,
                       "F0810 instance is unchanged by the F0811 advance (caller commits)");
            expect_int(c->label, r.crossedCell, 1,
                       "F0219 lines 714-719 cross-square gate still fires on the grace tick");
            expect_int(c->label, r.resultKind, c->expectedResultKind,
                       "F0219 lines 714-749 side-cell blocker impact on grace tick");
            expect_int(c->label, r.despawn, 1,
                       "F0217 lines 607-608 unlinks the thrown item after the grace-tick impact");
            expect_int(c->label, r.newMapIndex, d.sourceMapIndex,
                       "F0811 grace-tick side-cell impact retains source mapIndex (not blocked X,Y)");
            expect_int(m->label, r.newMapX, m->sourceMapX,
                       "F0811 grace-tick side-cell impact retains source X");
            expect_int(m->label, r.newMapY, m->sourceMapY,
                       "F0811 grace-tick side-cell impact retains source Y");
            expect_int(m->label, r.newMapX == m->blockerMapX
                                 && r.newMapY == m->blockerMapY, 0,
                       "F0811 grace-tick side-cell impact does not materialise the thrown item "
                       "in the blocked side cell");
            expect_int(m->label, pOut.mapX, m->sourceMapX,
                       "F0811 grace-tick side-cell impact outNewState keeps source X");
            expect_int(m->label, pOut.mapY, m->sourceMapY,
                       "F0811 grace-tick side-cell impact outNewState keeps source Y");
            expect_int(c->label, r.emittedCombatAction, 0,
                       "F0217 grace-tick side-cell wall/door/fluxcage/projectile impact has no combat target");
            expect_int(c->label, r.emittedExplosion, 0,
                       "F0820 grace-tick side-cell impact: kinetic thrown item has no explosion");
            expect_int(c->label, r.emittedDoorDestructionEvent,
                       c->expectedDoorDestructionEvent,
                       "F0217 grace-tick closed-door side-cell impact still schedules door destruction");
            expect_int(c->label, r.outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
                       0,
                       "F0217 grace-tick impact stops the projectile rescheduling chain");
            if (c->expectedDoorDestructionEvent) {
                expect_int(c->label, r.outNextTick.mapX, m->blockerMapX,
                           "F0217 grace-tick closed-door side-cell event targets blocked X");
                expect_int(c->label, r.outNextTick.mapY, m->blockerMapY,
                           "F0217 grace-tick closed-door side-cell event targets blocked Y");
            }
            expect_int(m->label, pOut.cell, m->expectedImpactCell,
                       "F0219 lines 717-727 grace-tick impact returns before the side-lane "
                       "cell parity commit (F0811 outNewState)");
            expect_int(m->label, r.newCell, pOut.cell,
                       "F0811 grace-tick side-cell impact result cell mirrors outNewState");
        }
    }
}

/* ---- Test 9: square-byte encoding for the wall square ------- */
static void test_wall_square_byte_encoding(void)
{
    unsigned char squareByte;
    int elementType;
    int wallState;

    printf("test_wall_square_byte_encoding\n");

    /* Mirror DEFS.H C00..C5 square encoding: high 3 bits = element
     * type, low 3 bits = state/attributes. A C00_ELEMENT_WALL square
     * has element=0, no extra attributes, so the byte is 0. */
    squareByte = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) | 0);
    elementType = (squareByte & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    wallState   = squareByte & DUNGEON_SQUARE_MASK_ATTRIBS;

    expect_int("wall.byte.element",   elementType, DUNGEON_ELEMENT_WALL,
               SIDEWALL_DEFS_ANCHOR);
    expect_int("wall.byte.attribs",   wallState,   0,
               SIDEWALL_DEFS_ANCHOR);
}

/* ---- Test 10: source-evidence anchor mentions --------------- */
static void test_source_evidence_mentions_required_anchors(void)
{
    printf("test_source_evidence_mentions_required_anchors\n");

    expect_contains("anchor.f0217",
                    SIDEWALL_REDMCSB_F0217_ANCHOR, "F0217",
                    "ReDMCSB wall travel branch");
    expect_contains("anchor.f0219",
                    SIDEWALL_REDMCSB_F0219_ANCHOR, "F0219",
                    "ReDMCSB per-tick advance");
    expect_contains("anchor.f0820",
                    SIDEWALL_REDMCSB_F0820_ANCHOR, "583-602",
                    "ReDMCSB non-explosion impact sound path");
    expect_contains("anchor.f0218",
                    SIDEWALL_REDMCSB_F0218_ANCHOR, "F0218",
                    "ReDMCSB projectile-vs-projectile scan");
    expect_contains("anchor.sound",
                    SIDEWALL_REDMCSB_SOUND_ANCHOR, "C04_SOUND_WOODEN_THUD",
                    "ReDMCSB non-explosion impact sound path");
    expect_contains("anchor.parity",
                    SIDEWALL_REDMCSB_PARITY_ANCHOR, "parity",
                    "ReDMCSB cell parity rule");
    expect_contains("anchor.f0212",
                    SIDEWALL_REDMCSB_F0212_ANCHOR, "F0212",
                    "ReDMCSB projectile create + C48/C49 first-move schedule");
    expect_contains("anchor.grace",
                    SIDEWALL_REDMCSB_GRACE_ANCHOR, "grace",
                    "ReDMCSB first-move grace flag skip");
}

int main(void)
{
    printf("probe=dm1_v1_projectile_side_wall_impact_pc34_compat\n");

    test_f0814_wall_blocker_is_wall();
    test_f0811_side_wall_impact_kinetic_per_direction();
    test_f0811_thrown_item_side_cell_blockers();
    test_f0810_f0811_f0813_created_thrown_item_side_cell_blockers();
    test_f0810_f0811_first_move_grace_thrown_item_side_cell_blockers();
    test_f0811_magical_fireball_wall_impact_creates_explosion();
    test_f0820_lightning_zero_adjusted_wall_impact_skips_explosion_and_sound();
    test_f0820_lightning_wall_impact_creates_cell_explosion();
    test_f0820_harm_non_material_wall_impact_creates_cell_explosion();
    test_f0820_slime_wall_impact_emits_wooden_thud_without_explosion();
    test_f0820_poison_bolt_zero_adjusted_wall_impact_skips_explosion_and_sound();
    test_f0820_poison_bolt_wall_impact_creates_centered_cloud();
    test_f0820_thrown_poison_potion_wall_impact_creates_centered_cloud();
    test_f0820_wall_impact_kinetic_dispatch();
    test_f0811_open_door_wall_impact_emits_wooden_thud();
    test_wall_square_byte_encoding();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_projectile_side_wall_impact_pc34_compat "
               "failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    if (g_assertions < 60) {
        printf("FAIL dm1_v1_projectile_side_wall_impact_pc34_compat "
               "assertions=%d (expect >= 60)\n",
               g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_projectile_side_wall_impact_pc34_compat "
           "%d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
