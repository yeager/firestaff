/*
 * test_dm1_v1_object_interaction_pc34_compat.c — CTest gate
 *
 * Tests source-locked to ReDMCSB object/item interaction:
 *   1. Pick up / drop from dungeon floor
 *   2. Throw mechanics (stamina cost, kinetic energy, trajectory)
 *   3. Use actions (eat food, drink potion, read scroll)
 *   4. Container (chest) open/close/transfer
 *   5. Weight/encumbrance calculation
 *   6. Stat modifier application (equip/unequip)
 *   7. Slot click swap (leader hand ↔ inventory slot)
 */
#include "dm1_v1_object_interaction_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %-58s", name); } while(0)
#define PASS() do { printf("[PASS]\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("[FAIL] %s\n", msg); tests_failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

/* ── Test 1: Pick up from floor ──────────────────────────────────── */
static void test_pick_up_from_floor(void) {
    TEST("Pick up object from dungeon floor");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv champs[4];
    dm1_obj_champion_inv_init(&champs[0]);
    ctx.leaderIndex = 0;

    uint16_t thing = 0x1400; /* type=5 (weapon), index=0 */
    int ok = dm1_obj_pick_up_from_floor(&ctx, champs, thing, 32, 10);
    CHECK(ok == 1, "pick up should succeed");
    CHECK(ctx.leaderHand.thing == thing, "leader hand should hold thing");
    CHECK(ctx.leaderEmptyHanded == 0, "leader should not be empty-handed");
    CHECK(champs[0].load == 10, "champion load should include object weight");
    PASS();
}

/* ── Test 2: Pick up fails when hand full ────────────────────────── */
static void test_pick_up_hand_full(void) {
    TEST("Pick up fails when hand is full");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv champs[4];
    dm1_obj_champion_inv_init(&champs[0]);
    ctx.leaderIndex = 0;

    dm1_obj_pick_up_from_floor(&ctx, champs, 0x1400, 32, 10);
    int ok = dm1_obj_pick_up_from_floor(&ctx, champs, 0x1401, 33, 8);
    CHECK(ok == 0, "second pick up should fail");
    CHECK(ctx.leaderHand.thing == 0x1400, "original thing should remain");
    PASS();
}

/* ── Test 3: Drop to floor ───────────────────────────────────────── */
static void test_drop_to_floor(void) {
    TEST("Drop object to dungeon floor");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv champs[4];
    dm1_obj_champion_inv_init(&champs[0]);
    ctx.leaderIndex = 0;

    dm1_obj_pick_up_from_floor(&ctx, champs, 0x1400, 32, 10);
    int ok = dm1_obj_drop_to_floor(&ctx, champs);
    CHECK(ok == 1, "drop should succeed");
    CHECK(ctx.leaderEmptyHanded == 1, "hand should be empty after drop");
    CHECK(ctx.leaderHand.thing == DM1_OBJ_THING_NONE, "thing should be NONE");
    CHECK(champs[0].load == 0, "load should be 0 after drop");
    PASS();
}

/* ── Test 4: Throwing stamina cost (F0305) ───────────────────────── */
static void test_throwing_stamina_cost(void) {
    TEST("Throwing stamina cost (F0305 formula)");
    /* weight=10 → halfWeight=5 → clamp(5,1,10)=5, no loop iterations */
    CHECK(dm1_obj_get_throwing_stamina_cost(10) == 5, "weight 10 → cost 5");
    /* weight=2 → halfWeight=1 → clamp(1,1,10)=1 */
    CHECK(dm1_obj_get_throwing_stamina_cost(2) == 1, "weight 2 → cost 1");
    /* weight=20 → halfWeight=10 → clamp(10,1,10)=10, no extra (10-10=0) */
    CHECK(dm1_obj_get_throwing_stamina_cost(20) == 10, "weight 20 → cost 10");
    PASS();
}

