/* test_dm2_v1_drops_gdat_real_data.c — DM2 real-data drop tables (Lane E,
 * cycle 16).
 *
 * Verifies the real-data drop route against the local canonical
 * GRAPHICS.DAT: CREATURES word fields 0x0A..0x14 carry the source drop words
 * (skproject/SKWINSPX/src/v4/skcrture.cpp:2092-2100
 * DROP_CREATURE_POSSESSION), resolved in source order through
 * dm2_v1_drops_resolve_gdat_creature_drops and through the live
 * creature death path (dm2_v1_creature_load_ai_table_from_gdat →
 * dm2_v1_creature_death_check observer).
 *
 * Proven local GDAT facts (PC English canonical GRAPHICS.DAT):
 *   type 24 (GLOP):        [0x0A]=0x8E10  [0x0B]=0x9D10
 *   type 14 (ATTACK MIN):  [0x0A]=0x0410  [0x0B]=0x8412
 *   type  0 (TREE):        [0x0A]=0x9241
 *   type 19 (THORN DEMON): no drop word fields (fallback path stays)
 *
 * Also verifies the combat defense route stays fail-closed against this
 * GDAT: the CREATURE_AI (0x19) category is absent locally, so no creature
 * defense can be proven and dm2_v1_combat_resolve_attack_on_creature must
 * reject explicitly.
 *
 * Skips cleanly when no local canonical DM2 data is present.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_combat.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_drops.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *f = fopen(path, "rb");
    long size;
    uint8_t *b;
    if (!f || fseek(f, 0, SEEK_END) != 0 || (size = ftell(f)) <= 0 ||
        fseek(f, 0, SEEK_SET) != 0) {
        if (f) fclose(f);
        return NULL;
    }
    b = malloc((size_t)size);
    if (!b || fread(b, 1u, (size_t)size, f) != (size_t)size) {
        free(b);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)size;
    return b;
}

static int load_graphics_dat(uint8_t **graphics, size_t *graphics_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char default_root[1024];
    char graphics_path[1100];

    if (!root || !root[0]) {
        if (!home || !home[0]) return 0;
        snprintf(default_root, sizeof(default_root),
                 "%s/.firestaff/data/dm2/data", home);
        root = default_root;
    }
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    *graphics = read_file(graphics_path, graphics_size);
    return *graphics != NULL;
}

/* Independent LCG replica (c_random.cpp:13-31) for expected rolls. */
static uint32_t g_rep_state;

static uint32_t replica_rand(void)
{
    uint32_t v = g_rep_state * 0xbb40e62du + 11u;
    g_rep_state = v;
    return v >> 8;
}

static uint16_t replica_rand16(uint16_t n)
{
    return n == 0 ? 0 : (uint16_t)(replica_rand() % n);
}

