#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <assert.h>
#include <string.h>

unsigned short F0511_DUNGEON_GetSquareFirstThing_Compat(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things,
    int mapIndex,
    int mapX,
    int mapY)
{
    (void)dungeon;
    (void)things;
    (void)mapIndex;
    (void)mapX;
    (void)mapY;
    return THING_ENDOFLIST;
}

unsigned short F0512_DUNGEON_GetThingNext_Compat(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    (void)things;
    (void)thing;
    return THING_ENDOFLIST;
}

static unsigned short make_thing(unsigned int type, unsigned int index)
{
    return (unsigned short)(((type & 0x0fu) << 10) | (index & 0x03ffu));
}

static void test_cedt004_wrappers_read_loaded_raw_thing_data(void)
{
    struct DungeonThings_Compat things;
    unsigned char weapon_records[8];
    unsigned short torch = make_thing(THING_TYPE_WEAPON, 0);
    (void)torch;
    unsigned short axe = make_thing(THING_TYPE_WEAPON, 1);
    (void)axe;

    memset(&things, 0, sizeof(things));
    memset(weapon_records, 0, sizeof(weapon_records));

    things.loaded = 1;
    things.rawThingData[THING_TYPE_WEAPON] = weapon_records;
    things.thingCounts[THING_TYPE_WEAPON] = 2;

    weapon_records[2] = 2;  /* OBJECT_INFO 23 + 2 -> torch icon family. */
    weapon_records[3] = 0x90u; /* Source torch charge variant: lit stage 3. */

    weapon_records[4 + 2] = 10; /* OBJECT_INFO 23 + 10 -> weapon icon 33. */

    assert(F7018_GetThingData(&things, torch) == &weapon_records[0]);
    assert(F7018_GetThingData(&things, axe) == &weapon_records[4]);
    assert(F7019_GetObjectInfoIndex(&things, torch) == 25);
    assert(F7019_GetObjectInfoIndex(&things, axe) == 33);
    assert(F7017_GetIconIndex(&things, torch) == 7);
    assert(F7017_GetIconIndex(&things, axe) == 34);
}

static void test_cedt004_wrappers_fail_closed_without_raw_records(void)
{
    struct DungeonThings_Compat things;
    unsigned short weapon = make_thing(THING_TYPE_WEAPON, 0);
    (void)weapon;

    memset(&things, 0, sizeof(things));

    assert(F7018_GetThingData(0, weapon) == 0);
    assert(F7019_GetObjectInfoIndex(0, weapon) == -1);
    assert(F7017_GetIconIndex(0, weapon) == -1);

    things.loaded = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 1;

    assert(F7018_GetThingData(&things, weapon) == 0);
    assert(F7019_GetObjectInfoIndex(&things, weapon) == -1);
    assert(F7017_GetIconIndex(&things, weapon) == -1);
    assert(F7018_GetThingData(&things, THING_NONE) == 0);
    assert(F7019_GetObjectInfoIndex(&things, THING_ENDOFLIST) == -1);
}

static void test_cedt004_source_evidence_names_no_synthetic_boundaries(void)
{
    const char *evidence = F7017_F7018_F7019_CEDT004_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "CEDT004.C:188") != 0);
    assert(strstr(evidence, "CEDT004.C:203") != 0);
    assert(strstr(evidence, "CEDT004.C:196") != 0);
    assert(strstr(evidence, "DUNGEON.C F0156/F0141") != 0);
    assert(strstr(evidence, "OBJECT.C F0033") != 0);
    assert(strstr(evidence, "do not synthesize") != 0);
    assert(strstr(evidence, "graphics resources") != 0);
}

int main(void)
{
    test_cedt004_wrappers_read_loaded_raw_thing_data();
    test_cedt004_wrappers_fail_closed_without_raw_records();
    test_cedt004_source_evidence_names_no_synthetic_boundaries();
    return 0;
}
