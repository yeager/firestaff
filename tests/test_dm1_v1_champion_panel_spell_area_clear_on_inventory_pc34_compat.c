#include "firestaff/dm1/v1/champion_panel/spell_area_clear_on_inventory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * DM1 V1 champion-panel spell-area clear-on-inventory test.
 *
 * Pinned contracts (all anchored to the ReDMCSB WIP20210206 PC 3.4 path):
 * - PANEL.C:2244-2440 F0355_INVENTORY_Toggle_CPSE full open/close cycle.
 * - PANEL.C:2370-2440 F0355 open path: G0423 set + C017 blit + chrome
 *   labels. F0394 is NEVER called on the open path.
 * - PANEL.C:2310-2335 F0355 close path: F0334 + F0395 + F0357 + F0098.
 *   F0394 is NEVER called on the close path.
 * - DEFS.H:504 G0514_i_MagicCasterChampionIndex byte-stable across the
 *   open/close cycle.
 * - The caster's SymbolStep + Symbols[0..3] rune buffer are byte-stable
 *   across the open/close cycle.
 * - The F0077/F0078 mouse screen-update bracketing is balanced across
 *   the open/close cycle.
 * - Negative inputs (dead inventory champion, pressing mouth/eye, no
 *   inventory session) are rejected with no state change.
 */

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d expected=%d anchor=%s\n",
                label, actual, expected, anchor);
    }
}

static void check_string(const char *label, const char *actual,
                         const char *expected, const char *anchor)
{
    ++g_assertions;
    if (!actual || !expected || strcmp(actual, expected) != 0) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got='%s' expected='%s' anchor=%s\n",
                label,
                actual ? actual : "(null)",
                expected ? expected : "(null)",
                anchor);
    }
}

static void reset_champions(DM1_V1_CpsaciChampionPc34 *champions)
{
    int i;
    memset(champions, 0, sizeof(champions[0]) * DM1_V1_CPSACI_CHAMPION_COUNT_PC34);
    for (i = 0; i < DM1_V1_CPSACI_CHAMPION_COUNT_PC34; ++i) {
        champions[i].index = i;
        champions[i].current_health = 100;
        champions[i].symbol_step = 2;
        champions[i].symbols[0] = '\0';
    }
}

static void type_runes(DM1_V1_CpsaciChampionPc34 *champion, const char *s)
{
    size_t n = strlen(s);
    if (n > DM1_V1_CPSACI_CHAMPION_SYMBOL_MAX_PC34) {
        n = DM1_V1_CPSACI_CHAMPION_SYMBOL_MAX_PC34;
    }
    memcpy(champion->symbols, s, n);
    champion->symbols[n] = '\0';
}