/* ── Test 5: Throw object ────────────────────────────────────────── */
static void test_throw_object(void) {
    TEST("Throw object from leader hand");
    dm1_obj_seed_rng(42);
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv ch;
    dm1_obj_champion_inv_init(&ch);
    ctx.leaderIndex = 0;

    /* Put a dagger in leader hand */
    ctx.leaderHand.thing = 0x1408;
    ctx.leaderHand.iconIndex = 32;
    ctx.leaderHand.weight = 8;
    ctx.leaderEmptyHanded = 0;

    DM1_ThrowResult res = dm1_obj_throw(&ctx, &ch, -1, 8, 50, 5);
    CHECK(res.success == 1, "throw should succeed");
    CHECK(res.kineticEnergy > 0, "kinetic energy should be positive");
    CHECK(res.attack >= 40 && res.attack <= 200, "attack should be bounded 40-200");
    CHECK(res.stepEnergy >= 5, "step energy should be >= 5");
    CHECK(res.staminaCost > 0, "stamina cost should be positive");
    CHECK(ctx.leaderEmptyHanded == 1, "hand should be empty after throw");
    PASS();
}

/* ── Test 6: Use item — eat food ─────────────────────────────────── */
static void test_use_eat(void) {
    TEST("Use item: eat food");
    DM1_ObjectState obj = {0};
    obj.thing = 0x2800;  /* junk type */
    obj.iconIndex = 168; /* apple */
    obj.weight = 3;
    obj.useAction = DM1_USE_ACTION_EAT;

    DM1_UseResult res = dm1_obj_use_item(&obj);
    CHECK(res.consumed == 1, "food should be consumed");
    CHECK(res.foodGain > 0, "food gain should be positive");
    CHECK(res.poisoned == 0, "normal food should not poison");
    PASS();
}

/* ── Test 7: Use item — drink potion ─────────────────────────────── */
static void test_use_drink(void) {
    TEST("Use item: drink potion");
    DM1_ObjectState obj = {0};
    obj.thing = 0x2000;  /* potion type */
    obj.iconIndex = 150;
    obj.weight = 3;
    obj.useAction = DM1_USE_ACTION_DRINK;
    obj.chargeCount = 3;

    DM1_UseResult res = dm1_obj_use_item(&obj);
    CHECK(res.consumed == 1, "potion should be consumed");
    CHECK(res.healthGain > 0, "health gain should be positive");
    CHECK(res.staminaGain > 0, "stamina gain should be positive");
    PASS();
}

/* ── Test 8: Use item — drink water ──────────────────────────────── */
static void test_use_drink_water(void) {
    TEST("Use item: drink water flask");
    DM1_ObjectState obj = {0};
    obj.thing = 0x2000;
    obj.iconIndex = DM1_ICON_WATER_FLASK;
    obj.weight = 5;
    obj.useAction = DM1_USE_ACTION_DRINK;

    DM1_UseResult res = dm1_obj_use_item(&obj);
    CHECK(res.consumed == 1, "water should be consumed");
    CHECK(res.waterGain > 0, "water gain should be positive");
    PASS();
}

/* ── Test 9: Use item — read scroll ──────────────────────────────── */
static void test_use_read(void) {
    TEST("Use item: read scroll");
    DM1_ObjectState obj = {0};
    obj.thing = 0x1C00;  /* scroll type */
    obj.iconIndex = DM1_ICON_SCROLL_OPEN;
    obj.weight = 1;
    obj.useAction = DM1_USE_ACTION_READ;

    DM1_UseResult res = dm1_obj_use_item(&obj);
    CHECK(res.scrollRead == 1, "scroll should trigger read");
    CHECK(res.consumed == 0, "scroll should not be consumed");
    PASS();
}

/* ── Test 10: Use poisoned food ──────────────────────────────────── */
static void test_use_poisoned(void) {
    TEST("Use item: eat poisoned food");
    DM1_ObjectState obj = {0};
    obj.thing = 0x2800;
    obj.iconIndex = 168;
    obj.weight = 3;
    obj.useAction = DM1_USE_ACTION_EAT;
    obj.poisoned = 1;

    DM1_UseResult res = dm1_obj_use_item(&obj);
    CHECK(res.consumed == 1, "poisoned food should be consumed");
    CHECK(res.poisoned == 1, "should report poisoned");
    PASS();
}

/* ── Test 11: Chest open/close ───────────────────────────────────── */
static void test_chest_open_close(void) {
    TEST("Container: open and close chest");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);

    uint16_t contents[4] = { 0x1400, 0x1401, 0x2000, 0x2800 };
    dm1_obj_open_chest(&ctx, 0x2400, contents, 4);

    CHECK(ctx.chest.openChestThing == 0x2400, "chest should be open");
    CHECK(ctx.chest.chestSlots[0] == 0x1400, "slot 0 should have first item");
    CHECK(ctx.chest.chestSlots[3] == 0x2800, "slot 3 should have fourth item");
    CHECK(ctx.chest.chestSlots[4] == DM1_OBJ_THING_NONE, "slot 4 should be empty");

    dm1_obj_close_chest(&ctx);
    CHECK(ctx.chest.openChestThing == DM1_OBJ_THING_NONE, "chest should be closed");
    CHECK(ctx.chest.chestSlots[0] == DM1_OBJ_THING_NONE, "slots should be cleared");
    PASS();
}

