#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_PC34_COMPAT_H

/*
 * DM1 V1 mirror-candidate C546 eye-route C09-leader-hand-chest
 * slot-swap gate.
 *
 * Lane: a C040 mirror candidate is live (G0299_ui_CandidateChampionOrdinal
 * != 0), the leader's C01_SLOT_ACTION_HAND contains a C09 container
 * (chest), and the user clicks the C546 eye zone. PANEL.C F0352 takes the
 * eye-press branch, calls PANEL.C F0342_INVENTORY_DrawPanel_Object with
 * C1_TRUE pressingEye, which routes through the CHANGE7_27_FIX
 * pressing-eye/pressing-mouth branch that always closes the prior open
 * chest (CHEST.C F0334_INVENTORY_CloseChest, F0334:79-130) and then
 * calls CHEST.C F0333_INVENTORY_OpenAndDrawChest with P0694_B_PressingEye
 * = C1_TRUE so the leader-hand chest is opened with the C145 action-hand
 * open-icon suppressed (F0333:43-46). The C040 candidate must remain
 * live (G0299 unchanged) during the eye press; on release, PANEL.C F0353
 * calls PANEL.C F0347_INVENTORY_DrawPanel which always closes the chest
 * via F0334 first (CHANGE8_09_FIX, F0347:1650) and then re-derives the
 * panel from G0299 — restoring the C040 resurrect/reincarnate panel via
 * PANEL.C F0346 (F0347:1654). The eye press/release therefore performs
 * an auto-slot-swap on three presentation slots:
 *   (a) the G0424_i_PanelContent slot swaps 568 (C040) -> 569
 *       (PANEL_CHEST) on press, then back to 568 on release,
 *   (b) the v1OpenChestThing slot swaps to the leader-hand chest on
 *       press (with v1OpenChestOpenedByEye = 1), and is auto-cleared
 *       to C0xFFFF_THING_NONE on release (F0347 -> F0334),
 *   (c) the C09 action-hand icon stays at C144 (closed) during the
 *       eye route because P0694_B_PressingEye suppresses the C145
 *       open-icon draw in F0333:43-46.
 *
 * The gate pins the assertion surface:
 *   - G0299_ui_CandidateChampionOrdinal is byte-stable across the
 *     press and release legs (C040 candidate not consumed),
 *   - G0424_i_PanelContent reaches 569 on press and 568 on release,
 *   - v1OpenChestThing reaches the leader-hand chest with
 *     v1OpenChestOpenedByEye = 1 on press and C0xFFFF_THING_NONE on
 *     release,
 *   - G0425_aT_ChestSlots[i] (i = 0..7) is populated from the chest
 *     Slot list on press and re-cleared to C0xFFFF_THING_NONE on
 *     release,
 *   - the C09 action-hand icon stays at C144 across press and release
 *     (the F0333:43-46 P0694_B_PressingEye suppression guard fires
 *     exactly once),
 *   - the eye-icon graphic reaches C203 (looking) on press and
 *     C202 (not looking) on release,
 *   - the leader-hand object is preserved across both legs,
 *   - G0331_B_PressingEye is set on press and cleared on release,
 *   - F0334_INVENTORY_CloseChest is called exactly once (from F0347
 *     on release), and F0333_INVENTORY_OpenAndDrawChest is called
 *     exactly once (from F0342 on press),
 *   - the F0352/F0353 viewport redraws balance (2 calls total).
 *
 * ReDMCSB anchors (PC 3.4 path, MEDIA529 family):
 *   - PANEL.C F0352:2111-2159 (F0352_INVENTORY_ProcessCommand71_ClickOnEye)
 *     dispatches the C546 eye click, sets G0331_B_PressingEye, draws
 *     C203_ICON_EYE_LOOKING, calls F0342 with the leader hand object and
 *     C1_TRUE pressingEye, then F0097_DUNGEONVIEW_DrawViewport.
 *   - PANEL.C F0353:2162-2192 (F0353_INVENTORY_DrawStopPressingEye)
 *     draws C202_ICON_EYE_NOT_LOOKING, calls F0347, F0097, restores
 *     leader hand object name, and M523_MOUSE_ShowPointer.
 *   - PANEL.C F0342:1055-1180 (F0342_INVENTORY_DrawPanel_Object) on
 *     pressingEye=C1_TRUE calls F0334 first (CHANGE7_27_FIX), then
 *     dispatches C09_THING_TYPE_CONTAINER to F0333 with
 *     P0694_B_PressingEye.
 *   - CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest) populates
 *     G0425_aT_ChestSlots[0..7] from the container Slot list (up to 8
 *     visible, CHANGE8_08_FIX) and skips the C09 action-hand C145
 *     open-icon draw when P0694_B_PressingEye is set (F0333:43-46).
 *   - CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest) clears
 *     G0426_T_OpenChest, rewrites the container Slot list from the
 *     non-empty G0425 entries, and clears each G0425 slot to
 *     C0xFFFF_THING_NONE (CHANGE8_09_FIX).
 *   - PANEL.C F0347:1639-1693 (F0347_INVENTORY_DrawPanel) always calls
 *     F0334 first (CHANGE8_09_FIX, F0347:1650) and then routes to
 *     F0346_INVENTORY_DrawPanel_ResurrectReincarnate when G0299 != 0
 *     (F0347:1654).
 *   - PANEL.C F0346:1619-1637 (F0346_INVENTORY_DrawPanel_ResurrectReincarnate)
 *     sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE and
 *     draws the C040 resurrect/reincarnate panel chrome.
 *   - DEFS.H:2088 C040_COMMAND_OPEN_RESURRECT_REINCARNATE_PANEL,
 *     C160_COMMAND_CLICK_IN_RESURRECT_REINCARNATE_PANEL_YES,
 *     C546_ZONE_EYE, M568_PANEL_RESURRECT_REINCARNATE,
 *     C09_THING_TYPE_CONTAINER, C144_ICON_CONTAINER_CHEST_CLOSED,
 *     C145_ICON_CONTAINER_CHEST_OPEN, C202_ICON_EYE_NOT_LOOKING,
 *     C203_ICON_EYE_LOOKING, C537..C544 chest slot zones,
 *     G0299_ui_CandidateChampionOrdinal, G0424_i_PanelContent,
 *     G0425_aT_ChestSlots[8], G0426_T_OpenChest.
 *   - DEFS.H:2091 C0xFFFF_THING_NONE, C0xFFFE_THING_ENDOFLIST.
 *
 * Disjoint from:
 *   - dm1_v1_mirror_candidate_c040_eye_live_candidate_pc34_compat
 *       (C040 candidate live + eye press/release with a chest ALREADY
 *       open via the normal F0333 path; this lane covers a leader-hand
 *       chest on the eye press and asserts the F0333:43-46
 *       P0694_B_PressingEye suppression branch),
 *   - dm1_v1_mirror_candidate_c160_close_while_rotation_pending_pc34_compat
 *       (C160 Yes click while leader rotation is in flight),
 *   - dm1_v1_mirror_candidate_close_while_resurrect_pending_with_inventory_pickup
 *       (close+pickup overlap),
 *   - dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit
 *       (panel redraw after F0355 close-inventory dispatch),
 *   - dm1_v1_chest_eye_open_to_action_hand_switch
 *       (chest A in action-hand + chest B in leader hand + eye click
 *       on leader hand; the swap target is the action-hand chest, not
 *       a C040 candidate),
 *   - dm1_v1_chest_auto_close_on_leader_death
 *       (F0319 -> F0334 close during death, no eye route),
 *   - dm1_v1_mirror_candidate_resurrect_chest_close_order
 *       (resurrect + chest-close ordering, no eye route),
 *   - dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation
 *       (F0302 leader rotation during resurrect-pending, no eye route),
 *   - dm1_v1_mirror_candidate_teleporter_survival
 *       (party-teleport while C040 panel is alive, no eye route),
 *   - dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation
 *       (C045 accept while leader rotation is in flight),
 *   - dm1_v1_mirror_candidate_inventory_click_during_rotation
 *       (C038 scroll pickup while rotation is in flight),
 *   - dm1_v1_mirror_candidate_double_open_close_guard
 *       (F0333 same-chest reopen guard, no eye route),
 *   - dm1_v1_mirror_candidate_chest_close_pending_panel
 *       (C160 close while panel is still drawing),
 *   - dm1_v1_mirror_candidate_c159_click_rotation_combo
 *       (C159 champion-icon click + rotation combo, no eye route),
 *   - dm1_v1_mirror_candidate_c545_accept_during_rotation
 *       (C045 food/water accept while rotation in flight),
 *   - dm1_v1_mirror_candidate_c545_drop_while_panel_live
 *       (C545 drop while C040 panel live, no eye route),
 *   - dm1_v1_mirror_candidate_full_chain
 *       (full C160 close chain, no eye route),
 *   - dm1_v1_mirror_candidate_close_button
 *       (C160 close click without eye press),
 *   - dm1_v1_mirror_candidate_keyboard_browse_occupied_slot
 *       (keyboard browse with occupied C30 slot, no eye route),
 *   - dm1_v1_mirror_candidate_party_direction
 *       (party-direction keyboard rotation, no eye route),
 *   - dm1_v1_mirror_candidate_resurrect_double_candidate_race
 *       (double-candidate race on resurrect),
 *   - dm1_v1_mirror_candidate_resurrect_champion_switch_reopen
 *       (resurrect + champion switch + C040 reopen),
 *   - dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate
 *       (close then rotate then reopen).
 *
 * Source-locked contract-only: no real-asset or original-DOS pixel
 * parity claim. The state model is a synthetic
 * M11_InventoryState-shaped local struct with byte-level fields, and
 * the dispatch loop is the ReDMCSB-derived contract-only runtime
 * function `dm1_v1_mirror_candidate_eye_slot_swap_run_pc34`.
 */

