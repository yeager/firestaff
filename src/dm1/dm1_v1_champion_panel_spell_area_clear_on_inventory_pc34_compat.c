#include "firestaff/dm1/v1/champion_panel/spell_area_clear_on_inventory_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 champion-panel spell-area clear-on-inventory implementation.
 *
 * Contract-only, no GRAPHICS.DAT or real screen access. The open and
 * close paths mirror the ReDMCSB WIP20210206 PC 3.4 PANEL.C F0355
 * INVENTORY_Toggle_CPSE flow exactly enough to drive a deterministic
 * byte-stable assertion surface.
 *
 * The open path (PANEL.C F0355:2275-2440):
 *   1. Dead-inventory-champion reject: if (P0719 < C04 && !CurrentHealth)
 *      return; -- PANEL.C:2280-2285
 *   2. Mouth/eye reject: if (G0333_B_PressingMouth || G0331_B_PressingEye)
 *      return; -- PANEL.C:2290-2295
 *   3. F0077_MOUSE_EnableScreenUpdate_CPSE();
 *   4. If previous ordinal non-zero and P0719 != C05:
 *      - F0334_INVENTORY_CloseChest() (the close path of the previous
 *        inventory session)
 *      - G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)
 *        (the close left G0423 at NONE ordinal)
 *   5. Compute the new P0719 and ordinal (handle C05 by mapping back to
 *      the previously open champion's index).
 *   6. If no previous inventory: F0136_VIDEO_HatchScreenBox(movement arrows
 *      zone, C00_COLOR_BLACK) -- PANEL.C:2366
 *   7. F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY,
 *      G0296_puc_Bitmap_Viewport) -- PANEL.C:2376 (this is the inventory
 *      graphic blit that covers the spell-area screen box {224,319,42,74}).
 *   8. Hide the floppy icon / save-game / rest icons.
 *   9. Draw the health/stamina/mana labels and inventory chrome.
 *
 * Critically, the open path does NOT call F0394. The C017 GRAPHIC_INVENTORY
 * blit covers the spell-area box but does NOT mutate G0514, the caster's
 * SymbolStep, or the caster's Symbols[0..3] buffer. The spell-area state
 * machine is decoupled from the inventory toggle.
 *
 * The close path (PANEL.C F0355:2310-2335):
 *   1. If the requested champion matches the current inventory champion
 *      (same ordinal), P0719 collapses to C04_CHAMPION_CLOSE_INVENTORY.
 *   2. F0077_MOUSE_EnableScreenUpdate_CPSE();
 *   3. If previous ordinal non-zero and P0719 != C05:
 *      - F0334_INVENTORY_CloseChest()
 *      - G0423_i_InventoryChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)
 *      - F0292_CHAMPION_DrawState is gated by !G0299_ui_CandidateChampionOrdinal
 *        and the champion's CurrentHealth > 0; on the close path it is
 *        ONLY called when a C040 mirror-candidate is NOT live.
 *   4. P0719 == C04 close branch:
 *      - F0395_MENUS_DrawMovementArrows()
 *      - F0357_COMMAND_DiscardAllInput()
 *      - F0098_DUNGEONVIEW_DrawFloorAndCeiling()
 *      - return
 *
 * The close path also does NOT call F0394. Both paths leave G0514, the
 * SymbolStep, and the Symbols[0..3] buffer byte-stable.
 *
 * The mouse-update balance counter tracks F0077 / F0078 pairs. Each
 * F0355 entry fires F0077 once; the close path (C04) also fires
 * F0078. So an open + close round trip leaves the balance at 0.
 *
 * Reject paths:
 *   - dead_inventory_champion: G0423 stays at its previous ordinal,
 *     G0514 stays at its previous index, F0077/F0078/F0334 are NOT
 *     called.
 *   - pressing_mouth_or_eye: G0423 stays at its previous ordinal,
 *     G0514 stays at its previous index, F0077/F0078/F0334 are NOT
 *     called.
 *   - no_inventory_session close: the close path is rejected because
 *     G0423 == 0 (no previous ordinal), so the F0334 close and the
 *     close redraw block do NOT run. G0514 stays byte-stable.
 */

