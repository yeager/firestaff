#include "dm2_v1_skill_query_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM2_V1_HeroSkills make_hero(void)
{
    DM2_V1_HeroSkills h;
    memset(&h, 0, sizeof(h));
    return h;
}

static void test_override_mode(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 10000;
    assert(dm2_v1_query_player_skill_lv(&h, 0, 1, 1) == 1);
    printf("test_override_mode OK\n");
}

static void test_null_hero(void)
{
    assert(dm2_v1_query_player_skill_lv(NULL, 0, 0, 0) == 1);
    printf("test_null_hero OK\n");
}

static void test_invalid_index(void)
{
    DM2_V1_HeroSkills h = make_hero();
    assert(dm2_v1_query_player_skill_lv(&h, -1, 0, 0) == 1);
    assert(dm2_v1_query_player_skill_lv(&h, 20, 0, 0) == 1);
    printf("test_invalid_index OK\n");
}

static void test_zero_xp(void)
{
    DM2_V1_HeroSkills h = make_hero();
    assert(dm2_v1_query_player_skill_lv(&h, 0, 0, 0) == 1);
    printf("test_zero_xp OK\n");
}

static void test_class_skill_level1(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x1ff;
    assert(dm2_v1_query_player_skill_lv(&h, 0, 0, 0) == 1);
    printf("test_class_skill_level1 OK\n");
}

static void test_class_skill_level2(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x200;
    assert(dm2_v1_query_player_skill_lv(&h, 0, 0, 0) == 2);
    printf("test_class_skill_level2 OK\n");
}

static void test_class_skill_level3(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x400;
    assert(dm2_v1_query_player_skill_lv(&h, 0, 0, 0) == 3);
    printf("test_class_skill_level3 OK\n");
}

static void test_class_skill_high(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x4000;
    /* 0x4000 halves 7 times before < 0x200: levels = 1 + 7 = 8.
     * Wait: 0x4000>>1=0x2000(2), >>2=0x1000(3), >>3=0x800(4),
     * >>4=0x400(5), >>5=0x200(6), 0x200>=0x200 so >>6=0x100(7).
     * 0x100 < 0x200, stop. level = 1 + 6 = 7. */
    assert(dm2_v1_query_player_skill_lv(&h, 0, 0, 0) == 7);
    printf("test_class_skill_high OK\n");
}

static void test_sub_skill_no_parent(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[4] = 0x500;
    /* Sub-skill index 4: parent class = (4-4)/4 = 0 => skill[4/4-1] = skill[0] = 0
     * xp = (0x500 + 0*1) >> 1 = 0x280
     * 0x280 >= 0x200 => level 2 */
    assert(dm2_v1_query_player_skill_lv(&h, 4, 0, 0) == 2);
    printf("test_sub_skill_no_parent OK\n");
}

static void test_sub_skill_with_parent(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x200;
    h.skill[4] = 0x200;
    /* Sub-skill index 4, no bonus:
     * parent_skill_idx = 4/4 - 1 = 0, multiplier = 1
     * xp = (0x200 + 0x200*1) >> 1 = 0x200
     * level 2 */
    assert(dm2_v1_query_player_skill_lv(&h, 4, 0, 0) == 2);
    printf("test_sub_skill_with_parent OK\n");
}

static void test_sub_skill_with_bonus(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x200;
    h.skill[4] = 0x200;
    h.sbonus[0] = 2;
    /* Sub-skill index 4, use_bonus=1:
     * parent_class = (4-4)/4 = 0, multiplier = sbonus[0]+1 = 3
     * parent_skill_idx = 0, parent xp = 0x200
     * xp = (0x200 + 0x200*3) >> 1 = (0x200 + 0x600) >> 1 = 0x400
     * 0x400 >= 0x200 => shift, level 2, 0x200 >= 0x200 => shift, level 3
     * Then add sbonus[4] = 0, so level 3 */
    assert(dm2_v1_query_player_skill_lv(&h, 4, 1, 0) == 3);
    printf("test_sub_skill_with_bonus OK\n");
}

static void test_sbonus_adds_to_level(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x200;
    h.sbonus[0] = 3;
    /* Base: xp 0x200 => level 2, then +3 = 5 */
    assert(dm2_v1_query_player_skill_lv(&h, 0, 1, 0) == 5);
    printf("test_sbonus_adds_to_level OK\n");
}

static void test_sbonus_negative_clamped(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[0] = 0x200;
    h.sbonus[0] = -5;
    /* Base: level 2, + (-5) = -3, clamped to 1 */
    assert(dm2_v1_query_player_skill_lv(&h, 0, 1, 0) == 1);
    printf("test_sbonus_negative_clamped OK\n");
}

static void test_wizard_sub_skill(void)
{
    DM2_V1_HeroSkills h = make_hero();
    h.skill[3] = 0x400;
    h.skill[16] = 0x100;
    /* Sub-skill 16: parent_skill_idx = 16/4 - 1 = 3
     * parent_class = (16-4)/4 = 3
     * multiplier = 1 (no bonus)
     * xp = (0x100 + 0x400*1) >> 1 = 0x500 >> 1 = 0x280
     * 0x280 >= 0x200 => level 2 */
    assert(dm2_v1_query_player_skill_lv(&h, 16, 0, 0) == 2);
    printf("test_wizard_sub_skill OK\n");
}

int main(void)
{
    test_override_mode();
    test_null_hero();
    test_invalid_index();
    test_zero_xp();
    test_class_skill_level1();
    test_class_skill_level2();
    test_class_skill_level3();
    test_class_skill_high();
    test_sub_skill_no_parent();
    test_sub_skill_with_parent();
    test_sub_skill_with_bonus();
    test_sbonus_adds_to_level();
    test_sbonus_negative_clamped();
    test_wizard_sub_skill();
    printf("All dm2_v1_skill_query tests passed.\n");
    return 0;
}