#define DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34 8

typedef struct DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 {
    int contractOnly;
    int candidateChampionOrdinal;     /* G0299_ui_CandidateChampionOrdinal */
    int leaderHandThing;              /* C01_SLOT_ACTION_HAND container */
    int openChestThing;               /* G0426_T_OpenChest */
    int v1OpenChestOpenedByEye;       /* P0694_B_PressingEye branch flag */
    int chestSlots[DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34];
    int panelContent;                 /* G0424_i_PanelContent */
    int pressingEye;                  /* G0331_B_PressingEye */
    int eyeIconGraphic;               /* C202/C203 */
    int ignoreMouseMovements;         /* G0597_B_IgnoreMouseMovements */
    int pointerHidden;                /* MOUSE pointer state */
    int f0333OpenCount;               /* F0333_INVENTORY_OpenAndDrawChest */
    int f0334CloseCount;              /* F0334_INVENTORY_CloseChest */
    int objectPanelDrawCount;         /* F0342 (container route) */
    int c040RedrawCount;              /* F0346/F0347 -> F0346 path */
    int viewportRedrawCount;          /* F0097 (F0352 + F0353) */
    /* Sentinel slot for the action-hand icon. C144 (closed) on eye
     * route; C145 (open) only when a chest is opened via the non-eye
     * F0333 path. The lane pins that the C144 -> C145 swap never
     * happens across the F0352/F0353 eye press/release legs. */
    int c09ActionHandIcon;
} DM1_V1_MirrorCandidateEyeSlotSwapStatePc34;

