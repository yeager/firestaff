#include <stdio.h>
#include <string.h>

#include "memory_projectile_pc34_compat.h"

static int expect(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

int main(void)
{
    struct ExplosionCreateInput_Compat input;
    struct ExplosionList_Compat list;
    struct TimelineEvent_Compat first;
    struct ExplosionInstance_Compat next;
    struct ExplosionTickResult_Compat tick;
    struct CellContentDigest_Compat digest;
    struct RngState_Compat rng;
    int slot = -1;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    memset(&list, 0, sizeof(list));
    memset(&first, 0, sizeof(first));
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = 96;
    input.mapIndex = 2;
    input.mapX = 5;
    input.mapY = 6;
    input.cell = EXPLOSION_CELL_CENTERED;
    input.centered = 1;
    input.currentTick = 100;
    input.ownerKind = -1;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;

    ok &= expect(F0213_EXPLOSION_Create_Compat(
                     &input, &list, &slot, &first) == 1,
                 "F0213 accepts a source-shaped C15 explosion input");
    ok &= expect(slot == 0 && list.count == 1 &&
                 first.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
                 first.mapIndex == input.mapIndex &&
                 first.mapX == input.mapX && first.mapY == input.mapY,
                 "F0213 publishes the matching source C25 owner event");

    memset(&digest, 0, sizeof(digest));
    digest.sourceMapIndex = input.mapIndex;
    digest.sourceMapX = input.mapX;
    digest.sourceMapY = input.mapY;
    F0730_COMBAT_RngInit_Compat(&rng, 0x2130220u);
    ok &= expect(F0220_EXPLOSION_ProcessEvent25_Compat(
                     &list.entries[slot], &digest, first.fireAtTick, &rng,
                     &next, &tick) == 1,
                 "F0220 consumes the live C15 explosion through its C25 event");
    ok &= expect(tick.resultKind == EXPLOSION_RESULT_ADVANCED_FRAME &&
                 tick.despawn == 0 && next.currentFrame == 1 &&
                 tick.outNextTick.kind == TIMELINE_EVENT_EXPLOSION_ADVANCE,
                 "F0220 retains the source smoke slot and next C25 receipt");
    ok &= expect(F0213_EXPLOSION_Create_Compat(
                     NULL, &list, &slot, &first) == 0 &&
                 F0220_EXPLOSION_ProcessEvent25_Compat(
                     NULL, &digest, first.fireAtTick, &rng, &next, &tick) == 0,
                 "F0213/F0220 reject absent C15/C25 source owners");

    puts(ok ? "PASS: DM1 F0213/F0220 C15-C25 runtime ownership" :
              "FAIL: DM1 F0213/F0220 C15-C25 runtime ownership");
    return ok ? 0 : 1;
}
