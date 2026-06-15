#include "../src/csb/csb_v1_chaos_cast_targeting_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK_REDMCSB(cond, anchor, msg) do { \
    if (cond) { passed++; printf("  PASS: %s [%s]\n", msg, anchor); } \
    else { failed++; printf("  FAIL: %s [%s]\n", msg, anchor); } \
} while (0)

static int line_offset(int y, int byte_x)
{
    return (y * CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH) + byte_x;
}

static void seed_party(CSB_V1_ChaosCastTargetingState *state)
{
    csb_v1_chaos_cast_targeting_init(state);
    csb_v1_chaos_cast_targeting_set_champion(state, 0, 100, 0, "ABCD", "Halk");
    csb_v1_chaos_cast_targeting_set_champion(state, 1, 100, 1, "EFGH", "Syra");
    csb_v1_chaos_cast_targeting_set_champion(state, 2, 100, 2, "IJKL", "Wuuf");
    csb_v1_chaos_cast_targeting_set_champion(state, 3, 100, 3, "MNOP", "Tiggy");
}

static void test_magic_caster_selection_and_line_build(void)
{
    CSB_V1_ChaosCastTargetingState state;
    int c2_symbol = line_offset(8, 2);
    int c3_symbol = line_offset(8, 8);

    seed_party(&state);

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(
            &state, 0) == CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB CASTER.C F0394 line ~2",
        "initial active magic caster selection is accepted");
    CHECK_REDMCSB(state.magic_caster_champion_index == 0,
        "G0514_i_MagicCasterChampionIndex",
        "G0514 tracks champion 0");
    CHECK_REDMCSB(state.spell_area_line1_caster == 0,
        "ReDMCSB CASTER.C:35-36",
        "spell-area line 1 records the initial caster");
    CHECK_REDMCSB(state.last_controls_champion_index == 0,
        "ReDMCSB SPELDRAW.C F0393 lines 2-96",
        "spell-area controls are drawn for champion 0");
    CHECK_REDMCSB(state.line1_blits == 1,
        "M520_F0021_MAIN_BlitToScreen",
        "line 1 is blitted once for the first caster");
    CHECK_REDMCSB(state.last_blit_offset == CSB_V1_CHAOS_TARGET_LINE2_OFFSET,
        "G1072_ai_Box_SpellAreaLine2",
        "line 1 blit starts at the G1072 line 2 screen offset");
    CHECK_REDMCSB(state.last_blit_stride == CSB_V1_CHAOS_TARGET_C160_BYTE_WIDTH,
        "C160_BYTE_WIDTH_SCREEN",
        "line 1 blit records C160_BYTE_WIDTH_SCREEN stride");
    CHECK_REDMCSB(state.last_blit_width == CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH,
        "C048_BYTE_WIDTH",
        "line 1 blit records C048_BYTE_WIDTH source rows");
    CHECK_REDMCSB(state.last_blit_rows == 11,
        "ReDMCSB CASTER.C:49-50",
        "line 1 blit skips first and last bitmap rows");
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_count_nonzero_line1_screen_bytes(
            &state) > 0,
        "M520_F0021_MAIN_BlitToScreen",
        "screen receives non-zero spell-area line bytes");
    CHECK_REDMCSB(state.spell_area_line_kind ==
            CSB_V1_CHAOS_TARGET_C3_CHAMPION,
        "ReDMCSB CASTER.C:60",
        "F0394 leaves G0515 rebuilt as the champion symbol line");
    CHECK_REDMCSB(state.spell_area_line[c3_symbol] == 'A',
        "C3_SPELL_AREA_CHAMPION_SYMBOLS",
        "champion 0 C3 symbols are stamped into G0515");

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(
            &state, 1) == CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB CASTER.C F0394 line ~2",
        "selecting a different champion is accepted");
    CHECK_REDMCSB(state.magic_caster_champion_index == 1,
        "G0514_i_MagicCasterChampionIndex",
        "G0514 reroutes to champion 1");
    CHECK_REDMCSB(state.spell_area_line1_caster == 1,
        "ReDMCSB CASTER.C:35-36",
        "spell-area line 1 is rerouted to champion 1");
    CHECK_REDMCSB(state.last_controls_champion_index == 1,
        "ReDMCSB SPELDRAW.C F0393 lines 2-96",
        "spell-area controls are redrawn for champion 1");
    CHECK_REDMCSB(state.line1_blits == 2,
        "M520_F0021_MAIN_BlitToScreen",
        "champion reroute performs a second line 1 blit");
    CHECK_REDMCSB(state.screen[CSB_V1_CHAOS_TARGET_LINE2_OFFSET +
            (8 * CSB_V1_CHAOS_TARGET_C160_BYTE_WIDTH) + 2] == 'f',
        "C2_SPELL_AREA_AVAILABLE_SYMBOLS",
        "screen line 1 preserves champion 1 available symbols");
    CHECK_REDMCSB(state.spell_area_line[c3_symbol] == 'E',
        "C3_SPELL_AREA_CHAMPION_SYMBOLS",
        "G0515 ends with champion 1 C3 symbols");

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_build_spell_area_line(
            &state, CSB_V1_CHAOS_TARGET_C2_AVAILABLE) ==
            CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB MENU.C F0392 lines 856-875",
        "explicit C2 line build succeeds");
    CHECK_REDMCSB(state.spell_area_line_kind ==
            CSB_V1_CHAOS_TARGET_C2_AVAILABLE,
        "C2_SPELL_AREA_AVAILABLE_SYMBOLS",
        "G0515 records C2 available-symbol line kind");
    CHECK_REDMCSB(state.spell_area_line[0] ==
            CSB_V1_CHAOS_TARGET_C2_AVAILABLE,
        "G0515_aui_Bitmap_SpellAreaLine",
        "C2 build preserves the G0515 buffer head");
    CHECK_REDMCSB(state.spell_area_line[c2_symbol + 0] == 'f',
        "ReDMCSB MENU.C:865-870",
        "C2 symbol 0 derives from champion 1 SymbolStep");
    CHECK_REDMCSB(state.spell_area_line[c2_symbol + 5] == 'k',
        "ReDMCSB MENU.C:867-870",
        "C2 symbol 5 derives from champion 1 SymbolStep");

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_build_spell_area_line(
            &state, CSB_V1_CHAOS_TARGET_C3_CHAMPION) ==
            CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB MENU.C F0392 lines 877-895",
        "explicit C3 line build succeeds");
    CHECK_REDMCSB(state.spell_area_line_kind ==
            CSB_V1_CHAOS_TARGET_C3_CHAMPION,
        "C3_SPELL_AREA_CHAMPION_SYMBOLS",
        "G0515 records C3 champion-symbol line kind");
    CHECK_REDMCSB(state.spell_area_line[0] ==
            CSB_V1_CHAOS_TARGET_C3_CHAMPION,
        "G0515_aui_Bitmap_SpellAreaLine",
        "C3 build updates the same buffer head");
    CHECK_REDMCSB(state.spell_area_line[c3_symbol + 0] == 'E',
        "ReDMCSB MENU.C:887-895",
        "C3 symbol 0 copies champion 1 Symbols");
    CHECK_REDMCSB(state.spell_area_line[c3_symbol + 3] == 'H',
        "ReDMCSB MENU.C:887-895",
        "C3 symbol 3 copies champion 1 Symbols");
}

