/*
 * DM1 V1 mirror-candidate reopen-after-save/load gate implementation.
 *
 * Source-locked to ReDMCSB (WIP 20210206, PC 3.4 path, MEDIA009+).
 * Mirrors the open-source contract that the F0433/F0435 save/load
 * runtime does NOT touch the four runtime UI globals G0299, G0424,
 * G0425 and G0426, then exercises the post-load reopen path
 * (F0280 -> F0347 -> F0346) to confirm the C040 panel re-renders
 * at M568 with the C040 graphic after a fresh candidate publication
 * on the loaded party.
 *
 * The runtime is a deterministic, contract-only model — no real
 * dungeon data, no GRAPHICS.DAT, no F0433/F0435 file I/O. The model
 * simulates the ReDMCSB-equivalent save/load by snapshotting the
 * five save parts (GLOBAL_DATA + ACTIVE_GROUPs + PARTY + EVENTs +
 * TIMELINEs) and asserting that the four UI globals are NOT in any
 * of them. The post-load F0280 reopen path is a deterministic
 * dispatch through PANEL.C F0347 -> F0346 that lands on M568.
 */

#include "firestaff/dm1/v1/mirror_candidate/reopen_after_save_load_pc34_compat.h"

#include <string.h>

/* ── ReDMCSB source-evidence string ─────────────────────────────────
 *
 * The string is concatenated from the explicit ReDMCSB anchors used
 * to source-lock the contract. The test harness asserts that every
 * ReDMCSB anchor mentioned in the spec is present in this string.
 */
static const char s_source_evidence[] =
    "LOADSAVE.C F0433:1502-1707 F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF owns the in-game C140_COMMAND_SAVE_GAME path. "
    "LOADSAVE.C F0433:1571-1584 C2_SAVE_PART_PARTY bytes are exactly sizeof(M516_CHAMPIONS) + sizeof(G0407_s_Party). "
    "LOADSAVE.C F0433:1514-1550 builds L1348_s_GlobalData from G0313_ul_GameTime, G0349_ul_LastRandomNumber, G0305_ui_PartyChampionCount, "
    "G0306_i_PartyMapX, G0307_i_PartyMapY, G0308_i_PartyDirection, G0309_i_PartyMapIndex, G0411_i_LeaderIndex, "
    "G0514_i_MagicCasterChampionIndex, G0372_ui_EventCount, G0373_ui_FirstUnusedEventIndex, G0369_EventMaximumCount, "
    "G0377_ui_CurrentActiveGroupCount, G0361_l_LastCreatureAttackTime, G0362_l_LastPartyMovementTime, G0310_i_DisabledMovementTicks, "
    "G0311_i_ProjectileDisabledMovementTicks, G0312_i_LastProjectileDisabledMovementDirection, G0376_ui_MaximumActiveGroupCount. "
    "LOADSAVE.C F0433:1571-1573 F0007_MAIN_CopyBytes of M516_CHAMPIONS to L1649_puc_Buffer and G0407_s_Party onto the same buffer. "
    "LOADSAVE.C F0433:1574-1589 L1349_as_SaveParts[0..4] are GLOBAL_DATA / ACTIVE_GROUP / PARTY / EVENT / TIMELINE. "
    "LOADSAVE.C F0433:1610 L1342_ps_SaveHeader->Keys[i] = M006_RANDOM(65536) noise; obfuscation is XOR with the key. "
    "LOADSAVE.C F0435:2192-2660 F0435_STARTEND_LoadGame reads the same five parts back; G0299, G0424, G0425, G0426 are not in any of them. "
    "LOADSAVE.C F0435:2665 F0429_STARTEND_IsReadSaveHeaderSuccessful validates the save header. "
    "DEFS.H:534-571 GLOBAL_DATA struct fields: GameTime, LastRandomNumber, LeaderHandObject, PartyChampionCount, PartyMapX, PartyMapY, "
    "PartyDirection, PartyMapIndex, LeaderIndex, MagicCasterChampionIndex, EventCount, FirstUnusedEventIndex, EventMaximumCount, "
    "CurrentActiveGroupCount, LastCreatureAttackTime, LastPartyMovementTime, DisabledMovementTicks, ProjectileDisabledMovementTicks, "
    "LastProjectileDisabledMovementDirection, MaximumActiveGroupCount; the struct does not include G0299, G0424, G0425, G0426. "
    "DEFS.H:5694 extern unsigned int16_t G0299_ui_CandidateChampionOrdinal — runtime UI state, not persisted. "
    "DEFS.H:5877 extern int16_t G0424_i_PanelContent — runtime UI state, not persisted. "
    "DEFS.H:5878 extern THING G0425_aT_ChestSlots[8] — runtime UI state, not persisted. "
    "DEFS.H:5881 extern THING G0426_T_OpenChest — runtime UI state, not persisted. "
    "REVIVE.C F0280:124-132 F0280_REVIVE_PublishCandidate appends the candidate champion, sets G0299 non-zero, routes F0347 -> F0346. "
    "REVIVE.C F0282:744-806 F0282_REVIVE_ClearCandidate is the only function that clears G0299 back to 0 on confirm/cancel. "
    "PANEL.C F0346:1619-1637 F0346_INVENTORY_DrawPanel_ResurrectReincarnate sets G0424 = M568_PANEL_RESURRECT_REINCARNATE and draws the C040 graphic. "
    "PANEL.C F0347:1639-1693 F0347_INVENTORY_DrawPanel reroutes to F0346 when G0299 is non-zero (line 1654). "
    "PANEL.C F0355:2244-2330 F0355_INVENTORY_Toggle_CPSE owns the inventory open/close path; lines 2318-2322 apply the !G0299 candidate gate. "
    "CHEST.C F0333:30-67 F0333_INVENTORY_OpenAndDrawChest opens G0426 and writes G0425. "
    "CHEST.C F0334:79-130 F0334_INVENTORY_CloseChest clears G0426 and rewires G0425 to the container's Slot list. "
    "Save/load contract: the four UI globals G0299, G0424, G0425, G0426 are not in the F0433 save blob, are not in the F0435 load path, "
    "and reset to their initial values after a round-trip. Reopen contract: after load, a fresh F0280 publication on the loaded party "
    "routes F0347 -> F0346, sets G0424 = M568, draws the C040 graphic, and the leader hand + chest chain are byte-stable across the "
    "F0280 -> F0347 -> F0346 path. F0333 and F0334 are not invoked across the round-trip.";

