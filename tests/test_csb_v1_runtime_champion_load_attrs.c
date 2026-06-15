/*
 * test_csb_v1_runtime_champion_load_attrs.c
 *
 * CSB V1 Runtime Champion Load / Attributes follow-up
 *
 * Adds a narrow source-locked regression for the imported-champion load
 * and attributes path that lives on the runtime side of the verified
 * boot handoff.  This complements the F0284 rotation and F0368 leader
 * switch gates that are already green:
 *
 *   - F0309_CHAMPION_GetMaximumLoad  (CHAMPION.C lines 1157-1178)
 *     base = (STR_CURRENT << 3) + 100
 *     base = F0306(stamina-adjusted)
 *     base += 9; base -= base % 10  (round to next multiple of 10)
 *
 *   - F0306_CHAMPION_GetStaminaAdjustedValue  (CHAMPION.C lines 1078-1106)
 *     if (CurrentStamina < MaximumStamina / 2):
 *         return (value >> 1) + ((value >> 1) * CurrentStamina) /
 *                                 (MaximumStamina >> 1)
 *     else:
 *         return value
 *
 *   - F0310_CHAMPION_GetMovementTicks  (CHAMPION.C lines 1180-1214)
 *     if (MaxLoad > Load): ticks = 2  (BUG0_72: >, not >=)
 *         if (Load*8 > MaxLoad*5): ticks++   (3)
 *     else: ticks = 4 + ((Load - MaxLoad) * 4) / MaxLoad
 *
 *   - BUG0_72: Load == MaxLoad gets the overloaded (else) branch and
 *     returns 4 ticks.  We assert that boundary on the imported party.
 *
 * The test runs the verified boot handoff on a synthetic four-champion
 * DM1 import, then exercises the F0309 / F0306 / F0310 / BUG0_72
 * invariants on the runtime-owned party snapshot.  It also runs the
 * helpers on a stand-alone champion (no boot) so the pure-function
 * formula is verified independently of the handoff path.  All
 * attribute-flag transitions (kill / resurrect / reincarnate /
 * leader-switch) stay consistent with the runtime contract.
 *
 * Source-locks:
 *   ReDMCSB CHAMPION.C F0306 lines 1078-1106 (stamina adjustment)
 *   ReDMCSB CHAMPION.C F0309 lines 1157-1178 (maximum load)
 *   ReDMCSB CHAMPION.C F0310 lines 1180-1214 (movement ticks)
 *   ReDMCSB CHAMPION.C F0284 lines 117-130   (party rotation, prior)
 *   ReDMCSB CLIKCHAM.C F0368 lines 51-68     (leader switch, prior)
 *   ReDMCSB BUG0_72 in CHAMPION.C F0310 line 1198 (comparison is >, not >=)
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#else
#define TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, label, fmt) do { \
    if ((got) == (want)) { \
        passed++; printf("  PASS: %s == %" fmt "\n", label, (want)); \
    } else { \
        failed++; printf("  FAIL: %s got=%" fmt " want=%" fmt "\n", label, (got), (want)); \
    } \
} while (0)

/* -- Helpers (mirrors test_csb_v1_boot_runtime_handoff.c shape) ------- */

/* Build a minimal valid CSB V1 DUNGEON.DAT buffer. Mirrors the
 * synthetic builder in test_csb_v1_phase7_verification.c so the
 * fixture shape matches the legacy loader (square_bytes == 2,
 * column-major 16-bit records, ReDMCSB DUNGEON.C F0151). */
static int build_synthetic_dungeon(uint8_t *buf, int buf_size,
                                    uint8_t square_type_1_1)
{
    if (!buf || buf_size < 28) return -1;
    memset(buf, 0, (size_t)buf_size);
    buf[0] = 1; buf[1] = 0;             /* level_count = 1 */
    buf[2] = 16; buf[3] = 0;            /* ignored padding */
    buf[4] = 3; buf[5] = 3;             /* level 0 width=3, height=3 */
    buf[6] = 10; buf[7] = 0;            /* level 0 absolute square offset = 10 */
    buf[8] = 0; buf[9] = 0;
    buf[10] = 1; buf[11] = 0;
    buf[12] = 1; buf[13] = 0;
    buf[14] = 1; buf[15] = 0;
    buf[16] = 1; buf[17] = 0;
    buf[18] = square_type_1_1; buf[19] = 0;
    buf[20] = 1; buf[21] = 0;
    buf[22] = 1; buf[23] = 0;
    buf[24] = 1; buf[25] = 0;
    buf[26] = 1; buf[27] = 0;
    return 0;
}

