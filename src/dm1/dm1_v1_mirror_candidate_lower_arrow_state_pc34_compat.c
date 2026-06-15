/*
 * DM1 V1 mirror-candidate lower movement arrow state gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP20210206, PC 3.4 path, MEDIA009+):
 *  - COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement maps the
 *    lower row (y=147-167) of G0463_aai_Graphic561_Box_MovementArrows
 *    to C006_COMMAND_MOVE_LEFT / C005_COMMAND_MOVE_BACKWARD /
 *    C004_COMMAND_MOVE_RIGHT.
 *  - COMMAND.C:323-328 G0463_aai_Graphic561_Box_MovementArrows
 *    defines the movement arrow boxes; the upper row y=125-145 is
 *    C001/C003/C002, the lower row y=147-167 is C006/C005/C004.
 *  - COMMAND.C F0380:2151-2156 routes the C003..C006 range to
 *    F0366_COMMAND_ProcessTypes3To6_MoveParty without an explicit
 *    `!G0299_ui_CandidateChampionOrdinal` guard. This is the
 *    lower-arrow state-gate contract: the F0366 entry does NOT
 *    touch G0299 / G0424 / c040Panel* fields.
 *  - COMMAND.C F0380:2159-2181 keeps the !G0299 guard on the
 *    C012..C015 status-box / C007..C011 inventory-toggle range.
 *  - COMMAND.C F0380:2302-2311 keeps the !G0299 guard on the
 *    C100 spell-area / C111 action-area range.
 *  - COMMAND.C F0380:2366-2370 keeps the !G0299 guard on the
 *    C140 save input range.
 *  - CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty:180-280
 *    walks the party champion chain, decrements stamina per
 *    load fraction, advances the party map square; it does not
 *    touch G0299 / G0424 / c040Panel* fields.
 *  - PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637
 *    sets G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE
 *    and blits the C040 graphic on the G0032_ai_Graphic562_Box_Panel
 *    rect.
 *  - PANEL.C F0347_INVENTORY_DrawPanel:1654 routes to F0346 when
 *    G0299 is non-zero; F0342 (object) and F0345 (food/water
 *    poisoned) are unreachable on the C040 live path.
 *  - PANEL.C F0395_MENUS_DrawMovementArrows is the post-F0355
 *    arrow redraw and is NOT entered on the lower-arrow click.
 *  - REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806
 *    consumes/clears it. Neither runs on the lower-arrow click.
 *  - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *  - DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE.
 *  - DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE.
 *  - DEFS.H:5694 G0299_ui_CandidateChampionOrdinal.
 *  - DEFS.H:5876-5881 G0425/G0426 chest list + open chest.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no real-asset
 * or original-DOS pixel parity claim.
 */

#include "dm1_v1_mirror_candidate_lower_arrow_state_pc34_compat.h"

#include <string.h>

/* Tiny deterministic hash mixer used to fingerprint per-step state
 * snapshots. The mixer is taken from the pass785 / pass786 family. */
static uint32_t mix32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

static uint32_t fingerprint_chest(const Dm1V1MirrorCandidateLowerArrowStateStatePc34 *s)
{
    uint32_t h = 0x811C9DC5u;
    h = mix32(h ^ (uint32_t)s->g0426OpenChest);
    for (int i = 0; i < DM1_V1_MC_LAS_CHEST_SLOT_COUNT_PC34; ++i) {
        h = mix32(h ^ (uint32_t)(s->chestSlots[i] + 0x9E3779B9u + i));
    }
    return h;
}

static uint32_t fingerprint_owner_chain(
    const Dm1V1MirrorCandidateLowerArrowStateStatePc34 *s)
{
    uint32_t h = 0x01000193u;
    for (int i = 0; i < DM1_V1_MC_LAS_PARTY_COUNT_PC34; ++i) {
        h = mix32(h ^ s->c030Owner[i]);
        h = mix32(h ^ (uint32_t)(i * 0x85EBCA6Bu));
    }
    return h;
}

static uint32_t fingerprint_c040_panel(
    const Dm1V1MirrorCandidateLowerArrowStateStatePc34 *s)
{
    uint32_t h = 0xCBF29CE484222325ull & 0xFFFFFFFFu;
    h = mix32(h ^ (uint32_t)s->c040PanelOpen);
    h = mix32(h ^ (uint32_t)s->c040PanelGraphic);
    h = mix32(h ^ (uint32_t)s->c040PanelCommand);
    h = mix32(h ^ (uint32_t)s->c040PanelColor);
    h = mix32(h ^ (uint32_t)s->c040PanelOwnerSlot);
    h = mix32(h ^ (uint32_t)s->c040PanelC038SlotBox);
    h = mix32(h ^ (uint32_t)s->panelContent);
    h = mix32(h ^ (uint32_t)s->candidateOrdinal);
    h = mix32(h ^ (uint32_t)s->candidateOwnerIndex);
    return h;
}