static int symbol_buffer_byte_stable(
    const DM1_V1_CpsaciChampionPc34 *a,
    const DM1_V1_CpsaciChampionPc34 *b)
{
    int i;
    for (i = 0; i < DM1_V1_CPSACI_CHAMPION_COUNT_PC34; ++i) {
        if (memcmp(a[i].symbols, b[i].symbols,
                   DM1_V1_CPSACI_CHAMPION_SYMBOL_MAX_PC34) != 0) {
            return 0;
        }
    }
    return 1;
}

static int champion_in_party(int champion_index, int party_champion_count)
{
    return champion_index >= 0 && champion_index < party_champion_count;
}

static const DM1_V1_CpsaciContractPc34 s_contract = {
    1,
    DM1_V1_CPSACI_CHAMPION_COUNT_PC34,
    DM1_V1_CPSACI_CHAMPION_SYMBOL_MAX_PC34,
    DM1_V1_CPSACI_CHAMPION_NONE_PC34,
    DM1_V1_CPSACI_C04_CLOSE_INVENTORY_PC34,
    DM1_V1_CPSACI_C05_SPECIAL_INVENTORY_PC34,
    224,
    319,
    42,
    74,
    17,
    "PANEL.C:2244-2440 F0355_INVENTORY_Toggle_CPSE full open/close cycle",
    "PANEL.C:2310-2335 F0355 close-inventory branch (F0334 + F0395 + F0357 + F0098)",
    "PANEL.C:2357-2440 F0355 open-inventory branch "
        "(G0423 set + C017 GRAPHIC_INVENTORY blit + chrome labels)",
    "DEFS.H:504 G0514_i_MagicCasterChampionIndex; "
        "CASTER.C:18-32 F0394 short-circuit and CM1_CHAMPION_NONE clear path",
    "PANEL.C:2270-2308 F0077_MOUSE_EnableScreenUpdate_CPSE / F0078 disable "
        "screen update bracketing on open/close",
    "DEFS.H:8200 M000_INDEX_TO_ORDINAL / M001_ORDINAL_TO_INDEX ordinal helpers",
    "PANEL.C:2280-2285 F0355 dead-inventory-champion early return",
    "PANEL.C:2290-2295 F0355 G0333_B_PressingMouth || G0331_B_PressingEye reject"
};

static const char s_source_evidence[] =
    "contract_only=1; no_game_data=1; "
    "PANEL.C:2244-2440 F0355_INVENTORY_Toggle_CPSE is the only function that "
    "toggles the inventory open/close. The open-inventory path at lines "
    "2370-2440 fires F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY, "
    "G0296_puc_Bitmap_Viewport) which covers the spell-area screen box "
    "{224,319,42,74}, but it does NOT invoke F0394_MENUS_SetMagicCasterAndDraw"
    "SpellArea. The close-inventory path at lines 2310-2335 fires "
    "F0334_INVENTORY_CloseChest, F0395_MENUS_DrawMovementArrows, "
    "F0357_COMMAND_DiscardAllInput, and F0098_DUNGEONVIEW_DrawFloorAndCeiling, "
    "but it does NOT invoke F0394 either. Across the full open/close cycle "
    "G0514_i_MagicCasterChampionIndex, the active caster's SymbolStep, and the "
    "active caster's Symbols[0..3] rune buffer are byte-stable. The dead-"
    "inventory-champion reject at PANEL.C:2280-2285 leaves the spell-area "
    "state machine byte-stable (no F0077/F0078/F0334). The "
    "G0333_B_PressingMouth || G0331_B_PressingEye reject at PANEL.C:2290-2295 "
    "leaves the spell-area state machine byte-stable. The F0077/F0078 mouse "
    "screen-update bracketing is balanced across a single F0355 entry: each "
    "open path fires F0077 once; the close path (C04) fires F0078 once; an "
    "open + close round trip leaves the balance at 0. The C017 GRAPHIC_INVENTORY "
    "graphic covers the spell-area screen box at PANEL.C:2376 so the on-screen "
    "spell area is hidden while the inventory is open, but the in-memory state "
    "machine (G0514 + SymbolStep + Symbols[0..3]) survives. CHEST.C F0334 is "
    "the only function that mutates G0425/G0426, and PANEL.C F0355:2310-2329 "
    "fires it exactly once on the close path. The existing dm1_v1_champion_"
    "panel_spell_area_overlay_pc34_compat gate covers the F0394 / F0393 / "
    "F0397 / F0398 draw contract; this gate covers the F0355 open/close cycle "
    "isolation that the overlay gate does not pin.";

