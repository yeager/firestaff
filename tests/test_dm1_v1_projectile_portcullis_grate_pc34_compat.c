/*
 * DM1 V1 runtime regression: a thrown kinetic item (dagger) versus a
 * closed portcullis door - the "grate" in dungeon-builder terminology.
 *
 * Source mapping (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   - PROJEXPL.C:F0217_PROJECTILE_HasImpactOccured door branch
 *     lines 485-505 is the four-part predicate that decides whether
 *     a projectile impact against a C04 door returns C0_FALSE
 *     (no impact -> caller keeps flying) or falls through to
 *     F0232_GROUP_IsDoorDestroyedByAttack:
 *       (1) Door state C5 (destroyed) - pass.
 *       (2) Door state C0/C1 (open/one-fourth) - pass.
 *       (3) G0275_as_CurrentMapDoorInfo[Type].Attributes carries
 *           MASK0x0002_PROJECTILES_CAN_PASS_THROUGH and the
 *           associated-thing type predicate (kinetic: random
 *           roll < launcher strength, with object AllowedSlots
 *           MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS; magical:
 *           subtype >= C0xFF83 excluding OPEN_DOOR) - pass.
 *       (4) otherwise -> F0232 destroy-by-attack.
 *     The portcullis (door type 0, defense 110, attribute
 *     MASK0x0002 | MASK0x0001) is the only one of the four
 *     G0254 door types with MASK0x0002, so it is the canonical
 *     "grate" for this branch. See DUNGEON.C:560-565 and
 *     796-801 for the G0254_as_Graphic559_DoorInfo table.
 *   - PROJEXPL.C:F0219_PROJECTILE_ProcessEvents48To49 per-tick
 *     advance inspects the destination square through F0814 and
 *     routes a closed door hit through F0816 + F0820 (memory
 *     layer mirror). When F0816 reports passes=1, F0811 does
 *     NOT goto RESOLVE - it breaks out of the closed-door
 *     switch, commits the cross-cell step on the destination
 *     square (PROJEXPL.C:714-725 explicitly does not roll back
 *     the move when F0217 returns C0_FALSE), then re-arms
 *     the next C49 PROJECTILE_MOVE via F0825.
 *   - DEFS.H:1039-1044 C0..C5 door states.
 *
 * Contract pinned by this test (one end-to-end destination/event
 * path through F0814 -> F0816 -> F0811/F0820):
 *   - F0814 classifies a closed portcullis (state 4 + MASK0x0002)
 *     as PROJECTILE_BLOCKER_CLOSED_DOOR. The door does NOT become
 *     a wall.
 *   - F0816 for a kinetic thrown item with launcherStrength > 0
 *     (PROJEXPL.C:490-500) rolls F0732_COMBAT_RngRandom against
 *     100; with NULL rng the roll is 0 and launcherStrength=100
 *     therefore always passes. passes=1 means the projectile
 *     does NOT consume its slot.
 *   - F0811 cross-cell advance: resultKind=PROJECTILE_RESULT_FLEW,
 *     despawn=0, emittedCombatAction/emittedExplosion/
 *     emittedDoorDestructionEvent/emittedDoorToggleEvent/
 *     emittedSoundCode all 0; outNewState is committed on the
 *     destination square (5,4) with the parity-rotated cell
 *     (cell-1)&3 = 3 for direction=0/cell=0; the next
 *     TIMELINE_EVENT_PROJECTILE_MOVE event is published at
 *     currentTick+1 (on party map; CHANGE7_20 default).
 *   - F0820 direct dispatch with passes=1 also returns
 *     PROJECTILE_RESULT_FLEW, despawn=0, and writes no
 *     combat/explosion/door/sound output.
 *
 * Non-overlap:
 *   - test_dm1_v1_ra_door_projectile_reject covers the F0217
 *     non-magic attack -> F0232 reject envelope for all four
 *     door types (RA/iron/wood/portcullis) using the
 *     dm1_v1_ra_door_projectile_reject_pc34_compat contract
 *     helper. This test does NOT use that helper; it goes
 *     through the live F0814/F0816/F0811/F0820 functions so
 *     it pins the F0219 per-tick cross-cell mirror, not just
 *     the static F0217 reject table.
 *   - test_dm1_v1_throw_into_open_door_cell covers F0814
 *     classification + F0811 + F0816 per door state (open,
 *     one-fourth, half, three-fourth, full, destroyed) for a
 *     thrown item WITHOUT destDoorAllowsProjectilePassThrough.
 *     None of those cases ever exercise the F0816 kinetic
 *     random-roll (launcherStrength>0) pass-through branch.
 *     This test sets destDoorAllowsProjectilePassThrough=1
 *     and launcherStrength=100 so the F0816 roll is the
 *     only thing that gates the result, and pins the
 *     full F0811->F0820 chain.
 *   - test_dm1_v1_projectile_explosion_render covers
 *     HARM_NON_MATERIAL (magical) pass-through with
 *     destDoorAllowsProjectilePassThrough=1. This test
 *     covers the KINETIC pass-through (F0217:490-500
 *     launcherStrength roll), which is a different
 *     code path inside F0816.
 *   - test_dm1_v1_projectile_side_wall_impact covers
 *     PROJECTILE_BLOCKER_WALL, BLOCKER_STAIRS, BLOCKER_FLUXCAGE,
 *     BLOCKER_OTHER_PROJECTILE, and BLOCKER_BOUNDARY. This
 *     test is the closed-portcullis-grate (BLOCKER_CLOSED_DOOR
 *     + pass-through) case.
 *   - test_dm1_v1_projectile_wall_impact_sound_gate covers
 *     the wall-impact non-explosion sound branch
 *     (C00/C04 wood/metallic thuds) for Open Door, non-weapon
 *     and weapon arrow projectiles. The pass-through path
 *     emits no sound; this test asserts emittedSoundCode=0.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

/* ---- ReDMCSB anchors used by the source-locked comments ----- */
#define PORTCULLIS_REDMCSB_F0217_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0217 lines 485-505 pass-through door branch"
#define PORTCULLIS_REDMCSB_F0219_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0219 lines 717-725 F0811 cross-cell mirror"
#define PORTCULLIS_REDMCSB_DOORINFO_ANCHOR \
    "ReDMCSB DUNGEON.C:560-565 / 796-801 G0254 door info table"