static void test_contract(void)
{
    const DM1_V1_CpsaciContractPc34 *c;
    const char *evidence;

    c = dm1_v1_champion_panel_spell_area_clear_on_inventory_contract_pc34();
    evidence =
        dm1_v1_champion_panel_spell_area_clear_on_inventory_source_evidence_pc34();

    check_int("contract.only", c->contract_only, 1,
              "contract-only no game data");
    check_int("contract.champions", c->champion_count, 4,
              "DEFS.H C100..C109 4-champion party");
    check_int("contract.symMax", c->symbol_max, 4,
              "MENUDRAW.C:96 max 4 champion symbols");
    check_int("contract.championNone", c->champion_none, -1,
              "DEFS.H CM1_CHAMPION_NONE sentinel");
    check_int("contract.c04Close", c->c04_close_inventory, 4,
              "DEFS.H C04_CHAMPION_CLOSE_INVENTORY");
    check_int("contract.c05Special", c->c05_special_inventory, 5,
              "DEFS.H C05_CHAMPION_SPECIAL_INVENTORY");
    check_int("contract.spellX0", c->spell_area_x0, 224,
              "DATA.C:119 spell-area screen box");
    check_int("contract.spellX1", c->spell_area_x1, 319,
              "DATA.C:119 spell-area screen box");
    check_int("contract.spellY0", c->spell_area_y0, 42,
              "DATA.C:119 spell-area screen box");
    check_int("contract.spellY1", c->spell_area_y1, 74,
              "DATA.C:119 spell-area screen box");
    check_int("contract.c017", c->c017_graphic_inventory, 17,
              "DEFS.H C017_GRAPHIC_INVENTORY");
    check_string("contract.toggle", c->inventory_toggle_anchor,
                 "PANEL.C:2244-2440 F0355_INVENTORY_Toggle_CPSE full open/close cycle",
                 "PANEL.C F0355 anchor");
    check_string("contract.closeBranch", c->close_branch_anchor,
                 "PANEL.C:2310-2335 F0355 close-inventory branch (F0334 + F0395 + F0357 + F0098)",
                 "PANEL.C F0355 close anchor");
    check_string("contract.openBranch", c->open_branch_anchor,
                 "PANEL.C:2357-2440 F0355 open-inventory branch "
                 "(G0423 set + C017 GRAPHIC_INVENTORY blit + chrome labels)",
                 "PANEL.C F0355 open anchor");
    check_string("contract.caster", c->caster_anchor,
                 "DEFS.H:504 G0514_i_MagicCasterChampionIndex; "
                 "CASTER.C:18-32 F0394 short-circuit and CM1_CHAMPION_NONE clear path",
                 "DEFS.H G0514 / CASTER.C F0394 anchor");
    check_string("contract.mouse", c->mouse_bracketing_anchor,
                 "PANEL.C:2270-2308 F0077_MOUSE_EnableScreenUpdate_CPSE / F0078 disable "
                 "screen update bracketing on open/close",
                 "PANEL.C F0077/F0078 anchor");
    check_string("contract.ordinals", c->ordinal_helpers_anchor,
                 "DEFS.H:8200 M000_INDEX_TO_ORDINAL / M001_ORDINAL_TO_INDEX ordinal helpers",
                 "DEFS.H M000/M001 anchor");
    check_string("contract.deadReject", c->dead_champion_reject_anchor,
                 "PANEL.C:2280-2285 F0355 dead-inventory-champion early return",
                 "PANEL.C dead-champion reject anchor");
    check_string("contract.mouthEyeReject", c->mouth_eye_reject_anchor,
                 "PANEL.C:2290-2295 F0355 G0333_B_PressingMouth || G0331_B_PressingEye reject",
                 "PANEL.C mouth/eye reject anchor");
    check_int("evidence.notNull", evidence != NULL ? 1 : 0, 1,
              "source_evidence_pc34 returns non-null");
}

/* Open-inventory with a live caster at index 0 must leave the caster
 * state machine byte-stable and must NOT call F0394. */
static void test_open_with_live_caster(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 0;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_with_live_caster returned 0\n");
        return;
    }
    check_int("open_live.accepted", out.accepted, 1,
              "PANEL.C F0355 open branch taken");
    check_int("open_live.reason", out.reject_reason,
              DM1_V1_CPSACI_REJECT_NONE_PC34, "no reject reason");
    check_int("open_live.f0394", out.fired_f0394_set_magic_caster, 0,
              "F0355 open path never calls F0394");
    check_int("open_live.f0334", out.fired_f0334_close_chest, 0,
              "no previous inventory so F0334 is not called");
    check_int("open_live.g0423", out.new_inventory_champion_ordinal, 1,
              "G0423 ordinal = M000_INDEX_TO_ORDINAL(0) = 1");
    check_int("open_live.g0514", out.new_magic_caster_champion_index, 0,
              "G0514 unchanged on open path");
    check_int("open_live.c017", out.loaded_c017_graphic_inventory_into_g0296,
              1, "C017_GRAPHIC_INVENTORY blit fires on open");
    check_int("open_live.floppy", out.hid_floppy_icon, 1,
              "hide floppy icon on open");
    check_int("open_live.labels", out.drew_health_stamina_mana_labels, 1,
              "draw health/stamina/mana labels on open");
    check_int("open_live.f0077", out.fired_f0077_enable_screen_update, 1,
              "F0077 enable screen update on open");
    check_int("open_live.f0078", out.fired_f0078_disable_screen_update, 1,
              "F0078 disable screen update at end of open path");
    check_int("open_live.mouseBal", out.mouse_update_balance, 0,
              "open path balances F0077 with F0078");
    check_int("open_live.hatchArrows",
              out.fired_f0136_hatch_movement_arrows_box, 1,
              "hatch movement arrows box on fresh open");
    check_int("open_live.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols[0..3] byte-stable on open");
    check_int("open_live.stepStable", out.caster_symbol_step_byte_stable, 1,
              "caster SymbolStep byte-stable on open");
    check_int("open_live.partyStable", out.symbols_buffer_byte_stable, 1,
              "all 4 champion symbol buffers byte-stable on open");
    check_string("open_live.runes", in.champions[0].symbols, "ABCD",
                 "caster Symbols[0..3] buffer is unchanged");
}

