#include "dm1_v1_unused_launcher_object_f0166_f0167_pc34_compat.h"

#include <assert.h>
#include <string.h>

static __attribute__((unused)) uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)((value >> 8) & 0xffu);
}

static __attribute__((unused)) uint16_t thing_ref(int type, int index)
{
    return (uint16_t)(((type & 15) << 10) | (index & 0x03ff));
}

static void build_context(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    uint8_t weaponRecords[4][4],
    uint8_t junkRecords[3][4])
{
    memset(context, 0, sizeof(*context));
    memset(weaponRecords, 0, 4 * 4);
    memset(junkRecords, 0, 3 * 4);

    write_le16(weaponRecords[0], DM1_V1_F0166_THING_END_OF_LIST_PC34);
    write_le16(weaponRecords[1], DM1_V1_F0166_THING_NONE_PC34);
    write_le16(weaponRecords[2], DM1_V1_F0166_THING_NONE_PC34);
    write_le16(weaponRecords[3], DM1_V1_F0166_THING_END_OF_LIST_PC34);
    write_le16(junkRecords[0], DM1_V1_F0166_THING_END_OF_LIST_PC34);
    write_le16(junkRecords[1], DM1_V1_F0166_THING_NONE_PC34);
    write_le16(junkRecords[2], DM1_V1_F0166_THING_END_OF_LIST_PC34);

    context->thingData[DM1_V1_F0166_THING_TYPE_WEAPON_PC34].records =
        &weaponRecords[0][0];
    context->thingData[DM1_V1_F0166_THING_TYPE_WEAPON_PC34].recordCount = 4;
    context->thingData[DM1_V1_F0166_THING_TYPE_WEAPON_PC34].recordSize = 4;
    context->thingData[DM1_V1_F0166_THING_TYPE_JUNK_PC34].records =
        &junkRecords[0][0];
    context->thingData[DM1_V1_F0166_THING_TYPE_JUNK_PC34].recordCount = 3;
    context->thingData[DM1_V1_F0166_THING_TYPE_JUNK_PC34].recordSize = 4;
}

static void test_source_evidence(void)
{
    const char *evidence = DM1_V1_F0166_F0167_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != 0);
    assert(strstr(evidence, "F0166_DUNGEON_GetUnusedThing") != 0);
    assert(strstr(evidence,
                  "F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator") != 0);
}

static void test_f0166_unused_scan(void)
{
    DM1_V1_UnusedThingContextF0166Pc34 context;
    DM1_V1_UnusedThingResultF0166Pc34 result;
    uint8_t weaponRecords[4][4];
    uint8_t junkRecords[3][4];

    build_context(&context, weaponRecords, junkRecords);
    memset(&result, 0x7f, sizeof(result));
    assert(DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
        &context, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, &result) == 1);
    assert(result.valid == 1);
    assert(result.thingType == DM1_V1_F0166_THING_TYPE_WEAPON_PC34);
    assert(result.thingIndex == 1);
    assert(result.thing == thing_ref(DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 1));
    assert(result.record == &weaponRecords[1][0]);
    assert(F0166_DUNGEON_GetUnusedThing(
               &context, DM1_V1_F0166_THING_TYPE_JUNK_PC34) ==
           thing_ref(DM1_V1_F0166_THING_TYPE_JUNK_PC34, 1));
}