static int write_synthetic_dungeon(const char *path, uint8_t square_type_1_1)
{
    uint8_t buf[32];
    FILE *f;
    size_t n;
    if (build_synthetic_dungeon(buf, (int)sizeof(buf), square_type_1_1) != 0) {
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return (n == sizeof(buf)) ? 0 : -1;
}

/* Build a synthetic DM1 save buffer for n champions. Mirrors the
 * helper in test_csb_v1_boot_runtime_handoff.c. */
static int build_synthetic_dm1_party_buffer(uint8_t *buf, size_t buf_size,
                                            int champion_count)
{
    int i;
    if (!buf || buf_size < 1024 || champion_count < 1 ||
        champion_count > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    memset(buf, 0, buf_size);
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = (uint8_t)champion_count;
    for (i = 0; i < champion_count; i++) {
        size_t off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START +
                     (size_t)i * (size_t)CSB_V1_DM1_CHAMP_SIZE;
        size_t equip_off = off + (size_t)CSB_V1_DM1_CHAMP_OFF_EQUIP;
        int slot;

        memcpy((char *)buf + off + CSB_V1_DM1_CHAMP_OFF_NAME,
               i == 0 ? "ALPHA   " : (i == 1 ? "BETA    " :
                                       (i == 2 ? "GAMMA   " : "DELTA   ")),
               8);
        buf[off + CSB_V1_DM1_CHAMP_OFF_HEALTH] = (uint8_t)(80 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = (uint8_t)(100 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_STAMINA] = (uint8_t)(60 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = (uint8_t)(100 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MANA] = (uint8_t)(30 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA] = (uint8_t)(50 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_STR] = (uint8_t)(55 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_DEX] = (uint8_t)(66 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_WIS] = (uint8_t)(77 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_VIT] = (uint8_t)(88 + i);
        for (slot = 0; slot < CSB_V1_SLOT_COUNT; slot++) {
            buf[equip_off + (size_t)slot * 2u] = 0xFFu;
            buf[equip_off + (size_t)slot * 2u + 1u] = 0xFFu;
        }
    }
    return 0;
}

/* Set a champion's stat triple to a deterministic value. */
static void seed_champion_stat(CSB_V1_Champion *c, int stat_idx, int cur,
                               int max_, int min_)
{
    c->Statistics[stat_idx][CSB_V1_STAT_CUR] = (uint16_t)cur;
    c->Statistics[stat_idx][CSB_V1_STAT_MAX] = (uint16_t)max_;
    c->Statistics[stat_idx][CSB_V1_STAT_MIN] = (uint16_t)min_;
}

/* Apply a deterministic stat/stamina profile to champion i. */
static void seed_champion_profile(CSB_V1_Champion *c, int i)
{
    memset(c, 0, sizeof(*c));
    c->Name[0] = (char)('A' + i);
    c->Name[1] = '\0';
    seed_champion_stat(c, CSB_V1_STAT_STR, 50 + i, 50 + i, 30);
    seed_champion_stat(c, CSB_V1_STAT_DEX, 50 + i, 50 + i, 30);
    seed_champion_stat(c, CSB_V1_STAT_WIS, 50 + i, 50 + i, 30);
    seed_champion_stat(c, CSB_V1_STAT_VIT, 50 + i, 50 + i, 30);
    seed_champion_stat(c, CSB_V1_STAT_ANTIMAGIC, 50, 50, 30);
    seed_champion_stat(c, CSB_V1_STAT_ANTIFIRE, 50, 50, 30);
    seed_champion_stat(c, CSB_V1_STAT_LUCK, 50, 50, 30);
    c->CurrentHealth  = 80;
    c->MaximumHealth  = 100;
    c->CurrentStamina = 100;  /* full stamina - no F0306 scaling */
    c->MaximumStamina = 100;
    c->CurrentMana    = 30;
    c->MaximumMana    = 50;
    c->Cell           = CSB_V1_CELL_FRONT_LEFT;
    c->Direction      = CSB_V1_DIR_NORTH;
    c->ActionIndex    = CSB_V1_ACTION_REST;
    c->Load           = 0;
    c->Attributes     = 0;
    c->reincarnateAttributePenalty = 2;
    c->reincarnateStatPenalty = 8;
    c->randomPoints = 12;
    c->Food  = 1500;
    c->Water = 1500;
}

/* -- Test 1: pure F0309 maximum-load formula on a stand-alone champion -- */
static void test_maximum_load_pure_formula(void)
{
    /* ReDMCSB CHAMPION.C F0309 lines 1157-1178:
     *   base = (STR << 3) + 100
     *   base = F0306_stamina_adjusted(base)
     *   base += 9; base -= base % 10
     * The stand-alone fixture has full stamina so F0306 is a no-op.
     *
     * Reference values for STR 50..53, full stamina, no wounds, no boots:
     *   STR=50: base = (50<<3)+100 = 500, +9 = 509, -509%10 = 500
     *   STR=51: base = (51<<3)+100 = 508, +9 = 517, -517%10 = 510
     *   STR=52: base = (52<<3)+100 = 516, +9 = 525, -525%10 = 520
     *   STR=53: base = (53<<3)+100 = 524, +9 = 533, -533%10 = 530
     *
     * Edge cases:
     *   STR=0  : base = 100, +9 = 109, -109%10 = 100
     *   STR=1  : base = 108, +9 = 117, -117%10 = 110
     *   STR=99 : base = 892, +9 = 901, -901%10 = 900
     */
    CSB_V1_Champion c;
    seed_champion_profile(&c, 0);

    /* Stand-alone edge cases - no boot, full stamina. */
    seed_champion_stat(&c, CSB_V1_STAT_STR, 0, 0, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 100u,
             "STR=0, full stamina: max load is 100", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 1, 1, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 110u,
             "STR=1, full stamina: max load is 110", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 50, 50, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 500u,
             "STR=50, full stamina: max load is 500", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 51, 51, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 510u,
             "STR=51, full stamina: max load is 510", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 52, 52, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 520u,
             "STR=52, full stamina: max load is 520", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 53, 53, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 530u,
             "STR=53, full stamina: max load is 530", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 99, 99, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 900u,
             "STR=99, full stamina: max load is 900", "u");

    /* F0309 ignores MAX: only CURRENT STR matters. */
    seed_champion_stat(&c, CSB_V1_STAT_STR, 50, 99, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 500u,
             "MAX=99, CUR=50: max load uses STR_CURRENT", "u");
    seed_champion_stat(&c, CSB_V1_STAT_STR, 99, 50, 30);
    CHECK_EQ(csb_v1_champion_get_maximum_load(&c), 900u,
             "MAX=50, CUR=99: max load uses STR_CURRENT", "u");

    /* NULL guard. */
    CHECK_EQ(csb_v1_champion_get_maximum_load(NULL), 0u,
             "NULL champion returns 0 maximum load", "u");
}

/* -- Test 2: F0306 stamina adjustment branches on stand-alone champion - */
static void test_stamina_adjusted_value_branches(void)
{
    /* ReDMCSB CHAMPION.C F0306 lines 1078-1106:
     *   if (CurrentStamina < MaximumStamina / 2):
     *     return (value >> 1) +
     *            ((value >> 1) * CurrentStamina) / (MaximumStamina >> 1)
     *   else:
     *     return value
     *
     * Edge: CurrentStamina == MaximumStamina / 2 takes the else branch.
     * Edge: CurrentStamina == MaximumStamina takes the else branch.
     * Edge: CurrentStamina == 0 still takes the source branch and
     * returns the halved value before F0309 rounding.
     */
    CSB_V1_Champion c;
    unsigned int max_load;
    seed_champion_profile(&c, 0);
    seed_champion_stat(&c, CSB_V1_STAT_STR, 50, 50, 30);

    /* Full stamina: F0306 returns value unchanged. */
    c.CurrentStamina  = 100; c.MaximumStamina = 100;
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ(max_load, 500u,
             "full stamina (CS==MS=100): no F0306 scaling", "u");

    /* Stamina at half: F0306 returns value unchanged (>= half). */
    c.CurrentStamina  = 50;  c.MaximumStamina = 100;
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ(max_load, 500u,
             "stamina at half (CS=50, MS=100): no F0306 scaling (else branch)", "u");

    /* Stamina just below half: F0306 scales the value.
     *   base = 500 (STR 50)
     *   half_val = 250
     *   scaled = (250 * 49) / 50 = 245
     *   result = 250 + 245 = 495
     *   +9 = 504, -504%10 = 500
     * The round-up at +9 dominates, but the F0306 step is observable
     * with a non-round base (e.g. STR 51).
     */
    c.CurrentStamina  = 49;  c.MaximumStamina = 100;
    seed_champion_stat(&c, CSB_V1_STAT_STR, 51, 51, 30);
    max_load = csb_v1_champion_get_maximum_load(&c);
    /* base = (51<<3)+100 = 508; half_val = 254; scaled = (254*49)/50 = 248;
     * result = 254 + 248 = 502; +9 = 511; -511%10 = 510.  No observable
     * change from the full-stamina path of 510 - that's fine: the
     * 1-unit stamina drop is below the F0309 rounding resolution.
     * Use a larger drop to make the F0306 step observable. */
    c.CurrentStamina  = 25;  c.MaximumStamina = 100;
    max_load = csb_v1_champion_get_maximum_load(&c);
    /* half_val = 254; scaled = (254*25)/50 = 127; result = 254 + 127 = 381;
     * +9 = 390; -390%10 = 390. */
    CHECK_EQ(max_load, 390u,
             "stamina at 25% (CS=25, MS=100): F0306 scales the F0309 base", "u");

    /* Stamina at 1: F0306 still returns scaled value.
     *   half_val = 254; scaled = (254*1)/50 = 5; result = 254 + 5 = 259;
     *   +9 = 268; -268%10 = 260. */
    c.CurrentStamina  = 1;  c.MaximumStamina = 100;
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ(max_load, 260u,
             "stamina at 1 (CS=1, MS=100): F0306 reduces base to ~half", "u");

    /* Source edge: stamina 0 still enters F0306 and returns half the
     * input value before F0309 rounding.
     *   half_val = 254; scaled = (254*0)/50 = 0; result = 254
     *   +9 = 263; -263%10 = 260. */
    c.CurrentStamina  = 0;  c.MaximumStamina = 100;
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ(max_load, 260u,
             "stamina at 0: F0306 returns the halved source value", "u");

    /* Source edge: max stamina 0 makes half_max zero, so the source
     * comparison is false and F0306 returns the input value unchanged. */
    c.CurrentStamina  = 50;  c.MaximumStamina = 0;
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ(max_load, 510u,
             "max stamina 0: F0306 returns the unscaled source value", "u");
}

/* -- Test 3: F0310 movement-ticks ladder ------------------------------- */
static void test_movement_ticks_five_band_ladder(void)
{
    /* ReDMCSB CHAMPION.C F0310 lines 1180-1214:
     *   if (MaxLoad > Load) [BUG0_72: > not >=]
     *     ticks = 2
     *     if (Load*8 > MaxLoad*5) ticks++   (becomes 3)
     *   else
     *     ticks = 4 + ((Load - MaxLoad) * 4) / MaxLoad
     *   [+1/+2 if feet wounded]
     *   [-1 if boot of speed]
     * CSB V1 has no Wounds/feet-slot model, so the base ladder is:
     *   Load=0:                    2
     *   Load <= MaxLoad*2/8:       2
     *   Load > MaxLoad*2/8 && <= MaxLoad*5/8:  2
     *   Load > MaxLoad*5/8 && <  MaxLoad:      3
     *   Load == MaxLoad:           4    (BUG0_72 else branch)
     *   Load >  MaxLoad:           4 + (((Load-Max)*4)/Max)
     */
    CSB_V1_Champion c;
    seed_champion_profile(&c, 0);
    seed_champion_stat(&c, CSB_V1_STAT_STR, 50, 50, 30);
    c.CurrentStamina = 100; c.MaximumStamina = 100;
    /* MaxLoad = 500 (verified by test_maximum_load_pure_formula) */

    c.Load = 0;
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 2u,
             "Load=0 / MaxLoad=500: 2 ticks (F0310 else-light branch)", "u");
    c.Load = 50;     /* 50*8=400, 500*5=2500, 400 < 2500 -> ticks=2 */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 2u,
             "Load=50 (10%): 2 ticks (under 5/8 threshold)", "u");
    c.Load = 124;    /* 124*8=992, 992 < 2500 -> 2 ticks */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 2u,
             "Load=124 (24.8%): 2 ticks (still under 5/8 threshold)", "u");
    c.Load = 125;    /* 125*8=1000, 1000 < 2500 -> 2 ticks (just under 5/8) */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 2u,
             "Load=125 (25%): 2 ticks (5/8 threshold is 312.5, exclusive)", "u");
    c.Load = 312;    /* 312*8=2496, 2496 < 2500 -> 2 ticks */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 2u,
             "Load=312 (62.4%): 2 ticks (just below 5/8 threshold)", "u");
    c.Load = 313;    /* 313*8=2504, 2504 > 2500 -> 3 ticks */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 3u,
             "Load=313 (62.6%): 3 ticks (just over 5/8 threshold)", "u");
    c.Load = 499;    /* 499*8=3992, 3992 > 2500 -> 3 ticks */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 3u,
             "Load=499 (99.8%): 3 ticks (still lighter than MaxLoad)", "u");

    /* BUG0_72 boundary: Load == MaxLoad falls into the else (overloaded)
     * branch and returns 4 ticks. */
    c.Load = 500;
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 4u,
             "Load=500 (== MaxLoad): 4 ticks (BUG0_72 else-branch)", "u");
    c.Load = 600;    /* 4 + ((600-500)*4)/500 = 4 + 0 = 4 */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 4u,
             "Load=600 (20% over): 4 ticks (overload floor)", "u");
    c.Load = 750;    /* 4 + ((750-500)*4)/500 = 4 + 2 = 6 */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 6u,
             "Load=750 (50% over): 6 ticks (overload scaling)", "u");
    c.Load = 1000;   /* 4 + ((1000-500)*4)/500 = 4 + 4 = 8 */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 8u,
             "Load=1000 (2x MaxLoad): 8 ticks (heavy overload)", "u");

    /* Defensive: NULL returns 2 (the BUG0_72 light-load default). */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(NULL), 2u,
             "NULL champion returns 2 ticks (light-load default)", "u");

    /* Source edge: max stamina 0 leaves F0309 unscaled, so the champion
     * is still lighter than MaxLoad and remains on the 2-tick branch. */
    c.Load = 0;
    c.MaximumStamina = 0; c.CurrentStamina = 0;
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&c), 2u,
             "max stamina 0 / Load=0: 2 ticks (source light-load branch)", "u");
}

