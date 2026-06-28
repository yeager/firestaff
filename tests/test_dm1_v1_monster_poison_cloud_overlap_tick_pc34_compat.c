/*
 * Narrow DM1 V1 mechanics regression for a poison cloud overlapping one
 * monster tile across one timeline tick boundary.
 */

#include "memory_projectile_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK_EQ(actual, expected, label) do { \
    int _a = (actual); \
    int _e = (expected); \
    if (_a != _e) { \
        fprintf(stderr, "FAIL %s: expected %d, got %d\n", label, _e, _a); \
        ++g_failures; \
    } \
} while (0)

static void test_single_monster_cloud_tick_boundary(void)
{
    struct ExplosionCreateInput_Compat create;
    struct ExplosionList_Compat explosions;
    struct TimelineEvent_Compat firstAdvance;
    struct TimelineEvent_Compat due;
    struct TimelineQueue_Compat timeline;
    struct CellContentDigest_Compat digest;
    struct ExplosionInstance_Compat next;
    struct ExplosionTickResult_Compat tick;
    int slot = -1;

    memset(&create, 0, sizeof(create));
    memset(&explosions, 0, sizeof(explosions));
    memset(&digest, 0, sizeof(digest));

    create.explosionType = C007_EXPLOSION_POISON_CLOUD;
    create.attack = 96;
    create.mapIndex = 0;
    create.mapX = 10;
    create.mapY = 11;
    create.cell = EXPLOSION_CELL_CENTERED;
    create.centered = 1;
    create.currentTick = 1000;
    create.ownerKind = PROJECTILE_OWNER_CHAMPION;
    create.ownerIndex = 2;
    create.creatorProjectileSlot = -1;

    digest.destMapIndex = 0;
    digest.destMapX = 10;
    digest.destMapY = 11;
    digest.destHasCreatureGroup = 1;
    digest.destCreatureType = 10;
    digest.destCreatureCellMask = 0x04;

    /* ReDMCSB source-lock:
     * PROJEXPL.C F0213 lines 158-165 links an explosion and schedules
     * C25_EVENT_EXPLOSION at G0313_ul_GameTime + 1.
     * PROJEXPL.C F0220 lines 817-818 computes poison-cloud attack; lines
     * 859-872 damage a non-party monster tile, decay Attack by 3, and
     * reschedule the same event at Map_Time + 1.
     * GROUP.C F0191/F0192 lines 932-1010 are the single all-creatures damage
     * and poison-resistance handoff for the monster group.
     */
    CHECK_EQ(F0821_EXPLOSION_Create_Compat(&create, &explosions, &slot,
                                           &firstAdvance),
             1, "create poison cloud");
    CHECK_EQ(slot, 0, "cloud occupies first explosion slot");
    CHECK_EQ(explosions.count, 1, "one cloud in explosion list");
    CHECK_EQ(explosions.entries[slot].cell, EXPLOSION_CELL_CENTERED,
             "cloud remains centered over monster tile");
    CHECK_EQ(firstAdvance.kind, TIMELINE_EVENT_EXPLOSION_ADVANCE,
             "first cloud event kind");
    CHECK_EQ((int)firstAdvance.fireAtTick, 1001, "first cloud event tick+1");
    CHECK_EQ(firstAdvance.aux0, slot, "first event carries cloud slot");

    CHECK_EQ(F0720_TIMELINE_Init_Compat(&timeline, 1000), 1,
             "timeline init");
    CHECK_EQ(F0721_TIMELINE_Schedule_Compat(&timeline, &firstAdvance), 1,
             "schedule first cloud event");
    CHECK_EQ(F0722_TIMELINE_Peek_Compat(&timeline, &due), 1,
             "peek first cloud event");
    CHECK_EQ(due.fireAtTick <= timeline.nowTick, 0,
             "cloud not due before boundary");
    CHECK_EQ(F0724_TIMELINE_Tick_Compat(&timeline, 1), 1,
             "advance to boundary");
    CHECK_EQ(F0722_TIMELINE_Peek_Compat(&timeline, &due), 1,
             "peek boundary cloud event");
    CHECK_EQ(due.fireAtTick <= timeline.nowTick, 1,
             "cloud due on boundary");
    CHECK_EQ(F0723_TIMELINE_Pop_Compat(&timeline, &due), 1,
             "pop boundary cloud event");
    CHECK_EQ(due.aux0, slot, "due event carries cloud slot");
    CHECK_EQ((int)timeline.count, 0, "one boundary pop consumes only event");

    CHECK_EQ(F0822_EXPLOSION_Advance_Compat(&explosions.entries[slot],
                                            &digest, due.fireAtTick, NULL,
                                            &next, &tick),
             1, "advance cloud on monster tile");
    CHECK_EQ(tick.resultKind, EXPLOSION_RESULT_ADVANCED_FRAME,
             "cloud advances one persistent frame");
    CHECK_EQ(tick.emittedCombatActionPartyCount, 0, "no party action");
    CHECK_EQ(tick.emittedCombatActionGroupCount, 1, "one monster group action");
    CHECK_EQ(tick.emittedDoorDestructionEvent, 0,
             "poison cloud emits no door event");
    CHECK_EQ(tick.outActionGroup.kind, COMBAT_ACTION_APPLY_DAMAGE_GROUP,
             "group damage action kind");
    CHECK_EQ(tick.outActionGroup.attackTypeCode, COMBAT_ATTACK_NORMAL,
             "poison cloud uses normal attack channel");
    CHECK_EQ(tick.outActionGroup.allowedWounds, 0,
             "monster cloud damage carries no wound mask");
    CHECK_EQ(tick.outActionGroup.targetMapIndex, 0, "group damage map");
    CHECK_EQ(tick.outActionGroup.targetMapX, 10, "group damage x");
    CHECK_EQ(tick.outActionGroup.targetMapY, 11, "group damage y");
    CHECK_EQ(tick.outActionGroup.targetCell, 0,
             "centered cloud targets centered group cell");
    CHECK_EQ(tick.outActionGroup.defenderSlotOrCreatureIndex, 10,
             "group damage preserves monster type");
    CHECK_EQ(tick.outActionGroup.scheduleDelayTicks, 0,
             "group damage resolves on boundary tick");
    CHECK_EQ(tick.outActionGroup.rawAttackValue, 4,
             "attack 96 → base 3 → F0192 resistance-adjusted for C10 (r=5): 3*8/6 = 4");
    CHECK_EQ(tick.despawn, 0, "cloud remains live");
    CHECK_EQ(next.attack, 93, "cloud attack decays by 3");
    CHECK_EQ(next.currentFrame, 1, "cloud frame increments once");
    CHECK_EQ(tick.outNextTick.kind, TIMELINE_EVENT_EXPLOSION_ADVANCE,
             "follow-up cloud event kind");
    CHECK_EQ((int)tick.outNextTick.fireAtTick, 1002,
             "follow-up cloud event tick+1");
    CHECK_EQ(tick.outNextTick.aux0, slot, "follow-up carries same cloud slot");
    CHECK_EQ(tick.outNextTick.aux2, 93, "follow-up carries decayed attack");
    CHECK_EQ(F0721_TIMELINE_Schedule_Compat(&timeline, &tick.outNextTick), 1,
             "schedule follow-up cloud event");
    CHECK_EQ(F0722_TIMELINE_Peek_Compat(&timeline, &due), 1,
             "peek follow-up cloud event");
    CHECK_EQ((int)timeline.nowTick, 1001, "timeline remains on boundary");
    CHECK_EQ(due.fireAtTick <= timeline.nowTick, 0,
             "follow-up cloud is not due on same boundary");
}

