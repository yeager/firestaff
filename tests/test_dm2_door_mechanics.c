/*
 * test_dm2_door_mechanics.c — DM2 V1 Door Mechanics Integration Tests
 *
 * Tests:
 *   1. Door state extraction from tile square (M036 macro parity)
 *   2. Door state setting in tile square (M037 macro parity)
 *   3. Door state machine: all 6 states parse and round-trip correctly
 *   4. Door type info table: defense and attribute values match ReDMCSB
 *   5. Movement blocking: party (height=1) vs door states
 *   6. Movement blocking: tall creature (height=3) vs door states
 *   7. Projectile passability: portcullis (type 0) always passable
 *   8. Projectile passability: wooden door blocks projectiles when closed
 *   9. Door destruction: wooden door destroyed by attack >= 42
 *  10. Door destruction: wooden door NOT destroyed by attack < 42
 *  11. Door destruction: iron door immune to melee (defense=230 > cap 100)
 *  12. Advance open: CLOSED → CLOSED_THREE_QUARTER → CLOSED_HALF → ...
 *  13. Advance close: OPEN → CLOSED_ONE_FOURTH → CLOSED_HALF → ...
 *  14. Door type parsing: GDAT2 0x09 → wooden, 0x0A → iron
 *  15. Door type parsing: ReDMCSB direct index 0-3
 *  16. Creature blocked by closed door: wooden CLOSED blocks
 *  17. Creature blocked by closed door: portcullis CLOSED does NOT block
 *  18. Melee immunity: portcullis/iron/RA immune, wooden not
 *  19. Champion wound on close: party on door square, state=1 → wounds
 *  20. Champion wound on close: state=OPEN → no wound
 *
 * Source refs:
 *   ReDMCSB DEFS.H:1039-1047 (door states C0-C5, M036/M037)
 *   ReDMCSB DEFS.H:1567-1572 (DOOR_INFO)
 *   ReDMCSB DUNGEON.C:560 (G0254_as_Graphic559_DoorInfo[4])
 *   ReDMCSB PROJEXPL.C:1554-1600 (F0232_GROUP_IsDoorDestroyedByAttack)
 *   ReDMCSB TIMELINE.C:750-810 (F0241_TIMELINE_ProcessEvent6_Square_Door)
 *   ReDMCSB GROUP.C:1190-1202 (creature blocked by door)
 *   ReDMCSB CLIKMENU.C:283-285 (movement blocked by door)
 */

#include "dm2_v1_door_mechanics.h"
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

/* ── Test 1: Door state extraction (M036 parity) ────────────────── */

static int test_door_state_extraction(void)
{
    /* M036_DOOR_STATE(square) = square & 0x0007 */
    /* Square with door state in lower 3 bits, other bits set */
    uint16_t square = 0x1234; /* upper bits, door state = 4 */
    int state = dm2_door_get_state(square);
    return state == 4;
}

static int test_door_state_extraction_all_states(void)
{
    int ok = 1;
    for (int s = 0; s <= 5; s++) {
        /* Encode state s in lower 3 bits, garbage in upper */
        uint16_t square = (uint16_t)(0xF000 | (uint16_t)s);
        int got = dm2_door_get_state(square);
        if (got != s) { ok = 0; printf("    state=%d expected, got %d\n", s, got); }
    }
    return ok;
}

/* ── Test 2: Door state setting (M037 parity) ──────────────────── */

static int test_door_state_setting_roundtrip(void)
{
    int ok = 1;
    for (int s = 0; s <= 5; s++) {
        uint16_t square = 0xABCD; /* arbitrary upper bits */
        uint16_t r = dm2_door_set_state(square, s);
        /* Verify: upper bits preserved, lower 3 bits = state */
        if ((r & 0xFFF8) != (square & 0xFFF8)) { ok = 0; }
        if ((r & 0x0007) != (uint16_t)s) { ok = 0; }
        /* Extract back */
        if (dm2_door_get_state(r) != s) { ok = 0; }
    }
    return ok;
}

/* ── Test 3: All 6 door states parse correctly ─────────────────── */

