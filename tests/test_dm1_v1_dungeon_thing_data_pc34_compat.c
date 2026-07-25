#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_get_thing_data_null(void)
{
    const unsigned char *data = dm1_v1_dungeon_get_thing_data_pc34(NULL, 0);
    (void)data;
    assert(data == NULL);
}

static void test_get_thing_data_empty(void)
{
    struct DungeonThings_Compat things;
    memset(&things, 0, sizeof(things));
    const unsigned char *data = dm1_v1_dungeon_get_thing_data_pc34(&things, 0);
    (void)data;
    assert(data == NULL);
}

static void test_get_object_subtype_null(void)
{
    int sub = dm1_v1_dungeon_get_object_subtype_pc34(NULL, 0);
    (void)sub;
    assert(sub == -1);
}

static void test_get_object_info_index_null(void)
{
    int info = dm1_v1_dungeon_get_object_info_index_pc34(NULL, 0);
    (void)info;
    assert(info == -1);
}

static void test_get_object_type_null(void)
{
    int t = dm1_v1_dungeon_get_object_type_pc34(NULL, 0);
    (void)t;
    assert(t == -1);
}

static void test_get_object_allowed_slots_null(void)
{
    unsigned int slots = dm1_v1_dungeon_get_object_allowed_slots_pc34(NULL, 0);
    (void)slots;
    assert(slots == 0);
}

static void test_get_object_icon_index_null(void)
{
    int icon = dm1_v1_dungeon_get_object_icon_index_pc34(NULL, 0, 0);
    (void)icon;
    assert(icon == -1);
}

static void test_f7017_null(void)
{
    int icon = F7017_GetIconIndex(NULL, 0);
    (void)icon;
    assert(icon == -1);
}

static void test_f7018_null(void)
{
    const unsigned char *data = F7018_GetThingData(NULL, 0);
    (void)data;
    assert(data == NULL);
}

static void test_f7019_null(void)
{
    int info = F7019_GetObjectInfoIndex(NULL, 0);
    (void)info;
    assert(info == -1);
}

static void test_cedt004_source_evidence(void)
{
    const char *ev = F7017_F7018_F7019_CEDT004_SourceEvidencePc34();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_source_evidence(void)
{
    const char *ev = dm1_v1_dungeon_thing_data_source_evidence_pc34();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_get_creature_attributes_null(void)
{
    unsigned short attrs = 0;
    int rc = dm1_v1_dungeon_get_creature_attributes_f0144_pc34(NULL, 0, &attrs);
    (void)rc;
    assert(rc == 0);
}

static void test_creature_allowed_on_map_null(void)
{
    DM1_V1_F0139_CreatureAllowedOnMapReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    int rc = dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
        NULL, NULL, 0, 0, &receipt);
    (void)rc;
    assert(rc == 0);
}

static void test_group_cells_null(void)
{
    unsigned int cells = 0;
    int rc = dm1_v1_dungeon_get_group_cells_f0145_pc34(
        NULL, NULL, 0, 0, 0, 0, &cells);
    (void)rc;
    assert(rc == 0);
}

static void test_group_directions_null(void)
{
    unsigned int dirs = 0;
    int rc = dm1_v1_dungeon_get_group_directions_f0147_pc34(
        NULL, NULL, 0, 0, 0, 0, &dirs);
    (void)rc;
    assert(rc == 0);
}

static void test_group_creature_ordinal_null(void)
{
    int ord = dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
        NULL, 0, 0, 0, 0);
    (void)ord;
    assert(ord == 0);
}

int main(void)
{
    test_get_thing_data_null();
    test_get_thing_data_empty();
    test_get_object_subtype_null();
    test_get_object_info_index_null();
    test_get_object_type_null();
    test_get_object_allowed_slots_null();
    test_get_object_icon_index_null();
    test_f7017_null();
    test_f7018_null();
    test_f7019_null();
    test_cedt004_source_evidence();
    test_source_evidence();
    test_get_creature_attributes_null();
    test_creature_allowed_on_map_null();
    test_group_cells_null();
    test_group_directions_null();
    test_group_creature_ordinal_null();

    puts("ok: DM1 dungeon thing data (Q-DM1-04) 17 tests passed");
    return 0;
}