#define PORTCULLIS_DEFS_ANCHOR \
    "ReDMCSB DEFS.H:1039-1044 C0..C5 door states"
#define PORTCULLIS_F0816_ANCHOR \
    "ReDMCSB PROJEXPL.C:490-500 F0816 kinetic launcherStrength roll"
#define PORTCULLIS_F0820_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0217 lines 502-504 F0820 mirror impact branch"
#define PORTCULLIS_CHANGE720_ANCHOR \
    "ReDMCSB PROJEXPL.C CHANGE7_20_IMPROVEMENT: +1 on party map, +3 elsewhere"
#define PORTCULLIS_PARITY_ANCHOR \
    "ReDMCSB PROJEXPL.C:721-725 cell parity rule"
#define PORTCULLIS_DOOR_TYPE_PORTCULLIS  0
#define PORTCULLIS_DOOR_TYPE_WOODEN      1
#define PORTCULLIS_DOOR_TYPE_IRON        2
#define PORTCULLIS_DOOR_TYPE_RA          3

#define PORTCULLIS_DOOR_ATTR_PROJECTILES_CAN_PASS  0x0002
#define PORTCULLIS_DOOR_ATTR_CREATURES_CAN_SEE    0x0001

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
    if (haystack == 0 || needle == 0 || strstr(haystack, needle) == 0) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

/* ---- Fixture: a thrown dagger-style kinetic projectile NORTH ---- */

static void make_thrown_dagger(struct ProjectileInstance_Compat* p)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex          = 0;
    p->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p->projectileSubtype  = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    p->ownerKind          = PROJECTILE_OWNER_CHAMPION;
    p->ownerIndex         = 0;
    p->mapIndex           = 0;
    p->mapX               = 5;
    p->mapY               = 5;
    p->cell               = 0;
    p->direction          = 0; /* NORTH */
    p->kineticEnergy      = 40;
    p->attack             = 24;
    p->stepEnergy         = 4;
    p->firstMoveGraceFlag = 0;
    p->attackTypeCode     = COMBAT_ATTACK_NORMAL;
    /* PROJEXPL.C:490-500 - dagger / rock thrown items have
     * launcherStrength set by the F0810 call site. We pin 100
     * here so F0816's roll (NULL rng -> 0) always passes. */
    p->launcherStrength   = 100;
    p->flags              = 0;
}

/* ---- Fixture: a closed portcullis (grate) at (5,4) NORTH of (5,5) - */

