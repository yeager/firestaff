/*
 * DM1 V1 champion-panel spell-area clear-on-inventory gate.
 *
 * Contract-only, no-asset fixture. This pins the *narrow* slice of the
 * PANEL.C F0355_INVENTORY_Toggle_CPSE open/close cycle that the
 * existing champion-panel spell-area family does not cover:
 *
 *   - When a champion is the live magic caster
 *     (G0514_i_MagicCasterChampionIndex != CM1_CHAMPION_NONE) and the
 *     player toggles the inventory open via F0355 with the caster's
 *     ordinal (or via C05_CHAMPION_SPECIAL_INVENTORY), the engine
 *     MUST NOT mutate the spell-area state machine:
 *       * G0514_i_MagicCasterChampionIndex stays at the live caster
 *         index (not CM1_CHAMPION_NONE, not a different champion).
 *       * The caster's SymbolStep field stays byte-stable.
 *       * The caster's Symbols[0..3] 4-byte rune buffer stays
 *         byte-stable (no truncation, no overwrite, no zero-fill).
 *       * No F0394_MENUS_SetMagicCasterAndDrawSpellArea call fires
 *         on the open path (PANEL.C F0355 never invokes F0394).
 *       * The C017 GRAPHIC_INVENTORY blit at F0355:2376 only paints
 *         the inventory bitmap into the G0296 viewport bitmap; the
 *         F0394 / F0393 draw-state machine does not re-emit.
 *   - The F0355 close path (called with C04_CHAMPION_CLOSE_INVENTORY)
 *     MUST also leave G0514_i_MagicCasterChampionIndex, SymbolStep,
 *     and the Symbols[0..3] buffer byte-stable. The C04 close path
 *     fires F0334_INVENTORY_CloseChest and F0098_DUNGEONVIEW_Draw
 *     FloorAndCeiling but never F0394 and never F0393.
 *   - Across an open/close round trip the F0077/F0078 mouse
 *     screen-update bracketing must balance (each open path opens
 *     the screen update; the matching close path closes it).
 *   - The G0423_i_InventoryChampionOrdinal DOES change (it is
 *     explicitly set on the open path and reset to CM1_CHAMPION_NONE
 *     ordinal on the close path), but the spell-area caster state
 *     is decoupled from the inventory champion.
 *   - Negative inputs are rejected with no state change:
 *       * Open while a dead champion is the inventory champion:
 *         the F0355 dead-champion early return leaves the spell-area
 *         state machine byte-stable.
 *       * Open while G0333_B_PressingMouth || G0331_B_PressingEye
 *         is set: the F0355 mouth/eye reject leaves the spell-area
 *         state machine byte-stable.
 *       * Close on a no-inventory session (G0423 == 0 ordinal,
 *         the open path never ran): the C04 close call is rejected
 *         by the same-champion / no-ordinal guard, leaving the
 *         spell-area state machine byte-stable.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *   - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244 owns the entire
 *     open/close cycle and the dead-champion + mouth/eye rejects
 *     at lines 2280-2285.
 *   - PANEL.C F0355:2310-2329 is the close-inventory branch: fires
 *     F0334_INVENTORY_CloseChest, sets G0423 = CM1_CHAMPION_NONE
 *     ordinal, and ends in F0395_MENUS_DrawMovementArrows +
 *     F0357_COMMAND_DiscardAllInput + F0098_DUNGEONVIEW_DrawFloor
 *     AndCeiling. None of these call F0394 or F0393.
 *   - PANEL.C F0355:2370-2440 is the open-inventory branch: sets
 *     G0423 = M000_INDEX_TO_ORDINAL(champion_index), expands
 *     C017 GRAPHIC_INVENTORY into G0296, hides the floppy icon,
 *     and emits the Health/Stamina/Mana labels. None of these
 *     call F0394 or F0393.
 *   - PANEL.C F0334_INVENTORY_CloseChest is the only function that
 *     mutates G0425/G0426 and the chest list.
 *   - CASTER.C:18-32 F0394_MENUS_SetMagicCasterAndDrawSpellArea
 *     never appears in the F0355 open/close paths; F0355 leaves
 *     the spell-area state machine alone.
 *   - DEFS.H:504 G0514_i_MagicCasterChampionIndex (the live caster).
 *   - DEFS.H:5876 G0423_i_InventoryChampionOrdinal (the open-panel
 *     champion, distinct from the caster).
 *   - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY sentinel.
 *   - DEFS.H:8200 M000_INDEX_TO_ORDINAL / M001_ORDINAL_TO_INDEX.
 *   - DEFS.H:780-820 C04..C05 ordinal sentinels and the
 *     CM1_CHAMPION_NONE sentinel.
 *
 * This fixture is intentionally disjoint from the existing
 * champion-panel spell-area family:
 *   - dm1_v1_champion_panel_spell_area_overlay_pc34_compat pins the
 *     F0394 / F0393 / F0397 / F0398 draw contract (lines 1..3 of
 *     the spell area); it does NOT pin the F0355 inventory open/close
 *     cycle or the byte-stability of the caster state machine.
 *   - dm1_v1_champion_panel_portrait_state_redraw_pc34_compat pins
 *     the F0292_CHAMPION_DrawState portrait state redraw tuple.
 *   - dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat
 *     pins the F0349 panel recompute slice.
 *   - spell_area_routes (input dispatch) and dm1_v1_menu_render
 *     (orchestrator flag) gates are input-side, not state-side.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no
 * real-asset or original-DOS pixel parity claim. Contract-only.
 */

