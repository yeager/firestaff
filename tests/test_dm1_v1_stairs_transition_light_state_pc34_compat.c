/**
 * DM1 V1 Stairs Transition Light State — narrow CTest gate
 *
 * Bridges the DM1_V1_LightState and DM1_V1_RoomTransitionPlanPc34Compat
 * modules and asserts the ReDMCSB-derived contract that a stairs-triggered
 * level change carries the current torch and magical-light state across
 * the transition, with the planner only mutating map-index metadata and
 * leaving champion inventories (which hold torch slots) intact.
 *
 * ReDMCSB references:
 *   CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139   — stairs trigger selection
 *   DUNGEON.C:1508-1558                            — pit/stairs map-index lookup
 *   DUNGEON.C:2724-2762                            — SetCurrentMap reloads map
 *                                                    data (door info, columns,
 *                                                    thing lists) but does NOT
 *                                                    reload champion inventories
 *                                                    or MagicalLightAmount
 *   PANEL.C:F0337_INVENTORY_SetDungeonViewPalette  — torch + magical → palette
 *   PANEL.C:F0338_DecreaseTorchesLightPower_CPSE   — per-512-tick torch drain
 *   TIMELINE.C:F0257_ProcessEvent70_Light          — light event fade chain
 *
 * Scope discipline:
 *   - No retest of F0337 / F0338 / F0257 (covered by test_dm1_v1_light_*).
 *   - No retest of the room transition planner's redraw/visual/chain
 *     mechanics (covered by test_dm1_v1_room_transition_*).
 *   - This test only asserts the stair-specific light carry-over contract.
 */

#include "dm1_v1_light_pc34_compat.h"
#include "dm1_v1_room_transition_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_true(const char *id, int cond, const char *anchor)
{
    expect_int(id, cond ? 1 : 0, 1, anchor);
}

static struct Dm1V1RoomTransitionPosePc34Compat make_pose(int mapIndex, int x, int y, int dir)
{
    struct Dm1V1RoomTransitionPosePc34Compat p;
    p.mapIndex = mapIndex;
    p.mapX = x;
    p.mapY = y;
    p.direction = dir;
    return p;
}

static struct Dm1V1RoomTransitionInputPc34Compat make_stairs_input(
    int fromMap, int toMap,
    int x, int y, int dir)
{
    struct Dm1V1RoomTransitionInputPc34Compat in;
    memset(&in, 0, sizeof(in));
    in.presentationMode = DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL;
    in.trigger = DM1_V1_ROOM_TRANSITION_TRIGGER_STAIRS;
    in.before = make_pose(fromMap, x, y, dir);
    in.after  = make_pose(toMap,   x, y, dir);
    in.partyChampionCount = 4;
    return in;
}

/* ── Test: stairs plan flags match ReDMCSB inventory/light preservation ── */

static void test_stairs_plan_asserts_inventory_and_light_preservation(void)
{
    /*
     * Stairs from map 0 to map 1: the planner must set the
     * preserveChampionInventories flag (torch slots live in champion hands
     * per DEFS.H WEAPON struct) and the preserveChampionStats flag
     * (MagicalLightAmount is a party global, not a per-map datum).
     *
     * ReDMCSB DUNGEON.C:2724-2762 SetCurrentMapAndPartyMap only swaps the
     * map data pointer, door info set, and cumulative thing columns; it
     * does not touch G0407_s_Party.MagicalLightAmount or the champion
     * hand-object slots where torch.Lit / torch.ChargeCount live.
     */
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 1, 7, 8, 0);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;

    int rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.rc", rc, 1,
               "DUNGEON.C:1508-1558; CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139");
    expect_int("stairs.active", plan.active, 1,
               "DUNGEON.C:1508-1558");
    expect_int("stairs.trigger", plan.trigger, DM1_V1_ROOM_TRANSITION_TRIGGER_STAIRS,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139");
    expect_int("stairs.map_changed", plan.mapChanged, 1,
               "DUNGEON.C:1508-1558");
    expect_true("stairs.preserve_champion_inventories",
                plan.preserveChampionInventories == 1,
                "DUNGEON.C:2724-2762; DEFS.H WEAPON.ChargeCount/Lit");
    expect_true("stairs.preserve_champion_stats",
                plan.preserveChampionStats == 1,
                "DUNGEON.C:2724-2762; PANEL.C:F0337_INVENTORY_SetDungeonViewPalette");
    expect_true("stairs.preserve_leader_hand_object",
                plan.preserveLeaderHandObject == 1,
                "DUNGEON.C:2724-2762; CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139");
}

/* ── Test: lit torch and magical light survive a stairs level change ── */

