#include "csb_v1_chaos_cast_targeting_pc34_compat.h"

#include <string.h>

/* CSB V1 Chaos pre-cast targeting runtime slice.
 *
 * Source-locked to the pre-cast path:
 *   ReDMCSB CASTER.C F0394_MENUS_SetMagicCasterAndDrawSpellArea lines 2-60
 *     selects G0514_i_MagicCasterChampionIndex, builds C2 line 1, draws
 *     F0393 controls, blits with C048_BYTE_WIDTH/C160_BYTE_WIDTH_SCREEN,
 *     then rebuilds G0515_aui_Bitmap_SpellAreaLine as C3 champion symbols.
 *   ReDMCSB MENU.C F0392_MENUS_BuildSpellAreaLine lines 844-895 copies
 *     C2_SPELL_AREA_AVAILABLE_SYMBOLS or C3_SPELL_AREA_CHAMPION_SYMBOLS
 *     into G0515_aui_Bitmap_SpellAreaLine.
 *   ReDMCSB SPELDRAW.C F0393_MENUS_DrawSpellAreaControls lines 2-96 draws
 *     the selected caster controls.
 *   ReDMCSB COMMAND.C:2302-2306 accepts C100 spell-area input only when
 *     the command gate has admitted input and G0514_i_MagicCasterChampionIndex
 *     is not CM1_CHAMPION_NONE.
 *   ReDMCSB MENU.C:1633-1663 clears a completed spell-entry line.
 *   CSBWin/Chaos.cpp:584-588 InitializeE clears engine state before CSB.
 *   CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9 declare the dispatch frame set.
 *   CSBWin/DSA.cpp:465-531 QueueDSASwitchAction queues DSA timer actions.
 *   CSBWin/DSA.cpp:764-808 EX_GOSUB enters a nested targeting frame.
 *   CSBWin/CSBCode.cpp:11414 StartChaos enters the CSB utility/game path.
 */