#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_SPELL_AREA_CLEAR_ON_INVENTORY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_SPELL_AREA_CLEAR_ON_INVENTORY_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPSACI_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPSACI_CHAMPION_SYMBOL_MAX_PC34 4

/* DEFS.H caster / inventory sentinels. */
#define DM1_V1_CPSACI_CHAMPION_NONE_PC34 (-1)
#define DM1_V1_CPSACI_C04_CLOSE_INVENTORY_PC34 4
#define DM1_V1_CPSACI_C05_SPECIAL_INVENTORY_PC34 5

/* Reject reasons for negative inputs. */
typedef enum DM1_V1_CpsaciRejectReasonPc34 {
    DM1_V1_CPSACI_REJECT_NONE_PC34 = 0,
    DM1_V1_CPSACI_REJECT_DEAD_INVENTORY_CHAMPION_PC34 = 1,
    DM1_V1_CPSACI_REJECT_PRESSING_MOUTH_OR_EYE_PC34 = 2,
    DM1_V1_CPSACI_REJECT_NO_INVENTORY_SESSION_PC34 = 3,
    DM1_V1_CPSACI_REJECT_INVALID_CASTER_PC34 = 4
} DM1_V1_CpsaciRejectReasonPc34;

/* Mouse screen-update bracketing balance counter. */
typedef struct DM1_V1_CpsaciMouseUpdateBalancePc34 {
    int opens;
    int closes;
    int balanced; /* opens == closes */
} DM1_V1_CpsaciMouseUpdateBalancePc34;

/* Champion view used by the F0355 input/output. */
typedef struct DM1_V1_CpsaciChampionPc34 {
    int index;
    int current_health; /* 0 = dead, otherwise alive */
    unsigned int symbol_step; /* Champion->SymbolStep */
    char symbols[DM1_V1_CPSACI_CHAMPION_SYMBOL_MAX_PC34 + 1];
} DM1_V1_CpsaciChampionPc34;

/* F0355 INVENTORY_Toggle_CPSE open-inventory input. */
typedef struct DM1_V1_CpsaciOpenInputPc34 {
    int requested_champion_index; /* P0719 arg */
    int pressing_mouth;
    int pressing_eye;
    int mouse_update_already_open; /* F0077 was already called */
    int previous_inventory_champion_ordinal; /* G0423 ordinal (0 = no inventory) */
    int magic_caster_champion_index; /* G0514 before F0355 */
    int party_champion_count; /* G0305 */
    DM1_V1_CpsaciChampionPc34 champions[DM1_V1_CPSACI_CHAMPION_COUNT_PC34];
} DM1_V1_CpsaciOpenInputPc34;

/* F0355 INVENTORY_Toggle_CPSE close-inventory input. */
typedef struct DM1_V1_CpsaciCloseInputPc34 {
    int previous_inventory_champion_ordinal; /* G0423 ordinal */
    int magic_caster_champion_index; /* G0514 before F0355 */
    int mouse_update_already_open; /* F0077 was already called */
    int party_champion_count; /* G0305 */
    DM1_V1_CpsaciChampionPc34 champions[DM1_V1_CPSACI_CHAMPION_COUNT_PC34];
} DM1_V1_CpsaciCloseInputPc34;