static void make_closed_portcullis_grate_digest(
    struct CellContentDigest_Compat* d)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex                       = 0;
    d->sourceMapX                           = 5;
    d->sourceMapY                           = 5;
    d->sourceSquareType                     = PROJECTILE_ELEMENT_CORRIDOR;
    d->destMapIndex                         = 0;
    d->destMapX                             = 5;
    d->destMapY                             = 4; /* NORTH step */
    d->destSquareType                       = PROJECTILE_ELEMENT_DOOR;
    /* Portcullis door state C4 (CLOSED_FULL) is the typical
     * "closed grate" state. */
    d->destDoorState                        = PROJECTILE_DOOR_STATE_CLOSED_FULL;
    /* MASK0x0002 set on the G0254 door-info for the portcullis
     * (door type 0). M11 pre-tick translates that into this
     * digest flag. */
    d->destDoorAllowsProjectilePassThrough  = 1;
    d->destDoorHasButton                    = 0;
    d->destTeleporterNewDirection           = -1;
    d->destCreatureType                     = -1;
    d->destIsMapBoundary                    = 0;
}

/* ---- (1) F0814 - door type 0 closed = CLOSED_DOOR, not WALL ----- */

static void test_f0814_portcullis_grate_classified_as_closed_door(void)
{
    int blocker = -1;
    struct CellContentDigest_Compat d;

    printf("test_f0814_portcullis_grate_classified_as_closed_door\n");

    make_closed_portcullis_grate_digest(&d);
    expect_int("f0814.rc",
               F0814_PROJECTILE_InspectDestination_Compat(&d, &blocker),
               1, PORTCULLIS_REDMCSB_F0219_ANCHOR);
    expect_int("f0814.blocker", blocker, PROJECTILE_BLOCKER_CLOSED_DOOR,
               "PROJEXPL.C:F0217 lines 485-505 closed door is "
               "BLOCKER_CLOSED_DOOR even with MASK0x0002");
}

/* ---- (2) F0816 - kinetic launcherStrength roll always passes ---- */

static void test_f0816_kinetic_launcher_strength_passes_grate(void)
{
    int passes = -1;
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;

    printf("test_f0816_kinetic_launcher_strength_passes_grate\n");

    make_thrown_dagger(&p);
    make_closed_portcullis_grate_digest(&d);
    /* NULL rng -> F0732 returns 0 -> roll=0 < launcherStrength=100. */
    expect_int("f0816.rc",
               F0816_PROJECTILE_DoesPassThroughDoor_Compat(&p, &d, NULL,
                                                            &passes),
               1, PORTCULLIS_F0816_ANCHOR);
    expect_int("f0816.passes", passes, 1,
               "PROJEXPL.C:490-500 F0816 kinetic pass-through roll");
}

static void test_f0816_key_icon_cannot_pass_closed_grate(void)
{
    int passes = -1;
    struct ProjectileInstance_Compat p;
    struct CellContentDigest_Compat d;

    printf("test_f0816_key_icon_cannot_pass_closed_grate\n");

    make_thrown_dagger(&p);
    make_closed_portcullis_grate_digest(&d);
    p.reserved0 = PROJECTILE_ASSOCIATED_ICON_IRON_KEY;
    expect_int("f0816.key.rc",
               F0816_PROJECTILE_DoesPassThroughDoor_Compat(&p, &d, NULL,
                                                            &passes),
               1, "ReDMCSB PROJEXPL.C:F0217 lines 496-501");
    expect_int("f0816.key.passes", passes, 0,
               "PROJEXPL.C:F0217 lines 496-501 PC34 CHANGE2_04: "
               "C176..C191 key icons cannot pass through a closed door");
}

/* ---- (3) F0811 - cross-cell advance keeps projectile flying ----- */