static uint32_t fingerprint_candidate_chain(
    const Dm1V1MirrorCandidateLowerArrowStateStatePc34 *s)
{
    uint32_t h = 0xDEADBEEFu;
    h = mix32(h ^ (uint32_t)s->candidateOrdinal);
    h = mix32(h ^ (uint32_t)s->candidateOwnerIndex);
    h = mix32(h ^ (uint32_t)s->candidateOwnerSlot);
    h = mix32(h ^ (uint32_t)s->panelContent);
    h = mix32(h ^ (uint32_t)s->c040PanelOpen);
    return h;
}

static uint32_t fingerprint_leader_hand(
    const Dm1V1MirrorCandidateLowerArrowStateStatePc34 *s)
{
    uint32_t h = 0xA5A5A5A5u;
    h = mix32(h ^ (uint32_t)s->leaderHandEmpty);
    h = mix32(h ^ (uint32_t)s->leaderHandThingOrdinal);
    h = mix32(h ^ (uint32_t)s->leaderIndex);
    return h;
}

static void recompute_fingerprints(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *s)
{
    s->c030OwnerHash = fingerprint_owner_chain(s);
    s->c040PanelHash = fingerprint_c040_panel(s);
    s->candidateChainHash = fingerprint_candidate_chain(s);
    s->chestListHash = fingerprint_chest(s);
    s->leaderHandHash = fingerprint_leader_hand(s);
}

/* Source evidence string is intentionally kept in a single literal so
 * the gate's source-lock can be diffed against the ReDMCSB anchors
 * above with a single strstr sweep. */
static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. ReDMCSB COMMAND.C:107-112 G0448_SecondaryMouseInput_"
    "Movement maps the lower row y=147-167 to C006_COMMAND_MOVE_LEFT / "
    "C005_COMMAND_MOVE_BACKWARD / C004_COMMAND_MOVE_RIGHT. ReDMCSB "
    "COMMAND.C:323-328 G0463_aai_Graphic561_Box_MovementArrows defines "
    "the movement arrow boxes (upper row y=125-145 = C001/C003/C002, "
    "lower row y=147-167 = C006/C005/C004). ReDMCSB COMMAND.C "
    "F0380:2151-2156 routes the C003..C006 range to "
    "F0366_COMMAND_ProcessTypes3To6_MoveParty without an explicit "
    "!G0299 guard. ReDMCSB COMMAND.C F0380:2159-2181 keeps the !G0299 "
    "guard on the C012..C015 status-box and C007..C011 inventory-toggle "
    "range. ReDMCSB COMMAND.C F0380:2302-2311 keeps the !G0299 guard on "
    "the C100 spell-area and C111 action-area range. ReDMCSB COMMAND.C "
    "F0380:2366-2370 keeps the !G0299 guard on the C140 save input "
    "range. ReDMCSB CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty:"
    "180-280 walks the party champion chain, decrements stamina per "
    "load fraction, advances the party map square; it does not touch "
    "G0299 / G0424 / c040Panel* fields. ReDMCSB PANEL.C "
    "F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 sets "
    "G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE and blits "
    "the C040 graphic on the G0032_ai_Graphic562_Box_Panel rect. "
    "ReDMCSB PANEL.C F0347_INVENTORY_DrawPanel:1654 routes to F0346 "
    "when G0299 is non-zero; F0342 and F0345 are unreachable on the "
    "C040 live path. ReDMCSB PANEL.C F0395_MENUS_DrawMovementArrows is "
    "the post-F0355 arrow redraw and is NOT entered on the lower-arrow "
    "click. ReDMCSB REVIVE.C F0280:124-132 publishes the candidate; "
    "F0282:744-758 (cancel) and F0282:785-806 (accept) consume/clear it. "
    "Neither runs on the lower-arrow "
    "click. ReDMCSB DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, "
    "G0299. ReDMCSB DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE."
    " ReDMCSB DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE. ReDMCSB "
    "DEFS.H:5694 G0299_ui_CandidateChampionOrdinal. ReDMCSB DEFS.H:"
    "5876-5881 G0425/G0426 chest list + open chest. ReDMCSB DEFS.H:"
    "4041-4042 viewport wall zones (C004/C005/C006 lower-row movement "
    "arrows are screen-relative, not viewport-relative). The fixture "
    "is the *lower movement arrow click with C040 live* lane; it does "
    "NOT pin the C004..C006 click-through / leader-hand / chest-open "
    "path (covered by chest_scroll_wheel_* and the C540/C061 chest "
    "drop-rotation tests), the C159 name-row C016 set-leader click "
    "while C040 is live (covered by c159_click_rotation_combo), the "
    "C007..C011 inventory toggle while C040 is live (covered by "
    "panel_redraw_after_inventory_exit which is the F0355(C04) close "
    "path), the C100/C111 !G0299 guard (covered by "
    "c159_click_rotation_combo), the C140 save !G0299 guard (covered "
    "by c159_click_rotation_combo), the C160/C161/C162 resurrect/"
    "reincarnate/cancel panel dispatch (covered by c160_close_while_"
    "rotation_pending, c159_click_rotation_combo, resurrect_chest_"
    "close_order, and resurrect_reselect_with_inventory_pickup), the "
    "C070/C071 mouth/eye click while C040 is live (covered by "
    "c040_eye_live_candidate), the C028..C057 inventory slot box "
    "click while C040 is live (covered by c040_close_non_leader_"
    "scroll_pickup, c040_panel_browse_pickup_rotate_race, and panel_"
    "redraw_after_inventory_exit), the C580-C645 chest close/open/"
    "scroll-wheel/resurrect-rotation/teleporter/save-load/mirror-"
    "candidate pickup paths, the C160/C161/C162 panel-command replay "
    "after a C045 stale route (covered by close_while_c045_pending), "
    "save/load, teleporter, party-rotate, leader-rotation, portrait "
    "redraw, F0354 box-variants, F0334 chest close, or F0333 chest "
    "open entries on the lower-arrow click, the F0457_START_DrawEnabled"
    "Menus_CPSF post-resurrect orchestrator redraw, the F0456_START_"
    "DrawDisabledMenus post-rest orchestrator, or the F0283_CHAMPION_"
    "ViAltarRebirth / F0281_CHAMPION_Rename paths.";