/* Open-inventory with a dead inventory champion must be rejected. */
static void test_open_dead_champion_reject(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 1;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    in.champions[1].current_health = 0;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_dead_champion returned 0\n");
        return;
    }
    check_int("open_dead.accepted", out.accepted, 0,
              "dead inventory champion reject");
    check_int("open_dead.reason", out.reject_reason,
              DM1_V1_CPSACI_REJECT_DEAD_INVENTORY_CHAMPION_PC34,
              "PANEL.C:2280-2285 dead-champion reject");
    check_int("open_dead.f0077", out.fired_f0077_enable_screen_update, 0,
              "F0077 not called on reject");
    check_int("open_dead.f0078", out.fired_f0078_disable_screen_update, 0,
              "F0078 not called on dead-champion reject");
    check_int("open_dead.f0334", out.fired_f0334_close_chest, 0,
              "F0334 not called on reject");
    check_int("open_dead.c017", out.loaded_c017_graphic_inventory_into_g0296,
              0, "C017 blit skipped on reject");
    check_int("open_dead.g0423", out.new_inventory_champion_ordinal, 0,
              "G0423 unchanged on reject");
    check_int("open_dead.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols[0..3] byte-stable on reject");
    check_int("open_dead.stepStable", out.caster_symbol_step_byte_stable, 1,
              "caster SymbolStep byte-stable on reject");
    check_int("open_dead.partyStable", out.symbols_buffer_byte_stable, 1,
              "all 4 champion symbol buffers byte-stable on reject");
}

/* Open-inventory while pressing mouth must be rejected. */
static void test_open_pressing_mouth_reject(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 2;
    in.pressing_mouth = 1;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_pressing_mouth returned 0\n");
        return;
    }
    check_int("open_mouth.accepted", out.accepted, 0,
              "pressing mouth reject");
    check_int("open_mouth.reason", out.reject_reason,
              DM1_V1_CPSACI_REJECT_PRESSING_MOUTH_OR_EYE_PC34,
              "PANEL.C:2290-2295 mouth/eye reject");
    check_int("open_mouth.c017", out.loaded_c017_graphic_inventory_into_g0296,
              0, "C017 blit skipped on mouth reject");
    check_int("open_mouth.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on mouth reject");
}

/* Open-inventory while pressing eye must be rejected. */
static void test_open_pressing_eye_reject(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 3;
    in.pressing_mouth = 0;
    in.pressing_eye = 1;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_pressing_eye returned 0\n");
        return;
    }
    check_int("open_eye.accepted", out.accepted, 0,
              "pressing eye reject");
    check_int("open_eye.reason", out.reject_reason,
              DM1_V1_CPSACI_REJECT_PRESSING_MOUTH_OR_EYE_PC34,
              "PANEL.C:2290-2295 mouth/eye reject");
    check_int("open_eye.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on eye reject");
}

/* C05 special-inventory on a no-session state must be rejected. */
static void test_open_c05_no_session_reject(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index =
        DM1_V1_CPSACI_C05_SPECIAL_INVENTORY_PC34;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_c05_no_session returned 0\n");
        return;
    }
    check_int("open_c05_ns.accepted", out.accepted, 0,
              "C05 with no previous inventory is rejected");
    check_int("open_c05_ns.reason", out.reject_reason,
              DM1_V1_CPSACI_REJECT_NO_INVENTORY_SESSION_PC34,
              "no inventory session reject");
    check_int("open_c05_ns.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on no-session reject");
}