static void test_f0167_launcher_mapping_and_allocation(void)
{
    DM1_V1_UnusedThingContextF0166Pc34 context;
    DM1_V1_LauncherObjectResultF0167Pc34 launcher;
    uint8_t weaponRecords[4][4];
    uint8_t junkRecords[3][4];
    uint16_t thing;
    (void)thing;

    build_context(&context, weaponRecords, junkRecords);

    thing = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
        &context, 6, &launcher);
    assert(thing == thing_ref(DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 1));
    assert(launcher.valid == 1);
    assert(launcher.iconIndex == 6);
    assert(launcher.normalizedIconIndex == 4);
    assert(launcher.thingType == DM1_V1_F0166_THING_TYPE_WEAPON_PC34);
    assert(launcher.itemType == 2);
    assert(read_le16(weaponRecords[1]) ==
           DM1_V1_F0166_THING_END_OF_LIST_PC34);
    assert(read_le16(weaponRecords[1] + 2) == 2);

    thing = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
        &context, 51, &launcher);
    assert(thing == thing_ref(DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 2));
    assert(launcher.normalizedIconIndex == 51);
    assert(launcher.itemType == 27);
    assert(read_le16(weaponRecords[2] + 2) == 27);

    thing = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
        &context, 128, &launcher);
    assert(thing == thing_ref(DM1_V1_F0166_THING_TYPE_JUNK_PC34, 1));
    assert(launcher.thingType == DM1_V1_F0166_THING_TYPE_JUNK_PC34);
    assert(launcher.itemType == 25);
    assert(read_le16(junkRecords[1] + 2) == 25);
}

static void test_f0167_full_supported_icon_subset(void)
{
    static const struct {
        int icon;
        int normalized;
        int thingType;
        int itemType;
    } cases[] = {
        { 4, 4, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 2 },
        { 7, 4, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 2 },
        { 32, 32, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 8 },
        { 52, 52, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 28 },
        { 54, 54, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 30 },
        { 55, 55, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 31 },
        { 56, 56, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 32 },
        { 128, 128, DM1_V1_F0166_THING_TYPE_JUNK_PC34, 25 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_UnusedThingContextF0166Pc34 context;
        DM1_V1_LauncherObjectResultF0167Pc34 launcher;
        uint8_t weaponRecords[4][4];
        uint8_t junkRecords[3][4];
        uint16_t thing;
        (void)thing;

        build_context(&context, weaponRecords, junkRecords);
        thing = F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
            &context, cases[i].icon, &launcher);
        assert(thing != DM1_V1_F0166_THING_NONE_PC34);
        assert(launcher.valid == 1);
        assert(launcher.normalizedIconIndex == cases[i].normalized);
        assert(launcher.thingType == cases[i].thingType);
        assert(launcher.itemType == cases[i].itemType);
    }
}

static void test_fail_closed_inputs(void)
{
    DM1_V1_UnusedThingContextF0166Pc34 context;
    DM1_V1_UnusedThingResultF0166Pc34 result;
    DM1_V1_LauncherObjectResultF0167Pc34 launcher;
    (void)launcher;
    uint8_t weaponRecords[4][4];
    uint8_t junkRecords[3][4];

    build_context(&context, weaponRecords, junkRecords);
    assert(F0166_DUNGEON_GetUnusedThing(&context, -1) ==
           DM1_V1_F0166_THING_NONE_PC34);
    assert(F0166_DUNGEON_GetUnusedThing(&context, 12) ==
           DM1_V1_F0166_THING_NONE_PC34);
    assert(DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
        &context, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, 0) == 1);

    write_le16(weaponRecords[1], DM1_V1_F0166_THING_END_OF_LIST_PC34);
    write_le16(weaponRecords[2], DM1_V1_F0166_THING_END_OF_LIST_PC34);
    memset(&result, 0x7f, sizeof(result));
    assert(DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
        &context, DM1_V1_F0166_THING_TYPE_WEAPON_PC34, &result) == 0);
    assert(result.valid == 0);
    assert(result.thingType == DM1_V1_F0166_THING_TYPE_WEAPON_PC34);

    assert(F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
               &context, 99, &launcher) == DM1_V1_F0166_THING_NONE_PC34);
    assert(launcher.valid == 0);
    assert(launcher.iconIndex == 99);

    assert(F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
               &context, 32, &launcher) == DM1_V1_F0166_THING_NONE_PC34);
    assert(launcher.valid == 0);
    assert(launcher.thingType == DM1_V1_F0166_THING_TYPE_WEAPON_PC34);
    assert(launcher.itemType == 8);
}

int main(void)
{
    test_source_evidence();
    test_f0166_unused_scan();
    test_f0167_launcher_mapping_and_allocation();
    test_f0167_full_supported_icon_subset();
    test_fail_closed_inputs();
    return 0;
}
