#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_champion_lifecycle_pc34_compat.h"

static void test_select_null_safety(void)
{
    DM2_V1_SelectChampionReceipt receipt;
    int r = dm2_v1_select_champion(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_select_champion(NULL, NULL);
    assert(r == 0);
    printf("  PASS: select_null_safety\n");
}

static void test_select_party_full(void)
{
    DM2_V1_SelectChampionReceipt receipt;
    DM2_V1_SelectChampionRequest req;
    memset(&req, 0, sizeof(req));
    req.heroes_in_party = 4;
    int r = dm2_v1_select_champion(&req, &receipt);
    assert(r == 0);
    assert(receipt.party_full == 1);
    printf("  PASS: select_party_full\n");
}

static void test_select_without_source_record_fails_closed(void)
{
    DM2_V1_SelectChampionReceipt receipt;
    DM2_V1_SelectChampionRequest req;
    memset(&req, 0, sizeof(req));
    req.tile_x = 5;
    req.tile_y = 10;
    req.direction = 2;
    req.map_level = 0;
    req.heroes_in_party = 0;
    int r = dm2_v1_select_champion(&req, &receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.champion_selected == 0);
    assert(receipt.hero_index == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_without_source_record_fails_closed\n");
}

static void test_select_second_hero(void)
{
    DM2_V1_SelectChampionReceipt receipt;
    DM2_V1_SelectChampionRequest req;
    memset(&req, 0, sizeof(req));
    req.heroes_in_party = 2;
    int r = dm2_v1_select_champion(&req, &receipt);
    assert(r == 0);
    assert(receipt.hero_index == 2);
    assert(receipt.champion_selected == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_second_hero\n");
}

static void test_select_requires_exact_source_marker_identity(void)
{
    DM2_V1_SelectChampionReceipt receipt;
    DM2_V1_SelectChampionRequest req;
    DM2_V1_G1ChampionMirrorReceipt mirrors;

    memset(&mirrors, 0, sizeof(mirrors));
    mirrors.committed = 1;
    mirrors.mirror_count = 1;
    mirrors.mirrors[0].map = 0;
    mirrors.mirrors[0].x = 5;
    mirrors.mirrors[0].y = 10;
    mirrors.mirrors[0].direction = 2;
    mirrors.mirrors[0].actuator_data = 0x1ffu;
    mirrors.mirrors[0].dynamic_hero_type = 0xffu;
    mirrors.mirrors[0].dynamic_load_id = 0x1500ffffu;

    memset(&req, 0, sizeof(req));
    req.tile_x = 5;
    req.tile_y = 10;
    req.direction = 2;
    req.map_level = 0;
    req.source_mirrors = &mirrors;
    assert(dm2_v1_select_champion(&req, &receipt) == 0);
    assert(receipt.fail_closed == 1);
    assert(receipt.source_mirror_bound == 0);

    mirrors.mirrors[0].dynamic_load_id =
        DM2_V1_G1_CHAMPION_DYN4_RESOURCE_ID;
    assert(dm2_v1_select_champion(&req, &receipt) == 0);
    assert(receipt.source_mirror_bound == 1);
    assert(receipt.hero_type == -1);
    req.direction = 1;
    assert(dm2_v1_select_champion(&req, &receipt) == 0);
    assert(receipt.source_mirror_bound == 0);

    /* Other source-authored hero types retain their own derived selector;
     * the lifecycle seam must not overfit the PC G1 type-0xff case. */
    mirrors.mirrors[0].dynamic_hero_type = 2u;
    mirrors.mirrors[0].dynamic_load_id = 0x1602ffffu;
    req.direction = 2;
    assert(dm2_v1_select_champion(&req, &receipt) == 0);
    assert(receipt.source_mirror_bound == 1);
    assert(receipt.hero_type == 2);

    printf("  PASS: select_requires_exact_source_marker_identity\n");
}

static void test_revive_null_safety(void)
{
    DM2_V1_BringChampionToLifeReceipt receipt;
    int r = dm2_v1_bring_champion_to_life(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_bring_champion_to_life(NULL, NULL);
    assert(r == 0);
    printf("  PASS: revive_null_safety\n");
}

static void test_revive_invalid_index(void)
{
    DM2_V1_BringChampionToLifeReceipt receipt;
    DM2_V1_BringChampionToLifeRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 5;
    req.current_max_hp = 100;
    int r = dm2_v1_bring_champion_to_life(&req, &receipt);
    assert(r == 0);
    printf("  PASS: revive_invalid_index\n");
}

static void test_revive_hp_penalty(void)
{
    DM2_V1_BringChampionToLifeReceipt receipt;
    DM2_V1_BringChampionToLifeRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 0;
    req.current_max_hp = 200;
    int r = dm2_v1_bring_champion_to_life(&req, &receipt);
    assert(r == 1);
    assert(receipt.champion_revived == 1);
    /* 200 - 200/64 - 1 = 200 - 3 - 1 = 196 */
    assert(receipt.new_max_hp == 196);
    assert(receipt.new_cur_hp == 98);
    printf("  PASS: revive_hp_penalty\n");
}

static void test_revive_min_hp(void)
{
    DM2_V1_BringChampionToLifeReceipt receipt;
    DM2_V1_BringChampionToLifeRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 1;
    req.current_max_hp = 26;
    int r = dm2_v1_bring_champion_to_life(&req, &receipt);
    assert(r == 1);
    /* 26 - 26/64 - 1 = 26 - 0 - 1 = 25, clamped to 25 */
    assert(receipt.new_max_hp == 25);
    assert(receipt.new_cur_hp == 12);
    printf("  PASS: revive_min_hp\n");
}

static void test_revive_very_low_hp(void)
{
    DM2_V1_BringChampionToLifeReceipt receipt;
    DM2_V1_BringChampionToLifeRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_index = 2;
    req.current_max_hp = 10;
    int r = dm2_v1_bring_champion_to_life(&req, &receipt);
    assert(r == 1);
    /* 10 - 10/64 - 1 = 10 - 0 - 1 = 9, clamped to 25 */
    assert(receipt.new_max_hp == 25);
    assert(receipt.new_cur_hp == 12);
    printf("  PASS: revive_very_low_hp\n");
}

int main(void)
{
    printf("test_dm2_v1_champion_lifecycle_pc34_compat:\n");
    test_select_null_safety();
    test_select_party_full();
    test_select_without_source_record_fails_closed();
    test_select_second_hero();
    test_select_requires_exact_source_marker_identity();
    test_revive_null_safety();
    test_revive_invalid_index();
    test_revive_hp_penalty();
    test_revive_min_hp();
    test_revive_very_low_hp();
    printf("All champion lifecycle tests passed.\n");
    return 0;
}
