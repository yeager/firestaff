#include "memory_dungeon_dat_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RAM-only allocation contracts, not original-media or gameplay evidence.
 * ReDMCSB DUNGEON.C:62-79 / LOADSAVE.C:2040-2074. */
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; \
} } while (0)

static int empty_and_idempotent(void)
{
    static const int extra[16] = {0,0,0,0,75,100,120,0,5,0,140,0,0,0,60,50};
    static const int size[16] = {4,6,4,8,16,4,4,4,4,8,4,0,0,0,8,4};
    struct DungeonDatState_Compat dungeon = {0};
    struct DungeonThings_Compat things = {0}, before;
    int t, i;
    dungeon.loaded = things.loaded = 1;
    CHECK(F0506_DUNGEON_ReserveFreshPc34Pools_Compat(&dungeon, &things));
    CHECK(things.freshPc34PoolsReserved == 1);
    CHECK(things.squareFirstThingCount == 300);
    CHECK(dungeon.header.squareFirstThingCount == 300);
    for (i = 0; i < 300; ++i) CHECK(things.squareFirstThings[i] == 0xffff);
    for (t = 0; t < 16; ++t) {
        CHECK(things.thingCounts[t] == extra[t]);
        CHECK(dungeon.header.thingCounts[t] == extra[t]);
        if (!extra[t]) { CHECK(!things.rawThingData[t]); continue; }
        for (i = 0; i < extra[t]; ++i) {
            CHECK(things.rawThingData[t][i * size[t]] == 0xff);
            CHECK(things.rawThingData[t][i * size[t] + 1] == 0xff);
        }
    }
#define CHECK_DECODED(field, countField, expected) do { \
    CHECK(things.countField == expected); \
    for (i = 0; i < expected; ++i) CHECK(things.field[i].next == 0xffff); \
} while (0)
    CHECK_DECODED(groups, groupCount, 75);
    CHECK_DECODED(weapons, weaponCount, 100);
    CHECK_DECODED(armours, armourCount, 120);
    CHECK_DECODED(potions, potionCount, 5);
    CHECK_DECODED(junks, junkCount, 140);
    CHECK_DECODED(projectiles, projectileCount, 60);
    CHECK_DECODED(explosions, explosionCount, 50);
#undef CHECK_DECODED
    memcpy(&before, &things, sizeof(before));
    CHECK(F0506_DUNGEON_ReserveFreshPc34Pools_Compat(&dungeon, &things));
    CHECK(memcmp(&before, &things, sizeof(things)) == 0);
    CHECK(dungeon.header.squareFirstThingCount == 300);
    F0504_DUNGEON_FreeThingData_Compat(&things);
    return 0;
}

static int rollback_late_validation(void)
{
    struct DungeonDatState_Compat dungeon = {0}, dungeonBefore;
    struct DungeonThings_Compat things = {0}, thingsBefore;
    unsigned short sft = 0xfffe;
    dungeon.loaded = things.loaded = 1;
    dungeon.header.squareFirstThingCount = things.squareFirstThingCount = 1;
    things.squareFirstThings = &sft;
    /* Last decoded pool rejects after all raw and six decoded allocations.
     * Existing stack storage must neither be freed nor replaced on failure. */
    things.explosionCount = 1;
    memcpy(&dungeonBefore, &dungeon, sizeof(dungeon));
    memcpy(&thingsBefore, &things, sizeof(things));
    CHECK(!F0506_DUNGEON_ReserveFreshPc34Pools_Compat(&dungeon, &things));
    CHECK(memcmp(&dungeon, &dungeonBefore, sizeof(dungeon)) == 0);
    CHECK(memcmp(&things, &thingsBefore, sizeof(things)) == 0);
    CHECK(sft == 0xfffe);
    CHECK(!F0506_DUNGEON_ReserveFreshPc34Pools_Compat(NULL, &things));
    CHECK(!F0506_DUNGEON_ReserveFreshPc34Pools_Compat(&dungeon, NULL));
    return 0;
}

static int capped_original_records(void)
{
    struct DungeonDatState_Compat dungeon = {0};
    struct DungeonThings_Compat things = {0};
    int i;
    dungeon.loaded = things.loaded = 1;
#define SEED(kind, field, countField, bytes, limit) do { \
    int old = limit - 1; \
    dungeon.header.thingCounts[kind] = things.thingCounts[kind] = old; \
    things.countField = old; \
    things.rawThingData[kind] = malloc((size_t)old * bytes); \
    things.field = calloc((size_t)old, sizeof(*things.field)); \
    CHECK(things.rawThingData[kind] && things.field); \
    memset(things.rawThingData[kind], 0x5a, (size_t)old * bytes); \
    for (i = 0; i < old; ++i) things.field[i].next = 0x5a5a; \
} while (0)
    SEED(4, groups, groupCount, 16, 1024);
    SEED(5, weapons, weaponCount, 4, 1024);
    SEED(6, armours, armourCount, 4, 1024);
    SEED(8, potions, potionCount, 4, 1024);
    SEED(10, junks, junkCount, 4, 1024);
    SEED(14, projectiles, projectileCount, 8, 1024);
    SEED(15, explosions, explosionCount, 4, 768);
#undef SEED
    CHECK(F0506_DUNGEON_ReserveFreshPc34Pools_Compat(&dungeon, &things));
#define VERIFY(kind, field, countField, bytes, limit) do { \
    CHECK(things.countField == limit && things.thingCounts[kind] == limit); \
    CHECK(dungeon.header.thingCounts[kind] == limit); \
    for (i = 0; i < (limit - 1) * bytes; ++i) \
        CHECK(things.rawThingData[kind][i] == 0x5a); \
    for (i = 0; i < limit - 1; ++i) CHECK(things.field[i].next == 0x5a5a); \
    CHECK(things.field[limit - 1].next == 0xffff); \
    CHECK(things.rawThingData[kind][(limit - 1) * bytes] == 0xff); \
    CHECK(things.rawThingData[kind][(limit - 1) * bytes + 1] == 0xff); \
} while (0)
    VERIFY(4, groups, groupCount, 16, 1024);
    VERIFY(5, weapons, weaponCount, 4, 1024);
    VERIFY(6, armours, armourCount, 4, 1024);
    VERIFY(8, potions, potionCount, 4, 1024);
    VERIFY(10, junks, junkCount, 4, 1024);
    VERIFY(14, projectiles, projectileCount, 8, 1024);
    VERIFY(15, explosions, explosionCount, 4, 768);
#undef VERIFY
    F0504_DUNGEON_FreeThingData_Compat(&things);
    return 0;
}

int main(void)
{
    if (empty_and_idempotent() || rollback_late_validation() ||
        capped_original_records()) return 1;
    puts("ok: fresh PC34 source pools, capped preservation and atomic rollback");
    return 0;
}