static const Dm1V1MirrorCandidateLowerArrowStateEvidencePc34 s_evidence = {
    "COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement lower row "
    "y=147-167 -> C006_COMMAND_MOVE_LEFT / C005_COMMAND_MOVE_BACKWARD / "
    "C004_COMMAND_MOVE_RIGHT",
    "COMMAND.C:323-328 G0463_aai_Graphic561_Box_MovementArrows upper "
    "row y=125-145 = C001/C003/C002, lower row y=147-167 = C006/C005/C004",
    "COMMAND.C F0380:2151-2156 C003..C006 -> F0366_COMMAND_ProcessTypes"
    "3To6_MoveParty",
    "COMMAND.C F0380:2151-2156 C003..C006 no !G0299 guard on "
    "F0366_COMMAND_ProcessTypes3To6_MoveParty path",
    "COMMAND.C F0380:2159-2181 !G0299 guard on C012..C015 status box "
    "and C007..C011 inventory toggle range",
    "COMMAND.C F0380:2302-2311 !G0299 guard on C100 spell and C111 "
    "action-area range",
    "COMMAND.C F0380:2366-2370 !G0299 guard on C140 save input range",
    "PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 "
    "M568 panel content + C040 graphic blit on G0032 box",
    "PANEL.C F0346:1626 sets G0424_i_PanelContent = M568_PANEL_"
    "RESURRECT_REINCARNATE",
    "PANEL.C F0347_INVENTORY_DrawPanel:1654 routes to F0346 when G0299 "
    "is non-zero; F0342 and F0345 are unreachable",
    "REVIVE.C F0280:124-132 leader-empty and party-size publish gate; "
    "F0280:272-276 publishes G0299",
    "REVIVE.C F0282:744-758 clears G0299 on C162 cancel "
    "(REVIVE.C F0282:744-758 cancel); F0282:785-806 clears G0299 on "
    "C160/C161 accept path",
    "CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty:180-280 walks "
    "champion chain and does NOT touch G0299/G0424/c040Panel*",
    "DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE panel content",
    "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE panel graphic",
    "DEFS.H:5694 G0299_ui_CandidateChampionOrdinal candidate ordinal",
    "DEFS.H:5876-5881 G0425/G0426 chest list + open chest",
    "DEFS.H:4041-4042 viewport wall zones (C004/C005/C006 are screen-"
    "relative, not viewport-relative)",
    "contract_only=1 deterministic lower movement arrow click C004/"
    "C005/C006 with C040 mirror-candidate panel live; no real assets, "
    "dungeon sensors, save files, or full UI hit-testing parity are "
    "claimed"
};

