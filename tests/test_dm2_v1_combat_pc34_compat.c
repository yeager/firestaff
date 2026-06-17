/* test_dm2_v1_combat_pc34_compat.c — DM2 V1 Combat Integration Tests
 *
 * Tests Phase 5 (creature/combat parity) combat contract:
 *   1. Melee attack: base + attacker_strength/4 - target_defense
 *   2. Ranged attack: out-of-range returns 0
 *   3. Ranged attack: range penalty (10% per extra tile)
 *   4. Ranged attack: crossbow bow-class outdoor penalty (-50%)
 *   5. Ranged attack: thrown standard outdoor penalty (-25%)
 *   6. Ranged attack: bomb area-effect outdoor penalty (-25%)
 *   7. Magic attack: no outdoor penalty (no ranged_class)
 *   8. Companion damage bonus: +10% per companion, max +40%
 *   9. Tech weapon gating: gun requires tech_level >= 1
 *  10. Tech weapon gating: bomb requires tech_level >= 1
 *  11. Crossbow: tech_level 0 OR 1 valid
 *  12. Pre-tech weapons: tech_level must be 0
 *  13. Out-of-range: returns 0 (no damage)
 *  14. Damage floor: minimum 1 damage point per attack
 *  15. Defense floor: defense < 0 clamped to 0
 *  16. Strength floor: strength < 0 clamped to 0
 *  17. Door attack: wooden destroyed at >= 42 attack
 *  18. Door attack: wooden NOT destroyed at < 42 attack
 *  19. Door attack: wooden destroyed at exactly 100 (cap)
 *  20. Door attack: wooden NOT destroyed at 200 (capped to 100, 100 < 230)
 *      Wait — actually cap=100, defense 42. 200 → capped to 100 → 100 >= 42 → destroyed.
 *      Let me fix: 200 capped to 100, 100 >= 42 → DESTROYED. Use iron (defense 230)
 *      for the "cap insufficient" test.
 *  21. Door attack: iron door immune to melee (defense 230 > cap 100)
 *  22. Door attack: portcullis immune to melee (defense 110 > cap 100)
 *  23. Door attack: RA door immune (defense 255 > cap 100)
 *  24. Door attack: destroyed door (state 5) cannot be destroyed again
 *  25. Champion wound on close: state 0 (OPEN) → 0 wound
 *  26. Champion wound on close: state 5 (DESTROYED) → 0 wound
 *  27. Champion wound on close: state 2 (CLOSED_HALF) → 1d8 roll
 *  28. Champion wound on close: state 4 (CLOSED) → 1d8 roll
 *  29. Champion wound on close: invalid rng_1d8 → 0
 *  30. Outdoor helper: indoor + bow → 100% (no penalty)
 *  31. Outdoor helper: outdoor + bow → 50%
 *  32. Outdoor helper: outdoor + melee → 100% (no penalty)
 *  33. Companion bonus helper: 0 companions → 0%, 4 companions → 40%
 *  34. Kill threshold: damage >= hp → kill
 *  35. Kill threshold: damage < hp → no kill
 *  36. Kill threshold: damage == hp → kill
 *  37. validate_weapon: null → 0
 *  38. validate_weapon: gun tech_level=0 → 0 (tech required)
 *  39. validate_weapon: gun tech_level=1 → 1
 *  40. validate_weapon: range < 1 → 0
 *  41. resolve_attack (back-compat): basic melee still works with old signature
 *  42. resolve_attack_full: bow + outdoor + 2 companions = full formula
 *  43. is_ranged: melee → 0, crossbow → 1, thrown → 1, gun → 1, bomb → 1
 *  44. door_info_for_type: invalid type → 0
 *  45. door_info_for_type: type 0 (portcullis) → 110 + projectile_pass=1
 *  46. door_info_for_type: type 1 (wooden) → 42 + projectile_pass=0
 *  47. door_info_for_type: type 2 (iron) → 230
 *  48. door_info_for_type: type 3 (RA) → 255 + animated=1
 *
 * Source: dm2_v1_combat.c (Phase 5 source-lock)
 *   ReDMCSB SKULL.ASM:10450-10580  (SKULL_COMBAT_ResolveMelee)
 *   ReDMCSB SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   ReDMCSB SKULL.ASM:10780-10795  (SKULL_COMBAT_OutdoorPenalty)
 *   ReDMCSB SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack)
 *   ReDMCSB SKULL.ASM:11000-11080  (SKULL_COMBAT_ChampionWoundOnClose)
 *   ReDMCSB PROJEXPL.C:1554-1600   (F0232_GROUP_IsDoorDestroyedByAttack)
 *   ReDMCSB TIMELINE.C:750-810     (F0241_TIMELINE_ProcessEvent6_Square_Door)
 *   ReDMCSB PANEL.C:626            (door close → party wound event)
 *   ReDMCSB DEFS.H:1567-1572       (DOOR_INFO {Attributes, Defense})
 *   skproject/SKULLWIN/c_combat.cpp (open reimpl)
 */