static void test_clear_and_dispatch_gates(void)
{
    CSB_V1_ChaosCastTargetingState state;
    CSB_V1_ChaosTargetCommand command = {
        CSB_V1_CHAOS_TARGET_COMMAND_SPELL_AREA, 0, 0, 4
    };
    int c3_symbol = line_offset(8, 8);

    seed_party(&state);
    csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(&state, 2);

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_accept_spell_area_command(
            &state, &command) == CSB_V1_CHAOS_TARGET_REJECTED,
        "ReDMCSB COMMAND.C:2302-2306",
        "spell-area dispatch is blocked before the command gate admits input");
    CHECK_REDMCSB(state.dispatch_started == 0,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "_CALL frame is not selected before the gate");
    CHECK_REDMCSB(state.dsa_queue_count == 0,
        "CSBWin/DSA.cpp:465-531 QueueDSASwitchAction",
        "DSA queue is untouched before the gate");
    CHECK_REDMCSB(state.ex_gosub_depth == 0,
        "CSBWin/DSA.cpp:764-808 EX_GOSUB",
        "EX_GOSUB frame is not established before the gate");

    command.command_gate_admitted = 1;
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_accept_spell_area_command(
            &state, &command) == CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB COMMAND.C:2302-2306",
        "spell-area dispatch is accepted after the command gate admits input");
    CHECK_REDMCSB(state.dispatch_frame_index == 4,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "_CALL4 dispatch frame is selected");
    CHECK_REDMCSB(state.dispatch_started == 1,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "pre-cast dispatch has started");
    CHECK_REDMCSB(state.dsa_queue_count == 1,
        "CSBWin/DSA.cpp:465-531 QueueDSASwitchAction",
        "one DSA switch action is queued");
    CHECK_REDMCSB(state.ex_gosub_depth == 1,
        "CSBWin/DSA.cpp:764-808 EX_GOSUB",
        "one targeting EX_GOSUB frame is established");
    CHECK_REDMCSB(state.dsa_buffer_head ==
            CSB_V1_CHAOS_TARGET_C3_CHAMPION,
        "C3_SPELL_AREA_CHAMPION_SYMBOLS",
        "DSA frame records the C3 symbol buffer head");
    CHECK_REDMCSB(state.dsa_active_caster == 2,
        "G0514_i_MagicCasterChampionIndex",
        "DSA frame records the active caster");
    CHECK_REDMCSB(state.spell_area_line[c3_symbol] == 'I',
        "G0515_aui_Bitmap_SpellAreaLine",
        "accepted dispatch sees champion 2 symbols");

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_build_spell_area_line(
            &state, CSB_V1_CHAOS_TARGET_C2_AVAILABLE) ==
            CSB_V1_CHAOS_TARGET_READY,
        "C2_SPELL_AREA_AVAILABLE_SYMBOLS",
        "C2 symbol line can be the active pre-cast buffer");
    command.requested_call_frame = 9;
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_accept_spell_area_command(
            &state, &command) == CSB_V1_CHAOS_TARGET_READY,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "_CALL9 dispatch frame is accepted after the gate");
    CHECK_REDMCSB(state.dispatch_frame_index == 9,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "_CALL9 is recorded as the current dispatch frame");
    CHECK_REDMCSB(state.dsa_buffer_head ==
            CSB_V1_CHAOS_TARGET_C2_AVAILABLE,
        "C2_SPELL_AREA_AVAILABLE_SYMBOLS",
        "DSA frame records the C2 symbol buffer head");
    CHECK_REDMCSB(state.dsa_active_caster == 2,
        "G0514_i_MagicCasterChampionIndex",
        "DSA frame preserves champion 2 as the active caster");
    CHECK_REDMCSB(state.dsa_queue_count == 2,
        "CSBWin/DSA.cpp:465-531 QueueDSASwitchAction",
        "second admitted command queues one more DSA action");

    command.requested_call_frame = 10;
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_accept_spell_area_command(
            &state, &command) == CSB_V1_CHAOS_TARGET_BAD_FRAME,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "dispatch rejects frames outside _CALL0 through _CALL9");

    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(
            &state, CSB_V1_CHAOS_TARGET_CHAMPION_NONE) ==
            CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB CASTER.C:27-33",
        "setting CM1_CHAMPION_NONE clears the spell area");
    CHECK_REDMCSB(state.magic_caster_champion_index ==
            CSB_V1_CHAOS_TARGET_CHAMPION_NONE,
        "G0514_i_MagicCasterChampionIndex",
        "G0514 is CM1_CHAMPION_NONE after clearing");
    CHECK_REDMCSB(state.spell_area_line_visible == 0,
        "ReDMCSB CASTER.C:27-33",
        "F0394 clear path hides spell-area line 1");
    CHECK_REDMCSB(csb_v1_chaos_cast_targeting_count_nonzero_line_bytes(
            &state) == 0,
        "G0515_aui_Bitmap_SpellAreaLine",
        "F0394 clear path zeroes the spell-area line bitmap");
    command.requested_call_frame = 0;
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_accept_spell_area_command(
            &state, &command) == CSB_V1_CHAOS_TARGET_NO_CASTER,
        "ReDMCSB COMMAND.C:2302-2306",
        "post-cleared state stays invisible to the next dispatch");

    seed_party(&state);
    csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(&state, 3);
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_clear_completed_spell_line(&state) ==
            CSB_V1_CHAOS_TARGET_READY,
        "ReDMCSB MENU.C:1633-1663",
        "completed spell-entry clear path succeeds");
    CHECK_REDMCSB(state.clear_line_count == 1,
        "ReDMCSB MENU.C:1633-1663",
        "clear-line path is counted once");
    CHECK_REDMCSB(state.spell_area_line_kind == 0,
        "ReDMCSB MENU.C:1655-1658",
        "clear-line path removes the active spell-area bitmap kind");
    CHECK_REDMCSB(state.spell_area_line_visible == 0,
        "ReDMCSB MENU.C:1655-1658",
        "clear-line path leaves the next cast invisible");
    CHECK_REDMCSB(csb_v1_chaos_cast_targeting_count_nonzero_line_bytes(
            &state) == 0,
        "G0515_aui_Bitmap_SpellAreaLine",
        "MENU.C clear-line path leaves G0515 all zero");
    CHECK_REDMCSB(
        csb_v1_chaos_cast_targeting_accept_spell_area_command(
            &state, &command) == CSB_V1_CHAOS_TARGET_NO_CASTER,
        "ReDMCSB MENU.C:1633-1663",
        "zero G0515 blocks the next pre-cast dispatch");
}