static void csb_v1_target_copy_text(char *dst, size_t dst_size,
    const char *src)
{
    size_t i;

    if (!dst || dst_size == 0) {
        return;
    }
    memset(dst, 0, dst_size);
    if (!src) {
        return;
    }
    for (i = 0; i + 1 < dst_size && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
}

static void csb_v1_target_prime_templates(
    CSB_V1_ChaosCastTargetingState *state)
{
    int i;

    for (i = 0; i < CSB_V1_CHAOS_TARGET_SPELL_LINE_BYTES; i++) {
        state->spell_area_lines[1][i] = (uint8_t)(0x20u + (uint8_t)(i & 7));
        state->spell_area_lines[2][i] = (uint8_t)(0x40u + (uint8_t)(i & 7));
    }
    state->spell_area_lines[1][0] = CSB_V1_CHAOS_TARGET_C2_AVAILABLE;
    state->spell_area_lines[2][0] = CSB_V1_CHAOS_TARGET_C3_CHAMPION;
}

static void csb_v1_target_stamp_text(uint8_t *line, int y, int byte_x,
    const char *text, int max_chars)
{
    int i;
    int offset = (y * CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH) + byte_x;

    for (i = 0; i < max_chars && text && text[i] != '\0'; i++) {
        if (offset + i < CSB_V1_CHAOS_TARGET_SPELL_LINE_BYTES) {
            line[offset + i] = (uint8_t)text[i];
        }
    }
}

static int csb_v1_target_valid_champion(
    const CSB_V1_ChaosCastTargetingState *state, int champion_index)
{
    return state &&
        champion_index >= 0 &&
        champion_index < CSB_V1_CHAOS_TARGET_MAX_CHAMPIONS &&
        champion_index < state->party_champion_count &&
        state->champions[champion_index].current_health > 0;
}

static void csb_v1_target_blit_line2_to_screen(
    CSB_V1_ChaosCastTargetingState *state)
{
    int y;

    for (y = 1; y < 12; y++) {
        memcpy(&state->screen[CSB_V1_CHAOS_TARGET_LINE2_OFFSET +
                   (y * CSB_V1_CHAOS_TARGET_C160_BYTE_WIDTH)],
            &state->spell_area_line[y * CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH],
            CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH);
    }
    state->last_blit_offset = CSB_V1_CHAOS_TARGET_LINE2_OFFSET;
    state->last_blit_stride = CSB_V1_CHAOS_TARGET_C160_BYTE_WIDTH;
    state->last_blit_width = CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH;
    state->last_blit_rows = 11;
    state->line1_blits++;
}

void csb_v1_chaos_cast_targeting_init(CSB_V1_ChaosCastTargetingState *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->magic_caster_champion_index = CSB_V1_CHAOS_TARGET_CHAMPION_NONE;
    state->party_champion_count = CSB_V1_CHAOS_TARGET_MAX_CHAMPIONS;
    state->last_controls_champion_index = CSB_V1_CHAOS_TARGET_CHAMPION_NONE;
    state->last_blit_offset = -1;
    state->last_blit_stride = -1;
    state->dispatch_frame_index = -1;
    state->dsa_active_caster = CSB_V1_CHAOS_TARGET_CHAMPION_NONE;
    csb_v1_target_prime_templates(state);
}

int csb_v1_chaos_cast_targeting_set_champion(
    CSB_V1_ChaosCastTargetingState *state, int champion_index,
    int current_health, int symbol_step, const char *symbols,
    const char *name)
{
    if (!state || champion_index < 0 ||
        champion_index >= CSB_V1_CHAOS_TARGET_MAX_CHAMPIONS) {
        return CSB_V1_CHAOS_TARGET_INVALID;
    }

    state->champions[champion_index].current_health = current_health;
    state->champions[champion_index].symbol_step = symbol_step;
    csb_v1_target_copy_text(state->champions[champion_index].symbols,
        sizeof(state->champions[champion_index].symbols), symbols);
    csb_v1_target_copy_text(state->champions[champion_index].name,
        sizeof(state->champions[champion_index].name), name);
    return CSB_V1_CHAOS_TARGET_READY;
}

int csb_v1_chaos_cast_targeting_build_spell_area_line(
    CSB_V1_ChaosCastTargetingState *state, int spell_area_bitmap_line)
{
    CSB_V1_ChaosTargetChampion *champion;
    char available[7];
    int i;

    if (!state ||
        !csb_v1_target_valid_champion(state,
            state->magic_caster_champion_index)) {
        return CSB_V1_CHAOS_TARGET_NO_CASTER;
    }

    champion = &state->champions[state->magic_caster_champion_index];
    if (spell_area_bitmap_line == CSB_V1_CHAOS_TARGET_C2_AVAILABLE) {
        memcpy(state->spell_area_line, state->spell_area_lines[1],
            sizeof(state->spell_area_line));
        for (i = 0; i < 6; i++) {
            available[i] = (char)('`' + (6 * champion->symbol_step) + i);
        }
        available[6] = '\0';
        csb_v1_target_stamp_text(state->spell_area_line, 8, 2, available, 6);
        state->spell_area_line[0] = CSB_V1_CHAOS_TARGET_C2_AVAILABLE;
        state->spell_area_line[1] = (uint8_t)state->magic_caster_champion_index;
    } else if (spell_area_bitmap_line == CSB_V1_CHAOS_TARGET_C3_CHAMPION) {
        memcpy(state->spell_area_line, state->spell_area_lines[2],
            sizeof(state->spell_area_line));
        csb_v1_target_stamp_text(state->spell_area_line, 8, 8,
            champion->symbols, 4);
        state->spell_area_line[0] = CSB_V1_CHAOS_TARGET_C3_CHAMPION;
        state->spell_area_line[1] = (uint8_t)state->magic_caster_champion_index;
    } else {
        return CSB_V1_CHAOS_TARGET_INVALID;
    }

    state->spell_area_line_kind = spell_area_bitmap_line;
    return CSB_V1_CHAOS_TARGET_READY;
}

int csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(
    CSB_V1_ChaosCastTargetingState *state, int champion_index)
{
    if (!state) {
        return CSB_V1_CHAOS_TARGET_INVALID;
    }
    if (champion_index == state->magic_caster_champion_index) {
        return CSB_V1_CHAOS_TARGET_REJECTED;
    }
    if (champion_index != CSB_V1_CHAOS_TARGET_CHAMPION_NONE &&
        !csb_v1_target_valid_champion(state, champion_index)) {
        return CSB_V1_CHAOS_TARGET_REJECTED;
    }

    if (champion_index == CSB_V1_CHAOS_TARGET_CHAMPION_NONE) {
        state->magic_caster_champion_index = CSB_V1_CHAOS_TARGET_CHAMPION_NONE;
        memset(state->spell_area_line, 0, sizeof(state->spell_area_line));
        state->spell_area_line_kind = 0;
        state->spell_area_line_visible = 0;
        state->spell_area_line1_caster = CSB_V1_CHAOS_TARGET_CHAMPION_NONE;
        state->clear_line_count++;
        return CSB_V1_CHAOS_TARGET_READY;
    }

    state->magic_caster_champion_index = champion_index;
    if (csb_v1_chaos_cast_targeting_build_spell_area_line(state,
            CSB_V1_CHAOS_TARGET_C2_AVAILABLE) != CSB_V1_CHAOS_TARGET_READY) {
        return CSB_V1_CHAOS_TARGET_INVALID;
    }
    csb_v1_target_blit_line2_to_screen(state);
    state->spell_area_line_visible = 1;
    state->spell_area_line1_caster = champion_index;
    state->last_controls_champion_index = champion_index;
    return csb_v1_chaos_cast_targeting_build_spell_area_line(state,
        CSB_V1_CHAOS_TARGET_C3_CHAMPION);
}

int csb_v1_chaos_cast_targeting_clear_completed_spell_line(
    CSB_V1_ChaosCastTargetingState *state)
{
    if (!state) {
        return CSB_V1_CHAOS_TARGET_INVALID;
    }
    memset(state->spell_area_line, 0, sizeof(state->spell_area_line));
    state->spell_area_line_kind = 0;
    state->spell_area_line_visible = 0;
    state->clear_line_count++;
    return CSB_V1_CHAOS_TARGET_READY;
}

int csb_v1_chaos_cast_targeting_accept_spell_area_command(
    CSB_V1_ChaosCastTargetingState *state,
    const CSB_V1_ChaosTargetCommand *command)
{
    if (!state || !command ||
        command->command_id != CSB_V1_CHAOS_TARGET_COMMAND_SPELL_AREA) {
        return CSB_V1_CHAOS_TARGET_INVALID;
    }
    if (!command->command_gate_admitted ||
        command->candidate_champion_ordinal != 0) {
        return CSB_V1_CHAOS_TARGET_REJECTED;
    }
    if (!csb_v1_target_valid_champion(state,
            state->magic_caster_champion_index) ||
        !state->spell_area_line_visible ||
        state->spell_area_line[0] == 0) {
        return CSB_V1_CHAOS_TARGET_NO_CASTER;
    }
    if (command->requested_call_frame < 0 ||
        command->requested_call_frame > 9) {
        return CSB_V1_CHAOS_TARGET_BAD_FRAME;
    }

    state->dispatch_frame_index = command->requested_call_frame;
    state->dispatch_started = 1;
    state->dsa_queue_count++;
    state->ex_gosub_depth = 1;
    state->dsa_buffer_head = state->spell_area_line[0];
    state->dsa_active_caster = state->magic_caster_champion_index;
    return CSB_V1_CHAOS_TARGET_READY;
}

int csb_v1_chaos_cast_targeting_count_nonzero_line_bytes(
    const CSB_V1_ChaosCastTargetingState *state)
{
    int i;
    int count = 0;

    if (!state) {
        return 0;
    }
    for (i = 0; i < CSB_V1_CHAOS_TARGET_SPELL_LINE_BYTES; i++) {
        if (state->spell_area_line[i] != 0) {
            count++;
        }
    }
    return count;
}

int csb_v1_chaos_cast_targeting_count_nonzero_line1_screen_bytes(
    const CSB_V1_ChaosCastTargetingState *state)
{
    int y;
    int x;
    int count = 0;

    if (!state) {
        return 0;
    }
    for (y = 1; y < 12; y++) {
        for (x = 0; x < CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH; x++) {
            if (state->screen[CSB_V1_CHAOS_TARGET_LINE2_OFFSET +
                    (y * CSB_V1_CHAOS_TARGET_C160_BYTE_WIDTH) + x] != 0) {
                count++;
            }
        }
    }
    return count;
}

const char *csb_v1_chaos_cast_targeting_anchor_text(void)
{
    return
        "ReDMCSB CASTER.C F0394_MENUS_SetMagicCasterAndDrawSpellArea lines 2-60\n"
        "ReDMCSB MENU.C F0392_MENUS_BuildSpellAreaLine lines 844-895\n"
        "ReDMCSB SPELDRAW.C F0393_MENUS_DrawSpellAreaControls lines 2-96\n"
        "ReDMCSB COMMAND.C:2302-2306 spell-area command gate\n"
        "ReDMCSB MENU.C:1633-1663 clear completed spell-entry line\n"
        "CSBWin/Chaos.cpp:584-588 InitializeE\n"
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9 dispatch frame declarations\n"
        "CSBWin/DSA.cpp:465-531 QueueDSASwitchAction\n"
        "CSBWin/DSA.cpp:764-808 EX_GOSUB\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos\n"
        "C048_BYTE_WIDTH C160_BYTE_WIDTH_SCREEN G0514_i_MagicCasterChampionIndex "
        "G1072_ai_Box_SpellAreaLine2 G0515_aui_Bitmap_SpellAreaLine";
}