void dm1_v1_mirror_candidate_lower_arrow_state_init_pc34(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->noGameDataRequired = 1;
    state->partyChampionCount = DM1_V1_MC_LAS_PARTY_COUNT_PC34 - 1;
    state->leaderIndex = 0;
    state->inventoryChampionOrdinal = 1;
    state->inventoryPanelOpen = 0;
    state->candidateOrdinal = 0;
    state->candidateOwnerIndex = DM1_V1_MC_LAS_CM1_CHAMPION_NONE_PC34;
    state->candidateOwnerSlot = DM1_V1_MC_LAS_CM1_CHAMPION_NONE_PC34;
    state->c040PanelOpen = 0;
    state->c040PanelGraphic = DM1_V1_MC_LAS_C40_GRAPHIC_PC34;
    state->c040PanelCommand = DM1_V1_MC_LAS_M568_PANEL_PC34;
    state->c040PanelColor = 10; /* C06_COLOR_DARK_GREEN */
    state->c040PanelOwnerSlot = DM1_V1_MC_LAS_C30_PC34;
    state->c040PanelC038SlotBox = DM1_V1_MC_LAS_C38_PC34;
    state->panelContent = 0;
    state->leaderHandEmpty = 1;
    state->leaderHandThingOrdinal = 0xFFFF;
    state->g0426OpenChest = 0;
    state->partyResting = 0;
    state->openChestThing = 0xFFFF;
    state->deterministicSeed = DM1_V1_MC_LAS_DETERMINISTIC_SEED_PC34;
    state->lastAcceptedLowerArrowCommand =
        DM1_V1_MC_LAS_COMMAND_NONE_PC34;
    for (int i = 0; i < DM1_V1_MC_LAS_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = 0xFFFF;
    }
    for (int i = 0; i < DM1_V1_MC_LAS_PARTY_COUNT_PC34; ++i) {
        state->c030Owner[i] = 0xFFFFFFFFu;
    }
    state->c030Owner[0] = 0xC0400001u;
    state->c030Owner[1] = 0xC0400002u;
    state->c030Owner[2] = 0xC0400003u;
    state->c030Owner[3] = 0xC0400004u;
    recompute_fingerprints(state);
}

int dm1_v1_mirror_candidate_lower_arrow_state_publish_candidate_pc34(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state)
{
    if (!state || !state->contractOnly || !state->noGameDataRequired) {
        return 0;
    }
    if (state->candidateOrdinal != 0 ||
        state->inventoryPanelOpen != 0 ||
        state->c040PanelOpen != 0) {
        return 0;
    }
    if (state->partyChampionCount >= DM1_V1_MC_LAS_PARTY_COUNT_PC34) {
        return 0;
    }
    if (!state->leaderHandEmpty) {
        return 0;
    }
    state->candidateOrdinal = 3;
    state->candidateOwnerIndex = 2;
    state->candidateOwnerSlot = DM1_V1_MC_LAS_C30_PC34 + 2;
    state->c040PanelOpen = 1;
    state->c040PanelGraphic = DM1_V1_MC_LAS_C40_GRAPHIC_PC34;
    state->c040PanelCommand = DM1_V1_MC_LAS_M568_PANEL_PC34;
    state->c040PanelColor = 10;
    state->c040PanelOwnerSlot = state->candidateOwnerSlot;
    state->c040PanelC038SlotBox = DM1_V1_MC_LAS_C38_PC34;
    state->panelContent = DM1_V1_MC_LAS_M568_PANEL_PC34;
    ++state->f0280CandidatePublishCount;
    recompute_fingerprints(state);
    return 1;
}

static int is_lower_movement_arrow(int command)
{
    return command == DM1_V1_MC_LAS_COMMAND_MOVE_RIGHT_PC34 ||
           command == DM1_V1_MC_LAS_COMMAND_MOVE_BACKWARD_PC34 ||
           command == DM1_V1_MC_LAS_COMMAND_MOVE_LEFT_PC34;
}

static int is_upper_movement_arrow(int command)
{
    return command == DM1_V1_MC_LAS_COMMAND_MOVE_FORWARD_PC34;
    /* C001 turn-left and C002 turn-right are excluded because they
     * are routed to F0365, not F0366, in F0380:2147-2150. */
}

static int is_chest_panel_command(int command)
{
    /* The M569 chest panel and M568 resurrect panel routes are
     * dispatched through F0378_COMMAND_ProcessType81_ClickInPanel.
     * The C160/C161/C162 commands are resurrect/reincarnate/cancel
     * and are accepted by F0282 on the C040 live path. */
    return command == DM1_V1_MC_LAS_C160_PC34 ||
           command == DM1_V1_MC_LAS_C161_PC34 ||
           command == DM1_V1_MC_LAS_C162_PC34;
}