/* Open-inventory with a CM1_CHAMPION_NONE caster must not mutate G0514. */
static void test_open_with_no_caster(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 0;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index =
        DM1_V1_CPSACI_CHAMPION_NONE_PC34;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_no_caster returned 0\n");
        return;
    }
    check_int("open_nc.accepted", out.accepted, 1,
              "open with no caster is still accepted");
    check_int("open_nc.g0514", out.new_magic_caster_champion_index,
              DM1_V1_CPSACI_CHAMPION_NONE_PC34,
              "G0514 stays at CM1_CHAMPION_NONE");
    check_int("open_nc.f0394", out.fired_f0394_set_magic_caster, 0,
              "F0394 not called on open with no caster");
    check_int("open_nc.c017", out.loaded_c017_graphic_inventory_into_g0296,
              1, "C017 blit still fires on open with no caster");
}

/* Open-inventory when an inventory session is already active must
 * close the previous session (F0334) then open the new one. */
static void test_open_while_inventory_active(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 1;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 1; /* champion 0 open */
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_active returned 0\n");
        return;
    }
    check_int("open_active.accepted", out.accepted, 1,
              "open with active inventory accepted");
    check_int("open_active.f0334", out.fired_f0334_close_chest, 1,
              "F0334 close chest on prior inventory");
    check_int("open_active.f0098", out.fired_f0098_draw_floor_and_ceiling,
              1, "F0098 redraw floor+ceiling on prior close");
    check_int("open_active.g0423", out.new_inventory_champion_ordinal, 2,
              "G0423 ordinal = M000_INDEX_TO_ORDINAL(1) = 2");
    check_int("open_active.c017", out.loaded_c017_graphic_inventory_into_g0296,
              1, "C017 blit on new open");
    check_int("open_active.g0514", out.new_magic_caster_champion_index, 0,
              "G0514 byte-stable across prior-close + new-open");
    check_int("open_active.f0394", out.fired_f0394_set_magic_caster, 0,
              "F0394 never called by F0355");
    check_int("open_active.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols[0..3] byte-stable across prior-close + new-open");
}

/* Close-inventory with a live caster must leave the caster state
 * machine byte-stable. */
static void test_close_with_live_caster(void)
{
    DM1_V1_CpsaciCloseInputPc34 in;
    DM1_V1_CpsaciCloseOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.previous_inventory_champion_ordinal = 1; /* champion 0 open */
    in.magic_caster_champion_index = 0;
    in.mouse_update_already_open = 1;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL close_live returned 0\n");
        return;
    }
    check_int("close_live.accepted", out.accepted, 1,
              "close-inventory accepted");
    check_int("close_live.f0394", out.fired_f0394_set_magic_caster, 0,
              "F0355 close path never calls F0394");
    check_int("close_live.f0334", out.fired_f0334_close_chest, 1,
              "F0334 close chest on inventory exit");
    check_int("close_live.f0395", out.fired_f0395_draw_movement_arrows, 1,
              "F0395 redraw movement arrows on close");
    check_int("close_live.f0357", out.fired_f0357_discard_all_input, 1,
              "F0357 discard all input on close");
    check_int("close_live.f0098", out.fired_f0098_draw_floor_and_ceiling, 1,
              "F0098 redraw floor+ceiling on close");
    check_int("close_live.f0077", out.fired_f0077_enable_screen_update, 1,
              "F0077 enable screen update on close");
    check_int("close_live.f0078", out.fired_f0078_disable_screen_update, 1,
              "F0078 disable screen update on close");
    check_int("close_live.mouseBal", out.mouse_update_balance, 0,
              "close balances F0077 with F0078");
    check_int("close_live.g0423", out.new_inventory_champion_ordinal, 0,
              "G0423 ordinal = NONE on close");
    check_int("close_live.g0514", out.new_magic_caster_champion_index, 0,
              "G0514 byte-stable on close");
    check_int("close_live.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on close");
    check_int("close_live.stepStable", out.caster_symbol_step_byte_stable, 1,
              "caster SymbolStep byte-stable on close");
}

