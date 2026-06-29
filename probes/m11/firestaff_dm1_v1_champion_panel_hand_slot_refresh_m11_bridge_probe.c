/*
 * DM1 V1 champion-panel hand-slot refresh M11 geometry bridge.
 *
 * Firestaff-side, data-free probe. It connects the source-locked F0296
 * action-hand slotbox walk-order model to the M11 status-hand zone helpers,
 * proving the live frame-path geometry exposes the same action-hand slotbox
 * sequence (1, 3, 5, 7) for the four compact champion status panels.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0296:1226-1231 walks slotbox indices
 *   0..(G0305_ui_PartyChampionCount << 1)-1, computes
 *   L0885_i_ChampionIndex = slotBoxIndex >> 1, and dispatches the
 *   action-hand route only when M070_HAND_SLOT_INDEX(slotBoxIndex)
 *   is C01_SLOT_ACTION_HAND.
 *   ReDMCSB DEFS.H:1878 defines M070_HAND_SLOT_INDEX(slotboxindex)
 *   as slotboxindex & 0x0001.
 *
 * This is not original DOS screenshot parity and does not load game data.
 */
#include "firestaff/dm1/v1/champion_panel/hand_slot_refresh_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char *G2159_puc_Bitmap_Source;
unsigned char *G2160_puc_Bitmap_Destination;

static int g_checks;
static int g_failures;

static void expect_int(const char *label, int got, int want)
{
    ++g_checks;
    if (got != want) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
    } else {
        printf("PASS %s got=%d\n", label, got);
    }
}

static void expect_true(const char *label, int ok)
{
    ++g_checks;
    if (!ok) {
        ++g_failures;
        fprintf(stderr, "FAIL %s\n", label);
    } else {
        printf("PASS %s\n", label);
    }
}

static int rects_disjoint_horizontally(int ax, int aw, int bx)
{
    return ax + aw <= bx;
}

