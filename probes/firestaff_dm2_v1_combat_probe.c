/* firestaff_dm2_v1_combat_probe.c — DM2 V1 Combat Headless Probe
 *
 * Phase 5 (creature/combat parity) headless verification probe.
 * Runs without game data, headless-safe, deterministic, and self-checks
 * the full DM2 V1 combat contract end-to-end.
 *
 * Source-lock:
 *   ReDMCSB SKULL.ASM:10450-10580  (SKULL_COMBAT_ResolveMelee)
 *   ReDMCSB SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   ReDMCSB SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack)
 *   ReDMCSB SKULL.ASM:11000-11080  (SKULL_COMBAT_ChampionWoundOnClose)
 *   ReDMCSB PROJEXPL.C:1554-1600   (F0232_GROUP_IsDoorDestroyedByAttack)
 *   ReDMCSB TIMELINE.C:750-810     (F0241_TIMELINE_ProcessEvent6_Square_Door)
 *   ReDMCSB DEFS.H:1567-1572       (DOOR_INFO)
 *   skproject/SKULLWIN/c_combat.cpp (open reimpl)
 *
 * Probe contract:
 *   - Returns 0 on success, 1 on first failure.
 *   - Prints a single PASS/FAIL summary line at the end.
 *   - Each assertion increments a counter; counters are checked at end.
 *
 * Coverage (12 assertions):
 *   1.  Melee damage formula
 *   2.  Ranged out-of-range returns 0
 *   3.  Ranged range penalty (10%/tile)
 *   4.  Crossbow bow-class outdoor penalty (-50%)
 *   5.  Thrown standard outdoor penalty (-25%)
 *   6.  Magic has no outdoor penalty
 *   7.  Companion damage bonus (+10% per companion, max +40%)
 *   8.  Tech weapon gating: gun tech_level=0 → 0 damage
 *   9.  Door destruction: wooden (defense 42) at attack 50 → destroyed
 *  10.  Door destruction: iron (defense 230) immune to melee (cap 100)
 *  11.  Champion wound on close: state 4 + rng 5 → 5
 *  12.  Door info for type: portcullis (0) → 110 + projectile_pass=1
 */

#include "dm2_v1_combat.h"
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures   = 0;

#define CHECK(cond_) do { \
    g_assertions++; \
    if (!(cond_)) { \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond_); \
        g_failures++; \
    } \
} while (0)

int main(void) {
    printf("DM2 V1 Combat Resolver — Phase 5 headless probe\n");
    printf("Source: SKULL.ASM:10450-10580, 10620-10710, 10800-10890, 11000-11080\n");

    /* 1. Melee damage formula: 10 + 16/4 - 0 = 14 */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_MELEE, 10, 1, 0, 0 };
        int d = dm2_v1_combat_resolve_attack(&w, 16, 0, 1);
        CHECK(d == 14);
    }

    /* 2. Ranged out-of-range: range=3, distance=5 → 0 */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_THROWN, 8, 3, 1, 0 };
        int d = dm2_v1_combat_resolve_attack(&w, 12, 0, 5);
        CHECK(d == 0);
    }

    /* 3. Ranged range penalty: thrown 8/3, strength 12, dist 3
     *    base = 8 + 3 = 11, range_penalty = (3-1)*11/10 = 2, damage = 9 */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_THROWN, 8, 3, 1, 0 };
        int d = dm2_v1_combat_resolve_attack(&w, 12, 0, 3);
        CHECK(d == 9);
    }

    /* 4. Crossbow bow-class outdoor: base 8, str 12, range 5, dist 2
     *    base = 11, range_pen = 1, damage = 10, outdoor bow = 5 */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_CROSSBOW, 8, 5, 1, 1 };
        int d = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 2, 1, 0);
        CHECK(d == 5);
    }

    /* 5. Thrown standard outdoor: base 8, str 12, range 5, dist 2
     *    base = 11, range_pen = 1, damage = 10, outdoor standard = 7 */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_THROWN, 8, 5, 1, 0 };
        int d = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 2, 1, 0);
        CHECK(d == 7);
    }

    /* 6. Magic no outdoor penalty: 20 + 12/4 = 23 (same indoor/outdoor) */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_MAGIC, 20, 5, 0, 0 };
        int d_in  = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 1, 0, 0);
        int d_out = dm2_v1_combat_resolve_attack_full(&w, 12, 0, 1, 1, 0);
        CHECK(d_in == 23 && d_out == 23);
    }

    /* 7. Companion bonus: melee 10, str 16 → base 14
     *    0 comps = 14, 4 comps = 14 + 14*40/100 = 19 */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_MELEE, 10, 1, 0, 0 };
        int d0 = dm2_v1_combat_resolve_attack_full(&w, 16, 0, 1, 0, 0);
        int d4 = dm2_v1_combat_resolve_attack_full(&w, 16, 0, 1, 0, 4);
        CHECK(d0 == 14 && d4 == 19);
    }

    /* 8. Tech gating: gun tech_level=0 → invalid → 0 damage */
    {
        DM2_V1_WeaponInfo w = { DM2_WEAPON_GUN, 10, 5, 1, 0 };
        int d = dm2_v1_combat_resolve_attack(&w, 12, 0, 1);
        CHECK(d == 0);
        /* Sanity: gun with tech_level=1 works */
        w.tech_level = 1;
        d = dm2_v1_combat_resolve_attack(&w, 12, 0, 1);
        CHECK(d == 13);  /* 10 + 12/4 = 13 */
    }

    /* 9. Door destruction: wooden (type 1, defense 42), state 4 (CLOSED), attack 50 → 1 */
    {
        int r = dm2_v1_combat_resolve_door_attack(1, 4, 50);
        CHECK(r == 1);
    }

    /* 10. Door destruction: iron (type 2, defense 230), attack 200 → cap 100 < 230 → 0 */
    {
        int r = dm2_v1_combat_resolve_door_attack(2, 4, 200);
        CHECK(r == 0);
    }

    /* 11. Champion wound on close: state 4 (CLOSED), rng 5 → 5 */
    {
        int w = dm2_v1_combat_resolve_champion_wound_on_close(4, 5);
        CHECK(w == 5);
    }

    /* 12. Door info: portcullis (0) → defense 110, projectile_pass=1 */
    {
        int pp = 0, an = 99;
        int d = dm2_v1_combat_door_info_for_type(0, &pp, &an);
        CHECK(d == 110 && pp == 1);
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: DM2 V1 Combat Phase 5 headless probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}