/* Close-inventory with no prior session must be rejected. */
static void test_close_no_session_reject(void)
{
    DM1_V1_CpsaciCloseInputPc34 in;
    DM1_V1_CpsaciCloseOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.mouse_update_already_open = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL close_no_session returned 0\n");
        return;
    }
    check_int("close_ns.accepted", out.accepted, 0,
              "close with no session rejected");
    check_int("close_ns.reason", out.reject_reason,
              DM1_V1_CPSACI_REJECT_NO_INVENTORY_SESSION_PC34,
              "no inventory session reject");
    check_int("close_ns.f0334", out.fired_f0334_close_chest, 0,
              "F0334 not called on no-session close");
    check_int("close_ns.f0395", out.fired_f0395_draw_movement_arrows, 0,
              "F0395 not called on no-session close");
    check_int("close_ns.f0357", out.fired_f0357_discard_all_input, 0,
              "F0357 not called on no-session close");
    check_int("close_ns.f0098", out.fired_f0098_draw_floor_and_ceiling, 0,
              "F0098 not called on no-session close");
    check_int("close_ns.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on no-session close");
}

/* Open + close round trip: full cycle, byte-stability of caster state
 * and mouse-update balance must hold. */
static void test_round_trip_basic(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    int post_close_ordinal;
    int post_close_caster;
    int symbols_stable;
    int step_stable;
    int mouse_balanced;
    int f0394_calls;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 0;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");
    in.champions[0].symbol_step = 3;

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
             &in, &post_close_ordinal, &post_close_caster, &symbols_stable,
             &step_stable, &mouse_balanced, &f0394_calls)) {
        ++g_failures;
        fprintf(stderr, "FAIL round_trip_basic returned 0\n");
        return;
    }
    check_int("rt_basic.ordinal", post_close_ordinal, 0,
              "G0423 ordinal NONE after round trip");
    check_int("rt_basic.caster", post_close_caster, 0,
              "G0514 byte-stable after round trip");
    check_int("rt_basic.symStable", symbols_stable, 1,
              "all 4 champion symbol buffers byte-stable after round trip");
    check_int("rt_basic.stepStable", step_stable, 1,
              "caster SymbolStep byte-stable after round trip");
    check_int("rt_basic.mouseBalanced", mouse_balanced, 1,
              "F0077/F0078 mouse-update bracketing balanced after round trip");
    check_int("rt_basic.f0394", f0394_calls, 0,
              "F0394 never called across the round trip");
    check_string("rt_basic.runes", in.champions[0].symbols, "ABCD",
                 "caster Symbols buffer survived the round trip");
    check_int("rt_basic.runesLen", (int)strlen(in.champions[0].symbols), 4,
              "caster Symbols buffer is 4 runes after round trip");
}

/* Round trip while a different champion is the live caster. */
static void test_round_trip_other_caster(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    int post_close_ordinal;
    int post_close_caster;
    int symbols_stable;
    int step_stable;
    int mouse_balanced;
    int f0394_calls;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 2; /* open inventory for champion 2 */
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 1; /* champion 1 is the caster */
    in.party_champion_count = 4;
    type_runes(&in.champions[1], "WXYZ");
    in.champions[1].symbol_step = 5;

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
             &in, &post_close_ordinal, &post_close_caster, &symbols_stable,
             &step_stable, &mouse_balanced, &f0394_calls)) {
        ++g_failures;
        fprintf(stderr, "FAIL round_trip_other_caster returned 0\n");
        return;
    }
    check_int("rt_other.ordinal", post_close_ordinal, 0,
              "G0423 NONE after round trip");
    check_int("rt_other.caster", post_close_caster, 1,
              "G0514 still at champion 1 after round trip");
    check_int("rt_other.symStable", symbols_stable, 1,
              "all 4 champion symbol buffers byte-stable after round trip");
    check_int("rt_other.stepStable", step_stable, 1,
              "caster SymbolStep byte-stable after round trip");
    check_int("rt_other.mouseBalanced", mouse_balanced, 1,
              "F0077/F0078 balanced after round trip");
    check_int("rt_other.f0394", f0394_calls, 0,
              "F0394 never called across the round trip");
    check_string("rt_other.runes", in.champions[1].symbols, "WXYZ",
                 "caster Symbols buffer survived the round trip");
    check_int("rt_other.runesLen", (int)strlen(in.champions[1].symbols), 4,
              "caster Symbols buffer is 4 runes after round trip");
    check_int("rt_other.stepVal", (int)in.champions[1].symbol_step, 5,
              "caster SymbolStep is still 5 after round trip");
}