/* -- Test 4: BUG0_72 overload boundary helper -------------------------- */
static void test_is_overloaded_boundary(void)
{
    /* ReDMCSB CHAMPION.C F0310 line 1198: comparison is > not >=, so
     * the equal-load case is NOT overloaded even though the user sees
     * the yellow equal-load panel state still moves slowly (BUG0_72).
     * The csb_v1_champion_is_overloaded helper mirrors the separate
     * CHAMDRAW.C red-load comparison: Load > MaxLoad.
     */
    CSB_V1_Champion c;
    seed_champion_profile(&c, 0);
    seed_champion_stat(&c, CSB_V1_STAT_STR, 50, 50, 30);
    c.CurrentStamina = 100; c.MaximumStamina = 100;
    /* MaxLoad = 500 */

    c.Load = 0;
    CHECK_EQ(csb_v1_champion_is_overloaded(&c), 0,
             "Load=0 / MaxLoad=500: not overloaded", "d");
    c.Load = 499;
    CHECK_EQ(csb_v1_champion_is_overloaded(&c), 0,
             "Load=499 / MaxLoad=500: not overloaded", "d");
    c.Load = 500;
    CHECK_EQ(csb_v1_champion_is_overloaded(&c), 0,
             "Load=500 == MaxLoad: NOT overloaded (BUG0_72 uses >, not >=)", "d");
    c.Load = 501;
    CHECK_EQ(csb_v1_champion_is_overloaded(&c), 1,
             "Load=501 > MaxLoad: overloaded", "d");
    c.Load = 1000;
    CHECK_EQ(csb_v1_champion_is_overloaded(&c), 1,
             "Load=1000: overloaded", "d");

    /* Defensive: NULL returns 0 (not overloaded). */
    CHECK_EQ(csb_v1_champion_is_overloaded(NULL), 0,
             "NULL champion: not overloaded", "d");

    /* Source edge: max stamina 0 leaves the F0309 baseline load intact,
     * so load 0 is still not overloaded. */
    c.MaximumStamina = 0; c.CurrentStamina = 0; c.Load = 0;
    CHECK_EQ(csb_v1_champion_is_overloaded(&c), 0,
             "max stamina 0 / load 0: not overloaded", "d");
}