static void test_cloud_square_overlap_ignores_quarter_cell_mask(void)
{
    struct ExplosionInstance_Compat cloud;
    struct CellContentDigest_Compat digest;
    struct ExplosionInstance_Compat next;
    struct ExplosionTickResult_Compat tick;

    memset(&cloud, 0, sizeof(cloud));
    memset(&digest, 0, sizeof(digest));

    cloud.slotIndex = 3;
    cloud.explosionType = C007_EXPLOSION_POISON_CLOUD;
    cloud.mapIndex = 0;
    cloud.mapX = 12;
    cloud.mapY = 9;
    cloud.cell = 0;
    cloud.centered = 0;
    cloud.attack = 64;
    cloud.currentFrame = 7;
    cloud.maxFrames = 30;
    cloud.ownerKind = PROJECTILE_OWNER_CHAMPION;
    cloud.ownerIndex = 1;
    cloud.creatorProjectileSlot = -1;

    digest.destMapIndex = 0;
    digest.destMapX = 12;
    digest.destMapY = 9;
    digest.destHasCreatureGroup = 1;
    digest.destCreatureType = 15;
    digest.destCreatureCellMask = 0x04;

    /* ReDMCSB source-lock:
     * PROJEXPL.C F0220 lines 800-808 fetches the group with
     * F0175_GROUP_GetThing(squareX, squareY), and lines 859-864 call
     * F0191_GROUP_GetDamageAllCreaturesOutcome for that square-level
     * group. It does not call F0176_GROUP_GetCreatureOrdinalInCell for
     * poison clouds, so a cloud in cell 0 still damages a group whose
     * quarter-cell mask only contains cell 2.
     */
    CHECK_EQ(F0822_EXPLOSION_Advance_Compat(&cloud, &digest, 2000, NULL,
                                            &next, &tick),
             1, "advance off-cell cloud on monster tile");
    CHECK_EQ(tick.emittedCombatActionPartyCount, 0,
             "off-cell cloud emits no party action");
    CHECK_EQ(tick.emittedCombatActionGroupCount, 1,
             "off-cell cloud still emits one group action");
    CHECK_EQ(tick.outActionGroup.kind, COMBAT_ACTION_APPLY_DAMAGE_GROUP,
             "off-cell cloud group damage action kind");
    CHECK_EQ(tick.outActionGroup.targetMapX, 12, "off-cell cloud target x");
    CHECK_EQ(tick.outActionGroup.targetMapY, 9, "off-cell cloud target y");
    CHECK_EQ(tick.outActionGroup.targetCell, 0,
             "off-cell cloud preserves explosion cell metadata");
    CHECK_EQ(tick.outActionGroup.defenderSlotOrCreatureIndex, 15,
             "off-cell cloud preserves creature type");
    CHECK_EQ(tick.outActionGroup.rawAttackValue, 8,
             "attack 64 -> base 2 -> F0192 for C15 (r=1): 2*8/2 = 8");
    CHECK_EQ(tick.resultKind, EXPLOSION_RESULT_ADVANCED_FRAME,
             "off-cell cloud advances one persistent frame");
    CHECK_EQ(tick.despawn, 0, "off-cell cloud remains live");
    CHECK_EQ(next.attack, 61, "off-cell cloud attack decays by 3");
    CHECK_EQ(tick.outNextTick.kind, TIMELINE_EVENT_EXPLOSION_ADVANCE,
             "off-cell cloud schedules follow-up");
    CHECK_EQ((int)tick.outNextTick.fireAtTick, 2001,
             "off-cell cloud follow-up tick+1");
    CHECK_EQ(tick.outNextTick.aux0, 3,
             "off-cell cloud follow-up carries same slot");
    CHECK_EQ(tick.outNextTick.aux2, 61,
             "off-cell cloud follow-up carries decayed attack");
}