/* Round trip with no live caster (CM1_CHAMPION_NONE). */
static void test_round_trip_no_caster(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    int post_close_ordinal;
    int post_close_caster;
    int symbols_stable;
    int step_stable;
    int mouse_balanced;
    int f0394_calls;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 0;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index =
        DM1_V1_CPSACI_CHAMPION_NONE_PC34;
    in.party_champion_count = 4;

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
             &in, &post_close_ordinal, &post_close_caster, &symbols_stable,
             &step_stable, &mouse_balanced, &f0394_calls)) {
        ++g_failures;
        fprintf(stderr, "FAIL round_trip_no_caster returned 0\n");
        return;
    }
    check_int("rt_nc.ordinal", post_close_ordinal, 0,
              "G0423 NONE after round trip");
    check_int("rt_nc.caster", post_close_caster,
              DM1_V1_CPSACI_CHAMPION_NONE_PC34,
              "G0514 still at CM1_CHAMPION_NONE after round trip");
    check_int("rt_nc.symStable", symbols_stable, 1,
              "all 4 champion symbol buffers byte-stable after round trip");
    check_int("rt_nc.stepStable", step_stable, 1,
              "step byte-stable (no caster)");
    check_int("rt_nc.mouseBalanced", mouse_balanced, 1,
              "F0077/F0078 balanced after round trip");
    check_int("rt_nc.f0394", f0394_calls, 0,
              "F0394 never called across the round trip");
}

/* Round trip that crosses a prior-inventory close. The prior
 * inventory is closed by F0334, the new one is opened with C017. The
 * caster state machine must be byte-stable through both halves. */
static void test_round_trip_with_prior_inventory(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    int post_close_ordinal;
    int post_close_caster;
    int symbols_stable;
    int step_stable;
    int mouse_balanced;
    int f0394_calls;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 2;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 1; /* champion 0 open */
    in.magic_caster_champion_index = 1; /* champion 1 is the caster */
    in.party_champion_count = 4;
    type_runes(&in.champions[1], "LMNO");
    in.champions[1].symbol_step = 7;

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
             &in, &post_close_ordinal, &post_close_caster, &symbols_stable,
             &step_stable, &mouse_balanced, &f0394_calls)) {
        ++g_failures;
        fprintf(stderr, "FAIL round_trip_prior returned 0\n");
        return;
    }
    check_int("rt_prior.ordinal", post_close_ordinal, 0,
              "G0423 NONE after round trip");
    check_int("rt_prior.caster", post_close_caster, 1,
              "G0514 still at champion 1 after round trip");
    check_int("rt_prior.symStable", symbols_stable, 1,
              "all 4 champion symbol buffers byte-stable after round trip");
    check_int("rt_prior.stepStable", step_stable, 1,
              "caster SymbolStep byte-stable after round trip");
    check_int("rt_prior.mouseBalanced", mouse_balanced, 1,
              "F0077/F0078 balanced after round trip");
    check_int("rt_prior.f0394", f0394_calls, 0,
              "F0394 never called across the round trip");
    check_string("rt_prior.runes", in.champions[1].symbols, "LMNO",
                 "caster Symbols buffer survived the round trip");
    check_int("rt_prior.stepVal", (int)in.champions[1].symbol_step, 7,
              "caster SymbolStep is still 7 after round trip");
}

/* Open-inventory with an empty rune buffer (no caster typed anything)
 * must leave the empty buffer byte-stable. */
static void test_open_empty_symbols(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 1;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    in.champions[0].symbols[0] = '\0';

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_empty returned 0\n");
        return;
    }
    check_int("open_empty.accepted", out.accepted, 1,
              "open with empty symbol buffer accepted");
    check_int("open_empty.symStable", out.caster_symbols_byte_stable, 1,
              "empty Symbols[0..3] buffer byte-stable on open");
    check_int("open_empty.len", (int)strlen(in.champions[0].symbols), 0,
              "empty Symbols[0..3] buffer is still empty");
}

/* Open + close in the same call path: when the requested champion
 * matches the previous inventory champion, F0355 collapses the call
 * to a close. The caster state must still be byte-stable. */