static void test_f0811_thrown_dagger_advances_through_grate(void)
{
    struct ProjectileInstance_Compat in;
    struct ProjectileInstance_Compat out;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;
    int parityNewCell;

    printf("test_f0811_thrown_dagger_advances_through_grate\n");

    make_thrown_dagger(&in);
    make_closed_portcullis_grate_digest(&d);
    memset(&out, 0, sizeof(out));
    memset(&r, 0, sizeof(r));

    expect_int("f0811.rc",
               F0811_PROJECTILE_Advance_Compat(&in, &d, 700u, NULL,
                                                &out, &r),
               1, PORTCULLIS_REDMCSB_F0219_ANCHOR);
    /* F0217 lines 485-505 + F0811 cross-cell mirror: when F0816
     * reports passes=1, F0811 does NOT goto RESOLVE - it breaks
     * out of the closed-door switch, falls through to the
     * cross-cell commit code, materialises the projectile on the
     * destination square, then re-arms C49 PROJECTILE_MOVE.
     * ReDMCSB PROJEXPL.C:F0219 lines 717-755 explicitly does not
     * roll back the move when F0217 returns C0_FALSE; the
     * projectile keeps the new square/cell so the next C49 tick
     * advances one cell past the grate. */
    expect_int("f0811.result", r.resultKind, PROJECTILE_RESULT_FLEW,
               "F0816 pass=1 -> F0811 RESULT_FLEW, no impact");
    expect_int("f0811.despawn", r.despawn, 0,
               "F0816 pass=1 -> F0811 despawn=0, slot kept");
    expect_int("f0811.crossed", r.crossedCell, 1,
               "F0219 lines 717-725 cross-cell gate fires");
    expect_int("f0811.new_x", r.newMapX, d.destMapX,
               "F0811 pass-through: newMapX = destMapX (committed)");
    expect_int("f0811.new_y", r.newMapY, d.destMapY,
               "F0811 pass-through: newMapY = destMapY (committed)");
    /* Parity rule (PROJEXPL.C:721-725): direction=0 (even),
     * cell=0 (even) -> newCell = (0 - 1) & 3 = 3. */
    parityNewCell = (in.cell - 1) & 3;
    expect_int("f0811.new_cell", r.newCell, parityNewCell,
               "PROJEXPL.C:721-725 parity rule: newCell = (cell-1)&3 "
               "for direction=0/cell=0");
    expect_int("f0811.kinetic", out.kineticEnergy,
               in.kineticEnergy - in.stepEnergy,
               "F0219 lines 706-712 energy decrements before impact gate");
    expect_int("f0811.attack", out.attack, in.attack - in.stepEnergy,
               "F0219 lines 711-714 attack decrements before impact gate");
    /* No impact side effects: no combat action, no explosion,
     * no door destruction, no door toggle, no impact sound. */
    expect_int("f0811.combat", r.emittedCombatAction, 0,
               "Pass-through path emits no combat action");
    expect_int("f0811.explosion", r.emittedExplosion, 0,
               "Pass-through path emits no explosion");
    expect_int("f0811.door_destroy", r.emittedDoorDestructionEvent, 0,
               "Pass-through path never calls F0232 / F0819");
    expect_int("f0811.door_toggle", r.emittedDoorToggleEvent, 0,
               "Pass-through path never opens an Open Door branch");
    expect_int("f0811.sound", r.emittedSoundCode, 0,
               "Pass-through path emits no impact sound");
    /* The F0811 reschedule path emits a C49 PROJECTILE_MOVE
     * event at currentTick+1 (CHANGE7_20 party map), with the
     * projectile now sitting on the destination square. */
    expect_int("f0811.next_kind", r.outNextTick.kind,
               TIMELINE_EVENT_PROJECTILE_MOVE,
               "F0825 schedules C49 PROJECTILE_MOVE on party map");
    expect_int("f0811.next_tick", (int)r.outNextTick.fireAtTick, 701,
               "F0825 +1 on party map (CHANGE7_20_IMPROVEMENT)");
    expect_int("f0811.next_x", r.outNextTick.mapX, d.destMapX,
               "F0825 reschedule carries committed destMapX");
    expect_int("f0811.next_y", r.outNextTick.mapY, d.destMapY,
               "F0825 reschedule carries committed destMapY");
    expect_int("f0811.next_aux0", r.outNextTick.aux0, in.slotIndex,
               "F0825 reschedule carries projectile slot");
}

/* ---- (4) F0820 - direct dispatch with passes=1 returns FLEW ---- */

static void test_f0820_direct_pass_through_returns_flew(void)
{
    struct ProjectileInstance_Compat in;
    struct CellContentDigest_Compat d;
    struct ProjectileTickResult_Compat r;

    printf("test_f0820_direct_pass_through_returns_flew\n");

    make_thrown_dagger(&in);
    make_closed_portcullis_grate_digest(&d);
    memset(&r, 0, sizeof(r));

    expect_int("f0820.rc",
               F0820_PROJECTILE_ResolveCollision_Compat(
                   &in, &d, PROJECTILE_RESULT_HIT_DOOR, 701u, NULL, &r),
               1, PORTCULLIS_F0820_ANCHOR);
    /* When F0816 reports passes=1, F0820 returns FLEW + despawn=0
     * and does NOT populate outNextTick (the caller re-arms
     * the next C49 PROJECTILE_MOVE via F0825). */
    expect_int("f0820.result", r.resultKind, PROJECTILE_RESULT_FLEW,
               "F0820 with passes=1 returns PROJECTILE_RESULT_FLEW");
    expect_int("f0820.despawn", r.despawn, 0,
               "F0820 with passes=1 does not consume the projectile");
    expect_int("f0820.combat", r.emittedCombatAction, 0,
               "F0820 pass-through emits no combat action");
    expect_int("f0820.explosion", r.emittedExplosion, 0,
               "F0820 pass-through emits no explosion");
    expect_int("f0820.door_destroy", r.emittedDoorDestructionEvent, 0,
               "F0820 pass-through never emits door destruction");
    expect_int("f0820.door_toggle", r.emittedDoorToggleEvent, 0,
               "F0820 pass-through never emits door toggle");
    expect_int("f0820.sound", r.emittedSoundCode, 0,
               "F0820 pass-through emits no impact sound");
    expect_int("f0820.next_kind", r.outNextTick.kind, 0,
               "F0820 pass-through path does not arm F0825 itself");
}