#define TEST_AI_GLOP 24
#define TEST_AI_ATTACK_MINION 14
#define TEST_AI_TREE 0

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DropRng rng;
    DM2_V1_DropSlotReceipt rc[DM2_DROP_SLOT_COUNT];
    DM2_V1_DropGdatReceipt gd;

    if (!load_graphics_dat(&graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical DM2 GRAPHICS.DAT was not accepted\n", stderr);
        free(graphics);
        return 1;
    }

    /* ── Fail-closed guards ── */
    dm2_v1_drops_rng_init(&rng);
    CHECK(dm2_v1_drops_resolve_gdat_creature_drops(NULL, TEST_AI_GLOP, &rng,
                                                   rc, &gd) == 0,
          "NULL loader rejected");
    CHECK(gd.valid && gd.rejected_no_loader, "no-loader receipt flag");
    CHECK(dm2_v1_drops_resolve_gdat_creature_drops(&loader, -1, &rng, rc,
                                                   &gd) == 0,
          "negative type rejected");
    CHECK(gd.rejected_type_out_of_range, "out-of-range receipt flag");
    CHECK(dm2_v1_drops_resolve_gdat_creature_drops(&loader, 0x100, &rng, rc,
                                                   &gd) == 0,
          "oversized type rejected");

    /* ── GLOP (type 24): two proven drop words, source-ordered ──
     * 0x8E10: item 284, base 1, extra range 1 → RAND16(2)
     * 0x9D10: item 314, base 1, extra range 1 → RAND16(2) */
    g_rep_state = 0;
    {
        int roll_a = replica_rand16(2);
        int roll_b = replica_rand16(2);
        dm2_v1_drops_rng_init(&rng);
        int admitted = dm2_v1_drops_resolve_gdat_creature_drops(
            &loader, TEST_AI_GLOP, &rng, rc, &gd);
        CHECK(admitted == 2, "GLOP admits two drop slots");
        CHECK(gd.valid && gd.words_present >= 2, "GLOP words present");
        CHECK(rc[0].admitted && rc[0].word == 0x8E10u &&
                  rc[0].item_id == 284 && rc[0].base_count == 1 &&
                  rc[0].extra_range == 1 && rc[0].extra_roll == roll_a,
              "GLOP slot 0 word 0x8E10 decoded in source order");
        CHECK(rc[1].admitted && rc[1].word == 0x9D10u &&
                  rc[1].item_id == 314 && rc[1].extra_roll == roll_b,
              "GLOP slot 1 word 0x9D10 decoded in source order");
        CHECK(gd.first_item_id == 284 &&
                  gd.total_count == (1 + roll_a) + (1 + roll_b),
              "GLOP aggregate receipt");
        CHECK(rng.random == g_rep_state, "RNG stream matches replica");
    }

    /* ── ATTACK MINION (type 14): 0x0410 → item 8; 0x8412 → item 264 ── */
    g_rep_state = 0;
    {
        int roll_a = replica_rand16(2);
        int roll_b = replica_rand16(2);
        dm2_v1_drops_rng_init(&rng);
        int admitted = dm2_v1_drops_resolve_gdat_creature_drops(
            &loader, TEST_AI_ATTACK_MINION, &rng, rc, &gd);
        CHECK(admitted == 2, "ATTACK MINION admits two drop slots");
        CHECK(rc[0].item_id == 8 && rc[0].base_count == 1 &&
                  rc[0].extra_roll == roll_a,
              "ATTACK MINION slot 0 item 8");
        CHECK(rc[1].item_id == 264 && rc[1].base_count == 3 &&
                  rc[1].extra_roll == roll_b,
              "ATTACK MINION slot 1 item 264, base 3");
        CHECK(gd.total_count == (1 + roll_a) + (3 + roll_b),
              "ATTACK MINION aggregate");
    }

    /* ── TREE (type 0): single word 0x9241 → item 292, base 2, extra 4 ── */
    g_rep_state = 0;
    {
        int roll = replica_rand16(5);
        dm2_v1_drops_rng_init(&rng);
        int admitted = dm2_v1_drops_resolve_gdat_creature_drops(
            &loader, TEST_AI_TREE, &rng, rc, &gd);
        CHECK(admitted == 1, "TREE admits one drop slot");
        CHECK(rc[0].word == 0x9241u && rc[0].item_id == 292 &&
                  rc[0].base_count == 2 && rc[0].extra_range == 4 &&
                  rc[0].extra_roll == roll && rc[0].final_count == 2 + roll,
              "TREE word 0x9241 decoded");
        CHECK(gd.first_item_id == 292 && gd.total_count == 2 + roll,
              "TREE aggregate");
    }

    /* ── THORN DEMON (type 19): no drop word fields → nothing admitted ── */
    {
        dm2_v1_drops_rng_init(&rng);
        int admitted = dm2_v1_drops_resolve_gdat_creature_drops(
            &loader, DM2_AI_THORN_DEMON, &rng, rc, &gd);
        CHECK(admitted == 0 && gd.admitted == 0 && gd.total_count == 0,
              "THORN DEMON has no GDAT drop words (fallback path intact)");
    }

    /* ── Live chain: real GDAT session drives the death-drop observer ── */
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) >= 0,
          "AI table loader accepts the real GDAT session");
    CHECK(dm2_v1_creature_drop_slots_loaded(TEST_AI_GLOP) == 1,
          "GLOP drop words imported from real GDAT");
    CHECK(dm2_v1_creature_drop_slot_word(TEST_AI_GLOP, 0) == 0x8E10u,
          "GLOP slot word 0 matches GDAT");
    CHECK(dm2_v1_creature_drop_slot_word(TEST_AI_GLOP, 1) == 0x9D10u,
          "GLOP slot word 1 matches GDAT");
    {
        int roll_a;
        int roll_b;
        int slot;
        DM2_V1_CreatureDeathDropObserver obs;

        g_rep_state = 0;
        roll_a = replica_rand16(2);
        roll_b = replica_rand16(2);

        dm2_v1_creature_test_reset_instances();
        dm2_v1_creature_reset_death_observer();
        dm2_v1_creature_drop_rng_reset();

        slot = dm2_v1_creature_spawn(TEST_AI_GLOP, 3, 4, 0, 1, 0);
        CHECK(slot >= 0, "GLOP spawn");
        dm2_v1_creature_deal_damage(slot, 9999);
        dm2_v1_creature_tick();

        CHECK(dm2_v1_creature_last_death_drop(&obs) == 1, "death observed");
        CHECK(obs.source_ordered == 1, "death drop is source-ordered");
        CHECK(obs.source_slots_admitted == 2, "observer admits two slots");
        CHECK(obs.dropped == 1 && obs.item_id == 284,
              "observer drops real GDAT item 284");
        CHECK(obs.count == (1 + roll_a) + (1 + roll_b),
              "observer count matches replica rolls");
    }

    /* ── Combat defense route stays fail-closed against this GDAT ──
     * The local PC English GRAPHICS.DAT has no CREATURE_AI (0x19)
     * category, so no creature defense can be proven; the real-data
     * combat route must reject explicitly instead of inventing one. */
    {
        uint16_t defense = 0xFFFFu;
        DM2_V1_WeaponInfo melee = { DM2_WEAPON_MELEE, 10, 1, 0, 0 };
        DM2_V1_CombatCreatureReceipt cr;

        CHECK(dm2_v1_creature_ai_defense(TEST_AI_GLOP, &defense) == 0 &&
                  defense == 0u,
              "creature defense unproven without a CREATURE_AI row");
        dm2_v1_combat_bind_creature_defense_fn(dm2_v1_creature_ai_defense);
        CHECK(dm2_v1_combat_resolve_attack_on_creature(
                  &melee, 16, TEST_AI_GLOP, 100, 1, 0, 0, &cr) == 0,
              "attack on creature rejected without proven defense");
        CHECK(cr.valid && cr.rejected_defense_unproven && cr.damage == 0,
              "defense-unproven receipt");
        dm2_v1_combat_bind_creature_defense_fn(NULL);
    }

    dm2_v1_creature_reset_ai_table();
    free(graphics);

    if (g_failures == 0) {
        printf("PASS: dm2_v1_drops_gdat_real_data (GLOP/ATTACK MINION/TREE "
               "drop words, death observer, fail-closed combat defense)\n");
        return 0;
    }
    fprintf(stderr, "FAILURES: %d\n", g_failures);
    return 1;
}
