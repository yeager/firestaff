/*
 * pass745 DM1 V1 focused projectile wall-impact sound/event regression.
 *
 * Exact path pinned here:
 *   Open Door projectile, non-explosion associated thing, moves north from
 *   corridor (5,5) cell 0 into a C00 wall square at (5,4).
 *   A second mundane non-weapon projectile uses the same C49 movement path
 *   to prove the C04 sound is not an Open Door special case.
 *   A weapon-backed kinetic arrow hits the same wall and proves the paired
 *   C00 metallic-thud branch is still gated to weapon associated things.
 *
 * ReDMCSB WIP20210206 anchors:
 *   - PROJEXPL.C:F0219 lines 717-725 detects cross-square wall travel and
 *     calls F0217 before committing the move.
 *   - PROJEXPL.C:F0217 lines 459 and 560-600 exclude Open Door from
 *     CreateExplosionOnImpact, skip F0213, and request the non-weapon
 *     C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM. The same
 *     sound branch requests C00_SOUND_METALLIC_THUD when the associated
 *     thing type is C05_THING_TYPE_WEAPON.
 *   - PROJEXPL.C:F0217 lines 607-608 unlinks/deletes the projectile after
 *     the impact, so no next projectile move event is scheduled.
 */

#include <stdio.h>
#include <string.h>

#include "dm1_v1_sound_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

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

static void make_open_door_projectile(struct ProjectileInstance_Compat* p)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex = 0;
    p->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    p->projectileSubtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
    p->ownerKind = PROJECTILE_OWNER_CHAMPION;
    p->ownerIndex = 0;
    p->mapIndex = 0;
    p->mapX = 5;
    p->mapY = 5;
    p->cell = 0;
    p->direction = 0;
    p->kineticEnergy = 40;
    p->attack = 24;
    p->stepEnergy = 4;
    p->firstMoveGraceFlag = 0;
    p->attackTypeCode = COMBAT_ATTACK_MAGIC;
    p->flags = 0;
}

static void make_non_weapon_projectile(struct ProjectileInstance_Compat* p)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex = 1;
    p->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p->projectileSubtype = 1;
    p->ownerKind = PROJECTILE_OWNER_CHAMPION;
    p->ownerIndex = 0;
    p->mapIndex = 0;
    p->mapX = 5;
    p->mapY = 5;
    p->cell = 0;
    p->direction = 0;
    p->kineticEnergy = 32;
    p->attack = 16;
    p->stepEnergy = 4;
    p->firstMoveGraceFlag = 0;
    p->attackTypeCode = COMBAT_ATTACK_NORMAL;
    p->flags = 0;
}

static void make_weapon_arrow_projectile(struct ProjectileInstance_Compat* p)
{
    memset(p, 0, sizeof(*p));
    p->slotIndex = 2;
    p->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    p->ownerKind = PROJECTILE_OWNER_CHAMPION;
    p->ownerIndex = 0;
    p->mapIndex = 0;
    p->mapX = 5;
    p->mapY = 5;
    p->cell = 0;
    p->direction = 0;
    p->kineticEnergy = 36;
    p->attack = 20;
    p->stepEnergy = 4;
    p->firstMoveGraceFlag = 0;
    p->attackTypeCode = COMBAT_ATTACK_NORMAL;
    p->flags = 0;
}

static void make_north_wall_digest(struct CellContentDigest_Compat* d)
{
    memset(d, 0, sizeof(*d));
    d->sourceMapIndex = 0;
    d->sourceMapX = 5;
    d->sourceMapY = 5;
    d->sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    d->destMapIndex = 0;
    d->destMapX = 5;
    d->destMapY = 4;
    d->destSquareType = PROJECTILE_ELEMENT_WALL;
    d->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    d->destTeleporterNewDirection = -1;
    d->destCreatureType = -1;
}