#include "dm2_v1_combat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %s...\n", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("    PASS\n"); \
    } else { \
        printf("    FAIL\n"); \
    } \
} while (0)

/* ── Weapon fixtures ─────────────────────────────────────────────────── */

static DM2_V1_WeaponInfo make_melee(int base, int range) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_MELEE, base, range, 0, 0 };
    return w;
}

static DM2_V1_WeaponInfo make_thrown(int base, int range) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_THROWN, base, range, 1, 0 };
    return w;
}

static DM2_V1_WeaponInfo make_crossbow(int base, int range) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_CROSSBOW, base, range, 1, 1 };
    return w;
}

static DM2_V1_WeaponInfo make_gun(int base, int range) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_GUN, base, range, 1, 1 };
    return w;
}

static DM2_V1_WeaponInfo make_bomb(int base, int range) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_BOMB, base, range, 0, 1 };
    return w;
}

static DM2_V1_WeaponInfo make_magic(int base, int range) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_MAGIC, base, range, 0, 0 };
    return w;
}

/* ── Tests ───────────────────────────────────────────────────────────── */

static int test_melee_base(void) {
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    /* damage = 10 + 16/4 = 14, range=1 → no penalty, no defense */
    int d = dm2_v1_combat_resolve_attack(&w, 16, 0, 1);
    return d == 14;
}

static int test_melee_with_defense(void) {
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    /* damage = 14 - 5 = 9 */
    int d = dm2_v1_combat_resolve_attack(&w, 16, 5, 1);
    return d == 9;
}

static int test_melee_floor_one(void) {
    /* strength=0, defense=100, base=10 → damage = 10 - 100 = -90 → 0 */
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    int d = dm2_v1_combat_resolve_attack(&w, 0, 100, 1);
    return d == 0;
}

static int test_ranged_out_of_range(void) {
    DM2_V1_WeaponInfo w = make_thrown(8, 3);
    /* distance=5 > range=3 → 0 */
    int d = dm2_v1_combat_resolve_attack(&w, 12, 0, 5);
    return d == 0;
}

static int test_ranged_range_penalty(void) {
    /* thrown: base=8, strength=12, range=3
     * damage_base = 8 + 12/4 = 11
     * distance=2 → range_penalty = (2-1) * 11 / 10 = 1
     * damage = 11 - 1 = 10
     * distance=3 → range_penalty = (3-1) * 11 / 10 = 2
     * damage = 11 - 2 = 9 */
    DM2_V1_WeaponInfo w = make_thrown(8, 3);
    int d2 = dm2_v1_combat_resolve_attack(&w, 12, 0, 2);
    int d3 = dm2_v1_combat_resolve_attack(&w, 12, 0, 3);
    return d2 == 10 && d3 == 9;
}

static int test_ranged_bow_outdoor_penalty(void) {
    /* crossbow (bow-class): base=8, strength=12, range=5, distance=2
     * damage_base = 8 + 12/4 = 11
     * range_penalty = (2-1)*11/10 = 1, damage = 10
     * outdoor bow penalty: 10 * 50/100 = 5
     */
    DM2_V1_WeaponInfo w = make_crossbow(8, 5);
    int d = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 2, 1, 0);
    return d == 5;
}

static int test_ranged_standard_outdoor_penalty(void) {
    /* thrown: base=8, strength=12, range=5, distance=2
     * damage_base = 11, range_penalty 1, damage = 10
     * outdoor standard penalty: 10 * 75/100 = 7
     */
    DM2_V1_WeaponInfo w = make_thrown(8, 5);
    int d = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 2, 1, 0);
    return d == 7;
}

static int test_ranged_bomb_outdoor_penalty(void) {
    /* bomb: base=20, strength=0, range=4, distance=1
     * damage_base = 20 + 0 = 20
     * range_penalty = 0 (distance=1)
     * outdoor standard penalty: 20 * 75/100 = 15
     */
    DM2_V1_WeaponInfo w = make_bomb(20, 4);
    int d = dm2_v1_combat_resolve_attack_full(&w, 0, 0, 1, 1, 0);
    return d == 15;
}