static void test_open_collapses_to_close(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    DM1_V1_CpsaciOpenOutputPc34 out;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 0;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 1; /* champion 0 open */
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "PQRS");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             &in, &out)) {
        ++g_failures;
        fprintf(stderr, "FAIL open_collapse returned 0\n");
        return;
    }
    check_int("open_col.accepted", out.accepted, 1,
              "same-champion open collapses to close");
    check_int("open_col.g0423", out.new_inventory_champion_ordinal, 0,
              "G0423 ordinal NONE after collapsed close");
    check_int("open_col.g0514", out.new_magic_caster_champion_index, 0,
              "G0514 byte-stable after collapsed close");
    check_int("open_col.f0334", out.fired_f0334_close_chest, 1,
              "F0334 close chest on collapsed close");
    check_int("open_col.f0394", out.fired_f0394_set_magic_caster, 0,
              "F0394 not called on collapsed close");
    check_int("open_col.c017", out.loaded_c017_graphic_inventory_into_g0296,
              0, "C017 blit skipped on collapsed close");
    check_int("open_col.symStable", out.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on collapsed close");
}

/* Round trip with a 3-rune caster buffer (not all 4 slots used). */
static void test_round_trip_short_runes(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    int post_close_ordinal;
    int post_close_caster;
    int symbols_stable;
    int step_stable;
    int mouse_balanced;
    int f0394_calls;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 1;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 0;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "AAA");
    in.champions[0].symbol_step = 1;

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
             &in, &post_close_ordinal, &post_close_caster, &symbols_stable,
             &step_stable, &mouse_balanced, &f0394_calls)) {
        ++g_failures;
        fprintf(stderr, "FAIL round_trip_short returned 0\n");
        return;
    }
    check_int("rt_short.ordinal", post_close_ordinal, 0,
              "G0423 NONE after round trip");
    check_int("rt_short.caster", post_close_caster, 0,
              "G0514 byte-stable after round trip");
    check_int("rt_short.symStable", symbols_stable, 1,
              "all 4 champion symbol buffers byte-stable after round trip");
    check_int("rt_short.stepStable", step_stable, 1,
              "caster SymbolStep byte-stable after round trip");
    check_int("rt_short.mouseBalanced", mouse_balanced, 1,
              "F0077/F0078 balanced after round trip");
    check_int("rt_short.f0394", f0394_calls, 0,
              "F0394 never called across the round trip");
    check_int("rt_short.len", (int)strlen(in.champions[0].symbols), 3,
              "caster Symbols buffer is 3 runes after round trip");
}

/* Round trip with 4 different typed rune contents and varying
 * SymbolStep values for every champion. */
static void test_round_trip_diverse(void)
{
    DM1_V1_CpsaciOpenInputPc34 in;
    int post_close_ordinal;
    int post_close_caster;
    int symbols_stable;
    int step_stable;
    int mouse_balanced;
    int f0394_calls;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.requested_champion_index = 3;
    in.pressing_mouth = 0;
    in.pressing_eye = 0;
    in.mouse_update_already_open = 0;
    in.previous_inventory_champion_ordinal = 0;
    in.magic_caster_champion_index = 2;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ABCD");
    type_runes(&in.champions[1], "EFGH");
    type_runes(&in.champions[2], "IJKL");
    type_runes(&in.champions[3], "MNOP");
    in.champions[0].symbol_step = 0;
    in.champions[1].symbol_step = 1;
    in.champions[2].symbol_step = 2;
    in.champions[3].symbol_step = 3;

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
             &in, &post_close_ordinal, &post_close_caster, &symbols_stable,
             &step_stable, &mouse_balanced, &f0394_calls)) {
        ++g_failures;
        fprintf(stderr, "FAIL round_trip_diverse returned 0\n");
        return;
    }
    check_int("rt_div.ordinal", post_close_ordinal, 0,
              "G0423 NONE after round trip");
    check_int("rt_div.caster", post_close_caster, 2,
              "G0514 still at champion 2 after round trip");
    check_int("rt_div.symStable", symbols_stable, 1,
              "all 4 champion symbol buffers byte-stable after round trip");
    check_int("rt_div.stepStable", step_stable, 1,
              "caster SymbolStep byte-stable after round trip");
    check_int("rt_div.mouseBalanced", mouse_balanced, 1,
              "F0077/F0078 balanced after round trip");
    check_int("rt_div.f0394", f0394_calls, 0,
              "F0394 never called across the round trip");
    check_string("rt_div.runes0", in.champions[0].symbols, "ABCD",
                 "champion 0 runes survived");
    check_string("rt_div.runes1", in.champions[1].symbols, "EFGH",
                 "champion 1 runes survived");
    check_string("rt_div.runes2", in.champions[2].symbols, "IJKL",
                 "champion 2 (caster) runes survived");
    check_string("rt_div.runes3", in.champions[3].symbols, "MNOP",
                 "champion 3 runes survived");
    check_int("rt_div.step0", (int)in.champions[0].symbol_step, 0,
              "champion 0 step survived");
    check_int("rt_div.step1", (int)in.champions[1].symbol_step, 1,
              "champion 1 step survived");
    check_int("rt_div.step2", (int)in.champions[2].symbol_step, 2,
              "champion 2 (caster) step survived");
    check_int("rt_div.step3", (int)in.champions[3].symbol_step, 3,
              "champion 3 step survived");
}