static void test_open_door_projectile_wall_impact_wooden_thud(void)
{
    struct ProjectileInstance_Compat in;
    struct ProjectileInstance_Compat out;
    struct CellContentDigest_Compat digest;
    struct ProjectileTickResult_Compat result;

    printf("test_open_door_projectile_wall_impact_wooden_thud\n");

    make_open_door_projectile(&in);
    make_north_wall_digest(&digest);
    memset(&out, 0, sizeof(out));
    memset(&result, 0, sizeof(result));

    expect_int("advance.rc",
               F0811_PROJECTILE_Advance_Compat(&in, &digest, 745u, NULL,
                                                &out, &result),
               1,
               "ReDMCSB PROJEXPL.C:F0219 lines 717-725 wall impact dispatch");
    expect_int("result.kind", result.resultKind, PROJECTILE_RESULT_HIT_WALL,
               "ReDMCSB PROJEXPL.C:F0219 lines 721-725 C00_ELEMENT_WALL");
    expect_int("result.crossed", result.crossedCell, 1,
               "ReDMCSB PROJEXPL.C:F0219 lines 717-724 crosses before wall impact");
    expect_int("result.despawn", result.despawn, 1,
               "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile");
    expect_int("result.sound", result.emittedSoundCode, DM1_SND_WOODEN_THUD,
               "ReDMCSB PROJEXPL.C:F0217 lines 587-600 non-weapon impact sound");
    expect_int("result.explosion", result.emittedExplosion, 0,
               "ReDMCSB PROJEXPL.C:F0217 lines 459 and 560-586 Open Door skips explosion");
    expect_int("result.combat", result.emittedCombatAction, 0,
               "Wall impact has no champion or creature target");
    expect_int("result.door_destroy", result.emittedDoorDestructionEvent, 0,
               "Wall impact does not run the door destruction branch");
    expect_int("result.door_toggle", result.emittedDoorToggleEvent, 0,
               "Wall impact does not run the Open Door door-button branch");
    expect_int("result.next_kind", result.outNextTick.kind, 0,
               "ReDMCSB PROJEXPL.C:F0217 impact return stops C49 reschedule");
    expect_int("result.new_x", result.newMapX, in.mapX,
               "F0219 resolves wall hit before committing to destination X");
    expect_int("result.new_y", result.newMapY, in.mapY,
               "F0219 resolves wall hit before committing to destination Y");
    expect_int("out.x", out.mapX, in.mapX,
               "F0811 outNewState remains on the source square");
    expect_int("out.y", out.mapY, in.mapY,
               "F0811 outNewState remains on the source square");
    expect_int("out.kinetic", out.kineticEnergy, in.kineticEnergy - in.stepEnergy,
               "F0219 lines 699-714 decrements energy before wall impact");
    expect_int("out.attack", out.attack, in.attack - in.stepEnergy,
               "F0219 lines 711-714 decrements attack before wall impact");
}

static void test_non_weapon_projectile_wall_impact_wooden_thud(void)
{
    struct ProjectileInstance_Compat in;
    struct ProjectileInstance_Compat out;
    struct CellContentDigest_Compat digest;
    struct ProjectileTickResult_Compat result;

    printf("test_non_weapon_projectile_wall_impact_wooden_thud\n");

    make_non_weapon_projectile(&in);
    make_north_wall_digest(&digest);
    memset(&out, 0, sizeof(out));
    memset(&result, 0, sizeof(result));

    expect_int("nonweapon.advance.rc",
               F0811_PROJECTILE_Advance_Compat(&in, &digest, 746u, NULL,
                                                &out, &result),
               1,
               "ReDMCSB PROJEXPL.C:F0219 lines 717-725 wall impact dispatch");
    expect_int("nonweapon.result.kind", result.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "ReDMCSB PROJEXPL.C:F0219 lines 721-725 C00_ELEMENT_WALL");
    expect_int("nonweapon.result.crossed", result.crossedCell, 1,
               "ReDMCSB PROJEXPL.C:F0219 lines 717-724 crosses before wall impact");
    expect_int("nonweapon.result.sound", result.emittedSoundCode,
               DM1_SND_WOODEN_THUD,
               "ReDMCSB PROJEXPL.C:F0217 lines 587-600 non-weapon impact sound");
    expect_int("nonweapon.result.explosion", result.emittedExplosion, 0,
               "ReDMCSB PROJEXPL.C:F0217 lines 560-586 skips explosion for non-explosion thing");
    expect_int("nonweapon.result.despawn", result.despawn, 1,
               "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile");
    expect_int("nonweapon.result.next_kind", result.outNextTick.kind, 0,
               "ReDMCSB PROJEXPL.C:F0217 impact return stops C49 reschedule");
    expect_int("nonweapon.result.new_x", result.newMapX, in.mapX,
               "F0219 resolves wall hit before committing to destination X");
    expect_int("nonweapon.result.new_y", result.newMapY, in.mapY,
               "F0219 resolves wall hit before committing to destination Y");
    expect_int("nonweapon.out.kinetic", out.kineticEnergy,
               in.kineticEnergy - in.stepEnergy,
               "F0219 lines 699-714 decrements energy before wall impact");
}