static void test_source_lock_and_no_cooldown_scope(void)
{
    CSB_V1_ChaosCastTargetingState state;
    const char *anchors = csb_v1_chaos_cast_targeting_anchor_text();

    seed_party(&state);
    state.cooldown_ticks = 7;
    csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(&state, 0);

    CHECK_REDMCSB(strstr(anchors, "CASTER.C F0394") != NULL,
        "ReDMCSB CASTER.C F0394_MENUS_SetMagicCasterAndDrawSpellArea",
        "source-lock anchors cite F0394 caster selection");
    CHECK_REDMCSB(strstr(anchors, "F0392_MENUS_BuildSpellAreaLine") != NULL,
        "ReDMCSB MENU.C F0392_MENUS_BuildSpellAreaLine",
        "source-lock anchors cite F0392 spell-area composition");
    CHECK_REDMCSB(strstr(anchors, "F0393_MENUS_DrawSpellAreaControls") != NULL,
        "ReDMCSB SPELDRAW.C F0393_MENUS_DrawSpellAreaControls",
        "source-lock anchors cite F0393 spell-area controls");
    CHECK_REDMCSB(strstr(anchors, "COMMAND.C:2302-2306") != NULL,
        "ReDMCSB COMMAND.C:2302-2306",
        "source-lock anchors cite the command gate");
    CHECK_REDMCSB(strstr(anchors, "MENU.C:1633-1663") != NULL,
        "ReDMCSB MENU.C:1633-1663",
        "source-lock anchors cite the clear-line path");
    CHECK_REDMCSB(strstr(anchors, "Chaos.cpp:584-588") != NULL,
        "CSBWin/Chaos.cpp:584-588 InitializeE",
        "source-lock anchors cite InitializeE clear state");
    CHECK_REDMCSB(strstr(anchors, "_CALL0-_CALL9") != NULL,
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9",
        "source-lock anchors cite the pre-cast dispatch frame set");
    CHECK_REDMCSB(strstr(anchors, "QueueDSASwitchAction") != NULL,
        "CSBWin/DSA.cpp:465-531 QueueDSASwitchAction",
        "source-lock anchors cite DSA queueing");
    CHECK_REDMCSB(strstr(anchors, "EX_GOSUB") != NULL,
        "CSBWin/DSA.cpp:764-808 EX_GOSUB",
        "source-lock anchors cite nested DSA call frame entry");
    CHECK_REDMCSB(strstr(anchors, "StartChaos") != NULL,
        "CSBWin/CSBCode.cpp:11414 StartChaos",
        "source-lock anchors cite the CSB utility/game path");
    CHECK_REDMCSB(strstr(anchors, "C048_BYTE_WIDTH") != NULL,
        "C048_BYTE_WIDTH",
        "source-lock anchors include C048_BYTE_WIDTH");
    CHECK_REDMCSB(strstr(anchors, "C160_BYTE_WIDTH_SCREEN") != NULL,
        "C160_BYTE_WIDTH_SCREEN",
        "source-lock anchors include C160_BYTE_WIDTH_SCREEN");
    CHECK_REDMCSB(strstr(anchors, "G0514_i_MagicCasterChampionIndex") != NULL,
        "G0514_i_MagicCasterChampionIndex",
        "source-lock anchors include G0514");
    CHECK_REDMCSB(strstr(anchors, "G1072_ai_Box_SpellAreaLine2") != NULL,
        "G1072_ai_Box_SpellAreaLine2",
        "source-lock anchors include G1072");
    CHECK_REDMCSB(strstr(anchors, "G0515_aui_Bitmap_SpellAreaLine") != NULL,
        "G0515_aui_Bitmap_SpellAreaLine",
        "source-lock anchors include G0515");
    CHECK_REDMCSB(state.cooldown_ticks == 7,
        "post-cast cooldown slice disjointness",
        "pre-cast targeting does not decrement cooldown ticks");
    CHECK_REDMCSB(state.cooldown_tick_decrements == 0,
        "post-cast cooldown slice disjointness",
        "pre-cast targeting records no cooldown tick decrement");
}

int main(void)
{
    printf("=== CSB V1 Chaos Cast Targeting Pre-Cast Regression Gate ===\n\n");
    test_magic_caster_selection_and_line_build();
    test_clear_and_dispatch_gates();
    test_source_lock_and_no_cooldown_scope();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