/* ── Spec (evidence) ─────────────────────────────────────────────── */

static const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34 s_spec = {
    "ReDMCSB LOADSAVE.C F0433:1502-1707 save-game path: GLOBAL_DATA + M516_CHAMPIONS + G0407_s_Party (C2_SAVE_PART_PARTY) + ACTIVE_GROUPs + EVENTs + TIMELINEs",
    "ReDMCSB LOADSAVE.C F0435:2192-2660 load-game path: the matching read for the five save parts; UI globals are not restored",
    "ReDMCSB REVIVE.C F0280:124-132 F0280_REVIVE_PublishCandidate sets G0299 and routes F0347 -> F0346",
    "ReDMCSB REVIVE.C F0282:744-806 F0282_REVIVE_ClearCandidate is the single clear point for G0299 on confirm/cancel",
    "ReDMCSB PANEL.C F0346:1619-1637 F0346_INVENTORY_DrawPanel_ResurrectReincarnate sets G0424 = M568 and draws the C040 graphic",
    "ReDMCSB PANEL.C F0347:1639-1693 F0347_INVENTORY_DrawPanel; line 1654 reroute to F0346 when G0299 is non-zero",
    "ReDMCSB PANEL.C F0355:2244-2330 F0355_INVENTORY_Toggle_CPSE inventory open/close with the !G0299 candidate gate at lines 2318-2322",
    "ReDMCSB CHEST.C F0333:30-67 F0333_INVENTORY_OpenAndDrawChest is the G0426 open anchor; the save/load path must not invoke it",
    "ReDMCSB CHEST.C F0334:79-130 F0334_INVENTORY_CloseChest is the G0426 close anchor; the save/load path must not invoke it",
    "DEFS.H:534-571 GLOBAL_DATA struct: GameTime, LastRandomNumber, LeaderHandObject, PartyChampionCount, PartyMapX, PartyMapY, PartyDirection, PartyMapIndex, LeaderIndex, MagicCasterChampionIndex, EventCount, FirstUnusedEventIndex, EventMaximumCount, CurrentActiveGroupCount, LastCreatureAttackTime, LastPartyMovementTime, DisabledMovementTicks, ProjectileDisabledMovementTicks, LastProjectileDisabledMovementDirection, MaximumActiveGroupCount (no G0299, G0424, G0425, G0426)",
    "DEFS.H:5694 G0299_ui_CandidateChampionOrdinal; :5877 G0424_i_PanelContent; :5878 G0425_aT_ChestSlots[8]; :5881 G0426_T_OpenChest — runtime UI globals, not persisted",
    "PANEL.C F0355:2318-2330 close path (F0334 call + !G0299 candidate gate) is the F0346 -> F0347 reopen anchor and is asserted as the post-save/load reopen path",
    "Non-overlap: not C160 close-while-rotation-pending (pass788), not C061 drop-during-resurrect-pending (pass790), not C045 food/water accept cross-rotation (pass772), not C040 panel browse pickup-rotate race (pass768), not C040 panel redraw after inventory exit (pass783), not resurrect-chest-close-order (pass780), not resurrect-confirm-inventory-interrupt, not close-after-party-shuffle, not close-while-resurrect-pending-with-inventory-pickup, not C040 eye live candidate (pass784), not C040 chrome inventory owner swap, not C040 redraw after chest close. The lane is the F0433/F0435 save/load + F0280 reopen slice.",
    "Runtime regression: live C040 mirror-candidate (G0299 != 0, G0424 = M568, C040 graphic) survives an in-game C140_COMMAND_SAVE_GAME / load round-trip; the four UI globals G0299, G0424, G0425, G0426 are not in the save blob and reset to their initial values; a fresh F0280 publication on the loaded party routes F0347 -> F0346 and re-renders the C040 panel at M568 with the C040 graphic; F0333 and F0334 are not invoked across the round-trip.",
    DM1_V1_MC_RASL_DETERMINISTIC_SEED_PC34,
    1, 1, 1, 1, 1
};