typedef struct DM1_V1_MirrorCandidateEyeSlotSwapResultPc34 {
    int accepted;
    int assertionCount;
    /* C040 candidate is byte-stable across press/release. */
    int candidateOrdinalBefore;
    int candidateOrdinalAfterPress;
    int candidateOrdinalAfterRelease;
    /* G0424 panel-content slot-swap. */
    int panelContentBefore;
    int panelContentAfterPress;
    int panelContentAfterRelease;
    /* G0426 + v1OpenChestOpenedByEye slot-swap. */
    int openChestBefore;
    int openChestAfterPress;
    int openChestAfterRelease;
    int openedByEyeAfterPress;
    int openedByEyeAfterRelease;
    /* G0425 chest-slot chain. */
    int chestSlotsPopulatedOnPress;
    int chestSlotsClearedOnRelease;
    /* C09 action-hand icon suppression. */
    int c09IconBefore;
    int c09IconAfterPress;
    int c09IconAfterRelease;
    /* Eye-zone icon redraw. */
    int eyeIconBefore;
    int eyeIconAfterPress;
    int eyeIconAfterRelease;
    /* G0331_B_PressingEye flag. */
    int pressingEyeAfterPress;
    int pressingEyeAfterRelease;
    /* Leader-hand object stability. */
    int leaderHandPreserved;
    /* Pointer/mouse ignore flags. */
    int pointerHiddenAfterPress;
    int pointerHiddenAfterRelease;
    int ignoreMouseAfterPress;
    int ignoreMouseAfterRelease;
    /* F0333/F0334 dispatch counts. */
    int f0333CalledOnce;
    int f0334CalledOnce;
    /* F0342 (object panel draw) and F0346 (C040 redraw) counts. */
    int objectPanelDrawnOnPress;
    int c040RedrawnOnRelease;
    /* F0352/F0353 viewport redraw balance (one press + one release). */
    int viewportRedrawCount;
} DM1_V1_MirrorCandidateEyeSlotSwapResultPc34;

typedef struct DM1_V1_MirrorCandidateEyeSlotSwapSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *f0352Anchor;
    const char *f0353Anchor;
    const char *f0342Anchor;
    const char *f0347Anchor;
    const char *f0346Anchor;
    const char *f0333Anchor;
    const char *f0334Anchor;
    int c040PanelContent;
    int chestPanelContent;
    int c546EyeZone;
    int eyeLookingGraphic;
    int eyeNotLookingGraphic;
    int c144ClosedChestIcon;
    int c145OpenChestIcon;
    int c09ThingTypeContainer;
} DM1_V1_MirrorCandidateEyeSlotSwapSpecPc34;

const DM1_V1_MirrorCandidateEyeSlotSwapSpecPc34 *
dm1_v1_mirror_candidate_eye_slot_swap_spec_pc34(void);

const char *
dm1_v1_mirror_candidate_eye_slot_swap_source_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_eye_slot_swap_non_overlap_pc34(void);

void dm1_v1_mirror_candidate_eye_slot_swap_init_pc34(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state);

int dm1_v1_mirror_candidate_eye_slot_swap_run_pc34(
    DM1_V1_MirrorCandidateEyeSlotSwapResultPc34 *out);

#endif