/* Close-inventory called twice in a row: the second call must be
 * rejected (no prior session). */
static void test_close_twice_rejects(void)
{
    DM1_V1_CpsaciCloseInputPc34 in;
    DM1_V1_CpsaciCloseOutputPc34 out1;
    DM1_V1_CpsaciCloseOutputPc34 out2;

    memset(&in, 0, sizeof(in));
    reset_champions(in.champions);
    in.previous_inventory_champion_ordinal = 1;
    in.magic_caster_champion_index = 0;
    in.mouse_update_already_open = 1;
    in.party_champion_count = 4;
    type_runes(&in.champions[0], "ZZZZ");

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
             &in, &out1)) {
        ++g_failures;
        fprintf(stderr, "FAIL close_twice 1 returned 0\n");
        return;
    }
    check_int("close_twice.first", out1.accepted, 1,
              "first close accepted");

    /* Second close on a no-session state. */
    in.previous_inventory_champion_ordinal = 0;
    in.mouse_update_already_open = 0;
    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
             &in, &out2)) {
        ++g_failures;
        fprintf(stderr, "FAIL close_twice 2 returned 0\n");
        return;
    }
    check_int("close_twice.second", out2.accepted, 0,
              "second close rejected (no session)");
    check_int("close_twice.f0334_2", out2.fired_f0334_close_chest, 0,
              "F0334 not called on second close");
    check_int("close_twice.f0394_2", out2.fired_f0394_set_magic_caster, 0,
              "F0394 not called on second close");
    check_int("close_twice.symStable", out2.caster_symbols_byte_stable, 1,
              "caster Symbols byte-stable on second close reject");
    check_string("close_twice.runes", in.champions[0].symbols, "ZZZZ",
                 "caster Symbols buffer survived both closes");
}

int main(void)
{
    test_contract();
    test_open_with_live_caster();
    test_open_dead_champion_reject();
    test_open_pressing_mouth_reject();
    test_open_pressing_eye_reject();
    test_open_c05_no_session_reject();
    test_open_with_no_caster();
    test_open_while_inventory_active();
    test_open_empty_symbols();
    test_open_collapses_to_close();
    test_close_with_live_caster();
    test_close_no_session_reject();
    test_close_twice_rejects();
    test_round_trip_basic();
    test_round_trip_other_caster();
    test_round_trip_no_caster();
    test_round_trip_with_prior_inventory();
    test_round_trip_short_runes();
    test_round_trip_diverse();

    if (g_failures != 0) {
        fprintf(stderr, "== test_dm1_v1_champion_panel_spell_area_clear_on_"
                "inventory_pc34_compat FAILED ==\n");
        fprintf(stderr, "== assertions=%d failures=%d ==\n",
                g_assertions, g_failures);
        return 1;
    }
    printf("== DM1 V1 champion panel spell-area clear-on-inventory slice ==\n");
    printf("PASS test_dm1_v1_champion_panel_spell_area_clear_on_inventory_"
           "pc34_compat assertions=%d\n", g_assertions);
    return 0;
}
