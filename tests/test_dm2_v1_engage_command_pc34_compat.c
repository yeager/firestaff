/*
 * test_dm2_v1_engage_command_pc34_compat.c — unit tests for the
 * DM2 hand action dispatcher.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_engage_command_pc34_compat.h"

static DM2_V1_EngageCommandRequest make_request(int action_id)
{
    DM2_V1_EngageCommandRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_alive = 1;
    req.hero_hp = 100;
    req.hero_max_hp = 200;
    req.hero_mp = 50;
    req.hero_abs_dir = 0;
    req.item_handle = -1;
    req.creature_at_target = -1;
    req.cmd.action_id = (int16_t)action_id;
    req.cmd.delay = 64;
    req.cmd.power_random = 10;
    req.cmd.skill_exp = 5;
    return req;
}

static void test_null_safety(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    int r = dm2_v1_engage_command(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_engage_command(NULL, NULL);
    assert(r == 0);

    printf("  PASS: null_safety\n");
}

static void test_dead_hero(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(1);
    req.hero_alive = 0;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);
    assert(receipt.hero_dead == 1);

    printf("  PASS: dead_hero\n");
}

static void test_attack(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(2); /* action_id 2 -> case 1 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.attack_queued == 1);
    assert(receipt.action_type == DM2_ENGAGE_ATTACK);
    assert(receipt.cooldown_applied == 64);

    printf("  PASS: attack\n");
}

static void test_attack_min_delay(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(2);
    req.cmd.delay = 10; /* below min 32 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.cooldown_applied == 32);

    printf("  PASS: attack_min_delay\n");
}

static void test_cast_missile(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(3); /* action_id 3 -> case 2 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.spell_cast == 1);
    assert(receipt.fail_closed == 1);

    printf("  PASS: cast_missile\n");
}

static void test_wield_weapon(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(4); /* case 3 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.weapon_wielded == 1);

    printf("  PASS: wield_weapon\n");
}

static void test_confuse(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(5); /* case 4 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.creature_confused == 1);

    printf("  PASS: confuse\n");
}

static void test_light(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(6); /* case 5 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.light_toggled == 1);

    printf("  PASS: light\n");
}

static void test_consume(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(16); /* case 15 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.consumed == 1);

    printf("  PASS: consume\n");
}

static void test_heal_self(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(36); /* case 35 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.healed_self == 1);

    printf("  PASS: heal_self\n");
}

static void test_heal_self_full_hp(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(36);
    req.hero_hp = req.hero_max_hp;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);
    assert(receipt.healed_self == 0);

    printf("  PASS: heal_self_full_hp\n");
}

static void test_heal_self_no_mp(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(36);
    req.hero_mp = 0;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);

    printf("  PASS: heal_self_no_mp\n");
}

static void test_release_minion(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(48); /* case 47 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.minion_released == 1);

    printf("  PASS: release_minion\n");
}

static void test_epilogue_stamina(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(2); /* attack */
    req.cmd.power_random = 15;
    req.cmd.skill_exp = 8;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.stamina_cost == 15);
    assert(receipt.skill_exp_gained == 8);

    printf("  PASS: epilogue_stamina\n");
}

static void test_noop_action(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(1); /* case 0 = noop */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);
    assert(receipt.success == 0);

    printf("  PASS: noop_action\n");
}

int main(void)
{
    printf("test_dm2_v1_engage_command_pc34_compat:\n");
    test_null_safety();
    test_dead_hero();
    test_attack();
    test_attack_min_delay();
    test_cast_missile();
    test_wield_weapon();
    test_confuse();
    test_light();
    test_consume();
    test_heal_self();
    test_heal_self_full_hp();
    test_heal_self_no_mp();
    test_release_minion();
    test_epilogue_stamina();
    test_noop_action();
    printf("All engage command tests passed.\n");
    return 0;
}