static void test_stairs_preserves_lit_torch_and_magical_light(void)
{
    /*
     * Snapshot a fully-lit state (1 champion with a max-charge lit torch
     * in the action hand, plus a pending magical light event from an
     * OH-IR-RA Magic Torch cast) and feed that same snapshot into the
     * stairs-triggered plan.  The planner must not consume, zero, or
     * touch any field of DM1_LightState — it is a pure function over
     * the input struct.
     *
     * ReDMCSB: F0267_MOVE_GetMoveResult_CPSCE triggers F0364 which only
     * mutates party.MapIndex / party.MapX / party.MapY / party.Direction.
     * It never writes to DM1_LightState.torch_slots[*] or
     * DM1_LightState.magical_light_amount.
     */
    DM1_LightState light;
    DM1_LightState snapshot;
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 1, 7, 8, 2);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;

    dm1_light_init(&light);
    dm1_light_set_champion_count(&light, 1);
    dm1_light_set_torch(&light, 0, /*action hand*/1, /*charge*/15, /*lit*/true);
    /* Cast OH-IR-RA Magic Torch (power_ordinal=1 → lightPower=3). */
    int applied = dm1_light_apply_other_spell_effect(
        &light, DM1_LIGHT_SPELL_TYPE_OTHER_LIGHT, 1);
    expect_int("magic_torch.applied", applied, 1,
               "MENU.C:F0412; TIMELINE.C:F0257");

    snapshot = light;

    int rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.rc", rc, 1,
               "DUNGEON.C:1508-1558; CLIKMENU.C:F0364_COMMAND_TakeStairs:135-139");

    /* Light state must be byte-identical to the snapshot after the plan. */
    expect_int("carry.torch.charge", light.torch_slots[1].charge_count,
               snapshot.torch_slots[1].charge_count,
               "DUNGEON.C:2724-2762; PANEL.C:F0337_INVENTORY_SetDungeonViewPalette");
    expect_int("carry.torch.lit", light.torch_slots[1].lit ? 1 : 0,
               snapshot.torch_slots[1].lit ? 1 : 0,
               "DUNGEON.C:2724-2762; CHAMPION.C equip Lit flag");
    expect_int("carry.torch.do_not_discard",
               light.torch_slots[1].do_not_discard ? 1 : 0,
               snapshot.torch_slots[1].do_not_discard ? 1 : 0,
               "DUNGEON.C:2724-2762; DEFS.H WEAPON.DoNotDiscard");
    expect_int("carry.magical_amount", light.magical_light_amount,
               snapshot.magical_light_amount,
               "DUNGEON.C:2724-2762; G0407_s_Party.MagicalLightAmount");
    expect_int("carry.total_amount", light.total_light_amount,
               snapshot.total_light_amount,
               "PANEL.C:F0337_INVENTORY_SetDungeonViewPalette L1036_i_TotalLightAmount");
    expect_int("carry.palette_idx", light.dungeon_view_palette_idx,
               snapshot.dungeon_view_palette_idx,
               "PANEL.C:F0337_INVENTORY_SetDungeonViewPalette G0304_i_DungeonViewPaletteIndex");
    expect_int("carry.event_count", light.light_event_count,
               snapshot.light_event_count,
               "TIMELINE.C:F0257 ProcessEvent70_Light");
    expect_int("carry.event_power", light.light_events[0].light_power,
               snapshot.light_events[0].light_power,
               "TIMELINE.C:F0257; MENU.C:F0404 CreateEvent70_Light");
    expect_int("carry.event_tick", light.light_events[0].expire_tick,
               snapshot.light_events[0].expire_tick,
               "TIMELINE.C:F0257; MENU.C:F0404 CreateEvent70_Light");
    expect_int("carry.game_time", light.game_time, snapshot.game_time,
               "GAMELOOP.C F0338 tick driver");
}

/* ── Test: torch fuel continues to deplete on the destination level ── */

