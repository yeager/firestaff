/*
 * DM1 V1 regression: side-cell projectile travel into a champion
 * damage handoff.
 *
 * Source mapping (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   - PROJEXPL.C:F0219 lines 717-735 crosses to the next square
 *     from either the forward lane or M017_NEXT(direction), then
 *     applies the cell parity step before the move result is committed.
 *   - PROJEXPL.C:F0217 lines 509-558 routes champion projectile
 *     impacts into the HEAD|TORSO pending-damage path.
 *   - MOVESENS.C:F0266 lines 272-310 documents the adjacent-move
 *     intermediary-cell projectile impact map; this test pins the
 *     matching side-lane cell handoff at the M10 projectile boundary.
 *
 * Contract:
 *   - A thrown kinetic projectile in M017_NEXT(direction) crosses the
 *     square boundary, flips to the parity target cell, and hits a
 *     champion occupying that destination cell.
 *   - F0811 reports HIT_CHAMPION, consumes the projectile, and emits
 *     COMBAT_ACTION_APPLY_DAMAGE_CHAMPION with HEAD|TORSO wounds.
 *   - The impact result is committed to the destination square/cell,
 *     unlike side-cell wall/door blockers which retain the source.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "memory_projectile_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#define SIDE_CELL_REDMCSB_F0219_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0219 lines 717-735 side-lane cross/parity step"
#define SIDE_CELL_REDMCSB_F0217_CHAMPION_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0217 lines 509-558 champion projectile damage"
#define SIDE_CELL_REDMCSB_F0266_ANCHOR \
    "ReDMCSB MOVESENS.C:F0266 lines 272-310 intermediary projectile impact map"
#define SIDE_CELL_REDMCSB_F0810_ANCHOR \
    "ReDMCSB PROJEXPL.C:F0212 lines 43-92 projectile create + first move"

typedef struct {
    int direction;
    int sideCell;
    int sourceMapX;
    int sourceMapY;
    int destMapX;
    int destMapY;
    int expectedNewCell;
    const char* label;
} SideCellChampionCase;

static const SideCellChampionCase kCases[4] = {
    { 0, 1, 5, 5, 5, 4, 2, "north_from_east_side_cell"  },
    { 1, 2, 5, 5, 6, 5, 3, "east_from_south_side_cell"  },
    { 2, 3, 5, 5, 5, 6, 0, "south_from_west_side_cell" },
    { 3, 0, 5, 5, 4, 5, 1, "west_from_north_side_cell" },
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

static void make_side_cell_digest(
    struct CellContentDigest_Compat* d,
    const SideCellChampionCase* c)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex = 0;
    d->sourceMapX = c->sourceMapX;
    d->sourceMapY = c->sourceMapY;
    d->sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    d->destMapIndex = 0;
    d->destMapX = c->destMapX;
    d->destMapY = c->destMapY;
    d->destSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    d->destHasChampion = 1;
    d->destChampionCellMask = 1 << c->expectedNewCell;
    d->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    d->destTeleporterNewDirection = -1;
    d->destCreatureType = -1;
}

static void make_thrown_item_create_input(
    struct ProjectileCreateInput_Compat* in,
    const SideCellChampionCase* c,
    int currentTick)
{
    memset(in, 0, sizeof(*in));
    in->category = PROJECTILE_CATEGORY_KINETIC;
    in->subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    in->ownerKind = PROJECTILE_OWNER_CHAMPION;
    in->ownerIndex = 0;
    in->mapIndex = 0;
    in->mapX = c->sourceMapX;
    in->mapY = c->sourceMapY;
    in->cell = c->sideCell;
    in->direction = c->direction;
    in->kineticEnergy = 40;
    in->attack = 28;
    in->stepEnergy = 4;
    in->currentTick = currentTick;
    in->attackTypeCode = COMBAT_ATTACK_NORMAL;
    in->firstMoveGraceFlag = 0;
}

static void test_side_cell_champion_damage_handoff(void)
{
    int i;
    printf("test_side_cell_champion_damage_handoff\n");

    for (i = 0; i < 4; ++i) {
        const SideCellChampionCase* c = &kCases[i];
        struct ProjectileCreateInput_Compat createIn;
        struct ProjectileList_Compat list;
        struct TimelineEvent_Compat firstMoveEvent;
        struct CellContentDigest_Compat digest;
        struct ProjectileInstance_Compat outProjectile;
        struct ProjectileTickResult_Compat result;
        uint32_t moveTick = 701u + (uint32_t)i;
        int blocker = -1;
        int slot = -1;

        memset(&list, 0, sizeof(list));
        memset(&firstMoveEvent, 0, sizeof(firstMoveEvent));
        memset(&outProjectile, 0, sizeof(outProjectile));
        memset(&result, 0, sizeof(result));

        make_thrown_item_create_input(&createIn, c, 700 + i);
        make_side_cell_digest(&digest, c);

        expect_int("f0810.create",
                   F0810_PROJECTILE_Create_Compat(&createIn, &list, &slot,
                                                   &firstMoveEvent),
                   1, SIDE_CELL_REDMCSB_F0810_ANCHOR);
        expect_int(c->label, slot, 0,
                   "F0810 first available projectile slot");
        expect_int(c->label, list.count, 1,
                   "F0810 projectile list owns the thrown item before impact");
        expect_int(c->label, firstMoveEvent.kind,
                   TIMELINE_EVENT_PROJECTILE_MOVE,
                   SIDE_CELL_REDMCSB_F0810_ANCHOR);
        expect_int(c->label, firstMoveEvent.cell, c->sideCell,
                   "F0212 first movement event preserves the side-lane cell");

        expect_int("f0814.side_cell_champion",
                   F0814_PROJECTILE_InspectDestination_Compat(&digest, &blocker),
                   1, SIDE_CELL_REDMCSB_F0219_ANCHOR);
        expect_int(c->label, blocker, PROJECTILE_BLOCKER_OPEN,
                   "F0814 leaves champion handling to the F0811/F0820 impact path");

        expect_int("f0811.side_cell_champion",
                   F0811_PROJECTILE_Advance_Compat(&list.entries[slot], &digest,
                                                   moveTick, NULL,
                                                   &outProjectile, &result),
                   1, SIDE_CELL_REDMCSB_F0219_ANCHOR);
        expect_int(c->label, result.crossedCell, 1,
                   SIDE_CELL_REDMCSB_F0219_ANCHOR);
        expect_int(c->label, result.resultKind,
                   PROJECTILE_RESULT_HIT_CHAMPION,
                   SIDE_CELL_REDMCSB_F0217_CHAMPION_ANCHOR);
        expect_int(c->label, result.despawn, 1,
                   "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile after impact");
        expect_int(c->label, result.emittedCombatAction, 1,
                   SIDE_CELL_REDMCSB_F0217_CHAMPION_ANCHOR);
        expect_int(c->label, result.outAction.kind,
                   COMBAT_ACTION_APPLY_DAMAGE_CHAMPION,
                   SIDE_CELL_REDMCSB_F0217_CHAMPION_ANCHOR);
        expect_int(c->label, result.outAction.allowedWounds,
                   COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO,
                   "ReDMCSB PROJEXPL.C:F0217 line 557 HEAD|TORSO wound mask");
        expect_int(c->label, result.outAction.rawAttackValue, createIn.attack,
                   "F0815 impact attack carries the thrown item's attack value");
        expect_int(c->label, result.outAction.targetMapIndex,
                   digest.destMapIndex,
                   "F0818 champion action targets the destination mapIndex");
        expect_int(c->label, result.outAction.targetMapX, c->destMapX,
                   "F0818 champion action targets the destination X");
        expect_int(c->label, result.outAction.targetMapY, c->destMapY,
                   "F0818 champion action targets the destination Y");
        expect_int(c->label, result.outAction.targetCell,
                   c->expectedNewCell,
                   "F0818 champion action targets the parity-flipped side cell");
        expect_int(c->label, result.outAction.defenderSlotOrCreatureIndex,
                   c->expectedNewCell,
                   "F0818 champion index follows the destination cell ordinal");
        expect_int(c->label, result.outAction.attackerSlotOrCreatureIndex,
                   createIn.ownerIndex,
                   "F0818 champion action records the projectile owner index");
        expect_int(c->label, result.emittedExplosion, 0,
                   "Kinetic thrown-item champion hit does not emit an explosion");
        expect_int(c->label, result.emittedDoorDestructionEvent, 0,
                   "Champion hit does not emit a door destruction event");
        expect_int(c->label, result.outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE,
                   0,
                   "Champion impact stops projectile rescheduling");
        expect_int(c->label, outProjectile.mapX, c->destMapX,
                   "F0811 champion hit commits the projectile state to destination X");
        expect_int(c->label, outProjectile.mapY, c->destMapY,
                   "F0811 champion hit commits the projectile state to destination Y");
        expect_int(c->label, outProjectile.cell, c->expectedNewCell,
                   "F0811 side-lane champion hit commits the parity target cell");
        expect_int(c->label, result.newMapX, c->destMapX,
                   "F0811 result reports destination X for champion handoff");
        expect_int(c->label, result.newMapY, c->destMapY,
                   "F0811 result reports destination Y for champion handoff");
        expect_int(c->label, result.newCell, c->expectedNewCell,
                   "F0811 result reports the parity target cell");
        expect_int(c->label, result.newKineticEnergy,
                   createIn.kineticEnergy - createIn.stepEnergy,
                   "F0811 decrements kinetic energy before the champion handoff");
        expect_int(c->label, result.newAttack,
                   createIn.attack - createIn.stepEnergy,
                   "F0811 decrements the carried attack state before the champion handoff");

        expect_int("f0813.side_cell_champion_despawn",
                   F0813_PROJECTILE_Despawn_Compat(&list, slot),
                   1,
                   "ReDMCSB PROJEXPL.C:F0217 lines 607-608 projectile delete after damage handoff");
        expect_int(c->label, list.count, 0,
                   "F0813 releases the consumed thrown-item slot");
        expect_int(c->label, list.entries[slot].slotIndex, -1,
                   "F0813 marks the projectile slot unused after champion impact");
    }
}

static void test_source_evidence_mentions_required_anchors(void)
{
    printf("test_source_evidence_mentions_required_anchors\n");

    expect_contains("anchor.f0219",
                    SIDE_CELL_REDMCSB_F0219_ANCHOR, "F0219",
                    "ReDMCSB side-lane projectile travel");
    expect_contains("anchor.f0217",
                    SIDE_CELL_REDMCSB_F0217_CHAMPION_ANCHOR, "509-558",
                    "ReDMCSB champion projectile damage");
    expect_contains("anchor.f0266",
                    SIDE_CELL_REDMCSB_F0266_ANCHOR, "F0266",
                    "ReDMCSB intermediary projectile impact map");
    expect_contains("anchor.f0810",
                    SIDE_CELL_REDMCSB_F0810_ANCHOR, "F0212",
                    "ReDMCSB projectile create + first move");
}

int main(void)
{
    printf("probe=dm1_v1_projectile_side_cell_damage_handoff_pc34_compat\n");

    test_side_cell_champion_damage_handoff();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_projectile_side_cell_damage_handoff_pc34_compat "
               "failures=%d assertions=%d\n", g_failures, g_assertions);
        return 1;
    }
    if (g_assertions < 80) {
        printf("FAIL dm1_v1_projectile_side_cell_damage_handoff_pc34_compat "
               "assertions=%d (expect >= 80)\n", g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_projectile_side_cell_damage_handoff_pc34_compat "
           "%d/%d assertions\n", g_assertions, g_assertions);
    return 0;
}