/* ---- (5) Source-evidence: a contract string for the gate -------- */

static void test_source_evidence_string(void)
{
    /* The contract helper for the F0217/F0219 door-impact gate
     * already documents the source anchors. This test keeps a
     * local short evidence string in case a future refactor
     * moves the helper, so the gate stays self-describing. */
    static const char kEvidence[] =
        "contract_only=1; no GRAPHICS.DAT/DUNGEON.DAT load. "
        "ReDMCSB PROJEXPL.C:F0217 lines 485-505 four-part "
        "pass-through door predicate. ReDMCSB PROJEXPL.C:490-500 "
        "F0816 kinetic launcherStrength random roll. ReDMCSB "
        "PROJEXPL.C:F0217 lines 496-501 PC34 CHANGE2_04 blocks "
        "C176..C191 key icons from closed-door pass-through. ReDMCSB "
        "PROJEXPL.C:F0219 lines 717-725 F0811 cross-cell mirror. "
        "ReDMCSB PROJEXPL.C:721-725 cell parity rule (cell-1)&3. "
        "ReDMCSB DUNGEON.C:560-565 and 796-801 G0254_as_Graphic559_"
        "DoorInfo portcullis entry: door type 0, defense 110, "
        "attributes MASK0x0002_PROJECTILES_CAN_PASS_THROUGH | "
        "MASK0x0001_CREATURES_CAN_SEE_THROUGH. ReDMCSB DEFS.H:"
        "1039-1044 C0..C5 door states. CHANGE7_20_IMPROVEMENT: "
        "F0825 schedules C49 +1 on party map, +3 elsewhere. "
        "Non-overlap: this gate is the closed-portcullis "
        "pass-through F0811/F0820 chain, not pass745/pass563 "
        "wall-impact sound, not the RA-door projectile-rejection "
        "contract helper, not the open/partly-open door throw "
        "test, and not the HARM_NON_MATERIAL magical "
        "pass-through render test.";

    expect_contains("evidence.f0217", kEvidence,
                    "PROJEXPL.C:F0217 lines 485-505",
                    "F0217 source evidence");
    expect_contains("evidence.f0816", kEvidence,
                    "F0816 kinetic launcherStrength",
                    "F0816 source evidence");
    expect_contains("evidence.keys", kEvidence,
                    "C176..C191 key icons",
                    "PC34 key-through-door fix source evidence");
    expect_contains("evidence.f0219", kEvidence,
                    "PROJEXPL.C:F0219 lines 717-725",
                    "F0219 source evidence");
    expect_contains("evidence.parity", kEvidence,
                    "PROJEXPL.C:721-725",
                    "Cell parity rule source evidence");
    expect_contains("evidence.dungeon", kEvidence,
                    "DUNGEON.C:560-565 and 796-801",
                    "G0254 door info source evidence");
    expect_contains("evidence.portcullis", kEvidence,
                    "door type 0, defense 110",
                    "Portcullis door type 0 source evidence");
    expect_contains("evidence.mask", kEvidence,
                    "MASK0x0002_PROJECTILES_CAN_PASS_THROUGH",
                    "Portcullis attribute source evidence");
    expect_contains("evidence.defs", kEvidence,
                    "DEFS.H:1039-1044",
                    "Door state encoding source evidence");
    expect_contains("evidence.change720", kEvidence,
                    "CHANGE7_20_IMPROVEMENT",
                    "F0825 delay source evidence");
    expect_contains("evidence.nonoverlap", kEvidence,
                    "not pass745/pass563",
                    "Non-overlap marker");
}

int main(void)
{
    test_f0814_portcullis_grate_classified_as_closed_door();
    test_f0816_kinetic_launcher_strength_passes_grate();
    test_f0816_key_icon_cannot_pass_closed_grate();
    test_f0811_thrown_dagger_advances_through_grate();
    test_f0820_direct_pass_through_returns_flew();
    test_source_evidence_string();

    if (g_failures) {
        printf("FAILURES=%d ASSERTIONS=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("OK dm1_v1_projectile_portcullis_grate_pc34_compat "
           "assertions=%d\n", g_assertions);
    return 0;
}