/* Output: post-F0355 state of the spell-area state machine. */
typedef struct DM1_V1_CpsaciOpenOutputPc34 {
    int valid;
    DM1_V1_CpsaciRejectReasonPc34 reject_reason;
    int accepted; /* 1 = F0355 took the open path; 0 = rejected */
    int new_inventory_champion_ordinal; /* G0423 after F0355 */
    int new_magic_caster_champion_index; /* G0514 after F0355 */
    int fired_f0394_set_magic_caster; /* F0394 invocation count this call */
    int fired_f0334_close_chest; /* F0334 invocation count this call */
    int fired_f0077_enable_screen_update;
    int fired_f0078_disable_screen_update;
    int fired_f0098_draw_floor_and_ceiling;
    int fired_f0136_hatch_movement_arrows_box;
    int loaded_c017_graphic_inventory_into_g0296;
    int hid_floppy_icon;
    int drew_health_stamina_mana_labels;
    int mouse_update_balance;
    int caster_symbols_byte_stable;
    int caster_symbol_step_byte_stable;
    int symbols_buffer_byte_stable; /* 4-byte runes for all 4 champions */
} DM1_V1_CpsaciOpenOutputPc34;

typedef struct DM1_V1_CpsaciCloseOutputPc34 {
    int valid;
    DM1_V1_CpsaciRejectReasonPc34 reject_reason;
    int accepted; /* 1 = F0355 took the close path; 0 = rejected */
    int new_inventory_champion_ordinal; /* G0423 after F0355 */
    int new_magic_caster_champion_index; /* G0514 after F0355 */
    int fired_f0394_set_magic_caster; /* F0394 invocation count this call */
    int fired_f0334_close_chest;
    int fired_f0077_enable_screen_update;
    int fired_f0078_disable_screen_update;
    int fired_f0098_draw_floor_and_ceiling;
    int fired_f0395_draw_movement_arrows;
    int fired_f0357_discard_all_input;
    int mouse_update_balance;
    int caster_symbols_byte_stable;
    int caster_symbol_step_byte_stable;
    int symbols_buffer_byte_stable; /* 4-byte runes for all 4 champions */
} DM1_V1_CpsaciCloseOutputPc34;

/* Static contract reference. */
typedef struct DM1_V1_CpsaciContractPc34 {
    int contract_only;
    int champion_count;
    int symbol_max;
    int champion_none;
    int c04_close_inventory;
    int c05_special_inventory;
    int spell_area_x0; /* 224 */
    int spell_area_x1; /* 319 */
    int spell_area_y0; /* 42 */
    int spell_area_y1; /* 74 */
    int c017_graphic_inventory;
    const char *inventory_toggle_anchor;
    const char *close_branch_anchor;
    const char *open_branch_anchor;
    const char *caster_anchor;
    const char *mouse_bracketing_anchor;
    const char *ordinal_helpers_anchor;
    const char *dead_champion_reject_anchor;
    const char *mouth_eye_reject_anchor;
} DM1_V1_CpsaciContractPc34;

const DM1_V1_CpsaciContractPc34 *
dm1_v1_champion_panel_spell_area_clear_on_inventory_contract_pc34(void);

const char *
dm1_v1_champion_panel_spell_area_clear_on_inventory_source_evidence_pc34(void);

int dm1_v1_champion_panel_spell_area_clear_on_inventory_open_pc34(
    const DM1_V1_CpsaciOpenInputPc34 *input,
    DM1_V1_CpsaciOpenOutputPc34 *out);

int dm1_v1_champion_panel_spell_area_clear_on_inventory_close_pc34(
    const DM1_V1_CpsaciCloseInputPc34 *input,
    DM1_V1_CpsaciCloseOutputPc34 *out);

int dm1_v1_champion_panel_spell_area_clear_on_inventory_round_trip_pc34(
    const DM1_V1_CpsaciOpenInputPc34 *open_input,
    int *post_close_inventory_ordinal,
    int *post_close_magic_caster_index,
    int *symbols_byte_stable,
    int *symbol_step_byte_stable,
    int *mouse_update_balanced,
    int *f0394_call_count);

#ifdef __cplusplus
}
#endif

#endif