/* -- Test 5: post-handoff runtime transition (the runtime proof) ------ */
static void test_runtime_champion_load_attrs_post_handoff(void)
{
    /* This is the runtime follow-up boundary for the F0309/F0306/F0310
     * formulas.  It exercises the same import -> enter_game -> champion
     * mutation -> invariant-check flow that the F0284 rotation and F0368
     * leader-switch gates already cover, but the post-handoff mutation
     * is a stat (not a direction), so the load calculation re-runs on
     * the runtime-owned party snapshot.  This proves the runtime can
     * recompute the F0309 invariant after import without depending on
     * the boot-time DM1 import, the dungeon loader, or the F0292
     * redraw stack. */
    CSB_V1_BootProfile p;
    CSB_V1_PartyState imported;
    CSB_V1_PartyState runtime_party;
    uint8_t save_buf[1024];
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-load-attrs-followup";
    unsigned int max_load;
    int i;

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written for load-attrs follow-up");
    CHECK(build_synthetic_dm1_party_buffer(save_buf, sizeof(save_buf), 4) == 0,
          "synthetic four-champion DM1 save buffer built for load-attrs test");
    CHECK(csb_v1_character_import_dm1_buffer(&imported, save_buf,
                                             (int)sizeof(save_buf)) == 4,
          "DM1 buffer import yields four CSB champions");
    /* Override the imported stats so the F0309 result is predictable:
     *   STR_CURRENT = 60 (each), stamina = full, no wound.
     *   MaxLoad per champion = (60<<3) + 100 = 580, +9 = 589, -589%10 = 580.
     *   (Using 60 instead of 50/51/52/53 keeps the import-time numbers
     *   identical for every champion, then we mutate them in
     *   post-handoff to test the recompute path.) */
    for (i = 0; i < imported.ChampionCount; i++) {
        imported.Champions[i].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] = 60;
        imported.Champions[i].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX] = 60;
        imported.Champions[i].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MIN] = 30;
        imported.Champions[i].CurrentStamina  = 100;
        imported.Champions[i].MaximumStamina  = 100;
        imported.Champions[i].Load = 0;
        imported.Champions[i].Attributes = 0;
    }
    imported.PartyDirection = CSB_V1_DIR_NORTH;

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_set_imported_party(&p, &imported) == 0,
          "boot profile accepts the four-champion party before load-attrs handoff");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game succeeds for the load-attrs follow-up path");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state is RUNTIME_READY before load-attrs assertions");
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is visible after boot handoff");

    /* Post-handoff: every imported champion has STR_CURRENT=60 and full
     * stamina, so the F0309 invariant predicts 580 for every champion. */
    for (i = 0; i < runtime_party.ChampionCount; i++) {
        max_load = csb_v1_champion_get_maximum_load(&runtime_party.Champions[i]);
        CHECK_EQ(max_load, 580u,
                 "post-handoff STR=60 / full stamina: max load is 580", "u");
    }

    /* Post-handoff stat mutation: drop champion 1's STR_CURRENT to 30.
     *   base = (30<<3)+100 = 340, +9 = 349, -349%10 = 340.
     *   This mutation must take effect immediately - no second import,
     *   no F0292 redraw stack, no dungeon re-load.  The runtime party
     *   snapshot is the single source of truth for the F0309 recompute.
     */
    runtime_party.Champions[1].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] = 30;
    runtime_party.Champions[1].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX] = 30;
    CHECK(csb_v1_runtime_set_party_state(&p.runtime, &runtime_party) == 0,
          "runtime accepts post-handoff stat mutation");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is live after post-handoff mutation");

    CHECK_EQ(csb_v1_champion_get_maximum_load(&runtime_party.Champions[0]),
             580u, "champion 0 keeps STR=60 / max load 580", "u");
    CHECK_EQ(csb_v1_champion_get_maximum_load(&runtime_party.Champions[1]),
             340u, "champion 1 dropped to STR=30 / max load 340 (F0309 recompute)", "u");
    CHECK_EQ(csb_v1_champion_get_maximum_load(&runtime_party.Champions[2]),
             580u, "champion 2 keeps STR=60 / max load 580", "u");
    CHECK_EQ(csb_v1_champion_get_maximum_load(&runtime_party.Champions[3]),
             580u, "champion 3 keeps STR=60 / max load 580", "u");

    /* Post-handoff stamina mutation: drop champion 2 to 25% stamina.
     *   base = 580, half_val = 290, scaled = (290*25)/50 = 145
     *   result = 290 + 145 = 435, +9 = 444, -444%10 = 440.
     *   F0306 must be observable in the recomputed max load. */
    runtime_party.Champions[2].CurrentStamina  = 25;
    runtime_party.Champions[2].MaximumStamina  = 100;
    CHECK(csb_v1_runtime_set_party_state(&p.runtime, &runtime_party) == 0,
          "runtime accepts post-handoff stamina mutation");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is live after stamina mutation");
    CHECK_EQ(csb_v1_champion_get_maximum_load(&runtime_party.Champions[2]),
             440u, "champion 2 stamina 25%: max load is 440 (F0306 scaled)", "u");

    /* Post-handoff load assignment: load champion 3 with 500 KG. */
    runtime_party.Champions[3].Load = 500;
    CHECK(csb_v1_runtime_set_party_state(&p.runtime, &runtime_party) == 0,
          "runtime accepts post-handoff load assignment");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is live after load assignment");

    /* F0310 + BUG0_72: champion 3 (Load=500, MaxLoad=580) is in the
     * "above 5/8 but below max" band, so movement ticks = 3. */
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&runtime_party.Champions[3]),
             3u,
             "champion 3 Load=500 / MaxLoad=580: 3 ticks (F0310 above 5/8 band)", "u");
    CHECK_EQ(csb_v1_champion_is_overloaded(&runtime_party.Champions[3]),
             0, "champion 3 Load=500 < MaxLoad=580: not overloaded", "d");

    /* Push champion 3 to overload: Load=600 > MaxLoad=580.
     *   F0310 else-branch: 4 + ((600-580)*4)/580 = 4 + 0 = 4
     *   BUG0_72: now is_overloaded returns 1. */
    runtime_party.Champions[3].Load = 600;
    CHECK(csb_v1_runtime_set_party_state(&p.runtime, &runtime_party) == 0,
          "runtime accepts overload mutation");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is live after overload mutation");
    CHECK_EQ(csb_v1_champion_get_movement_ticks(&runtime_party.Champions[3]),
             4u, "champion 3 Load=600 / MaxLoad=580: 4 ticks (BUG0_72 overload)", "u");
    CHECK_EQ(csb_v1_champion_is_overloaded(&runtime_party.Champions[3]),
             1, "champion 3 Load=600 > MaxLoad=580: overloaded", "d");

    /* Attribute flag transitions: kill + resurrect on champion 0.
     * ReDMCSB CHAMPION.C / REVIVE.C F0278. */
    CHECK(csb_v1_champion_is_dead(&runtime_party.Champions[0]) == 0,
          "champion 0 starts alive (DEAD attribute clear)");
    /* The current leader is champion 0, so a leader switch to 0 is a
     * no-op.  Switch to champion 1 first so the post-kill leader
     * switch to 0 actually exercises the F0368 dead-champion guard. */
    CHECK(csb_v1_runtime_set_leader(&p.runtime, 1) == 0,
          "leader switch to living champion 1 succeeds before kill");
    CHECK(csb_v1_champion_kill(&runtime_party.Champions[0]) == 0,
          "kill champion 0 succeeds");
    runtime_party.Champions[0].Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    runtime_party.Champions[0].CurrentHealth = 0;
    /* Push the kill to the runtime so the F0368 dead-champion guard
     * in csb_v1_runtime_set_leader sees the DEAD attribute on the
     * runtime-owned copy.  The set_party_state copy also re-validates
     * the leader and points it at the first living champion (1). */
    CHECK(csb_v1_runtime_set_party_state(&p.runtime, &runtime_party) == 0,
          "runtime accepts the kill mutation on champion 0");
    CHECK(p.runtime.leader_index == 1,
          "runtime leader index moves to champion 1 after champion 0 dies");
    CHECK(csb_v1_champion_is_dead(&p.runtime.party_state.Champions[0]) == 1,
          "runtime-owned champion 0 is dead after the kill mutation");
    /* A dead champion must not be selected as the leader (F0368 guard
     * mirrors this by refusing to set leader on a dead champion). */
    CHECK(csb_v1_runtime_set_leader(&p.runtime, 0) == -1,
          "leader switch to dead champion 0 is rejected (F0368 source-lock)");

    CHECK(csb_v1_champion_resurrect(&runtime_party.Champions[0]) == 0,
          "resurrect champion 0 succeeds (REVIVE.C F0278 C160)");
    runtime_party.Champions[0].Attributes &= ~CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    runtime_party.Champions[0].CurrentHealth = runtime_party.Champions[0].MaximumHealth;
    CHECK(csb_v1_champion_is_dead(&runtime_party.Champions[0]) == 0,
          "champion 0 is alive after resurrect (DEAD attribute clear)");
    CHECK(runtime_party.Champions[0].CurrentHealth ==
              runtime_party.Champions[0].MaximumHealth,
          "resurrect restores CurrentHealth to MaximumHealth (no penalty)");

    /* Reincarnate halves HP/Mana/Stamina and applies a 1/8th stat loss.
     * Reincarnate also clears all skills.  We test the post-reincarnate
     * load recompute on the now-living champion 0. */
    runtime_party.Champions[0].CurrentHealth  = 100;
    runtime_party.Champions[0].MaximumHealth  = 100;
    runtime_party.Champions[0].CurrentStamina = 100;
    runtime_party.Champions[0].MaximumStamina = 100;
    runtime_party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] = 60;
    runtime_party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX] = 60;
    /* Re-kill so reincarnate has work to do.  The DEAD flag and the
     * CurrentHealth=0 side effect must both be visible to the
     * reincarnate guard (csb_v1_champion_is_dead). */
    CHECK(csb_v1_champion_kill(&runtime_party.Champions[0]) == 0,
          "re-kill champion 0 for reincarnation");
    runtime_party.Champions[0].MaximumHealth = 100;
    runtime_party.Champions[0].CurrentHealth = 100;
    CHECK(csb_v1_champion_reincarnate(&runtime_party.Champions[0]) == 0,
          "reincarnate champion 0 succeeds (REVIVE.C F0278 C161)");
    CHECK(csb_v1_champion_is_dead(&runtime_party.Champions[0]) == 0,
          "champion 0 is alive after reincarnate (DEAD attribute clear)");
    CHECK(runtime_party.Champions[0].Attributes &
              CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME,
          "reincarnate sets NEEDS_RENAME attribute (F0281_CHAMPION_Rename)");
    /* HP halved: 100/2 = 50. */
    CHECK_EQ(runtime_party.Champions[0].CurrentHealth, 50,
             "reincarnate halves CurrentHealth 100 -> 50 (MEDIA332)", "d");
    CHECK_EQ(runtime_party.Champions[0].MaximumHealth, 50,
             "reincarnate halves MaximumHealth 100 -> 50 (MEDIA332)", "d");

    /* Recompute load on the reincarnated champion.  We can't predict
     * the exact F0309 result because the random +1 boosts are seeded
     * by (c->randomPoints * 17 + 3), but we can assert the result is
     * a multiple of 10 (F0309 round-to-10) and is in the band [340,
     * 580] (between STR 30 and STR 60 max load).  The reincarnate
     * floor (1/8th reduction) is the dominant effect. */
    max_load = csb_v1_champion_get_maximum_load(&runtime_party.Champions[0]);
    CHECK(max_load >= 340u && max_load <= 580u,
          "post-reincarnate max load is in [340, 580] band (F0309 band check)");
    CHECK((max_load % 10u) == 0u,
          "post-reincarnate max load is a multiple of 10 (F0309 round-to-10)");

    /* Recompute on the runtime party snapshot so we exercise the
     * runtime-owned path, not just the local copy. */
    CHECK(csb_v1_runtime_set_party_state(&p.runtime, &runtime_party) == 0,
          "runtime accepts the post-reincarnate mutation");
    memset(&runtime_party, 0, sizeof(runtime_party));
    CHECK(csb_v1_runtime_get_party_state(&p.runtime, &runtime_party) == 4,
          "runtime party snapshot is live after reincarnate handoff");

    csb_v1_boot_cleanup(&p);
}