static int test_all_door_states(void)
{
    /* Test that each state value is accepted and labeled */
    const char *labels[] = { "OPEN", "CLOSED_1/4", "CLOSED_1/2", "CLOSED_3/4", "CLOSED", "DESTROYED" };
    for (int s = 0; s <= 5; s++) {
        const char *l = dm2_door_state_label(s);
        if (!l || strcmp(l, labels[s]) != 0) {
            printf("    label[%d] = '%s', expected '%s'\n", s, l ? l : "NULL", labels[s]);
            return 0;
        }
    }
    return 1;
}

/* ── Test 4: Door type info table values ─────────────────────────── */

static int test_door_type_info_table(void)
{
    /* Type 0 — Portcullis: attrs = 0x0002|0x0001 = 0x0003, defense = 110 */
    if (dm2_door_get_defense(0) != 110) { printf("    portcullis def=%d\n", dm2_door_get_defense(0)); return 0; }
    if (dm2_door_get_attributes(0) != (DM2_DOOR_ATTR_PROJECTILES_CAN_PASS_THROUGH | DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH)) {
        printf("    portcullis attrs=0x%04X\n", dm2_door_get_attributes(0)); return 0;
    }

    /* Type 1 — Wooden: attrs = 0, defense = 42 */
    if (dm2_door_get_defense(1) != 42) { printf("    wooden def=%d\n", dm2_door_get_defense(1)); return 0; }
    if (dm2_door_get_attributes(1) != 0) { printf("    wooden attrs=0x%04X\n", dm2_door_get_attributes(1)); return 0; }

    /* Type 2 — Iron: attrs = 0, defense = 230 */
    if (dm2_door_get_defense(2) != 230) { printf("    iron def=%d\n", dm2_door_get_defense(2)); return 0; }
    if (dm2_door_get_attributes(2) != 0) { printf("    iron attrs=0x%04X\n", dm2_door_get_attributes(2)); return 0; }

    /* Type 3 — RA: attrs = 0x0004|0x0001 = 0x0005, defense = 255 */
    if (dm2_door_get_defense(3) != 255) { printf("    RA def=%d\n", dm2_door_get_defense(3)); return 0; }
    if (dm2_door_get_attributes(3) != (DM2_DOOR_ATTR_ANIMATED | DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH)) {
        printf("    RA attrs=0x%04X\n", dm2_door_get_attributes(3)); return 0;
    }

    return 1;
}

/* ── Test 5: Movement blocking for party (height=1) ────────────── */

static int test_movement_blocking_party(void)
{
    /* Party height = 1. States that block: > 1 and != 5 */
    int ok = 1;
    /* State 0 OPEN → NOT blocked */
    if (dm2_door_state_blocks_movement(0, 1) != 0) { printf("  state 0 blocks party?\n"); ok = 0; }
    /* State 1 CLOSED_1/4 → NOT blocked (height 1) */
    if (dm2_door_state_blocks_movement(1, 1) != 0) { printf("  state 1 blocks party?\n"); ok = 0; }
    /* State 2 CLOSED_1/2 → blocked (2 > 1) */
    if (dm2_door_state_blocks_movement(2, 1) != 1) { printf("  state 2 doesn't block party?\n"); ok = 0; }
    /* State 3 CLOSED_3/4 → blocked */
    if (dm2_door_state_blocks_movement(3, 1) != 1) { printf("  state 3 doesn't block party?\n"); ok = 0; }
    /* State 4 CLOSED → blocked */
    if (dm2_door_state_blocks_movement(4, 1) != 1) { printf("  state 4 doesn't block party?\n"); ok = 0; }
    /* State 5 DESTROYED → NOT blocked */
    if (dm2_door_state_blocks_movement(5, 1) != 0) { printf("  state 5 blocks party?\n"); ok = 0; }
    return ok;
}

/* ── Test 6: Movement blocking for tall creature (height=3) ─────── */