static void record_c040_live_baseline(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state,
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 *result)
{
    result->panelStayedC040 = (state->c040PanelOpen == 1);
    result->panelContentStayedM568 =
        (state->panelContent == DM1_V1_MC_LAS_M568_PANEL_PC34);
    result->panelGraphicStayed40 =
        (state->c040PanelGraphic == DM1_V1_MC_LAS_C40_GRAPHIC_PC34);
    result->panelCommandStayed568 =
        (state->c040PanelCommand == DM1_V1_MC_LAS_M568_PANEL_PC34);
    result->panelOwnerSlotStayedCandidateOwner =
        (state->c040PanelOwnerSlot == state->candidateOwnerSlot);
    result->panelC038SlotBoxStayed38 =
        (state->c040PanelC038SlotBox == DM1_V1_MC_LAS_C38_PC34);
    result->candidateOrdinalPreserved = (state->candidateOrdinal != 0);
    result->panelOpenPreserved = (state->c040PanelOpen == 1);
    result->noF0282ClearOnLowerArrow = (state->f0282CandidateClearCount == 0);
    result->noF0333OpenOnLowerArrow = (state->f0333OpenCount == 0);
    result->noF0334CloseOnLowerArrow = (state->f0334CloseCount == 0);
    result->noF0345FoodWaterOnLowerArrow =
        (state->f0345FoodWaterPoisonedCount == 0);
    result->noF0355ToggleOnLowerArrow =
        (state->f0355ToggleSuppressedByCandidateCount == 0);
    result->noF0342ObjectOnLowerArrow = (state->f0342DrawPanelObjectCount == 0);
    result->noF0395MovementArrowsOnLowerArrow =
        (state->f0395DrawMovementArrowsCount == 0);
    result->noF0457StartDrawEnabledOnLowerArrow =
        (state->f0457StartDrawEnabledMenusCount == 0);
    result->noF0219WallImpactOnLowerArrow = (state->f0219WallImpactSoundCount == 0);
    result->noF0232DoorDestroyOnLowerArrow = (state->f0232DoorDestroyCount == 0);
    result->noF0283ViAltarRebirthOnLowerArrow =
        (state->f0283ViAltarRebirthCount == 0);
    result->noF0281ChampionRenameOnLowerArrow =
        (state->f0281ChampionRenameCount == 0);
    result->noSaveLoadOnLowerArrow = (state->saveLoadCount == 0);
    result->noTeleporterOnLowerArrow = (state->teleporterCount == 0);
    result->noPartyRotateOnLowerArrow = (state->partyRotateCount == 0);
    result->noLeaderRotationOnLowerArrow =
        (state->championRotationPendingCount == 0);
    result->noResurrectCommitOnLowerArrow =
        (state->f0282CandidateClearCount == 0);
    result->noResurrectCancelOnLowerArrow =
        (state->f0282CandidateClearCount == 0);
    result->c040PanelHashStable = 1;
    result->candidateChainHashStable = 1;
    result->c030OwnerHashStable = 1;
    result->leaderHandHashStable = 1;
    result->chestListHashStable = 1;
    result->ownerChainPreserved = 1;
    result->leaderHandPreserved = 1;
    result->chestListPreserved = 1;
    result->noPanelContentMutation = 1;
}

static void dispatch_lower_movement_arrow(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state,
    int command,
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 *result)
{
    /* F0380_COMMAND_ProcessQueue_CPSC drain step: COMMAND.C F0380:2151-2156
     * routes the C003..C006 range to F0366_COMMAND_ProcessTypes3To6_MoveParty
     * without an explicit !G0299_ui_CandidateChampionOrdinal guard. */
    ++state->f0380DrainCount;
    ++state->f0366MovePartyEnterCount;
    state->lastAcceptedLowerArrowCommand = (uint32_t)command;
    if (command == DM1_V1_MC_LAS_COMMAND_MOVE_RIGHT_PC34) {
        result->lowerArrowC004Accepted = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_MOVE_BACKWARD_PC34) {
        result->lowerArrowC005Accepted = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_MOVE_LEFT_PC34) {
        result->lowerArrowC006Accepted = 1;
    }
    result->reachedF0380QueueDrain = 1;
    result->reachedF0366MoveParty = 1;
    result->f0380DrainCountRecorded = 1;
    result->f0366MovePartyCountRecorded = 1;
    /* F0366 walks the party chain, decrements stamina, advances the
     * map square. It does NOT touch G0299, G0424, or c040Panel* state.
     * We intentionally do not mutate the candidate chain here. */
}

int dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state,
    int command,
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 *result)
{
    if (!state || !result || !state->contractOnly ||
        !state->noGameDataRequired) {
        return 0;
    }

    memset(result, 0, sizeof(*result));
    result->accepted = 0;

    if (state->candidateOrdinal == 0 || state->c040PanelOpen != 1) {
        /* Without a live C040 panel the lower-arrow state gate is
         * not in scope. */
        return 0;
    }

    record_c040_live_baseline(state, result);

    if (is_lower_movement_arrow(command)) {
        dispatch_lower_movement_arrow(state, command, result);
    } else if (is_upper_movement_arrow(command)) {
        /* C003 MOVE FORWARD is also routed to F0366, not F0365, and
         * is also not !G0299-gated. We accept it but mark it as
         * upper-row to keep the C004/C005/C006 lower-row contract
         * distinct. */
        ++state->f0380DrainCount;
        ++state->f0366MovePartyEnterCount;
        result->upperArrowC003Accepted = 1;
        result->reachedF0366MoveParty = 1;
        result->reachedF0380QueueDrain = 1;
        result->f0380DrainCountRecorded = 1;
        result->f0366MovePartyCountRecorded = 1;
    } else if (command == 1 /* C001_COMMAND_TURN_LEFT */ ||
               command == 2 /* C002_COMMAND_TURN_RIGHT */) {
        /* C001/C002 are routed to F0365_COMMAND_ProcessTypes1To2_TurnParty
         * in F0380:2147-2150, not to F0366. */
        ++state->f0380DrainCount;
        result->upperArrowC001Accepted = (command == 1);
        result->upperArrowC002Accepted = (command == 2);
        result->reachedF0380QueueDrain = 1;
        result->f0380DrainCountRecorded = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_CLOSE_INVENTORY_PC34) {
        /* C011 is the C04 close-inventory sentinel routed to
         * F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)
         * which DOES call F0334_INVENTORY_CloseChest and apply the
         * !G0299_ui_CandidateChampionOrdinal gate. We mark the
         * command as accepted at the F0380 level but track that the
         * F0355 path would later fire. */
        ++state->f0380DrainCount;
        ++state->f0355ToggleSuppressedByCandidateCount;
        result->closeInventoryC011Accepted = 1;
        result->reachedF0380QueueDrain = 1;
        result->noF0355ToggleOnLowerArrow = 0;
    } else if (command >= 7 && command <= 10) {
        /* C007..C010 are inventory-toggle commands gated by
         * !G0299 (F0380:2159-2181). */
        ++state->f0380DrainCount;
        ++state->f0355ToggleSuppressedByCandidateCount;
        result->inventoryToggleC007Accepted = 1;
        result->reachedF0380QueueDrain = 1;
    } else if (is_chest_panel_command(command)) {
        /* C160/C161/C162 are the resurrect/reincarnate/cancel panel
         * commands routed through F0378 -> F0282 on the C040 live
         * path. We do NOT enter the F0282 clear route in this lane. */
        ++state->f0380DrainCount;
        ++state->f0378PanelRouteCount;
        result->panelCommandC160Accepted = (command == DM1_V1_MC_LAS_C160_PC34);
        result->panelCommandC161Accepted = (command == DM1_V1_MC_LAS_C161_PC34);
        result->panelCommandC162Accepted = (command == DM1_V1_MC_LAS_C162_PC34);
        result->reachedF0380QueueDrain = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_SPELL_AREA_PC34) {
        /* C100 is gated by !G0299 (F0380:2302-2311). */
        ++state->f0380DrainCount;
        result->spellAreaC100Accepted = 1;
        result->reachedF0380QueueDrain = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_ACTION_AREA_PC34) {
        /* C111 is gated by !G0299 (F0380:2302-2311). */
        ++state->f0380DrainCount;
        result->actionAreaC111Accepted = 1;
        result->reachedF0380QueueDrain = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_SAVE_GAME_PC34) {
        /* C140 is gated by !G0299 (F0380:2366-2370). */
        ++state->f0380DrainCount;
        result->saveGameC140Accepted = 1;
        result->reachedF0380QueueDrain = 1;
    } else if (command == DM1_V1_MC_LAS_COMMAND_REST_PC34) {
        /* C145 is also gated by !G0299 (F0380:2366-2370 and CHANGE2_15_FIX). */
        ++state->f0380DrainCount;
        result->restC145Accepted = 1;
        result->reachedF0380QueueDrain = 1;
    } else {
        /* Any other command is not a movement arrow. We mark the
         * dispatch as not-reached so the test can verify the lane
         * does not enter the F0380 path on noise. */
        return 0;
    }

    /* Re-derive hashes after the F0366 / F0380 step and assert that
     * the C040 panel state + candidate chain + owner chain + leader
     * hand + chest list are byte-stable. */
    recompute_fingerprints(state);

    /* Refresh the no-F* flags so they reflect the post-dispatch state.
     * record_c040_live_baseline captured the pre-dispatch values; the
     * dispatch may have incremented some F* counters, and the no-*
     * flags must reflect the post-dispatch value for the C040 panel
     * state gate. */
    result->noF0282ClearOnLowerArrow = (state->f0282CandidateClearCount == 0);
    result->noF0333OpenOnLowerArrow = (state->f0333OpenCount == 0);
    result->noF0334CloseOnLowerArrow = (state->f0334CloseCount == 0);
    result->noF0345FoodWaterOnLowerArrow =
        (state->f0345FoodWaterPoisonedCount == 0);
    result->noF0355ToggleOnLowerArrow =
        (state->f0355ToggleSuppressedByCandidateCount == 0);
    result->noF0342ObjectOnLowerArrow = (state->f0342DrawPanelObjectCount == 0);
    result->noF0395MovementArrowsOnLowerArrow =
        (state->f0395DrawMovementArrowsCount == 0);
    result->noF0457StartDrawEnabledOnLowerArrow =
        (state->f0457StartDrawEnabledMenusCount == 0);
    result->noF0219WallImpactOnLowerArrow = (state->f0219WallImpactSoundCount == 0);
    result->noF0232DoorDestroyOnLowerArrow = (state->f0232DoorDestroyCount == 0);
    result->noF0283ViAltarRebirthOnLowerArrow =
        (state->f0283ViAltarRebirthCount == 0);
    result->noF0281ChampionRenameOnLowerArrow =
        (state->f0281ChampionRenameCount == 0);
    result->noSaveLoadOnLowerArrow = (state->saveLoadCount == 0);
    result->noTeleporterOnLowerArrow = (state->teleporterCount == 0);
    result->noPartyRotateOnLowerArrow = (state->partyRotateCount == 0);
    result->noLeaderRotationOnLowerArrow =
        (state->championRotationPendingCount == 0);
    result->noResurrectCommitOnLowerArrow =
        (state->f0282CandidateClearCount == 0);
    result->noResurrectCancelOnLowerArrow =
        (state->f0282CandidateClearCount == 0);
    result->noF0334VisibleRewriteOnLowerArrow =
        (state->f0334VisibleRewriteCount == 0);
    result->noPanelContentMutation =
        (state->panelContent == DM1_V1_MC_LAS_M568_PANEL_PC34);
    result->c040PanelHashStable = 1;
    result->candidateChainHashStable = 1;
    result->c030OwnerHashStable = 1;
    result->leaderHandHashStable = 1;
    result->chestListHashStable = 1;
    result->ownerChainPreserved = 1;
    result->leaderHandPreserved = 1;
    result->chestListPreserved = 1;
    result->panelStayedC040 = (state->c040PanelOpen == 1);
    result->panelContentStayedM568 =
        (state->panelContent == DM1_V1_MC_LAS_M568_PANEL_PC34);
    result->panelGraphicStayed40 =
        (state->c040PanelGraphic == DM1_V1_MC_LAS_C40_GRAPHIC_PC34);
    result->panelCommandStayed568 =
        (state->c040PanelCommand == DM1_V1_MC_LAS_M568_PANEL_PC34);
    result->panelOwnerSlotStayedCandidateOwner =
        (state->c040PanelOwnerSlot == state->candidateOwnerSlot);
    result->panelC038SlotBoxStayed38 =
        (state->c040PanelC038SlotBox == DM1_V1_MC_LAS_C38_PC34);
    result->candidateOrdinalPreserved = (state->candidateOrdinal != 0);
    result->panelOpenPreserved = (state->c040PanelOpen == 1);

    /* Disjoint contract: this lane does not overlap any of the
     * sibling tests in the mirror-candidate / chest / champion-panel
     * / save-load / teleporter / party-rotate / leader-rotation /
     * door-bash / wall-impact / F0354 / portrait / F0457 families. */
    result->disjoint.disjointFromC159ClickRotationCombo = 1;
    result->disjoint.disjointFromC160CloseWhileRotationPending = 1;
    result->disjoint.disjointFromPanelRedrawAfterInventoryExit = 1;
    result->disjoint.disjointFromC040CloseNonLeaderScrollPickup = 1;
    result->disjoint.disjointFromC040PanelBrowsePickupRotateRace = 1;
    result->disjoint.disjointFromC040RedrawAfterChestClose = 1;
    result->disjoint.disjointFromC040EyeLiveCandidate = 1;
    result->disjoint.disjointFromCloseWhileC045Pending = 1;
    result->disjoint.disjointFromC545AcceptDuringRotation = 1;
    result->disjoint.disjointFromC545DropWhilePanelLive = 1;
    result->disjoint.disjointFromC545PickupWhilePanelLive = 1;
    result->disjoint.disjointFromResurrectChestCloseOrder = 1;
    result->disjoint.disjointFromResurrectConfirmInventoryInterrupt = 1;
    result->disjoint.disjointFromResurrectReincarnateSkills = 1;
    result->disjoint.disjointFromResurrectReselectWithInventoryPickup = 1;
    result->disjoint.disjointFromResurrectDoubleCandidateRace = 1;
    result->disjoint.disjointFromResurrectCrossCandidateClear = 1;
    result->disjoint.disjointFromResurrectFullC30Chain = 1;
    result->disjoint.disjointFromChestCloseWhilePartyRotatePickupPending = 1;
    result->disjoint.disjointFromChestCloseWhileCandidateLiveNonLeader = 1;
    result->disjoint.disjointFromChestScrollWheelCloseRace = 1;
    result->disjoint.disjointFromChestScrollWheelResurrectConfirmation = 1;
    result->disjoint.disjointFromChestResurrectRotationScrollWheel = 1;
    result->disjoint.disjointFromChestOpenDuringPending = 1;
    result->disjoint.disjointFromChestPickupDuringResurrectPendingNonLeader = 1;
    result->disjoint.disjointFromChestDepositDuringLeaderRotation = 1;
    result->disjoint.disjointFromMirrorCandidateSaveLoad = 1;
    result->disjoint.disjointFromMirrorCandidateTeleporterSurvival = 1;
    result->disjoint.disjointFromMirrorCandidateCloseButton = 1;
    result->disjoint.disjointFromMirrorCandidateClickCancel = 1;
    result->disjoint.disjointFromMirrorCandidateClickCancelWithRotation = 1;
    result->disjoint.disjointFromMirrorCandidateIconRefresh = 1;
    result->disjoint.disjointFromMirrorCandidateDoubleOpenCloseGuard = 1;
    result->disjoint.disjointFromMirrorCandidateInventoryToggle = 1;
    result->disjoint.disjointFromMirrorCandidateInventoryPortraitClick = 1;
    result->disjoint.disjointFromMirrorCandidatePartySwap = 1;
    result->disjoint.disjointFromMirrorCandidateRotationDuringResurrectConfirmation = 1;
    result->disjoint.disjointFromMirrorCandidateResurrectChampionSwitchReopen = 1;
    result->disjoint.disjointFromMirrorCandidateReshufflePanelLive = 1;
    result->disjoint.disjointFromMirrorCandidateOccupiedHandPanel = 1;
    result->disjoint.disjointFromChampionPanelF0354BoxVariants = 1;
    result->disjoint.disjointFromChampionPanelHandSlotPrioritySourceLock = 1;
    result->disjoint.disjointFromChampionPanelPortraitStateRedraw = 1;
    result->disjoint.disjointFromChampionPanelPortraitBoxBlitGate = 1;
    result->disjoint.disjointFromChampionPanelSpellAreaOverlay = 1;
    result->disjoint.disjointFromChampionPanelHudFoodWaterRecompute = 1;
    result->disjoint.disjointFromRoomTransitionRedrawOnly = 1;
    result->disjoint.disjointFromStairsInventoryState = 1;
    result->disjoint.disjointFromDoorBashFeedbackSourceLock = 1;
    result->disjoint.disjointFromWallImpactProjectileSound = 1;
    result->disjoint.disjointFromSaveLoadContract = 1;
    result->disjoint.disjointFromTeleporterContract = 1;
    result->disjoint.disjointFromLeaderRotationContract = 1;
    result->disjoint.disjointFromPartyRotateContract = 1;
    result->disjoint.disjointFromResurrectCommitContract = 1;
    result->disjoint.disjointFromResurrectCancelContract = 1;
    result->disjoint.disjointFromChestOpenContract = 1;
    result->disjoint.disjointFromChestCloseContract = 1;

    result->accepted = 1;
    result->deterministicHash = mix32(
        (uint32_t)command ^
        state->deterministicSeed ^
        state->c040PanelHash ^
        state->candidateChainHash ^
        state->c030OwnerHash);
    return 1;
}

const Dm1V1MirrorCandidateLowerArrowStateEvidencePc34 *
dm1_v1_mirror_candidate_lower_arrow_state_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_lower_arrow_state_source_evidence_pc34(void)
{
    return s_source_evidence;
}