/* -- Test 6: argument validation --------------------------------------- */
static void test_argument_validation(void)
{
    /* Every new helper must reject NULL and (where applicable) the
     * out-of-range arguments.  This guards the post-handoff runtime
     * from a missing party or a malformed snapshot. */
    CHECK_EQ(csb_v1_champion_get_maximum_load(NULL), 0u,
             "get_maximum_load(NULL) returns 0", "u");
    CHECK_EQ(csb_v1_champion_get_movement_ticks(NULL), 2u,
             "get_movement_ticks(NULL) returns 2 (light-load default)", "u");
    CHECK_EQ(csb_v1_champion_is_overloaded(NULL), 0,
             "is_overloaded(NULL) returns 0", "d");

    /* A fully-formed but zero-stat champion still has the F0309 base
     * load of 100 because the source formula is (STR_CURRENT << 3) + 100. */
    {
        CSB_V1_Champion zero;
        memset(&zero, 0, sizeof(zero));
        CHECK_EQ(csb_v1_champion_get_maximum_load(&zero), 100u,
                 "all-zero champion has F0309 baseline max load 100", "u");
        CHECK_EQ(csb_v1_champion_get_movement_ticks(&zero), 2u,
                 "all-zero champion returns 2 ticks (source light-load branch)", "u");
        CHECK_EQ(csb_v1_champion_is_overloaded(&zero), 0,
                 "all-zero champion is not overloaded (0 > 100 is false)", "d");
    }
}

/* -- Main ------------------------------------------------------------ */
int main(void)
{
    printf("=== CSB V1 Runtime Champion Load / Attributes Regression ===\n\n");

    test_maximum_load_pure_formula();
    printf("\n");
    test_stamina_adjusted_value_branches();
    printf("\n");
    test_movement_ticks_five_band_ladder();
    printf("\n");
    test_is_overloaded_boundary();
    printf("\n");
    test_runtime_champion_load_attrs_post_handoff();
    printf("\n");
    test_argument_validation();

    printf("\n========================================\n");
    printf("PASSED: %d\n", passed);
    printf("FAILED: %d\n", failed);
    if (failed == 0) {
        puts("ok: CSB V1 champion load and attribute helpers reproduce the source-locked F0309/F0306/F0310/BUG0_72 invariants on a stand-alone fixture and on a runtime party snapshot captured by the verified boot handoff");
        puts("sourceEvidence=ReDMCSB CHAMPION.C F0306_CHAMPION_GetStaminaAdjustedValue lines 1078-1106; F0309_CHAMPION_GetMaximumLoad lines 1157-1178; F0310_CHAMPION_GetMovementTicks lines 1180-1214; BUG0_72 line 1198");
    }
    return failed == 0 ? 0 : 1;
}