static int test_movement_blocking_tall_creature(void)
{
    /* Creature height = 3. States that block: > 3 and != 5 */
    int ok = 1;
    /* State 0 OPEN → NOT blocked */
    if (dm2_door_state_blocks_movement(0, 3) != 0) { printf("  state 0 blocks tall?\n"); ok = 0; }
    /* State 1 → NOT blocked (1 <= 3) */
    if (dm2_door_state_blocks_movement(1, 3) != 0) { printf("  state 1 blocks tall?\n"); ok = 0; }
    /* State 2 → NOT blocked (2 <= 3) */
    if (dm2_door_state_blocks_movement(2, 3) != 0) { printf("  state 2 blocks tall?\n"); ok = 0; }
    /* State 3 → NOT blocked (3 <= 3) */
    if (dm2_door_state_blocks_movement(3, 3) != 0) { printf("  state 3 blocks tall?\n"); ok = 0; }
    /* State 4 CLOSED → blocked (4 > 3) */
    if (dm2_door_state_blocks_movement(4, 3) != 1) { printf("  state 4 doesn't block tall?\n"); ok = 0; }
    /* State 5 DESTROYED → NOT blocked */
    if (dm2_door_state_blocks_movement(5, 3) != 0) { printf("  state 5 blocks tall?\n"); ok = 0; }
    return ok;
}

/* ── Test 7: Portcullis always passable to projectiles ──────────── */

static int test_portcullis_projectile_passable(void)
{
    /* Portcullis (type 0) has MASK0x0002_PROJECTILES_CAN_PASS_THROUGH */
    uint16_t portcullis_attrs = dm2_door_get_attributes(0);
    int ok = 1;
    /* Even CLOSED (4) portcullis allows projectiles */
    if (!dm2_door_is_passable_to_projectile(4, portcullis_attrs)) { printf("  closed portcullis blocks projectiles?\n"); ok = 0; }
    if (!dm2_door_is_passable_to_projectile(3, portcullis_attrs)) { printf("  3/4 portcullis blocks projectiles?\n"); ok = 0; }
    /* OPEN is always passable */
    if (!dm2_door_is_passable_to_projectile(0, portcullis_attrs)) { printf("  open portcullis blocks projectiles?\n"); ok = 0; }
    /* DESTROYED is always passable */
    if (!dm2_door_is_passable_to_projectile(5, portcullis_attrs)) { printf("  destroyed portcullis blocks projectiles?\n"); ok = 0; }
    return ok;
}

/* ── Test 8: Wooden door blocks projectiles when closed ────────── */

static int test_wooden_door_projectile_blocked(void)
{
    uint16_t wooden_attrs = dm2_door_get_attributes(1); /* = 0 */
    int ok = 1;
    /* OPEN → passable */
    if (!dm2_door_is_passable_to_projectile(0, wooden_attrs)) { printf("  open wooden blocks?\n"); ok = 0; }
    /* CLOSED_1/4 → passable (state <= 1) */
    if (!dm2_door_is_passable_to_projectile(1, wooden_attrs)) { printf("  1/4 wooden blocks?\n"); ok = 0; }
    /* CLOSED_HALF (2) → blocked (no MASK0x0002, state > 1) */
    if (dm2_door_is_passable_to_projectile(2, wooden_attrs)) { printf("  half wooden is passable?\n"); ok = 0; }
    /* CLOSED (4) → blocked */
    if (dm2_door_is_passable_to_projectile(4, wooden_attrs)) { printf("  closed wooden is passable?\n"); ok = 0; }
    /* DESTROYED → passable */
    if (!dm2_door_is_passable_to_projectile(5, wooden_attrs)) { printf("  destroyed wooden blocks?\n"); ok = 0; }
    return ok;
}

/* ── Test 9: Wooden door destroyed by attack >= 42 ─────────────── */

static int test_wooden_door_destroyed_at_defense(void)
{
    /* Wooden (type 1) defense = 42, melee/magic destructible */
    uint8_t flags = 0x01 | 0x02;
    int ok = 1;
    /* Attack 42 → destroyed */
    if (dm2_door_check_destruction(1, 4, 42, 0, flags) != DM2_DOOR_DESTROYED_YES) { printf("  atk=42 not destroyed?\n"); ok = 0; }
    /* Attack 43 → destroyed */
    if (dm2_door_check_destruction(1, 4, 43, 0, flags) != DM2_DOOR_DESTROYED_YES) { printf("  atk=43 not destroyed?\n"); ok = 0; }
    /* Attack 100 (max melee) → destroyed */
    if (dm2_door_check_destruction(1, 4, 100, 0, flags) != DM2_DOOR_DESTROYED_YES) { printf("  atk=100 not destroyed?\n"); ok = 0; }
    return ok;
}