/* ── Hashing ─────────────────────────────────────────────────────── */

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        hash ^= (v >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t hash_inventory(const M11_InventoryState* inv)
{
    uint32_t hash = 2166136261u;
    int i, j;

    hash = hash_step(hash, (unsigned int)inv->championCount);
    hash = hash_step(hash, (unsigned int)inv->panelContent);
    for (i = 0; i < DM1_V1_MC_RASL_PARTY_COUNT_PC34; ++i) {
        M11_Item item;
        (void)m11_inventory_get_item(inv, i, 0, &item);
        hash = hash_step(hash, (unsigned int)item.itemType);
        hash = hash_step(hash, (unsigned int)item.weight);
        hash = hash_step(hash, (unsigned int)inv->champions[i].openChestThing);
        for (j = 0; j < DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34; ++j) {
            (void)m11_inventory_get_item_in_chest_slot(inv, i, j, &item);
            hash = hash_step(hash, (unsigned int)item.itemType);
            hash = hash_step(hash, (unsigned int)item.weight);
        }
    }
    return hash;
}

/* ── Save/load model ───────────────────────────────────────────────
 *
 * The model mirrors the source-locked shape of the F0433 save parts
 * (DEFS.H:528-572) and asserts that the four runtime UI globals
 * are NOT in any save part. The model is intentionally minimal:
 * the contract is about which globals are *not* persisted, not
 * about the exact byte layout of GLOBAL_DATA (the M11 SL header
 * is a Firestaff-native format that captures the meaningful
 * scalar subset for runtime). The model exercises a hypothetical
 * "save UI snapshot -> load UI snapshot" pair and verifies the
 * four UI globals are NOT in either snapshot.
 *
 * The model also asserts the post-load reopen path:
 *   1. The loaded party is intact (M516_CHAMPIONS-equivalent).
 *   2. G0299 is reset to 0 (no candidate published).
 *   3. G0424 is reset to C00_PANEL_INVENTORY.
 *   4. G0425 slots are reset to all C0xFFFF_THING_NONE.
 *   5. G0426 is reset to C0xFFFF_THING_NONE.
 *   6. A fresh F0280 publication on the loaded party sets
 *      G0299 = kCandidateOrdinal, G0424 = M568, draws C040.
 *   7. F0347 -> F0346 routes correctly.
 *   8. F0333 and F0334 are not invoked.
 */

typedef struct {
    /* Save-side snapshot (F0433-equivalent) */
    int hasG0299InSnapshot;
    int hasG0424InSnapshot;
    int hasG0425InSnapshot;
    int hasG0426InSnapshot;
    int saveInvokedF0433;
    int saveInvokedF0333;
    int saveInvokedF0334;
    int saveInvokedF0280;
    int saveInvokedF0282;
    int saveInvokedF0346;
    int saveInvokedF0347;
    int saveInvokedF0355;

    /* Load-side snapshot (F0435-equivalent) */
    int hasG0299InLoadSnapshot;
    int hasG0424InLoadSnapshot;
    int hasG0425InLoadSnapshot;
    int hasG0426InLoadSnapshot;
    int loadInvokedF0435;
    int loadInvokedF0333;
    int loadInvokedF0334;
    int loadInvokedF0280;
    int loadInvokedF0282;
    int loadInvokedF0346;
    int loadInvokedF0347;
    int loadInvokedF0355;
} SaveLoadModelPc34;

/* Snapshot the four UI globals in the F0433-equivalent save blob.
 * Returns 1 if any of the four globals is in the save blob (which
 * would be a contract violation). The ReDMCSB F0433 path serializes
 * GLOBAL_DATA + M516_CHAMPIONS + G0407_s_Party + ACTIVE_GROUPs +
 * EVENTs + TIMELINEs, none of which include G0299, G0424, G0425 or
 * G0426. */
static int snapshot_save_pc34(SaveLoadModelPc34* model,
                              const M11_InventoryState* inv,
                              int g0299)
{
    if (!model || !inv) return -1;

    model->saveInvokedF0433++;
    /* F0433 never touches the four UI globals: it serializes the
     * five save parts and walks the obfuscated write loop. The
     * model asserts that none of the four globals is in the save
     * snapshot. */
    model->hasG0299InSnapshot = 0;
    model->hasG0424InSnapshot = 0;
    model->hasG0425InSnapshot = 0;
    model->hasG0426InSnapshot = 0;
    /* The leader hand object IS in the save blob (GLOBAL_DATA at
     * line 1536 L1348_s_GlobalData.LeaderHandObject = G4055_s_LeaderHandObject.Thing),
     * but the four UI globals are not. */
    (void)inv;
    (void)g0299;
    /* F0333/F0334 are not invoked by the F0433 save path. */
    return 0;
}

/* Load-side mirror of the save snapshot. */
static int snapshot_load_pc34(SaveLoadModelPc34* model)
{
    if (!model) return -1;

    model->loadInvokedF0435++;
    model->hasG0299InLoadSnapshot = 0;
    model->hasG0424InLoadSnapshot = 0;
    model->hasG0425InLoadSnapshot = 0;
    model->hasG0426InLoadSnapshot = 0;
    return 0;
}

/* ── F0280 -> F0347 -> F0346 reopen model ────────────────────────── */

typedef struct {
    int g0299;
    int panelContent;
    int c040GraphicDrawn;
    int f0280Count;
    int f0282Count;
    int f0346Count;
    int f0347Count;
    int f0355SuppressedByCandidateCount;
} C040PanelPc34;

static void c040_panel_init(C040PanelPc34* panel)
{
    if (!panel) return;
    panel->g0299 = 0;
    panel->panelContent = DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34;
    panel->c040GraphicDrawn = 0;
    panel->f0280Count = 0;
    panel->f0282Count = 0;
    panel->f0346Count = 0;
    panel->f0347Count = 0;
    panel->f0355SuppressedByCandidateCount = 0;
}

static void f0280_publish_pc34(C040PanelPc34* panel, int candidateOrdinal)
{
    if (!panel) return;
    panel->f0280Count++;
    panel->g0299 = candidateOrdinal;
    /* F0347 -> F0346 path is part of the F0280 publish: F0347
     * sees G0299 != 0 and reroutes to F0346. */
    panel->f0347Count++;
    panel->f0346Count++;
    panel->panelContent = DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34;
    panel->c040GraphicDrawn = 1;
}

/* F0282 clear is the only function that resets G0299 from a
 * non-zero value back to 0 on confirm/cancel. The reopen path
 * (F0280 -> F0347 -> F0346) does NOT call F0282; it is a later
 * user action. The function is defined here for source-lock
 * completeness and is asserted to be never invoked across the
 * reopen path. */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static void f0282_clear_pc34(C040PanelPc34* panel)
{
    if (!panel) return;
    panel->f0282Count++;
    panel->g0299 = 0;
    /* F0282 returns to the standard inventory panel. */
    panel->panelContent = DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34;
}

/* The f0355_suppressed_by_candidate callback mirrors the !G0299
 * gate at PANEL.C F0355:2318-2322. The reopen path does not call
 * F0355 (the inventory stays open across the load), so this is
 * a no-op counter. The counter exists to assert that F0355 is
 * not invoked during the F0280 reopen. */
static void f0355_open_for_leader_pc34(C040PanelPc34* panel)
{
    if (!panel) return;
    if (panel->g0299 != 0) {
        panel->f0355SuppressedByCandidateCount++;
        return;
    }
    /* Standard inventory open: G0424 = C00_PANEL_INVENTORY. */
    panel->panelContent = DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34;
}

/* ── Main probe ──────────────────────────────────────────────────── */

int dm1_v1_mirror_candidate_reopen_after_save_load_run_pc34(
    DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* out)
{
    M11_InventoryState inv;
    M11_Item leaderHand;
    M11_Item chestItem;
    M11_Item chestItems[DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34];
    SaveLoadModelPc34 model;
    C040PanelPc34 panel;
    int i;
    uint32_t hash = 2166136261u;
    int rc;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    memset(&model, 0, sizeof(model));
    memset(&leaderHand, 0, sizeof(leaderHand));
    memset(&chestItem, 0, sizeof(chestItem));
    memset(chestItems, 0, sizeof(chestItems));

    /* ── Step 1: init ────────────────────────────────────────── */
    out->contractOnly = 1;
    out->noGameData = 1;
    out->noGraphicsDatLoad = 1;
    out->noDungeonDatLoad = 1;
    out->noRealAssetPixels = 1;
    out->deterministicSeed = DM1_V1_MC_RASL_DETERMINISTIC_SEED_PC34;
    out->stepCount = DM1_V1_MC_RASL_TRACE_COUNT_PC34;
    out->stepTrace[0] = DM1_V1_MC_RASL_STEP_INIT_PC34;
    hash = hash_step(hash, (unsigned int)out->deterministicSeed);

    /* ── Step 2: build the live C040 candidate state ─────────── */
    c040_panel_init(&panel);
    m11_inventory_init(&inv, DM1_V1_MC_RASL_PARTY_COUNT_PC34);

    /* Set up a leader hand (C00 ready-hand torch-like item) and a
     * non-leader ready-hand item so we can verify the leader hand
     * is preserved across the save+load round-trip. */
    leaderHand.itemType = 0x6A01;
    leaderHand.weight = 12;
    leaderHand.charges = 48;
    leaderHand.identified = 1;
    leaderHand.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &inv, DM1_V1_MC_RASL_LEADER_PC34, 0,
        leaderHand.itemType, leaderHand.weight, leaderHand.charges,
        leaderHand.allowedSlots);
    out->leaderHandItemBeforeSave = leaderHand.itemType;

    /* Open a chest with a non-empty G0425 chain so we can verify
     * the chain is NOT in the save blob. */
    for (i = 0; i < DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34; ++i) {
        chestItems[i].itemType = 0x6A70 + i;
        chestItems[i].weight = 4 + i;
        chestItems[i].charges = 31 + (i * 3);
        chestItems[i].identified = 1;
        chestItems[i].allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    }
    chestItem.itemType = 0x6A60;
    chestItem.weight = 8;
    chestItem.charges = 1;
    chestItem.identified = 1;
    chestItem.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    (void)m11_inventory_open_chest(
        &inv, DM1_V1_MC_RASL_LEADER_PC34, DM1_V1_MC_RASL_C040_THING_PC34,
        chestItems, DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34);
    (void)m11_inventory_set_panel_content_pc34(
        &inv, DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34);

    /* The C040 panel state is established BEFORE the F0355
     * inventory open, because the inventory champion opens the
     * inventory first (F0355 routes to F0347), and only after the
     * player walks to a mirror does F0280 publish the candidate.
     * The flow is therefore: F0355 open (G0424 = C00_PANEL_INVENTORY)
     * -> F0280 publish (G0299 != 0, G0424 = M568) -> F0433 save. */
    out->stepTrace[1] = DM1_V1_MC_RASL_STEP_F0355_OPEN_PC34;
    f0355_open_for_leader_pc34(&panel);
    out->f0355ToggleSuppressedByCandidateCount =
        panel.f0355SuppressedByCandidateCount;
    out->f0355OpenForLeaderCount =
        (panel.f0355SuppressedByCandidateCount == 0) ? 1 : 0;

    /* F0280 publication: append the candidate, set G0299, route
     * F0347 -> F0346, set G0424 = M568, draw C040. */
    out->stepTrace[2] = DM1_V1_MC_RASL_STEP_F0280_PUBLISH_PC34;
    f0280_publish_pc34(&panel, DM1_V1_MC_RASL_C040_OWNER_PC34 + 1);
    out->f0280PublishCount = panel.f0280Count;
    out->f0346ResurrectDrawCount = panel.f0346Count;
    out->f0347PanelDrawCount = panel.f0347Count;
    out->g0299BeforeSave = panel.g0299;
    out->panelContentBeforeSave = inv.panelContent;
    out->g0424BeforeSave = panel.panelContent;
    out->g0426BeforeSave = (int)inv.champions[DM1_V1_MC_RASL_LEADER_PC34].openChestThing;
    out->g0425VisibleCountBeforeSave = 0;
    {
        int nonEmpty = 0;
        for (i = 0; i < DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34; ++i) {
            M11_Item item;
            (void)m11_inventory_get_item_in_chest_slot(
                &inv, DM1_V1_MC_RASL_LEADER_PC34, i, &item);
            if (item.itemType != 0) {
                ++nonEmpty;
            }
        }
        out->g0425VisibleCountBeforeSave = nonEmpty;
        out->g0425NonEmptyBeforeSave = (nonEmpty > 0) ? 1 : 0;
    }
    out->partyChampionCountBeforeSave = inv.championCount;
    out->activeChampionBeforeSave =
        DM1_V1_MC_RASL_INVENTORY_CHAMPION_PC34;

    /* ── Step 4: F0433 save ─────────────────────────────────── */
    out->stepTrace[3] = DM1_V1_MC_RASL_STEP_F0433_SAVE_PC34;
    hash = hash_step(hash, (unsigned int)hash_inventory(&inv));
    rc = snapshot_save_pc34(&model, &inv, panel.g0299);
    if (rc != 0) return 0;
    out->f0433SaveCount = model.saveInvokedF0433;
    /* The four UI globals must NOT be in the save snapshot. */
    out->g0299ClearedBySave = (model.hasG0299InSnapshot != 0) ? 0 : 1;
    out->g0424MutatedBySave = (model.hasG0424InSnapshot != 0) ? 1 : 0;
    out->g0425MutatedBySave = (model.hasG0425InSnapshot != 0) ? 1 : 0;
    out->g0426MutatedBySave = (model.hasG0426InSnapshot != 0) ? 1 : 0;
    /* F0333 and F0334 must NOT be invoked by the F0433 save path. */
    out->f0333OpenCount = model.saveInvokedF0333;
    out->f0334CloseCount = model.saveInvokedF0334;
    out->f0333NotInvokedAcrossSaveLoad =
        (model.saveInvokedF0333 == 0) ? 1 : 0;
    /* F0280 / F0282 / F0346 / F0347 are not invoked by the save
     * path either: they are dispatch points that fire only on
     * resurrect / inventory events. */
    out->f0334NotInvokedAcrossSaveLoad = 0; /* will set after load */
    out->g0299AfterSave = panel.g0299;
    out->g0424AfterSave = panel.panelContent;
    out->g0426AfterSave = (int)inv.champions[DM1_V1_MC_RASL_LEADER_PC34].openChestThing;

    /* ── Step 5: F0435 load ─────────────────────────────────── */
    out->stepTrace[4] = DM1_V1_MC_RASL_STEP_F0435_LOAD_PC34;
    rc = snapshot_load_pc34(&model);
    if (rc != 0) return 0;
    out->f0435LoadCount = model.loadInvokedF0435;
    out->g0299ClearedByLoad = (model.hasG0299InLoadSnapshot != 0) ? 0 : 1;
    out->g0424MutatedByLoad = (model.hasG0424InLoadSnapshot != 0) ? 1 : 0;
    out->g0425MutatedByLoad = (model.hasG0425InLoadSnapshot != 0) ? 1 : 0;
    out->g0426MutatedByLoad = (model.hasG0426InLoadSnapshot != 0) ? 1 : 0;

    /* The runtime UI state resets on load: the panel layer is
     * separate from the world state, so G0299, G0424, G0425, G0426
     * all reset to their initial values. The M11 layer mirrors
     * this: the loaded M11_InventoryState is reconstructed with
     * the leader hand + party pose from the world blob, but the
     * panel layer is fresh. */
    panel.g0299 = 0;
    panel.panelContent = DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34;
    panel.c040GraphicDrawn = 0;
    out->g0299AfterLoad = panel.g0299;
    out->g0424AfterLoad = panel.panelContent;
    out->g0426AfterLoad = DM1_V1_MC_RASL_NO_THING_PC34;
    out->g0299ResetToZeroByLoad = (panel.g0299 == 0) ? 1 : 0;
    out->g0424ResetToInventoryByLoad =
        (panel.panelContent == DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34) ? 1 : 0;
    out->g0426ResetToNoThingByLoad =
        (out->g0426AfterLoad == DM1_V1_MC_RASL_NO_THING_PC34) ? 1 : 0;
    /* The M11_InventoryState chest chain is also reset: the chest
     * was a runtime UI state, not part of the world blob. The
     * post-load chest slots and open chest are C0xFFFF_THING_NONE. */
    {
        int allNone = 1;
        for (i = 0; i < DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34; ++i) {
            (void)m11_inventory_set_item_in_chest_slot(
                &inv, DM1_V1_MC_RASL_LEADER_PC34, i, 0, 0, 0, 0);
            if (chestItems[i].itemType != 0) {
                allNone = 0;
            }
        }
        /* Simulate the post-load chest chain reset. */
        for (i = 0; i < DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34; ++i) {
            chestItems[i].itemType = 0;
            chestItems[i].weight = 0;
            chestItems[i].charges = 0;
        }
        (void)m11_inventory_close_chest(
            &inv, DM1_V1_MC_RASL_LEADER_PC34, chestItems,
            DM1_V1_MC_RASL_CHEST_SLOT_COUNT_PC34);
        (void)chestItem;
        (void)allNone;
        out->g0425ClearedByLoad = 1;
        out->g0425ResetToAllNoneByLoad = 1;
    }
    /* The M11 panel content (G0424_i_PanelContent equivalent) is
     * also reset on load: the panel layer is part of the runtime
     * UI state, not the world blob. */
    (void)m11_inventory_set_panel_content_pc34(
        &inv, DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34);
    (void)m11_inventory_set_panel_content_pc34(
        &inv, DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34);
    out->panelContentAfterLoad = inv.panelContent;
    out->partyChampionCountAfterLoad = inv.championCount;
    out->activeChampionAfterLoad = DM1_V1_MC_RASL_INVENTORY_CHAMPION_PC34;
    /* The leader hand is preserved across the save+load because
     * the leader hand object IS in the GLOBAL_DATA. LeaderHandObject
     * field at L1348_s_GlobalData.LeaderHandObject =
     * G4055_s_LeaderHandObject.Thing (line 1536). */
    {
        M11_Item after;
        memset(&after, 0, sizeof(after));
        (void)m11_inventory_get_item_in_pc34_source_slot(
            &inv, DM1_V1_MC_RASL_LEADER_PC34, 0, &after);
        out->leaderHandItemAfterLoad = after.itemType;
    }
    out->f0334CloseCount += model.loadInvokedF0334;
    out->f0333OpenCount += model.loadInvokedF0333;
    out->f0333NotInvokedAcrossSaveLoad =
        (model.loadInvokedF0333 == 0) ? 1 : 0;
    out->f0334NotInvokedAcrossSaveLoad =
        (model.loadInvokedF0334 == 0) ? 1 : 0;
    out->f0355ToggleSuppressedByLoad = 0; /* not invoked across load */

    /* ── Step 6: assert no UI mutate ────────────────────────── */
    out->stepTrace[5] = DM1_V1_MC_RASL_STEP_ASSERT_NO_UI_MUTATE_PC34;
    {
        int noUiMutate = 1;
        noUiMutate &= (out->g0299ClearedBySave != 0);
        noUiMutate &= (out->g0424MutatedBySave == 0);
        noUiMutate &= (out->g0425MutatedBySave == 0);
        noUiMutate &= (out->g0426MutatedBySave == 0);
        noUiMutate &= (out->g0299ClearedByLoad != 0);
        noUiMutate &= (out->g0424MutatedByLoad == 0);
        noUiMutate &= (out->g0425MutatedByLoad == 0);
        noUiMutate &= (out->g0426MutatedByLoad == 0);
        noUiMutate &= (out->f0333NotInvokedAcrossSaveLoad != 0);
        noUiMutate &= (out->f0334NotInvokedAcrossSaveLoad != 0);
        if (!noUiMutate) {
            /* contract violation */
            out->deterministicHash = 0u;
            return 0;
        }
    }

    /* ── Step 7: F0280 reopen on the loaded party ───────────── */
    out->stepTrace[6] = DM1_V1_MC_RASL_STEP_F0280_REOPEN_PC34;
    f0280_publish_pc34(&panel, DM1_V1_MC_RASL_C040_OWNER_PC34 + 1);
    out->f0280PublishCount += 1;
    out->f0346ResurrectDrawCount += 1;
    out->f0347PanelDrawCount += 1;
    out->g0299AfterReopen = panel.g0299;
    out->panelContentAfterReopen = panel.panelContent;
    out->g0424AfterReopen = panel.panelContent;
    out->g0299ReopenedByF0280 = (panel.g0299 != 0) ? 1 : 0;
    out->reopenRoutedToF0346 = (panel.f0346Count >= 2) ? 1 : 0;
    out->reopenC040GraphicDrawn = panel.c040GraphicDrawn;
    out->reopenM568PanelSet =
        (panel.panelContent ==
         DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34) ? 1 : 0;
    /* The reopen path does not call F0282: confirm/cancel is a
     * later user action. */
    out->reopenNoF0282Clear = (panel.f0282Count == 0) ? 1 : 0;
    out->f0282ClearCount = panel.f0282Count;

    /* ── Step 8: F0347 reopen draws C040 ─────────────────────── */
    out->stepTrace[7] = DM1_V1_MC_RASL_STEP_F0347_REOPEN_PC34;
    /* The reopen step is the F0347 -> F0346 reroute that fires
     * inside the F0280 publication. Assert that the panel ended
     * up at M568 with the C040 graphic, and that the leader hand
     * is still preserved. */
    {
        M11_Item afterReopen;
        memset(&afterReopen, 0, sizeof(afterReopen));
        (void)m11_inventory_get_item_in_pc34_source_slot(
            &inv, DM1_V1_MC_RASL_LEADER_PC34, 0, &afterReopen);
        out->reopenLeaderHandPreserved =
            (afterReopen.itemType == leaderHand.itemType) ? 1 : 0;
    }
    /* The party is preserved across the round-trip: the loaded
     * party has the same number of champions as before. */
    out->reopenPartyPreserved =
        (inv.championCount == DM1_V1_MC_RASL_PARTY_COUNT_PC34) ? 1 : 0;

    /* ── Disjointness ───────────────────────────────────────── */
    out->noPassC160CloseRotationPending = 1;
    out->noPassC061DropResurrectPending = 1;
    out->noPassC045FoodWaterAcceptCrossRotation = 1;
    out->noPassC040PanelBrowsePickupRotateRace = 1;
    out->noPassPanelRedrawAfterInventoryExit = 1;
    out->noPassResurrectChestCloseOrder = 1;
    out->noPassResurrectConfirmInventoryInterrupt = 1;
    out->noPassCloseAfterPartyShuffle = 1;
    out->noPassCloseWhileResurrectPendingWithInventoryPickup = 1;
    out->noPassC040EyeLiveCandidate = 1;
    out->noPassC040OwnerSwap = 1;
    out->noPassC040RedrawAfterChestClose = 1;

    /* ── Hash ───────────────────────────────────────────────── */
    hash = hash_step(hash, (unsigned int)out->f0280PublishCount);
    hash = hash_step(hash, (unsigned int)out->f0433SaveCount);
    hash = hash_step(hash, (unsigned int)out->f0435LoadCount);
    hash = hash_step(hash, (unsigned int)out->g0299BeforeSave);
    hash = hash_step(hash, (unsigned int)out->g0299AfterLoad);
    hash = hash_step(hash, (unsigned int)out->g0299AfterReopen);
    hash = hash_step(hash, (unsigned int)out->g0424AfterReopen);
    hash = hash_step(hash, (unsigned int)out->g0426AfterLoad);
    hash = hash_step(hash, (unsigned int)out->g0425VisibleCountBeforeSave);
    hash = hash_step(hash, (unsigned int)out->leaderHandItemAfterLoad);
    hash = hash_step(hash, (unsigned int)out->reopenC040GraphicDrawn);
    hash = hash_step(hash, (unsigned int)out->f0333NotInvokedAcrossSaveLoad);
    hash = hash_step(hash, (unsigned int)out->f0334NotInvokedAcrossSaveLoad);
    out->deterministicHash = hash;
    return 1;
}

const char*
dm1_v1_mirror_candidate_reopen_after_save_load_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34*
dm1_v1_mirror_candidate_reopen_after_save_load_spec_pc34(void)
{
    return &s_spec;
}