static void test_weapon_arrow_projectile_wall_impact_metallic_thud(void)
{
    struct ProjectileInstance_Compat in;
    struct ProjectileInstance_Compat out;
    struct CellContentDigest_Compat digest;
    struct ProjectileTickResult_Compat result;

    printf("test_weapon_arrow_projectile_wall_impact_metallic_thud\n");

    make_weapon_arrow_projectile(&in);
    make_north_wall_digest(&digest);
    memset(&out, 0, sizeof(out));
    memset(&result, 0, sizeof(result));

    expect_int("weapon.advance.rc",
               F0811_PROJECTILE_Advance_Compat(&in, &digest, 747u, NULL,
                                                &out, &result),
               1,
               "ReDMCSB PROJEXPL.C:F0219 lines 717-725 wall impact dispatch");
    expect_int("weapon.result.kind", result.resultKind,
               PROJECTILE_RESULT_HIT_WALL,
               "ReDMCSB PROJEXPL.C:F0219 lines 721-725 C00_ELEMENT_WALL");
    expect_int("weapon.result.crossed", result.crossedCell, 1,
               "ReDMCSB PROJEXPL.C:F0219 lines 717-724 crosses before wall impact");
    expect_int("weapon.result.sound", result.emittedSoundCode,
               DM1_SND_METALLIC_THUD,
               "ReDMCSB PROJEXPL.C:F0217 lines 587-591 weapon impact sound");
    expect_int("weapon.result.explosion", result.emittedExplosion, 0,
               "ReDMCSB PROJEXPL.C:F0217 lines 560-586 skips explosion for non-explosion thing");
    expect_int("weapon.result.despawn", result.despawn, 1,
               "ReDMCSB PROJEXPL.C:F0217 lines 607-608 deletes projectile");
    expect_int("weapon.result.next_kind", result.outNextTick.kind, 0,
               "ReDMCSB PROJEXPL.C:F0217 impact return stops C49 reschedule");
    expect_int("weapon.result.new_x", result.newMapX, in.mapX,
               "F0219 resolves wall hit before committing to destination X");
    expect_int("weapon.result.new_y", result.newMapY, in.mapY,
               "F0219 resolves wall hit before committing to destination Y");
    expect_int("weapon.out.kinetic", out.kineticEnergy,
               in.kineticEnergy - in.stepEnergy,
               "F0219 lines 699-714 decrements energy before wall impact");
}

int main(void)
{
    printf("probe=dm1_v1_projectile_wall_impact_sound_gate_pc34_compat\n");
    test_open_door_projectile_wall_impact_wooden_thud();
    test_non_weapon_projectile_wall_impact_wooden_thud();
    test_weapon_arrow_projectile_wall_impact_metallic_thud();

    if (g_failures) {
        printf("FAIL dm1_v1_projectile_wall_impact_sound_gate_pc34_compat "
               "failures=%d assertions=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_projectile_wall_impact_sound_gate_pc34_compat "
           "%d/%d assertions\n", g_assertions, g_assertions);
    return 0;
}