/* ── Test 10: Wooden door NOT destroyed by attack < 42 ──────────── */

static int test_wooden_door_not_destroyed_below_defense(void)
{
    uint8_t flags = 0x01 | 0x02;
    int ok = 1;
    /* Attack 41 → NOT destroyed */
    if (dm2_door_check_destruction(1, 4, 41, 0, flags) != DM2_DOOR_DESTROYED_NO) { printf("  atk=41 destroyed?\n"); ok = 0; }
    /* Attack 1 → NOT destroyed */
    if (dm2_door_check_destruction(1, 4, 1, 0, flags) != DM2_DOOR_DESTROYED_NO) { printf("  atk=1 destroyed?\n"); ok = 0; }
    return ok;
}

/* ── Test 11: Iron door immune to melee ─────────────────────────── */

/* ── Test 11: Iron door — attack vs defense thresholds ──────────── */

/*
 * Iron door (type 2): defense = 230.
 *
 * F0232_GROUP_IsDoorDestroyedByAttack (PROJEXPL.C:1554-1600) only uses
 * the per-door destructibility flags for immunity checks — NOT the defense value.
 * So with flags=0x03 (both melee and magic destructible), iron is NOT immune to
 * melee; it simply fails the attack-vs-defense check (100 < 230 → NO destruction).
 *
 * The "melee attacks limited to 100" comment in DUNGEON.C:560 is a game-design
 * note about the weapon system, not F0232 logic.
 *
 * Source: ReDMCSB PROJEXPL.C:1554-1600
 */
static int test_iron_door_by_attack_strength(void)
{
    uint8_t flags = 0x01 | 0x02; /* both melee and magic destructible */
    int ok = 1;
    /* Iron defense=230. Melee atk=100 < 230 → NOT destroyed (but not immune either) */
    if (dm2_door_check_destruction(2, 4, 100, 0, flags) != DM2_DOOR_DESTROYED_NO) { printf("  iron melee atk=100 not NO?\n"); ok = 0; }
    /* Iron magic atk=230 >= 230 → destroyed */
    if (dm2_door_check_destruction(2, 4, 230, 1, flags) != DM2_DOOR_DESTROYED_YES) { printf("  iron magic atk=230 not destroyed?\n"); ok = 0; }
    /* Iron magic atk=229 < 230 → not destroyed */
    if (dm2_door_check_destruction(2, 4, 229, 1, flags) != DM2_DOOR_DESTROYED_NO) { printf("  iron magic atk=229 destroyed?\n"); ok = 0; }
    return ok;
}

/* ── Test 12: Advance open: CLOSED → OPEN ──────────────────────── */

static int test_advance_open(void)
{
    int ok = 1;
    /* CLOSED (4) → 3 */
    if (dm2_door_advance_open(4) != 3) { printf("  close(4)=%d\n", dm2_door_advance_open(4)); ok = 0; }
    /* CLOSED_3/4 (3) → 2 */
    if (dm2_door_advance_open(3) != 2) { printf("  close(3)=%d\n", dm2_door_advance_open(3)); ok = 0; }
    /* CLOSED_HALF (2) → 1 */
    if (dm2_door_advance_open(2) != 1) { printf("  close(2)=%d\n", dm2_door_advance_open(2)); ok = 0; }
    /* CLOSED_1/4 (1) → 0 */
    if (dm2_door_advance_open(1) != 0) { printf("  close(1)=%d\n", dm2_door_advance_open(1)); ok = 0; }
    /* OPEN (0) → sticky at 0 */
    if (dm2_door_advance_open(0) != 0) { printf("  close(0)=%d\n", dm2_door_advance_open(0)); ok = 0; }
    /* DESTROYED (5) → sticky at 5 */
    if (dm2_door_advance_open(5) != 5) { printf("  close(5)=%d\n", dm2_door_advance_open(5)); ok = 0; }
    return ok;
}

/* ── Test 13: Advance close: OPEN → CLOSED ─────────────────────── */