/* ── Test 12: Chest max 8 items (CHANGE8_08_FIX) ────────────────── */
static void test_chest_max_8(void) {
    TEST("Container: max 8 items in chest (CHANGE8_08_FIX)");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);

    uint16_t contents[12];
    for (int i = 0; i < 12; i++) contents[i] = (uint16_t)(0x1400 + i);

    dm1_obj_open_chest(&ctx, 0x2400, contents, 12);
    CHECK(ctx.chest.chestSlots[7] == 0x1407, "slot 7 should have 8th item");
    /* 9th+ items are silently dropped (not in chest) */
    PASS();
}

/* ── Test 13: Don't re-open same chest (CHANGE7_27_FIX) ─────────── */
static void test_chest_no_reopen(void) {
    TEST("Container: no re-open same chest (CHANGE7_27_FIX)");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);

    uint16_t c1[] = { 0x1400 };
    dm1_obj_open_chest(&ctx, 0x2400, c1, 1);
    /* Manually change a slot to detect if re-open would overwrite */
    ctx.chest.chestSlots[0] = 0x9999;

    uint16_t c2[] = { 0x1401 };
    dm1_obj_open_chest(&ctx, 0x2400, c2, 1);
    CHECK(ctx.chest.chestSlots[0] == 0x9999, "should NOT re-open same chest");
    PASS();
}

/* ── Test 14: Weight / max load ──────────────────────────────────── */
static void test_max_load(void) {
    TEST("Weight/encumbrance: F0309 max load calculation");
    DM1_ChampionInv ch;
    dm1_obj_champion_inv_init(&ch);
    /* Default strength current = 30 → (30*8)+100 = 340 */
    uint16_t ml = dm1_obj_get_max_load(&ch);
    /* 340, rounded to next 10 = 340 (already multiple). Actually: +9=349, 349-349%10=340 */
    CHECK(ml == 340, "max load for strength 30 should be 340");

    /* Reduce stamina below half → load should decrease */
    ch.currentStamina = 20;
    ch.maxStamina = 100;
    uint16_t ml2 = dm1_obj_get_max_load(&ch);
    CHECK(ml2 < ml, "reduced stamina should lower max load");
    PASS();
}

/* ── Test 15: Stat modifiers — equip Mace of Order ───────────────── */
static void test_stat_modifier_equip(void) {
    TEST("Stat modifiers: equip Mace of Order (Strength +5)");
    DM1_ChampionInv ch;
    dm1_obj_champion_inv_init(&ch);
    uint8_t before = ch.statistics[DM1_STAT_STRENGTH][1];

    dm1_obj_apply_stat_modifiers(&ch, DM1_SLOT_ACTION_HAND,
                                  DM1_ICON_MACE_OF_ORDER, 1);
    CHECK(ch.statistics[DM1_STAT_STRENGTH][1] == before + 5,
          "strength current should increase by 5");
    PASS();
}

/* ── Test 16: Stat modifiers — unequip ───────────────────────────── */
static void test_stat_modifier_unequip(void) {
    TEST("Stat modifiers: unequip Mace of Order (Strength -5)");
    DM1_ChampionInv ch;
    dm1_obj_champion_inv_init(&ch);

    dm1_obj_apply_stat_modifiers(&ch, DM1_SLOT_ACTION_HAND,
                                  DM1_ICON_MACE_OF_ORDER, 1);
    uint8_t after_equip = ch.statistics[DM1_STAT_STRENGTH][1];

    dm1_obj_apply_stat_modifiers(&ch, DM1_SLOT_ACTION_HAND,
                                  DM1_ICON_MACE_OF_ORDER, -1);
    CHECK(ch.statistics[DM1_STAT_STRENGTH][1] == after_equip - 5,
          "strength should return to original after unequip");
    PASS();
}

