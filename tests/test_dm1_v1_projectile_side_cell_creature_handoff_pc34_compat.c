/*
 * DM1 V1 regression: side-cell projectile travel into a creature
 * damage handoff.
 *
 * Source mapping (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   - PROJEXPL.C:F0219 lines 717-735 crosses to the next square
 *     from either the forward lane or M017_NEXT(direction), then
 *     applies the parity cell step before committing the move.
 *   - PROJEXPL.C:F0217 lines 521-558 routes creature projectile
 *     impacts through the CM1_ELEMENT_CREATURE damage path.
 *
 * Contract:
 *   - A thrown kinetic projectile in M017_NEXT(direction) crosses the
 *     square boundary, flips to the parity target cell, and hits a
 *     creature occupying that destination cell.
 *   - F0811 reports HIT_CREATURE, consumes the projectile, and emits
 *     COMBAT_ACTION_APPLY_DAMAGE_GROUP with the parity target cell.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory_combat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#define CREATURE_SIDE_CELL_F0219_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0219 lines 717-735 side-lane cross/parity step"
#define CREATURE_SIDE_CELL_F0217_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0217 lines 521-558 creature projectile damage"
#define CREATURE_SIDE_CELL_F0212_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0212 lines 43-92 projectile create + first move"

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

static void make_projectile(struct ProjectileCreateInput_Compat* in)
{
    memset(in, 0, sizeof(*in));
    in->category = PROJECTILE_CATEGORY_KINETIC;
    in->subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    in->ownerKind = PROJECTILE_OWNER_CHAMPION;
    in->ownerIndex = 2;
    in->mapIndex = 0;
    in->mapX = 5;
    in->mapY = 5;
    in->cell = 1;       /* M017_NEXT(north) side lane */
    in->direction = 0;  /* north */
    in->kineticEnergy = 36;
    in->attack = 24;
    in->stepEnergy = 4;
    in->currentTick = 900;
    in->attackTypeCode = COMBAT_ATTACK_NORMAL;
    in->firstMoveGraceFlag = 0;
}

static void make_digest(struct CellContentDigest_Compat* d)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex = 0;
    d->sourceMapX = 5;
    d->sourceMapY = 5;
    d->sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    d->destMapIndex = 0;
    d->destMapX = 5;
    d->destMapY = 4;
    d->destSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    d->destHasCreatureGroup = 1;
    d->destCreatureType = 17;
    d->destCreatureCellMask = 1 << 2; /* parity target from cell 1 north */
    d->destCreatureIsNonMaterial = 0;
    d->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    d->destTeleporterNewDirection = -1;
}

static void test_side_cell_creature_damage_handoff(void)
{
    struct ProjectileCreateInput_Compat createIn;
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat firstMoveEvent;
    struct CellContentDigest_Compat digest;
    struct ProjectileInstance_Compat outProjectile;
    struct ProjectileTickResult_Compat result;
    int blocker = -1;
    int slot = -1;

    printf("test_side_cell_creature_damage_handoff\n");

    memset(&list, 0, sizeof(list));
    memset(&firstMoveEvent, 0, sizeof(firstMoveEvent));
    memset(&outProjectile, 0, sizeof(outProjectile));
    memset(&result, 0, sizeof(result));

    make_projectile(&createIn);
    make_digest(&digest);

    expect_int("f0810.create",
               F0810_PROJECTILE_Create_Compat(&createIn, &list, &slot,
                                               &firstMoveEvent),
               1, CREATURE_SIDE_CELL_F0212_ANCHOR);
    expect_int("created.slot", slot, 0,
               "F0810 first available projectile slot");
    expect_int("first_move.cell", firstMoveEvent.cell, createIn.cell,
               "F0212 first movement event preserves the side-lane cell");

    expect_int("f0814.destination",
               F0814_PROJECTILE_InspectDestination_Compat(&digest, &blocker),
               1, CREATURE_SIDE_CELL_F0219_ANCHOR);
    expect_int("f0814.blocker", blocker, PROJECTILE_BLOCKER_OPEN,
               "F0814 leaves creature handling to the F0811/F0820 impact path");

    expect_int("f0811.advance",
               F0811_PROJECTILE_Advance_Compat(&list.entries[slot], &digest,
                                               901u, NULL,
                                               &outProjectile, &result),
               1, CREATURE_SIDE_CELL_F0219_ANCHOR);
    expect_int("crossed_cell", result.crossedCell, 1,
               CREATURE_SIDE_CELL_F0219_ANCHOR);
    expect_int("result.kind", result.resultKind,
               PROJECTILE_RESULT_HIT_CREATURE,
               CREATURE_SIDE_CELL_F0217_ANCHOR);
    expect_int("despawn", result.despawn, 1,
               "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile after impact");
    expect_int("combat.emitted", result.emittedCombatAction, 1,
               CREATURE_SIDE_CELL_F0217_ANCHOR);
    expect_int("combat.kind", result.outAction.kind,
               COMBAT_ACTION_APPLY_DAMAGE_GROUP,
               CREATURE_SIDE_CELL_F0217_ANCHOR);
    expect_int("combat.target_x", result.outAction.targetMapX, digest.destMapX,
               "F0817 creature action targets the destination square");
    expect_int("combat.target_y", result.outAction.targetMapY, digest.destMapY,
               "F0817 creature action targets the destination square");
    expect_int("combat.target_cell", result.outAction.targetCell, 2,
               "F0817 creature action targets the parity-flipped cell");
    expect_int("combat.defender", result.outAction.defenderSlotOrCreatureIndex,
               digest.destCreatureType,
               "F0817 records the creature type from the destination digest");
    expect_int("combat.attacker", result.outAction.attackerSlotOrCreatureIndex,
               createIn.ownerIndex,
               "F0817 records the projectile owner index");
    expect_int("combat.raw_attack", result.outAction.rawAttackValue,
               createIn.attack,
               "F0815 impact attack carries the thrown item's attack value");
    expect_int("explosion.emitted", result.emittedExplosion, 0,
               "Kinetic thrown-item creature hit does not emit an explosion");
    expect_int("reschedule", result.outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
               0,
               "Creature impact stops projectile rescheduling");
    expect_int("out_projectile.x", outProjectile.mapX, digest.destMapX,
               "F0811 creature hit commits the projectile state to destination X");
    expect_int("out_projectile.y", outProjectile.mapY, digest.destMapY,
               "F0811 creature hit commits the projectile state to destination Y");
    expect_int("out_projectile.cell", outProjectile.cell, 2,
               "F0811 side-lane creature hit commits the parity target cell");
    expect_int("result.new_cell", result.newCell, 2,
               "F0811 result reports the parity target cell");
    expect_int("result.new_energy", result.newKineticEnergy,
               createIn.kineticEnergy - createIn.stepEnergy,
               "F0811 decrements kinetic energy before the creature handoff");
    expect_int("result.new_attack", result.newAttack,
               createIn.attack - createIn.stepEnergy,
               "F0811 decrements the carried attack state before the creature handoff");
}

int main(void)
{
    test_side_cell_creature_damage_handoff();

    if (g_failures) {
        printf("FAIL dm1_v1_projectile_side_cell_creature_handoff_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }

    printf("PASS dm1_v1_projectile_side_cell_creature_handoff_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