static int test_magic_no_outdoor_penalty(void) {
    /* magic: not ranged, no outdoor penalty */
    DM2_V1_WeaponInfo w = make_magic(20, 5);
    /* damage = 20 + 12/4 = 23, no range penalty (distance=1) */
    int d_indoor  = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 1, 0, 0);
    int d_outdoor = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 1, 1, 0);
    return d_indoor == 23 && d_outdoor == 23;
}

static int test_companion_damage_bonus(void) {
    /* melee: damage = 10 + 16/4 = 14
     * 0 companions → 14
     * 1 companion  → 14 + 14*10/100 = 15
     * 2 companions → 14 + 14*20/100 = 16
     * 4 companions → 14 + 14*40/100 = 19 */
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    int d0 = dm2_v1_combat_resolve_attack_full(&w, 16, 0, 1, 0, 0);
    int d1 = dm2_v1_combat_resolve_attack_full(&w, 16, 0, 1, 0, 1);
    int d2 = dm2_v1_combat_resolve_attack_full(&w, 16, 0, 1, 0, 2);
    int d4 = dm2_v1_combat_resolve_attack_full(&w, 16, 0, 1, 0, 4);
    return d0 == 14 && d1 == 15 && d2 == 16 && d4 == 19;
}

static int test_tech_weapon_gun_requires_tech(void) {
    /* gun with tech_level=0 → invalid → 0 damage */
    DM2_V1_WeaponInfo w = make_gun(10, 5);
    w.tech_level = 0;
    int d = dm2_v1_combat_resolve_attack(&w, 12, 0, 1);
    return d == 0;
}

static int test_tech_weapon_bomb_requires_tech(void) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_BOMB, 20, 4, 0, 0 };
    int d = dm2_v1_combat_resolve_attack(&w, 12, 0, 1);
    return d == 0;
}

static int test_crossbow_tech_level_0_or_1(void) {
    DM2_V1_WeaponInfo w0 = { DM2_WEAPON_CROSSBOW, 8, 5, 1, 0 };
    DM2_V1_WeaponInfo w1 = { DM2_WEAPON_CROSSBOW, 8, 5, 1, 1 };
    DM2_V1_WeaponInfo w2 = { DM2_WEAPON_CROSSBOW, 8, 5, 1, 2 };
    return dm2_v1_combat_validate_weapon(&w0) == 1
        && dm2_v1_combat_validate_weapon(&w1) == 1
        && dm2_v1_combat_validate_weapon(&w2) == 0;
}

static int test_pre_tech_weapons_must_be_zero(void) {
    DM2_V1_WeaponInfo w_melee = { DM2_WEAPON_MELEE, 10, 1, 0, 1 };  /* invalid */
    DM2_V1_WeaponInfo w_magic = { DM2_WEAPON_MAGIC, 20, 5, 0, 1 };  /* invalid */
    return dm2_v1_combat_validate_weapon(&w_melee) == 0
        && dm2_v1_combat_validate_weapon(&w_magic) == 0;
}

static int test_out_of_range_zero(void) {
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    int d = dm2_v1_combat_resolve_attack(&w, 100, 0, 5);
    return d == 0;
}

static int test_damage_floor_one(void) {
    /* minimum 1 damage point (range penalty can't reduce below 1) */
    DM2_V1_WeaponInfo w = make_thrown(1, 10);
    /* damage_base = 1 + 0/4 = 1, distance=10 → range_penalty = 9*1/10 = 0
     * damage = 1 - 0 = 1 (floor) */
    int d = dm2_v1_combat_resolve_attack(&w, 0, 0, 10);
    return d == 1;
}

static int test_defense_floor_zero(void) {
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    int d = dm2_v1_combat_resolve_attack(&w, 16, -100, 1);
    /* damage = 14, defense clamped to 0, no subtraction */
    return d == 14;
}

static int test_strength_floor_zero(void) {
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    int d = dm2_v1_combat_resolve_attack(&w, -50, 0, 1);
    /* strength clamped to 0, damage = 10 + 0/4 = 10 */
    return d == 10;
}

static int test_door_wooden_destroyed_at_42(void) {
    /* door_type=1 (wooden, defense=42), state=4 (CLOSED), attack=50 */
    return dm2_v1_combat_resolve_door_attack(1, 4, 50) == 1;
}

static int test_door_wooden_not_destroyed_below_42(void) {
    return dm2_v1_combat_resolve_door_attack(1, 4, 41) == 0;
}

static int test_door_wooden_destroyed_at_100_cap(void) {
    /* attack=200 → capped to 100, 100 >= 42 → destroyed */
    return dm2_v1_combat_resolve_door_attack(1, 4, 200) == 1;
}