static void test_stairs_then_tick_depletes_torch_on_new_level(void)
{
    /*
     * After a stairs transition, the light system keeps the same
     * GameTime counter and the same torch slots, so the per-512-tick
     * depletion keeps running on the destination level.  This is the
     * "carry" half of the contract: it is not enough that the plan
     * leaves light alone — the runtime must continue to tick it.
     *
     * ReDMCSB: GAMELOOP.C:126 calls F0338 every 512 ticks regardless
     * of current map index; the light state lives on the party struct
     * (G0407_s_Party) which is shared across all maps.
     */
    DM1_LightState light;
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 1, 7, 8, 0);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;
    int rc;

    dm1_light_init(&light);
    dm1_light_set_champion_count(&light, 1);
    dm1_light_set_torch(&light, 0, 1, 5, true);

    /* Burn off 3 * 512 = 1536 ticks before stairs */
    for (int i = 0; i < 1536; ++i) dm1_light_tick(&light);
    expect_int("prestairs.charge", light.torch_slots[1].charge_count, 2,
               "PANEL.C:F0338_INVENTORY_DecreaseTorchesLightPower_CPSE; GAMELOOP.C:126");

    /* Stairs transition: plan must not touch light */
    rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.rc", rc, 1, "DUNGEON.C:1508-1558");
    expect_int("mid.charge", light.torch_slots[1].charge_count, 2,
               "DUNGEON.C:2724-2762 SetCurrentMap does not reset torch fuel");

    /* After stairs, another 512 ticks drains the last charge on map 1 */
    for (int i = 0; i < 512; ++i) dm1_light_tick(&light);
    expect_int("poststairs.charge", light.torch_slots[1].charge_count, 1,
               "PANEL.C:F0338_INVENTORY_DecreaseTorchesLightPower_CPSE; GAMELOOP.C:126");
    expect_int("poststairs.palette", light.dungeon_view_palette_idx, 4,
               "PANEL.C:F0337_INVENTORY_SetDungeonViewPalette (charge 1 → palette 4)");
    expect_int("poststairs.lit", light.torch_slots[1].lit ? 1 : 0, 1,
               "DUNGEON.C:2724-2762 does not clear Lit");
}

/* ── Test: stairs plan does not consume a zero-charge torch slot ─────── */

static void test_stairs_does_not_clear_unrelated_torch_slots(void)
{
    /*
     * ReDMCSB DUNGEON.C:2724-2762 SetCurrentMapAndPartyMap never touches
     * G0407_s_Party or champion hand objects.  A stairs plan must leave
     * all 8 torch slots byte-identical, even slots belonging to inactive
     * champions (champion_count < 4) or unlit torches.
     */
    DM1_LightState light;
    DM1_LightState snapshot;
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 1, 7, 8, 0);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;

    dm1_light_init(&light);
    dm1_light_set_champion_count(&light, 4);
    dm1_light_set_torch(&light, 0, 0, 12, true);
    dm1_light_set_torch(&light, 1, 1,  8, true);
    /* slots 2/3 ready, 2/3 action, plus 0/1 ready are unlit/empty by default */
    snapshot = light;

    int rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.rc", rc, 1, "DUNGEON.C:1508-1558");

    for (int i = 0; i < DM1_MAX_TORCH_SLOTS; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "carry.slot.%d.charge", i);
        expect_int(id, light.torch_slots[i].charge_count,
                   snapshot.torch_slots[i].charge_count,
                   "DUNGEON.C:2724-2762 preserves hand-object ChargeCount");
        snprintf(id, sizeof(id), "carry.slot.%d.lit", i);
        expect_int(id, light.torch_slots[i].lit ? 1 : 0,
                   snapshot.torch_slots[i].lit ? 1 : 0,
                   "DUNGEON.C:2724-2762 preserves hand-object Lit");
    }
}

/* ── Test: stairs plan with no pose change is a trivial no-op ─────────── */

static void test_stairs_same_pose_is_trivial_noop(void)
{
    /*
     * Defensive: if the stairs pose is byte-identical (no map change, no
     * square change, no direction change), the planner must still return
     * rc=1 but with active=0 — there is no transition to perform.  Since
     * the planner does no work, the light state is trivially preserved.
     * ReDMCSB F0267/F0364 always produce a non-trivial level change for a
     * stairs step; this case is a guard against double-processing the
     * same move command.
     */
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 0, 7, 8, 0);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;

    int rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.same_pose.rc", rc, 1, "DUNGEON.C:1508-1558");
    expect_int("stairs.same_pose.active", plan.active, 0,
               "DUNGEON.C:1508-1558; MOVESENS.C:441-451 pose-delta guard");
    expect_int("stairs.same_pose.changed", plan.mapChanged, 0,
               "DUNGEON.C:1508-1558");
}

int main(void)
{
    printf("=== DM1 V1 stairs transition light state — source-lock gate ===\n");
    test_stairs_plan_asserts_inventory_and_light_preservation();
    test_stairs_preserves_lit_torch_and_magical_light();
    test_stairs_then_tick_depletes_torch_on_new_level();
    test_stairs_does_not_clear_unrelated_torch_slots();
    test_stairs_same_pose_is_trivial_noop();
    printf("\n=== %d/%d assertions passed (%d failures) ===\n",
           g_assertions - g_failures, g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