static int test_advance_close(void)
{
    int ok = 1;
    /* OPEN (0) → 1 */
    if (dm2_door_advance_close(0) != 1) { printf("  open(0)=%d\n", dm2_door_advance_close(0)); ok = 0; }
    /* CLOSED_1/4 (1) → 2 */
    if (dm2_door_advance_close(1) != 2) { printf("  open(1)=%d\n", dm2_door_advance_close(1)); ok = 0; }
    /* CLOSED_HALF (2) → 3 */
    if (dm2_door_advance_close(2) != 3) { printf("  open(2)=%d\n", dm2_door_advance_close(2)); ok = 0; }
    /* CLOSED_3/4 (3) → 4 */
    if (dm2_door_advance_close(3) != 4) { printf("  open(3)=%d\n", dm2_door_advance_close(3)); ok = 0; }
    /* CLOSED (4) → sticky at 4 */
    if (dm2_door_advance_close(4) != 4) { printf("  open(4)=%d\n", dm2_door_advance_close(4)); ok = 0; }
    /* DESTROYED (5) → sticky at 5 */
    if (dm2_door_advance_close(5) != 5) { printf("  open(5)=%d\n", dm2_door_advance_close(5)); ok = 0; }
    return ok;
}

/* ── Test 14: Door type parsing — GDAT2 encoding ──────────────── */

static int test_door_type_parsing_gdat2(void)
{
    int ok = 1;
    uint8_t rec_wooden[4] = { 0x09, 4, 0, 0 };
    uint8_t rec_iron[4]   = { 0x0A, 4, 0, 0 };
    /* 0x09 GDAT2 clan → wooden (index 1) */
    if (dm2_door_get_type_from_thing_record(rec_wooden) != DM2_DOOR_TYPE_WOODEN) { printf("  0x09→wooden?\n"); ok = 0; }
    /* 0x0A GDAT2 dragon → iron (index 2) */
    if (dm2_door_get_type_from_thing_record(rec_iron) != DM2_DOOR_TYPE_IRON) { printf("  0x0A→iron?\n"); ok = 0; }
    return ok;
}

/* ── Test 15: Door type parsing — ReDMCSB direct index ────────── */

static int test_door_type_parsing_direct_index(void)
{
    int ok = 1;
    uint8_t rec[4] = { 0, 4, 0, 0 };
    for (int t = 0; t <= 3; t++) {
        rec[0] = (uint8_t)t;
        if (dm2_door_get_type_from_thing_record(rec) != t) { printf("  direct index %d failed\n", t); ok = 0; }
    }
    return ok;
}

/* ── Test 16: Creature blocked by wooden closed door ───────────── */

static int test_creature_blocked_by_wooden_closed(void)
{
    /* Wooden door: attrs=0 (no CREATURES_CAN_SEE_THROUGH) */
    uint16_t wooden_attrs = dm2_door_get_attributes(1);
    int ok = 1;
    /* CLOSED (4), non-material=0 → blocked */
    if (!dm2_door_creature_is_blocked_by_closed_door(4, wooden_attrs, 0)) { printf("  creature not blocked by wooden closed?\n"); ok = 0; }
    /* CLOSED_3/4 (3), non-material=0 → blocked */
    if (!dm2_door_creature_is_blocked_by_closed_door(3, wooden_attrs, 0)) { printf("  creature not blocked by wooden 3/4?\n"); ok = 0; }
    /* CLOSED (4), non-material=1 → NOT blocked */
    if (dm2_door_creature_is_blocked_by_closed_door(4, wooden_attrs, 1)) { printf("  non-mat blocked by wooden?\n"); ok = 0; }
    /* OPEN (0) → NOT blocked */
    if (dm2_door_creature_is_blocked_by_closed_door(0, wooden_attrs, 0)) { printf("  creature blocked by open?\n"); ok = 0; }
    return ok;
}

/* ── Test 17: Creature NOT blocked by portcullis closed door ───── */

static int test_creature_not_blocked_by_portcullis(void)
{
    /* Portcullis has CREATURES_CAN_SEE_THROUGH (0x0001) */
    uint16_t portcullis_attrs = dm2_door_get_attributes(0);
    int ok = 1;
    /* CLOSED portcullis → NOT blocked (has CREATURES_CAN_SEE_THROUGH) */
    if (dm2_door_creature_is_blocked_by_closed_door(4, portcullis_attrs, 0)) { printf("  creature blocked by portcullis?\n"); ok = 0; }
    /* Iron door: no see-through → blocked */
    uint16_t iron_attrs = dm2_door_get_attributes(2);
    if (!dm2_door_creature_is_blocked_by_closed_door(4, iron_attrs, 0)) { printf("  creature not blocked by iron?\n"); ok = 0; }
    return ok;
}