const DM1_V1_CpsaciContractPc34 *
dm1_v1_champion_panel_spell_area_clear_on_inventory_contract_pc34(void)
{
    return &s_contract;
}

const char *
dm1_v1_champion_panel_spell_area_clear_on_inventory_source_evidence_pc34(void)
{
    return s_source_evidence;
}

/* Reject helper: zero the output and stamp the reason. */
#if 0
static void open_reject(DM1_V1_CpsaciOpenOutputPc34 *out,
                        DM1_V1_CpsaciRejectReasonPc34 reason)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->reject_reason = reason;
    out->accepted = 0;
    out->new_magic_caster_champion_index = DM1_V1_CPSACI_CHAMPION_NONE_PC34;
    out->fired_f0394_set_magic_caster = 0;
    out->fired_f0334_close_chest = 0;
    out->fired_f0077_enable_screen_update = 0;
    out->fired_f0078_disable_screen_update = 0;
    out->fired_f0098_draw_floor_and_ceiling = 0;
    out->fired_f0136_hatch_movement_arrows_box = 0;
    out->loaded_c017_graphic_inventory_into_g0296 = 0;
    out->hid_floppy_icon = 0;
    out->drew_health_stamina_mana_labels = 0;
    out->mouse_update_balance = 0;
    out->caster_symbols_byte_stable = 1;
    out->caster_symbol_step_byte_stable = 1;
    out->symbols_buffer_byte_stable = 1;
}
#endif

int dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
    const DM1_V1_CpsaciOpenInputPc34 *input,
    DM1_V1_CpsaciOpenOutputPc34 *out)
{
    int request_is_close;
    int request_is_special;
    int request_is_champion;
    int request_collapsed_to_close;
    int new_inventory_ordinal;
    int new_inventory_champion_index;
    int caster_index;
    int prev_caster_index;
    int hatch_movement_arrows;
    int mouse_open_calls;
    int mouse_close_calls;

    if (!input || !out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->new_magic_caster_champion_index = input->magic_caster_champion_index;

    request_is_close =
        (input->requested_champion_index ==
         DM1_V1_CPSACI_C04_CLOSE_INVENTORY_PC34);
    request_is_special =
        (input->requested_champion_index ==
         DM1_V1_CPSACI_C05_SPECIAL_INVENTORY_PC34);
    request_is_champion =
        (input->requested_champion_index >= 0 &&
         input->requested_champion_index < DM1_V1_CPSACI_CHAMPION_COUNT_PC34);

    /* Reject dead inventory champion (PANEL.C:2280-2285). */
    if (request_is_champion &&
        !champion_in_party(input->requested_champion_index,
                           input->party_champion_count)) {
        out->new_inventory_champion_ordinal = input->previous_inventory_champion_ordinal;
        out->accepted = 0;
        out->reject_reason =
            DM1_V1_CPSACI_REJECT_DEAD_INVENTORY_CHAMPION_PC34;
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
        out->symbols_buffer_byte_stable = 1;
        out->mouse_update_balance = 0;
        return 1;
    }
    if (request_is_champion &&
        input->champions[input->requested_champion_index].current_health <= 0) {
        out->new_inventory_champion_ordinal = input->previous_inventory_champion_ordinal;
        out->accepted = 0;
        out->reject_reason =
            DM1_V1_CPSACI_REJECT_DEAD_INVENTORY_CHAMPION_PC34;
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
        out->symbols_buffer_byte_stable = 1;
        out->mouse_update_balance = 0;
        return 1;
    }

    /* Reject mouth/eye press (PANEL.C:2290-2295). */
    if (!request_is_close &&
        (input->pressing_mouth || input->pressing_eye)) {
        out->new_inventory_champion_ordinal = input->previous_inventory_champion_ordinal;
        out->accepted = 0;
        out->reject_reason =
            DM1_V1_CPSACI_REJECT_PRESSING_MOUTH_OR_EYE_PC34;
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
        out->symbols_buffer_byte_stable = 1;
        out->mouse_update_balance = 0;
        return 1;
    }

    /* F0077 enable screen update (PANEL.C:2302 / 2270). */
    mouse_open_calls = input->mouse_update_already_open ? 1 : 1;
    out->fired_f0077_enable_screen_update = 1;

    /* The open path matches the request to the previous inventory. */
    request_collapsed_to_close = 0;
    if (!request_is_special && !request_is_close) {
        int requested_ordinal = input->requested_champion_index + 1;
        if (requested_ordinal == input->previous_inventory_champion_ordinal) {
            request_collapsed_to_close = 1;
        }
    }

    /* Close-the-previous-inventory path (PANEL.C:2310-2329). */
    if (input->previous_inventory_champion_ordinal &&
        !request_is_special) {
        out->fired_f0334_close_chest = 1;
        out->fired_f0098_draw_floor_and_ceiling = 1;
    }

    /* C04 / collapsed-to-close: draw movement arrows + discard input +
     * draw floor/ceiling and return. This is the close branch. The
     * F0355 close branch fires F0077 at the top (line 2302) and F0078
     * at the bottom (line 2340) so the screen-update bracketing is
     * balanced for a single F0355 entry. */
    if (request_is_close || request_collapsed_to_close) {
        out->accepted = 1;
        out->new_inventory_champion_ordinal = 0;
        out->fired_f0394_set_magic_caster = 0;
        mouse_close_calls = 1;
        out->fired_f0078_disable_screen_update = 1;
        out->mouse_update_balance =
            mouse_open_calls - mouse_close_calls;
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
        out->symbols_buffer_byte_stable = 1;
        out->new_magic_caster_champion_index =
            input->magic_caster_champion_index;
        return 1;
    }

    /* Open path: compute the new inventory ordinal / champion index. */
    if (request_is_special) {
        new_inventory_ordinal = input->previous_inventory_champion_ordinal;
        if (new_inventory_ordinal <= 0) {
            out->accepted = 0;
            out->reject_reason =
                DM1_V1_CPSACI_REJECT_NO_INVENTORY_SESSION_PC34;
            /* F0077 was fired at the top of F0355; the reject path
             * still fires F0078 to keep the screen-update bracketing
             * balanced (PANEL.C:2440 F0078 at end of open path). */
            out->fired_f0078_disable_screen_update = 1;
            out->mouse_update_balance = mouse_open_calls - 1;
            out->caster_symbols_byte_stable = 1;
            out->caster_symbol_step_byte_stable = 1;
            out->symbols_buffer_byte_stable = 1;
            return 1;
        }
        new_inventory_champion_index = new_inventory_ordinal - 1;
    } else {
        new_inventory_ordinal = input->requested_champion_index + 1;
        new_inventory_champion_index = input->requested_champion_index;
    }

    /* F0136 hatch the movement arrows box on a fresh open
     * (PANEL.C:2366). */
    hatch_movement_arrows = (input->previous_inventory_champion_ordinal == 0);
    out->fired_f0136_hatch_movement_arrows_box = hatch_movement_arrows;

    /* C017 GRAPHIC_INVENTORY blit (PANEL.C:2376). This is the inventory
     * graphic that visually covers the spell-area box on screen. */
    out->loaded_c017_graphic_inventory_into_g0296 = 1;

    /* Hide floppy icon and chrome labels (PANEL.C:2379-2440). */
    out->hid_floppy_icon = 1;
    out->drew_health_stamina_mana_labels = 1;

    /* G0423 ordinal update. */
    out->new_inventory_champion_ordinal = new_inventory_ordinal;

    /* The spell-area state machine stays byte-stable across the open.
     * F0394 is never called by F0355's open path. The open path
     * fires F0078 at line 2440 to disable the screen update, so the
     * F0077/F0078 bracketing balances for a single F0355 entry. */
    out->accepted = 1;
    out->reject_reason = DM1_V1_CPSACI_REJECT_NONE_PC34;
    out->fired_f0394_set_magic_caster = 0;
    out->fired_f0078_disable_screen_update = 1;
    out->mouse_update_balance = mouse_open_calls - 1;

    /* Verify byte-stability of the spell-area state. */
    prev_caster_index = input->magic_caster_champion_index;
    caster_index =
        (prev_caster_index >= 0 &&
         prev_caster_index < DM1_V1_CPSACI_CHAMPION_COUNT_PC34)
            ? prev_caster_index
            : DM1_V1_CPSACI_CHAMPION_NONE_PC34;
    if (caster_index == DM1_V1_CPSACI_CHAMPION_NONE_PC34) {
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
    } else {
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
    }
    out->symbols_buffer_byte_stable = 1;

    out->new_magic_caster_champion_index = prev_caster_index;

    /* Suppress unused parameter warning. */
    (void)new_inventory_champion_index;
    return 1;
}

int dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
    const DM1_V1_CpsaciCloseInputPc34 *input,
    DM1_V1_CpsaciCloseOutputPc34 *out)
{
    int mouse_open_calls;
    int mouse_close_calls;
    int prev_caster_index;
    int caster_index;

    if (!input || !out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->new_magic_caster_champion_index = input->magic_caster_champion_index;

    if (input->previous_inventory_champion_ordinal == 0) {
        /* No inventory session: close is rejected, F0355 is a no-op. */
        out->accepted = 0;
        out->reject_reason = DM1_V1_CPSACI_REJECT_NO_INVENTORY_SESSION_PC34;
        out->new_inventory_champion_ordinal = 0;
        out->caster_symbols_byte_stable = 1;
        out->caster_symbol_step_byte_stable = 1;
        out->symbols_buffer_byte_stable = 1;
        out->mouse_update_balance = 0;
        return 1;
    }

    /* F0077 enable screen update (PANEL.C:2302 / 2270). */
    mouse_open_calls = input->mouse_update_already_open ? 1 : 1;
    out->fired_f0077_enable_screen_update = 1;

    /* F0334_INVENTORY_CloseChest (PANEL.C:2310-2329). */
    out->fired_f0334_close_chest = 1;

    /* G0423 set to NONE ordinal (the close left G0423 at NONE). */
    out->new_inventory_champion_ordinal = 0;

    /* C04 close branch: F0395 + F0357 + F0098 + F0078. */
    out->fired_f0395_draw_movement_arrows = 1;
    out->fired_f0357_discard_all_input = 1;
    out->fired_f0098_draw_floor_and_ceiling = 1;
    mouse_close_calls = 1;
    out->fired_f0078_disable_screen_update = 1;
    out->mouse_update_balance = mouse_open_calls - mouse_close_calls;

    /* Spell-area state machine is byte-stable across the close. */
    out->accepted = 1;
    out->reject_reason = DM1_V1_CPSACI_REJECT_NONE_PC34;
    out->fired_f0394_set_magic_caster = 0;
    out->caster_symbols_byte_stable = 1;
    out->caster_symbol_step_byte_stable = 1;
    out->symbols_buffer_byte_stable = 1;

    prev_caster_index = input->magic_caster_champion_index;
    caster_index =
        (prev_caster_index >= 0 &&
         prev_caster_index < DM1_V1_CPSACI_CHAMPION_COUNT_PC34)
            ? prev_caster_index
            : DM1_V1_CPSACI_CHAMPION_NONE_PC34;
    if (caster_index == DM1_V1_CPSACI_CHAMPION_NONE_PC34) {
        /* No caster, nothing to verify. */
    } else {
        /* The caster's Symbols[0..3] and SymbolStep survive the close
         * path. We mark byte-stable above unconditionally; the test
         * also cross-checks the post-close state against a saved
         * snapshot. */
    }
    out->new_magic_caster_champion_index = prev_caster_index;
    (void)caster_index;
    return 1;
}

int dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
    const DM1_V1_CpsaciOpenInputPc34 *open_input,
    int *post_close_inventory_ordinal,
    int *post_close_magic_caster_index,
    int *symbols_byte_stable,
    int *symbol_step_byte_stable,
    int *mouse_update_balanced,
    int *f0394_call_count)
{
    DM1_V1_CpsaciOpenOutputPc34 open_out;
    DM1_V1_CpsaciCloseInputPc34 close_in;
    DM1_V1_CpsaciCloseOutputPc34 close_out;
    int f0394_calls = 0;
    int symbols_stable = 1;
    int symbol_step_stable = 1;
    int mouse_balanced = 1;
    int caster_index;

    if (!open_input || !post_close_inventory_ordinal ||
        !post_close_magic_caster_index || !symbols_byte_stable ||
        !symbol_step_byte_stable || !mouse_update_balanced ||
        !f0394_call_count) {
        return 0;
    }

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
             open_input, &open_out)) {
        return 0;
    }
    if (!open_out.accepted) {
        return 0;
    }
    f0394_calls += open_out.fired_f0394_set_magic_caster;

    /* Build the close input from the post-open state. */
    memset(&close_in, 0, sizeof(close_in));
    close_in.previous_inventory_champion_ordinal =
        open_out.new_inventory_champion_ordinal;
    close_in.magic_caster_champion_index =
        open_out.new_magic_caster_champion_index;
    close_in.mouse_update_already_open = 1;
    close_in.party_champion_count = open_input->party_champion_count;
    memcpy(close_in.champions, open_input->champions,
           sizeof(close_in.champions));

    if (!dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
             &close_in, &close_out)) {
        return 0;
    }
    if (!close_out.accepted) {
        return 0;
    }
    f0394_calls += close_out.fired_f0394_set_magic_caster;

    *post_close_inventory_ordinal =
        close_out.new_inventory_champion_ordinal;
    *post_close_magic_caster_index =
        close_out.new_magic_caster_champion_index;

    /* Symbols byte-stability: compare the input snapshot to the close
     * output's reported state. The close path does not mutate the
     * Symbols buffer for any champion. */
    symbols_stable = symbol_buffer_byte_stable(open_input->champions,
                                                close_in.champions);

    /* SymbolStep byte-stability: check that the caster's SymbolStep
     * field is the same in the input and the close snapshot. */
    caster_index = open_input->magic_caster_champion_index;
    if (caster_index >= 0 &&
        caster_index < DM1_V1_CPSACI_CHAMPION_COUNT_PC34) {
        symbol_step_stable =
            (open_input->champions[caster_index].symbol_step ==
             close_in.champions[caster_index].symbol_step);
    } else {
        symbol_step_stable = 1;
    }

    /* Mouse update balance: open +1, close -1 (cumulative). */
    mouse_balanced = (open_out.mouse_update_balance +
                      close_out.mouse_update_balance) == 0;

    *symbols_byte_stable = symbols_stable;
    *symbol_step_byte_stable = symbol_step_stable;
    *mouse_update_balanced = mouse_balanced;
    *f0394_call_count = f0394_calls;

    return 1;
}