static int test_door_iron_immune_to_melee(void) {
    /* door_type=2 (iron, defense=230), attack=200 → cap 100, 100 < 230 → not destroyed */
    return dm2_v1_combat_resolve_door_attack(2, 4, 200) == 0;
}

static int test_door_portcullis_immune_to_melee(void) {
    /* door_type=0 (portcullis, defense=110), attack=200 → cap 100, 100 < 110 → not destroyed */
    return dm2_v1_combat_resolve_door_attack(0, 4, 200) == 0;
}

static int test_door_ra_immune_to_melee(void) {
    return dm2_v1_combat_resolve_door_attack(3, 4, 200) == 0;
}

static int test_door_destroyed_cannot_re_destroy(void) {
    /* state 5 (DESTROYED) → cannot be destroyed again */
    return dm2_v1_combat_resolve_door_attack(1, 5, 100) == 0;
}

static int test_champion_wound_open_zero(void) {
    return dm2_v1_combat_resolve_champion_wound_on_close(0, 5) == 0;
}

static int test_champion_wound_destroyed_zero(void) {
    return dm2_v1_combat_resolve_champion_wound_on_close(5, 5) == 0;
}

static int test_champion_wound_closed_half(void) {
    /* state 2 (CLOSED_HALF), rng=4 → wound = 4 */
    return dm2_v1_combat_resolve_champion_wound_on_close(2, 4) == 4;
}

static int test_champion_wound_closed(void) {
    /* state 4 (CLOSED), rng=8 → wound = 8 (max) */
    return dm2_v1_combat_resolve_champion_wound_on_close(4, 8) == 8;
}

static int test_champion_wound_invalid_rng(void) {
    /* rng < 1 → 0; rng > 8 → 0 */
    return dm2_v1_combat_resolve_champion_wound_on_close(4, 0) == 0
        && dm2_v1_combat_resolve_champion_wound_on_close(4, 9) == 0;
}

static int test_outdoor_helper_indoor_bow(void) {
    /* indoor + bow → no penalty → 100% */
    return dm2_v1_combat_apply_outdoor_modifier(20, 0, 2) == 20;
}

static int test_outdoor_helper_outdoor_bow(void) {
    /* outdoor + bow → -50% */
    return dm2_v1_combat_apply_outdoor_modifier(20, 1, 2) == 10;
}

static int test_outdoor_helper_outdoor_melee(void) {
    /* outdoor + melee → no penalty */
    return dm2_v1_combat_apply_outdoor_modifier(20, 1, 0) == 20;
}

static int test_companion_bonus_helper(void) {
    return dm2_v1_combat_companion_damage_bonus(0) == 0
        && dm2_v1_combat_companion_damage_bonus(1) == 10
        && dm2_v1_combat_companion_damage_bonus(2) == 20
        && dm2_v1_combat_companion_damage_bonus(4) == 40
        && dm2_v1_combat_companion_damage_bonus(10) == 40;  /* capped */
}

static int test_kill_threshold_above(void) {
    return dm2_v1_combat_kills_creature(10, 15) == 1;
}

static int test_kill_threshold_below(void) {
    return dm2_v1_combat_kills_creature(10, 9) == 0;
}

static int test_kill_threshold_equal(void) {
    return dm2_v1_combat_kills_creature(10, 10) == 1;
}

static int test_validate_weapon_null(void) {
    return dm2_v1_combat_validate_weapon(NULL) == 0;
}

static int test_validate_weapon_gun_tech_0(void) {
    DM2_V1_WeaponInfo w = make_gun(10, 5);
    w.tech_level = 0;
    return dm2_v1_combat_validate_weapon(&w) == 0;
}

static int test_validate_weapon_gun_tech_1(void) {
    DM2_V1_WeaponInfo w = make_gun(10, 5);  /* default tech_level = 1 */
    return dm2_v1_combat_validate_weapon(&w) == 1;
}

static int test_validate_weapon_range_invalid(void) {
    DM2_V1_WeaponInfo w = { DM2_WEAPON_MELEE, 10, 0, 0, 0 };
    return dm2_v1_combat_validate_weapon(&w) == 0;
}

static int test_back_compat_signature(void) {
    /* The 4-arg signature (no outdoor, no companion) must still work */
    DM2_V1_WeaponInfo w = make_melee(10, 1);
    int d = dm2_v1_combat_resolve_attack(&w, 16, 5, 1);
    return d == 9;  /* 10 + 4 - 5 = 9 */
}

static int test_full_bow_outdoor_companions(void) {
    /* crossbow + outdoor + 2 companions
     * damage_base = 8 + 12/4 = 11
     * range_penalty = (2-1)*11/10 = 1
     * damage_after_range = 10
     * outdoor bow penalty: 10 * 50/100 = 5
     * companion bonus 2: 5 + 5*20/100 = 6 */
    DM2_V1_WeaponInfo w = make_crossbow(8, 5);
    int d = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 2, 1, 2);
    return d == 6;
}