/* ── Test 18: Melee immunity per door type ──────────────────────── */

static int test_melee_immunity(void)
{
    int ok = 1;
    /* Portcullis (def=110 > 100 cap) → immune */
    if (!dm2_door_is_immune_to_melee(0)) { printf("  portcullis not immune?\n"); ok = 0; }
    /* Wooden (def=42 <= 100 cap) → NOT immune */
    if (dm2_door_is_immune_to_melee(1)) { printf("  wooden immune?\n"); ok = 0; }
    /* Iron (def=230 > 100 cap) → immune */
    if (!dm2_door_is_immune_to_melee(2)) { printf("  iron not immune?\n"); ok = 0; }
    /* RA (def=255 > 100 cap) → immune */
    if (!dm2_door_is_immune_to_melee(3)) { printf("  RA not immune?\n"); ok = 0; }
    return ok;
}

/* ── Test 19: Champion wound on close ─────────────────────────── */

static int test_champion_wound_on_close(void)
{
    int ok = 1;
    /* Party count > 0, state = 1 (CLOSED_1/4, door is closing) → wounds */
    if (!dm2_door_wounds_champion_on_close(4, 1, 0)) { printf("  no wound at state=1?\n"); ok = 0; }
    /* State = 2 (CLOSED_HALF) → wounds */
    if (!dm2_door_wounds_champion_on_close(4, 2, 0)) { printf("  no wound at state=2?\n"); ok = 0; }
    /* State = 4 (CLOSED) → wounds */
    if (!dm2_door_wounds_champion_on_close(4, 4, 0)) { printf("  no wound at state=4?\n"); ok = 0; }
    /* State = 0 (OPEN) → no wound */
    if (dm2_door_wounds_champion_on_close(4, 0, 0)) { printf("  wound at state=0?\n"); ok = 0; }
    /* State = 5 (DESTROYED) → no wound */
    if (dm2_door_wounds_champion_on_close(4, 5, 0)) { printf("  wound at state=5?\n"); ok = 0; }
    /* Party count = 0 → no wound */
    if (dm2_door_wounds_champion_on_close(0, 1, 0)) { printf("  wound with no party?\n"); ok = 0; }
    return ok;
}

/* ── Test 20: Already-destroyed door returns immune ─────────────── */

static int test_destroyed_door_immune(void)
{
    uint8_t flags = 0x01 | 0x02;
    int ok = 1;
    DM2_DoorDestroyResult r = dm2_door_check_destruction(1, 5, 100, 0, flags);
    if (r != DM2_DOOR_DESTROYED_IMMUNE) { printf("  destroyed door not immune?\n"); ok = 0; }
    return ok;
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(void)
{
    printf("DM2 V1 Door Mechanics Tests\n");
    printf("============================\n");
    printf("Source: ReDMCSB DEFS.H:1039-1047, DUNGEON.C:560, PROJEXPL.C:1554-1600,\n");
    printf("        TIMELINE.C:750-810, GROUP.C:1190-1202, CLIKMENU.C:283-285\n\n");

    TEST(door_state_extraction);
    TEST(door_state_extraction_all_states);
    TEST(door_state_setting_roundtrip);
    TEST(all_door_states);
    TEST(door_type_info_table);
    TEST(movement_blocking_party);
    TEST(movement_blocking_tall_creature);
    TEST(portcullis_projectile_passable);
    TEST(wooden_door_projectile_blocked);
    TEST(wooden_door_destroyed_at_defense);
    TEST(wooden_door_not_destroyed_below_defense);
    TEST(iron_door_by_attack_strength);
    TEST(advance_open);
    TEST(advance_close);
    TEST(door_type_parsing_gdat2);
    TEST(door_type_parsing_direct_index);
    TEST(creature_blocked_by_wooden_closed);
    TEST(creature_not_blocked_by_portcullis);
    TEST(melee_immunity);
    TEST(champion_wound_on_close);
    TEST(destroyed_door_immune);

    printf("\n============================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("SOME TESTS FAILED\n");
        return 1;
    }
}