/* ── Test 17: Rabbit's Foot in chest should NOT apply modifier ───── */
static void test_rabbits_foot_in_chest(void) {
    TEST("Stat modifiers: Rabbit's Foot in chest (no effect, CHANGE7_25_FIX)");
    DM1_ChampionInv ch;
    dm1_obj_champion_inv_init(&ch);
    uint8_t before = ch.statistics[DM1_STAT_LUCK][1];

    dm1_obj_apply_stat_modifiers(&ch, DM1_SLOT_CHEST_1,
                                  DM1_ICON_RABBITS_FOOT, 1);
    CHECK(ch.statistics[DM1_STAT_LUCK][1] == before,
          "luck should NOT change for Rabbit's Foot in chest");
    PASS();
}

/* ── Test 18: Slot click — swap objects ──────────────────────────── */
static void test_slot_click_swap(void) {
    TEST("Slot click: swap leader hand with action hand");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv champs[4];
    dm1_obj_champion_inv_init(&champs[0]);
    ctx.leaderIndex = 0;

    /* Put dagger in leader hand */
    dm1_obj_put_in_leader_hand(&ctx, champs, 0x1408, 32, 8);
    /* Put sword in action hand */
    champs[0].slots[DM1_SLOT_ACTION_HAND] = 0x140A;
    champs[0].load += 12;

    /* Click on action hand slot → swap */
    int ok = dm1_obj_click_on_slot(&ctx, champs, 0, DM1_SLOT_ACTION_HAND,
                                    DM1_ALLOWED_HANDS);
    CHECK(ok == 1, "slot click should succeed");
    /* Leader hand should now have the sword */
    CHECK(ctx.leaderHand.thing == 0x140A, "leader should hold sword");
    /* Action hand should have the dagger */
    CHECK(champs[0].slots[DM1_SLOT_ACTION_HAND] == 0x1408,
          "action hand should hold dagger");
    PASS();
}

/* ── Test 19: Slot click — disallowed slot ───────────────────────── */
static void test_slot_click_disallowed(void) {
    TEST("Slot click: object rejected from incompatible slot");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv champs[4];
    dm1_obj_champion_inv_init(&champs[0]);
    ctx.leaderIndex = 0;

    /* Put something in leader hand with AllowedSlots = MOUTH only */
    dm1_obj_put_in_leader_hand(&ctx, champs, 0x2000, 150, 3);

    /* Try to put in head slot (requires HEAD bit = 0x0002) */
    int ok = dm1_obj_click_on_slot(&ctx, champs, 0, DM1_SLOT_HEAD,
                                    DM1_ALLOWED_MOUTH); /* only mouth allowed */
    CHECK(ok == 0, "should reject mouth-only object in head slot");
    CHECK(ctx.leaderHand.thing == 0x2000, "leader should still hold object");
    PASS();
}

/* ── Test 20: Add to slot updates load ───────────────────────────── */
static void test_add_to_slot_load(void) {
    TEST("Add/remove from slot updates champion load");
    DM1_ObjCtx ctx;
    dm1_obj_ctx_init(&ctx);
    DM1_ChampionInv ch;
    dm1_obj_champion_inv_init(&ch);

    uint16_t before = ch.load;
    dm1_obj_add_to_slot(&ctx, &ch, 0x1400, DM1_SLOT_READY_HAND, 32, 15);
    CHECK(ch.load == before + 15, "load should increase by weight");

    dm1_obj_remove_from_slot(&ctx, &ch, DM1_SLOT_READY_HAND, 15);
    CHECK(ch.load == before, "load should return to original");
    PASS();
}

/* ── Main ─────────────────────────────────────────────────────────── */
int main(void) {
    printf("DM1 V1 Object/Item Interaction System — source-lock gate\n");
    printf("─────────────────────────────────────────────────────────\n");

    test_pick_up_from_floor();
    test_pick_up_hand_full();
    test_drop_to_floor();
    test_throwing_stamina_cost();
    test_throw_object();
    test_use_eat();
    test_use_drink();
    test_use_drink_water();
    test_use_read();
    test_use_poisoned();
    test_chest_open_close();
    test_chest_max_8();
    test_chest_no_reopen();
    test_max_load();
    test_stat_modifier_equip();
    test_stat_modifier_unequip();
    test_rabbits_foot_in_chest();
    test_slot_click_swap();
    test_slot_click_disallowed();
    test_add_to_slot_load();

    printf("─────────────────────────────────────────────────────────\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed ? 1 : 0;
}