int main(int argc, char **argv)
{
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 model;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;
    int previousActionBoxX = -1;
    int previousActionBoxW = 0;
    int slot;
    const char *source =
        dm1_v1_champion_panel_hand_slot_refresh_source_evidence_pc34();

    (void)argc;
    (void)argv;

    memset(&model, 0, sizeof(model));
    memset(&result, 0, sizeof(result));

    dm1_v1_champion_panel_hand_slot_refresh_init_pc34(&model);
    expect_int("F0296 model run succeeds",
               dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
                   &model, &result),
               1);
    expect_int("F0296 model path is fully-alive walk",
               result.path,
               DM1_V1_DMHSR_PATH_FULLY_ALIVE_F0296_WALK_PC34);
    expect_int("F0296 model walks exactly four action-hand slotboxes",
               result.leaderHandSlotBoxesWalked, 4);
    expect_true("source evidence names CHAMDRAW.C F0296",
                source && strstr(source, "CHAMDRAW.C F0296") != NULL);
    expect_true("source evidence names M070_HAND_SLOT_INDEX",
                source && strstr(source, "M070_HAND_SLOT_INDEX") != NULL);

    for (slot = 0; slot < DM1_V1_DMHSR_PARTY_COUNT_PC34; ++slot) {
        int expectedSlotboxIndex = 2 * slot + DM1_V1_DMHSR_C01_SLOT_ACTION_HAND_PC34;
        int expectedReadyZoneId = 211 + 2 * slot;
        int expectedActionZoneId = 211 + expectedSlotboxIndex;
        int readyX, readyY, readyW, readyH;
        int actionX, actionY, actionW, actionH;
        int boxX, boxY, boxW, boxH;
        int iconX, iconY, iconW, iconH;
        int statusX, statusY, statusW, statusH;
        char label[160];

        snprintf(label, sizeof(label), "model slot %d source slotbox index", slot);
        expect_int(label, model.slotBoxWalkIndex[slot], expectedSlotboxIndex);

        snprintf(label, sizeof(label), "model slot %d champion index", slot);
        expect_int(label, model.slotBoxWalkChampionIndex[slot], slot);

        snprintf(label, sizeof(label), "M11 slot %d ready-hand zone id", slot);
        expect_int(label,
                   M11_GameView_GetV1StatusHandZoneId(slot, 0),
                   expectedReadyZoneId);

        snprintf(label, sizeof(label), "M11 slot %d action-hand zone id", slot);
        expect_int(label,
                   M11_GameView_GetV1StatusHandZoneId(slot, 1),
                   expectedActionZoneId);

        snprintf(label, sizeof(label), "M11 slot %d status box zone", slot);
        expect_true(label, M11_GameView_GetV1StatusBoxZone(
                           slot, &statusX, &statusY, &statusW, &statusH));

        snprintf(label, sizeof(label), "M11 slot %d ready-hand zone", slot);
        expect_true(label, M11_GameView_GetV1StatusHandZone(
                           slot, 0, &readyX, &readyY, &readyW, &readyH));

        snprintf(label, sizeof(label), "M11 slot %d action-hand zone", slot);
        expect_true(label, M11_GameView_GetV1StatusHandZone(
                           slot, 1, &actionX, &actionY, &actionW, &actionH));

        snprintf(label, sizeof(label), "M11 slot %d action hand is C01 odd index", slot);
        expect_int(label, expectedSlotboxIndex & 1, 1);

        snprintf(label, sizeof(label), "M11 slot %d hand zones share y", slot);
        expect_int(label, actionY, readyY);

        snprintf(label, sizeof(label), "M11 slot %d hand zones are 16x16", slot);
        expect_true(label, readyW == 16 && readyH == 16 &&
                           actionW == 16 && actionH == 16);

        snprintf(label, sizeof(label), "M11 slot %d action hand follows ready hand", slot);
        expect_true(label, actionX > readyX);

        snprintf(label, sizeof(label), "M11 slot %d action slotbox zone", slot);
        expect_true(label, M11_GameView_GetV1StatusHandSlotBoxZone(
                           slot, 1, &boxX, &boxY, &boxW, &boxH));

        snprintf(label, sizeof(label), "M11 slot %d action icon zone", slot);
        expect_true(label, M11_GameView_GetV1StatusHandIconZone(
                           slot, 1, &iconX, &iconY, &iconW, &iconH));

        snprintf(label, sizeof(label), "M11 slot %d slotbox is 18x18 over 16x16 icon", slot);
        expect_true(label, boxW == 18 && boxH == 18 &&
                           iconW == 16 && iconH == 16 &&
                           iconX == boxX + 1 && iconY == boxY + 1);

        snprintf(label, sizeof(label), "M11 slot %d slotbox starts at hand zone", slot);
        expect_true(label, boxX == actionX && boxY == actionY);

        snprintf(label, sizeof(label), "M11 slot %d action slotbox inside status row", slot);
        expect_true(label, boxX >= statusX && boxY >= statusY &&
                           boxX + boxW <= statusX + statusW &&
                           boxY + boxH <= statusY + statusH);

        if (slot > 0) {
            snprintf(label, sizeof(label),
                     "M11 slot %d action slotbox ordered after previous", slot);
            expect_true(label,
                        rects_disjoint_horizontally(previousActionBoxX,
                                                    previousActionBoxW,
                                                    boxX));
        }
        previousActionBoxX = boxX;
        previousActionBoxW = boxW;
    }

    expect_int("M11 rejects negative status-hand slot",
               M11_GameView_GetV1StatusHandZoneId(-1, 1), 0);
    expect_int("M11 rejects out-of-range status-hand slot",
               M11_GameView_GetV1StatusHandZoneId(4, 1), 0);
    expect_int("M11 rejects invalid hand index",
               M11_GameView_GetV1StatusHandZoneId(0, 2), 0);

    printf("%s dm1 v1 champion-panel F0296 hand-slot M11 bridge checks=%d\n",
           g_failures ? "FAIL" : "PASS", g_checks);
    return g_failures ? 1 : 0;
}