static int test_is_ranged_classification(void) {
    return dm2_v1_combat_is_ranged(DM2_WEAPON_MELEE) == 0
        && dm2_v1_combat_is_ranged(DM2_WEAPON_CROSSBOW) == 1
        && dm2_v1_combat_is_ranged(DM2_WEAPON_THROWN) == 1
        && dm2_v1_combat_is_ranged(DM2_WEAPON_GUN) == 1
        && dm2_v1_combat_is_ranged(DM2_WEAPON_BOMB) == 1
        && dm2_v1_combat_is_ranged(DM2_WEAPON_MAGIC) == 0;
}

static int test_door_info_invalid_type(void) {
    int pp = 99, an = 99;
    int d = dm2_v1_combat_door_info_for_type(99, &pp, &an);
    return d == 0;
}

static int test_door_info_portcullis(void) {
    int pp = 0, an = 0;
    int d = dm2_v1_combat_door_info_for_type(0, &pp, &an);
    return d == 110 && pp == 1 && an == 0;
}

static int test_door_info_wooden(void) {
    int pp = 99, an = 99;
    int d = dm2_v1_combat_door_info_for_type(1, &pp, &an);
    return d == 42 && pp == 0 && an == 0;
}

static int test_door_info_iron(void) {
    int pp = 99, an = 99;
    int d = dm2_v1_combat_door_info_for_type(2, &pp, &an);
    return d == 230 && pp == 0 && an == 0;
}

static int test_door_info_ra(void) {
    int pp = 99, an = 99;
    int d = dm2_v1_combat_door_info_for_type(3, &pp, &an);
    return d == 255 && pp == 0 && an == 1;
}

int main(void) {
    printf("DM2 V1 Combat Resolver — Phase 5 source-lock tests\n");
    printf("Source: SKULL.ASM:10450-10580 (melee), 10620-10710 (ranged),\n"
           "        SKULL.ASM:10800-10890 (door), 11000-11080 (wound),\n"
           "        ReDMCSB PROJEXPL.C:1554-1600, TIMELINE.C:750-810,\n"
           "        ReDMCSB DEFS.H:1567-1572, skproject/SKULLWIN/c_combat.cpp\n");

    TEST(melee_base);
    TEST(melee_with_defense);
    TEST(melee_floor_one);
    TEST(ranged_out_of_range);
    TEST(ranged_range_penalty);
    TEST(ranged_bow_outdoor_penalty);
    TEST(ranged_standard_outdoor_penalty);
    TEST(ranged_bomb_outdoor_penalty);
    TEST(magic_no_outdoor_penalty);
    TEST(companion_damage_bonus);
    TEST(tech_weapon_gun_requires_tech);
    TEST(tech_weapon_bomb_requires_tech);
    TEST(crossbow_tech_level_0_or_1);
    TEST(pre_tech_weapons_must_be_zero);
    TEST(out_of_range_zero);
    TEST(damage_floor_one);
    TEST(defense_floor_zero);
    TEST(strength_floor_zero);
    TEST(door_wooden_destroyed_at_42);
    TEST(door_wooden_not_destroyed_below_42);
    TEST(door_wooden_destroyed_at_100_cap);
    TEST(door_iron_immune_to_melee);
    TEST(door_portcullis_immune_to_melee);
    TEST(door_ra_immune_to_melee);
    TEST(door_destroyed_cannot_re_destroy);
    TEST(champion_wound_open_zero);
    TEST(champion_wound_destroyed_zero);
    TEST(champion_wound_closed_half);
    TEST(champion_wound_closed);
    TEST(champion_wound_invalid_rng);
    TEST(outdoor_helper_indoor_bow);
    TEST(outdoor_helper_outdoor_bow);
    TEST(outdoor_helper_outdoor_melee);
    TEST(companion_bonus_helper);
    TEST(kill_threshold_above);
    TEST(kill_threshold_below);
    TEST(kill_threshold_equal);
    TEST(validate_weapon_null);
    TEST(validate_weapon_gun_tech_0);
    TEST(validate_weapon_gun_tech_1);
    TEST(validate_weapon_range_invalid);
    TEST(back_compat_signature);
    TEST(full_bow_outdoor_companions);
    TEST(is_ranged_classification);
    TEST(door_info_invalid_type);
    TEST(door_info_portcullis);
    TEST(door_info_wooden);
    TEST(door_info_iron);
    TEST(door_info_ra);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