static void test_final_low_attack_cloud_damages_before_expiry(void)
{
    struct ExplosionInstance_Compat cloud;
    struct CellContentDigest_Compat digest;
    struct ExplosionInstance_Compat next;
    struct ExplosionTickResult_Compat tick;

    memset(&cloud, 0, sizeof(cloud));
    memset(&digest, 0, sizeof(digest));

    cloud.slotIndex = 5;
    cloud.explosionType = C007_EXPLOSION_POISON_CLOUD;
    cloud.mapIndex = 0;
    cloud.mapX = 16;
    cloud.mapY = 4;
    cloud.cell = EXPLOSION_CELL_CENTERED;
    cloud.centered = 1;
    cloud.attack = 5;
    cloud.currentFrame = 12;
    cloud.maxFrames = 30;
    cloud.ownerKind = PROJECTILE_OWNER_CHAMPION;
    cloud.ownerIndex = 0;
    cloud.creatorProjectileSlot = -1;

    digest.destMapIndex = 0;
    digest.destMapX = 16;
    digest.destMapY = 4;
    digest.destHasCreatureGroup = 1;
    digest.destCreatureType = 15;
    digest.destCreatureCellMask = 0x0F;

    /* ReDMCSB source-lock:
     * PROJEXPL.C F0220 lines 817-818 computes poison-cloud attack before
     * the switch body. Lines 860-864 apply party or creature-group damage,
     * and only then do lines 867-872 test Explosion->Attack >= 6 to decide
     * whether to subtract 3 and schedule another C25 event. A final weak
     * cloud therefore still damages a monster group before expiring.
     */
    CHECK_EQ(F0822_EXPLOSION_Advance_Compat(&cloud, &digest, 3000, NULL,
                                            &next, &tick),
             1, "advance final weak cloud on monster tile");
    CHECK_EQ(tick.emittedCombatActionPartyCount, 0,
             "final weak cloud emits no party action");
    CHECK_EQ(tick.emittedCombatActionGroupCount, 1,
             "final weak cloud still emits group action");
    CHECK_EQ(tick.outActionGroup.kind, COMBAT_ACTION_APPLY_DAMAGE_GROUP,
             "final weak cloud group damage action kind");
    CHECK_EQ(tick.outActionGroup.targetMapX, 16,
             "final weak cloud target x");
    CHECK_EQ(tick.outActionGroup.targetMapY, 4,
             "final weak cloud target y");
    CHECK_EQ(tick.outActionGroup.targetCell, 0,
             "final weak centered cloud targets centered group cell");
    CHECK_EQ(tick.outActionGroup.defenderSlotOrCreatureIndex, 15,
             "final weak cloud preserves creature type");
    CHECK_EQ(tick.outActionGroup.rawAttackValue, 4,
             "attack 5 -> base 1 -> F0192 for C15 (r=1): 1*8/2 = 4");
    CHECK_EQ(tick.resultKind, EXPLOSION_RESULT_ONE_SHOT,
             "final weak cloud expires after damage");
    CHECK_EQ(tick.despawn, 1, "final weak cloud despawns");
    CHECK_EQ(tick.newAttack, 0, "final weak cloud reports zero next attack");
    CHECK_EQ(next.attack, 5,
             "final weak cloud state is not decayed after expiry");
    CHECK_EQ(tick.outNextTick.kind, 0,
             "final weak cloud emits no follow-up event");
    CHECK_EQ((int)tick.outNextTick.fireAtTick, 0,
             "final weak cloud follow-up tick remains unset");
}

int main(void)
{
    printf("DM1 V1 poison cloud monster overlap tick regression\n");
    test_single_monster_cloud_tick_boundary();
    test_cloud_square_overlap_ignores_quarter_cell_mask();
    test_final_low_attack_cloud_damages_before_expiry();
    if (g_failures) {
        fprintf(stderr, "%d failure(s)\n", g_failures);
        return 1;
    }
    printf("All poison cloud overlap assertions passed.\n");
    return 0;
}
