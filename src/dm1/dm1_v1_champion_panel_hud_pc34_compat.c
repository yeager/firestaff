/*
 * DM1 V1 Champion Panel & Inventory HUD — pc34 compat implementation.
 *
 * Every constant and computation in this file is source-locked to
 * ReDMCSB_WIP20210206. Citations use function/line/variable names
 * from the ReDMCSB codebase.
 */

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include <stdio.h>
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 * Champion color table — G0046_auc_Graphic562_ChampionColor[4]
 * Source: ReDMCSB COORD.C (loaded from graphic 562 / layout-696).
 * Champion 0 = 7 (green), 1 = 11 (yellow), 2 = 8 (red), 3 = 14 (blue).
 * ══════════════════════════════════════════════════════════════════════ */
const uint8_t DM1_ChampionColor[DM1_CHAMPION_COUNT] = { 7, 11, 8, 14 };

/* ══════════════════════════════════════════════════════════════════════
 * Bar graph height — CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs
 *
 * Original algorithm (for HP and stamina):
 *   if (current > 0) {
 *       scaled = (((long)current << 10) * 25) / maximum;
 *       height = (scaled & 0x3FF) ? (scaled >> 10) + 1 : (scaled >> 10);
 *   } else { height = 0; }
 *
 * For mana: if (current > maximum) height = 25;
 * else same formula.
 *
 * The (<<10, *25, >>10) is a fixed-point 10.10 multiply by 25,
 * with round-up on any fractional remainder. Equivalent to
 * ceil(current * 25 / maximum) when current > 0.
 * ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_BarGraphHeight(int current, int maximum, int isMana)
{
    long scaled;
    if (current <= 0) return 0;
    if (maximum <= 0) return 0;
    if (isMana && current > maximum) return DM1_BAR_GRAPH_MAX_HEIGHT;
    scaled = (((long)current << 10) * DM1_BAR_GRAPH_MAX_HEIGHT) / (long)maximum;
    if (scaled & 0x3FFL)
        return (int)(scaled >> 10) + 1;
    else
        return (int)(scaled >> 10);
}

int DM1_ChampionPanel_BuildStatusBoxModel(
    int championIndex, int leaderIndex, int isInventoryChampion,
    int currentHealth, DM1_ChampionPanel_StatusBoxModel *outModel)
{
    (void)leaderIndex;

    if (!outModel || championIndex < 0 || championIndex >= DM1_CHAMPION_COUNT) {
        return 0;
    }

    memset(outModel, 0, sizeof(*outModel));
    outModel->fillColor = -1;
    outModel->graphicId = -1;
    outModel->nameColor = -1;
    outModel->nameBackgroundColor = -1;

    /* CHAMDRAW.C F0292 lines 771-789: status-box redraw fills the live
     * champion status box with C12 before shield borders/portrait/hands. */
    if (currentHealth > 0) {
        outModel->drawKind = DM1_STATUS_BOX_DRAW_ALIVE;
        outModel->fillColor = DM1_COLOR_DARKEST_GRAY;
        if (isInventoryChampion) {
            /* Lines 810-812: inventory champion draws the portrait and then
             * schedules only the statistics redraw in the local mask. */
            outModel->drawPortrait = 1;
            outModel->propagatedAttributes = DM1_ATTR_STATISTICS;
        } else {
            /* Lines 813-815: status-box redraw for other live champions
             * schedules name/title, bars/statistics, wounds and action hand. */
            outModel->propagatedAttributes =
                DM1_ATTR_NAME_TITLE | DM1_ATTR_STATISTICS |
                DM1_ATTR_WOUNDS | DM1_ATTR_ACTION_HAND;
        }
        return 1;
    }

    /* Lines 816-838: dead champions blit C008, print the name with C13 on
     * C01, draw the action icon, then jump to the end of F0292. */
    outModel->drawKind = DM1_STATUS_BOX_DRAW_DEAD;
    outModel->graphicId = DM1_GFX_DEAD_CHAMPION;
    outModel->nameColor = DM1_COLOR_LIGHTEST_GRAY;
    outModel->nameBackgroundColor = DM1_COLOR_DARK_GRAY;
    outModel->drawActionIcon = 1;
    outModel->stopAfterDead = 1;
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Slot box graphic — CHAMDRAW.C F0291_CHAMPION_DrawSlot
 *
 * For slots 0..5 (ready_hand..feet):
 *   if (slotIndex == 1 && isActingChampion) -> C035_GRAPHIC_SLOT_BOX_ACTING_HAND
 *   elif (wounds & (1 << slotIndex))        -> C034_GRAPHIC_SLOT_BOX_WOUNDED
 *   else                                    -> C033_GRAPHIC_SLOT_BOX_NORMAL
 *
 * For slots 6+ the slot box has no body-part graphic.
 * ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_SlotBoxGraphic(int slotIndex, uint16_t wounds,
                                      int isActingChampion)
{
    if (slotIndex < 0 || slotIndex > 5) return -1; /* no slot box graphic */
    if (slotIndex == DM1_SLOT_ACTION_HAND && isActingChampion)
        return DM1_GFX_SLOT_ACTING;
    if (wounds & (1u << slotIndex))
        return DM1_GFX_SLOT_WOUNDED;
    return DM1_GFX_SLOT_NORMAL;
}

/* ══════════════════════════════════════════════════════════════════════
 * Portrait screen X — CHAMDRAW.C F0354_INVENTORY_DrawStatusBoxPortrait
 *
 * Source: box left = champIdx * C69_CHAMPION_STATUS_BOX_SPACING + 7
 * The +7 is the portrait inset from the status-box left edge.
 * ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_PortraitScreenX(int champIdx)
{
    if (champIdx < 0 || champIdx >= DM1_CHAMPION_COUNT) return 0;
    return champIdx * DM1_STATUS_BOX_SPACING + DM1_PORTRAIT_OFFSET_X;
}

/* ══════════════════════════════════════════════════════════════════════
 * Name zone X — CHAMDRAW.C F0292 (C159_ZONE_CHAMPION_0_STATUS_BOX_NAME)
 *
 * Source: fill zone left = champIdx * C69_CHAMPION_STATUS_BOX_SPACING.
 * Name text is centered within the zone by F0650_PrintCenteredTextToScreenZone.
 * ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_NameZoneX(int champIdx)
{
    if (champIdx < 0 || champIdx >= DM1_CHAMPION_COUNT) return 0;
    return champIdx * DM1_STATUS_BOX_SPACING;
}

/* ══════════════════════════════════════════════════════════════════════
 * Bar graph screen XY — layout-696 C195_ZONE_FIRST_BAR_GRAPH
 *
 * Zone C195 + champIdx is HP bar for champion champIdx.
 * Stride +4 for stamina, +8 for mana.
 *
 * Source geometry from layout-696 (reconstructed zones):
 *   HP bars (C195..C198):    champIdx * 69 + 46, y=2, w=4, h=25
 *   Stamina bars (C199..C202): champIdx * 69 + 53, y=2, w=4, h=25
 *   Mana bars (C203..C206): champIdx * 69 + 60, y=2, w=4, h=25
 *
 * Bar spacing within the 24px bar-graph region: bars at offsets 0, 7, 14
 * relative to the bar-graph region left edge (at champIdx*69 + 46).
 * ══════════════════════════════════════════════════════════════════════ */
void DM1_ChampionPanel_BarGraphScreenXY(int champIdx, int statIndex,
                                         int *outX, int *outY)
{
    /* Bar-graph region starts at champIdx * 69 + 46 */
    int baseX = champIdx * DM1_STATUS_BOX_SPACING + 46;
    int barOffsets[3] = { 0, 7, 14 }; /* HP, stamina, mana within region */
    if (champIdx < 0 || champIdx >= DM1_CHAMPION_COUNT) {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
        return;
    }
    if (statIndex < 0 || statIndex >= DM1_BAR_GRAPH_COUNT) {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
        return;
    }
    if (outX) *outX = baseX + barOffsets[statIndex];
    if (outY) *outY = 2; /* All bars start at y=2 */
}

/* =====================================================================
 * Inventory champion HP/stamina/mana values - CHAMDRAW.C F0289/F0290.
 * PC34 draws seven-character "nnn/nnn" strings. Stamina is divided by
 * 10 before formatting, and the original PC34 source routes that stamina
 * text to C551_ZONE_MANA_VALUE, then mana text to C552_ZONE_STAMINA_VALUE.
 * ===================================================================== */
int DM1_ChampionPanel_StatusValueZone(int valueIndex)
{
    switch (valueIndex) {
    case DM1_STATUS_VALUE_HEALTH:
        return DM1_ZONE_HEALTH_VALUE;
    case DM1_STATUS_VALUE_STAMINA:
        return DM1_ZONE_MANA_VALUE;
    case DM1_STATUS_VALUE_MANA:
        return DM1_ZONE_STAMINA_VALUE;
    default:
        return -1;
    }
}

int DM1_ChampionPanel_FormatStatusValue(int valueIndex,
                                        int currentHealth, int maximumHealth,
                                        int currentStamina, int maximumStamina,
                                        int currentMana, int maximumMana,
                                        char *out, size_t outSize)
{
    int current;
    int maximum;
    int written;

    if (!out || outSize == 0) return 0;

    switch (valueIndex) {
    case DM1_STATUS_VALUE_HEALTH:
        current = currentHealth;
        maximum = maximumHealth;
        break;
    case DM1_STATUS_VALUE_STAMINA:
        current = currentStamina / 10;
        maximum = maximumStamina / 10;
        break;
    case DM1_STATUS_VALUE_MANA:
        current = currentMana;
        maximum = maximumMana;
        break;
    default:
        out[0] = 0;
        return 0;
    }

    written = snprintf(out, outSize, "%3d/%3d", current, maximum);
    return written >= 0 && (size_t)written < outSize;
}

/* =====================================================================
 * Inventory statistic current/max row - PANEL.C F0351.
 *
 * Source evidence:
 *   PANEL.C:2081-2091 reads Statistics[stat][C1_CURRENT] and
 *   Statistics[stat][C0_MAXIMUM], then colors only the current value:
 *   below maximum C08 red, above maximum C07 light green, equal C13
 *   lightest gray. PANEL.C:2098-2105 prints the "/nnn" maximum suffix
 *   separately in C13 lightest gray.
 * ===================================================================== */
int DM1_ChampionPanel_StatisticCurrentColor(int currentValue, int maximumValue)
{
    if (currentValue < maximumValue) {
        return DM1_COLOR_RED;
    }
    if (currentValue > maximumValue) {
        return DM1_COLOR_LIGHT_GREEN;
    }
    return DM1_COLOR_LIGHTEST_GRAY;
}

int DM1_ChampionPanel_StatisticMaximumColor(void)
{
    return DM1_COLOR_LIGHTEST_GRAY;
}

int DM1_ChampionPanel_FormatStatisticValue(int currentValue, int maximumValue,
                                           char *currentOut, size_t currentOutSize,
                                           char *maximumOut, size_t maximumOutSize)
{
    int currentWritten;
    int maximumWritten;

    if (!currentOut || !maximumOut || currentOutSize == 0 || maximumOutSize == 0) {
        return 0;
    }

    currentWritten = snprintf(currentOut, currentOutSize, "%3d", currentValue);
    maximumWritten = snprintf(maximumOut, maximumOutSize, "/%3d", maximumValue);
    return currentWritten >= 0 && maximumWritten >= 0 &&
           (size_t)currentWritten < currentOutSize &&
           (size_t)maximumWritten < maximumOutSize;
}

int DM1_ChampionPanel_BuildStatisticRowModel(
    int currentValue, int maximumValue,
    DM1_ChampionPanel_StatisticRowModel *outRow)
{
    if (!outRow) return 0;

    outRow->currentValue = currentValue;
    outRow->maximumValue = maximumValue;
    outRow->currentColor = DM1_ChampionPanel_StatisticCurrentColor(currentValue, maximumValue);
    outRow->maximumColor = DM1_ChampionPanel_StatisticMaximumColor();
    return DM1_ChampionPanel_FormatStatisticValue(currentValue, maximumValue,
                                                  outRow->currentText, sizeof(outRow->currentText),
                                                  outRow->maximumText, sizeof(outRow->maximumText));
}

int DM1_ChampionPanel_BuildStatisticTextRunModel(
    int statisticIndex, int currentValue, int maximumValue,
    DM1_ChampionPanel_StatisticTextRunModel *outRun)
{
    if (!outRun) return 0;
    if (statisticIndex < 0 || statisticIndex >= DM1_STATISTIC_ROW_COUNT) return 0;

    outRun->statisticIndex = statisticIndex;
    outRun->nameZone = DM1_ZONE_SKILL_VALUE;
    outRun->valueZone = DM1_ZONE_STATISTIC_VALUE;
    outRun->nameX = DM1_STATISTIC_NAME_REL_X;
    outRun->currentX = DM1_STATISTIC_CURRENT_REL_X;
    outRun->maximumX = DM1_STATISTIC_CURRENT_REL_X +
                       DM1_PANEL_TEXT_CHAR_WIDTH * 3;
    outRun->y = DM1_STATISTIC_FIRST_REL_Y +
                DM1_PANEL_TEXT_LINE_HEIGHT * statisticIndex;
    outRun->nameColor = DM1_COLOR_LIGHTEST_GRAY;
    outRun->currentColor =
        DM1_ChampionPanel_StatisticCurrentColor(currentValue, maximumValue);
    outRun->maximumColor = DM1_ChampionPanel_StatisticMaximumColor();
    return DM1_ChampionPanel_FormatStatisticValue(currentValue, maximumValue,
                                                  outRun->currentText, sizeof(outRun->currentText),
                                                  outRun->maximumText, sizeof(outRun->maximumText));
}

/* =====================================================================
 * Inventory champion load color/value - CHAMDRAW.C F0292.
 *
 * Source evidence:
 *   CHAMDRAW.C:958-967 chooses red if Load > F0309 maximum load, yellow
 *   if (Load << 3) > maxLoad * 5, otherwise lightest gray.
 *   CHAMDRAW.C:986-1017 builds current deci-kg, /, rounded max kg,
 *    KG, prints it to C555, then schedules the viewport refresh.
 *   CHAMDRAW.C:349-388 F0288 pads three-character integer fields with
 *   spaces when padding is requested.
 * ===================================================================== */
int DM1_ChampionPanel_LoadColor(int load, int maximumLoad)
{
    if (maximumLoad <= 0) {
        return load > 0 ? DM1_COLOR_RED : DM1_COLOR_LIGHTEST_GRAY;
    }
    if (load > maximumLoad) {
        return DM1_COLOR_RED;
    }
    if (((long)load << 3) > ((long)maximumLoad * 5L)) {
        return DM1_COLOR_YELLOW;
    }
    return DM1_COLOR_LIGHTEST_GRAY;
}

int DM1_ChampionPanel_FormatLoadValue(int load, int maximumLoad,
                                      char *out, size_t outSize)
{
    int loadKg;
    int loadTenths;
    int maximumKg;
    int written;

    if (!out || outSize == 0) return 0;

    loadKg = load / 10;
    loadTenths = load - (loadKg * 10);
    maximumKg = (maximumLoad + 5) / 10;
    written = snprintf(out, outSize, "%3d.%d/%3d KG", loadKg, loadTenths, maximumKg);
    return written >= 0 && (size_t)written < outSize;
}

int DM1_ChampionPanel_LoadValueZone(void)
{
    return DM1_ZONE_CHAMPION_LOAD_VALUE;
}

/* ══════════════════════════════════════════════════════════════════════
 * Inventory slot XY (viewport-relative) — layout-696 C507..C536
 *
 * The original ReDMCSB source defines slot box positions via the
 * G0030_as_Graphic562_SlotBoxes[46] table loaded from graphic 562
 * (layout-696). Slot boxes 8..37 are the inventory slots, viewport-
 * relative. The exact positions come from the zones_h_reconstruction.
 *
 * For the first 8 inventory slots (ready hand through quiver),
 * we provide the source-locked positions. Higher slots use a
 * computed grid layout matching the original backpack arrangement.
 * ══════════════════════════════════════════════════════════════════════ */
static const int s_inventorySlotX[30] = {
    /* SlotBox  8: Ready Hand  */   4,
    /* SlotBox  9: Action Hand */  24,
    /* SlotBox 10: Head        */  62,
    /* SlotBox 11: Torso       */  62,
    /* SlotBox 12: Legs        */  62,
    /* SlotBox 13: Feet        */  62,
    /* SlotBox 14: Pouch 2     */  98,
    /* SlotBox 15: Quiver L2/1 */ 116,
    /* SlotBox 16: Quiver L1/2 */ 134,
    /* SlotBox 17: Quiver L2/2 */ 152,
    /* SlotBox 18: Neck        */ 170,
    /* SlotBox 19: Pouch 1     */ 188,
    /* SlotBox 20: Quiver L1/1 */ 206,
    /* SlotBox 21: BP L1/1     */  98,
    /* SlotBox 22: BP L2/2     */ 116,
    /* SlotBox 23: BP L2/3     */ 134,
    /* SlotBox 24: BP L2/4     */ 152,
    /* SlotBox 25: BP L2/5     */ 170,
    /* SlotBox 26: BP L2/6     */ 188,
    /* SlotBox 27: BP L2/7     */ 206,
    /* SlotBox 28: BP L2/8     */  98,
    /* SlotBox 29: BP L2/9     */ 116,
    /* SlotBox 30: BP L1/2     */ 134,
    /* SlotBox 31: BP L1/3     */ 152,
    /* SlotBox 32: BP L1/4     */ 170,
    /* SlotBox 33: BP L1/5     */ 188,
    /* SlotBox 34: BP L1/6     */ 206,
    /* SlotBox 35: BP L1/7     */  98,
    /* SlotBox 36: BP L1/8     */ 116,
    /* SlotBox 37: BP L1/9     */ 134,
};

static const int s_inventorySlotY[30] = {
    /* SlotBox  8: Ready Hand  */  10,
    /* SlotBox  9: Action Hand */  10,
    /* SlotBox 10: Head        */  10,
    /* SlotBox 11: Torso       */  29,
    /* SlotBox 12: Legs        */  48,
    /* SlotBox 13: Feet        */  67,
    /* SlotBox 14: Pouch 2     */  10,
    /* SlotBox 15: Quiver L2/1 */  10,
    /* SlotBox 16: Quiver L1/2 */  10,
    /* SlotBox 17: Quiver L2/2 */  10,
    /* SlotBox 18: Neck        */  10,
    /* SlotBox 19: Pouch 1     */  10,
    /* SlotBox 20: Quiver L1/1 */  10,
    /* SlotBox 21: BP L1/1     */  29,
    /* SlotBox 22: BP L2/2     */  29,
    /* SlotBox 23: BP L2/3     */  29,
    /* SlotBox 24: BP L2/4     */  29,
    /* SlotBox 25: BP L2/5     */  29,
    /* SlotBox 26: BP L2/6     */  29,
    /* SlotBox 27: BP L2/7     */  29,
    /* SlotBox 28: BP L2/8     */  48,
    /* SlotBox 29: BP L2/9     */  48,
    /* SlotBox 30: BP L1/2     */  48,
    /* SlotBox 31: BP L1/3     */  48,
    /* SlotBox 32: BP L1/4     */  48,
    /* SlotBox 33: BP L1/5     */  48,
    /* SlotBox 34: BP L1/6     */  48,
    /* SlotBox 35: BP L1/7     */  67,
    /* SlotBox 36: BP L1/8     */  67,
    /* SlotBox 37: BP L1/9     */  67,
};

int DM1_ChampionPanel_InventorySlotXY(int slotBoxIndex,
                                       int *outX, int *outY)
{
    int idx = slotBoxIndex - DM1_SLOTBOX_FIRST_INVENTORY;
    if (idx < 0 || idx >= 30) return 0;
    if (outX) *outX = s_inventorySlotX[idx];
    if (outY) *outY = s_inventorySlotY[idx];
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Status hand slot XY — layout-696 C211..C218
 *
 * Source: zone C211 + champIdx*2 + handSlot
 *   Ready hand = parent status box + (4, 10)
 *   Action hand = parent status box + (24, 10)
 * ══════════════════════════════════════════════════════════════════════ */
void DM1_ChampionPanel_StatusHandSlotXY(int champIdx, int handSlot,
                                         int *outX, int *outY)
{
    int boxLeft = champIdx * DM1_STATUS_BOX_SPACING;
    if (champIdx < 0 || champIdx >= DM1_CHAMPION_COUNT) {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
        return;
    }
    if (outX) *outX = boxLeft + (handSlot == 0 ? 4 : 24);
    if (outY) *outY = 10;
}

/* ══════════════════════════════════════════════════════════════════════
 * Name color — CHAMDRAW.C F0292
 * Leader gets C09_COLOR_GOLD, others C13_COLOR_LIGHTEST_GRAY.
 * ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_NameColor(int champIdx, int leaderIdx)
{
    return (champIdx == leaderIdx) ? DM1_COLOR_GOLD : DM1_COLOR_LIGHTEST_GRAY;
}

/* ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_IsDeadStatusBox(int currentHealth)
{
    return currentHealth <= 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Self-test — validates source-locked constants and computations
 * ══════════════════════════════════════════════════════════════════════ */
int DM1_ChampionPanel_SelfTest(void)
{
    int failures = 0;

    /* Champion colors */
    if (DM1_ChampionColor[0] != 7)  { failures++; fprintf(stderr, "FAIL: ChampionColor[0] != 7\n"); }
    if (DM1_ChampionColor[1] != 11) { failures++; fprintf(stderr, "FAIL: ChampionColor[1] != 11\n"); }
    if (DM1_ChampionColor[2] != 8)  { failures++; fprintf(stderr, "FAIL: ChampionColor[2] != 8\n"); }
    if (DM1_ChampionColor[3] != 14) { failures++; fprintf(stderr, "FAIL: ChampionColor[3] != 14\n"); }

    /* Bar graph heights — F0287 algorithm */
    if (DM1_ChampionPanel_BarGraphHeight(0, 100, 0) != 0)  { failures++; fprintf(stderr, "FAIL: bar 0/100\n"); }
    if (DM1_ChampionPanel_BarGraphHeight(100, 100, 0) != 25) { failures++; fprintf(stderr, "FAIL: bar 100/100\n"); }
    if (DM1_ChampionPanel_BarGraphHeight(50, 100, 0) != 13) { failures++; fprintf(stderr, "FAIL: bar 50/100 expect 13\n"); }
    /* 50*25/100 = 12.5 → ceil = 13. Verify via fixed-point:
     * (50 << 10) * 25 / 100 = 51200 * 25 / 100 = 12800.
     * 12800 & 0x3FF = 0x200 (nonzero) → (12800 >> 10) + 1 = 12 + 1 = 13. ✓ */
    if (DM1_ChampionPanel_BarGraphHeight(1, 100, 0) != 1) { failures++; fprintf(stderr, "FAIL: bar 1/100\n"); }
    /* Mana > max → capped at 25 */
    if (DM1_ChampionPanel_BarGraphHeight(200, 100, 1) != 25) { failures++; fprintf(stderr, "FAIL: mana 200/100\n"); }
    /* Mana within range → normal calc */
    if (DM1_ChampionPanel_BarGraphHeight(50, 100, 1) != 13) { failures++; fprintf(stderr, "FAIL: mana 50/100\n"); }

    /* Slot box graphic — F0291 logic */
    if (DM1_ChampionPanel_SlotBoxGraphic(0, 0x0000, 0) != DM1_GFX_SLOT_NORMAL) { failures++; fprintf(stderr, "FAIL: slot0 normal\n"); }
    if (DM1_ChampionPanel_SlotBoxGraphic(0, 0x0001, 0) != DM1_GFX_SLOT_WOUNDED) { failures++; fprintf(stderr, "FAIL: slot0 wounded\n"); }
    if (DM1_ChampionPanel_SlotBoxGraphic(1, 0x0000, 1) != DM1_GFX_SLOT_ACTING) { failures++; fprintf(stderr, "FAIL: slot1 acting\n"); }
    if (DM1_ChampionPanel_SlotBoxGraphic(1, 0x0002, 0) != DM1_GFX_SLOT_WOUNDED) { failures++; fprintf(stderr, "FAIL: slot1 wounded\n"); }
    /* Acting overrides wounded: F0292 sets acting AFTER wound check */
    if (DM1_ChampionPanel_SlotBoxGraphic(1, 0x0002, 1) != DM1_GFX_SLOT_ACTING) { failures++; fprintf(stderr, "FAIL: slot1 acting+wounded\n"); }
    if (DM1_ChampionPanel_SlotBoxGraphic(6, 0, 0) != -1) { failures++; fprintf(stderr, "FAIL: slot6 no graphic\n"); }

    /* Portrait X */
    if (DM1_ChampionPanel_PortraitScreenX(0) != 7)   { failures++; fprintf(stderr, "FAIL: portrait0 X\n"); }
    if (DM1_ChampionPanel_PortraitScreenX(1) != 76)  { failures++; fprintf(stderr, "FAIL: portrait1 X\n"); }
    if (DM1_ChampionPanel_PortraitScreenX(3) != 214) { failures++; fprintf(stderr, "FAIL: portrait3 X\n"); }

    /* Name zone X */
    if (DM1_ChampionPanel_NameZoneX(0) != 0)   { failures++; fprintf(stderr, "FAIL: name0 X\n"); }
    if (DM1_ChampionPanel_NameZoneX(2) != 138) { failures++; fprintf(stderr, "FAIL: name2 X\n"); }

    /* Bar graph positions */
    {
        int bx, by;
        DM1_ChampionPanel_BarGraphScreenXY(0, 0, &bx, &by);
        if (bx != 46 || by != 2) { failures++; fprintf(stderr, "FAIL: bar0_0 XY (%d,%d)\n", bx, by); }
        DM1_ChampionPanel_BarGraphScreenXY(0, 1, &bx, &by);
        if (bx != 53 || by != 2) { failures++; fprintf(stderr, "FAIL: bar0_1 XY (%d,%d)\n", bx, by); }
        DM1_ChampionPanel_BarGraphScreenXY(0, 2, &bx, &by);
        if (bx != 60 || by != 2) { failures++; fprintf(stderr, "FAIL: bar0_2 XY (%d,%d)\n", bx, by); }
        DM1_ChampionPanel_BarGraphScreenXY(1, 0, &bx, &by);
        if (bx != 115 || by != 2) { failures++; fprintf(stderr, "FAIL: bar1_0 XY (%d,%d)\n", bx, by); }
    }

    /* Status hand slot positions */
    {
        int hx, hy;
        DM1_ChampionPanel_StatusHandSlotXY(0, 0, &hx, &hy);
        if (hx != 4 || hy != 10) { failures++; fprintf(stderr, "FAIL: hand0_0 XY (%d,%d)\n", hx, hy); }
        DM1_ChampionPanel_StatusHandSlotXY(0, 1, &hx, &hy);
        if (hx != 24 || hy != 10) { failures++; fprintf(stderr, "FAIL: hand0_1 XY (%d,%d)\n", hx, hy); }
        DM1_ChampionPanel_StatusHandSlotXY(1, 0, &hx, &hy);
        if (hx != 73 || hy != 10) { failures++; fprintf(stderr, "FAIL: hand1_0 XY (%d,%d)\n", hx, hy); }
    }

    /* Inventory slot XY — spot checks */
    {
        int sx, sy;
        if (!DM1_ChampionPanel_InventorySlotXY(8, &sx, &sy)) { failures++; fprintf(stderr, "FAIL: inv slot 8\n"); }
        else if (sx != 4 || sy != 10) { failures++; fprintf(stderr, "FAIL: inv slot 8 XY (%d,%d)\n", sx, sy); }
        if (!DM1_ChampionPanel_InventorySlotXY(10, &sx, &sy)) { failures++; fprintf(stderr, "FAIL: inv slot 10\n"); }
        else if (sx != 62 || sy != 10) { failures++; fprintf(stderr, "FAIL: inv slot 10 (Head) XY (%d,%d)\n", sx, sy); }
        if (DM1_ChampionPanel_InventorySlotXY(7, &sx, &sy)) { failures++; fprintf(stderr, "FAIL: inv slot 7 should fail\n"); }
        if (DM1_ChampionPanel_InventorySlotXY(38, &sx, &sy)) { failures++; fprintf(stderr, "FAIL: inv slot 38 out of range\n"); }
    }

    /* PANEL.C F0351 statistic current/max row color and formatting */
    if (DM1_ChampionPanel_StatisticCurrentColor(49, 50) != DM1_COLOR_RED) { failures++; fprintf(stderr, "FAIL: statistic 49/50 color\n"); }
    if (DM1_ChampionPanel_StatisticCurrentColor(50, 50) != DM1_COLOR_LIGHTEST_GRAY) { failures++; fprintf(stderr, "FAIL: statistic 50/50 color\n"); }
    if (DM1_ChampionPanel_StatisticCurrentColor(51, 50) != DM1_COLOR_LIGHT_GREEN) { failures++; fprintf(stderr, "FAIL: statistic 51/50 color\n"); }
    if (DM1_ChampionPanel_StatisticMaximumColor() != DM1_COLOR_LIGHTEST_GRAY) { failures++; fprintf(stderr, "FAIL: statistic max suffix color\n"); }
    {
        char currentValue[4];
        char maximumValue[5];
        DM1_ChampionPanel_StatisticRowModel row;
        if (!DM1_ChampionPanel_FormatStatisticValue(49, 50, currentValue, sizeof(currentValue), maximumValue, sizeof(maximumValue)) ||
            strcmp(currentValue, " 49") != 0 || strcmp(maximumValue, "/ 50") != 0) {
            failures++;
            fprintf(stderr, "FAIL: statistic row format got %s %s\n", currentValue, maximumValue);
        }
        if (!DM1_ChampionPanel_BuildStatisticRowModel(51, 50, &row) ||
            row.currentValue != 51 || row.maximumValue != 50 ||
            row.currentColor != DM1_COLOR_LIGHT_GREEN ||
            row.maximumColor != DM1_COLOR_LIGHTEST_GRAY ||
            strcmp(row.currentText, " 51") != 0 ||
            strcmp(row.maximumText, "/ 50") != 0) {
            failures++;
            fprintf(stderr, "FAIL: statistic row model\n");
        }
        {
            DM1_ChampionPanel_StatisticTextRunModel run;
            if (!DM1_ChampionPanel_BuildStatisticTextRunModel(2, 51, 50, &run) ||
                run.nameZone != DM1_ZONE_SKILL_VALUE ||
                run.valueZone != DM1_ZONE_STATISTIC_VALUE ||
                run.nameX != DM1_STATISTIC_NAME_REL_X ||
                run.currentX != DM1_STATISTIC_CURRENT_REL_X ||
                run.maximumX != DM1_STATISTIC_CURRENT_REL_X + DM1_PANEL_TEXT_CHAR_WIDTH * 3 ||
                run.y != DM1_STATISTIC_FIRST_REL_Y + DM1_PANEL_TEXT_LINE_HEIGHT * 2 ||
                run.nameColor != DM1_COLOR_LIGHTEST_GRAY ||
                run.currentColor != DM1_COLOR_LIGHT_GREEN ||
                run.maximumColor != DM1_COLOR_LIGHTEST_GRAY ||
                strcmp(run.currentText, " 51") != 0 ||
                strcmp(run.maximumText, "/ 50") != 0) {
                failures++;
                fprintf(stderr, "FAIL: statistic text run model\n");
            }
            if (DM1_ChampionPanel_BuildStatisticTextRunModel(6, 51, 50, &run)) {
                failures++;
                fprintf(stderr, "FAIL: statistic text run out-of-range index\n");
            }
        }
    }

    /* CHAMDRAW.C F0292 status-box redraw propagation */
    {
        DM1_ChampionPanel_StatusBoxModel model;
        if (!DM1_ChampionPanel_BuildStatusBoxModel(0, 0, 1, 77, &model) ||
            model.drawKind != DM1_STATUS_BOX_DRAW_ALIVE ||
            model.fillColor != DM1_COLOR_DARKEST_GRAY ||
            model.drawPortrait != 1 ||
            model.propagatedAttributes != DM1_ATTR_STATISTICS ||
            model.drawActionIcon != 0 ||
            model.stopAfterDead != 0) {
            failures++;
            fprintf(stderr, "FAIL: live inventory status-box model\n");
        }
        if (!DM1_ChampionPanel_BuildStatusBoxModel(1, 0, 0, 77, &model) ||
            model.drawKind != DM1_STATUS_BOX_DRAW_ALIVE ||
            model.fillColor != DM1_COLOR_DARKEST_GRAY ||
            model.drawPortrait != 0 ||
            model.propagatedAttributes !=
                (DM1_ATTR_NAME_TITLE | DM1_ATTR_STATISTICS |
                 DM1_ATTR_WOUNDS | DM1_ATTR_ACTION_HAND)) {
            failures++;
            fprintf(stderr, "FAIL: live status-box propagated attributes\n");
        }
        if (!DM1_ChampionPanel_BuildStatusBoxModel(2, 0, 0, 0, &model) ||
            model.drawKind != DM1_STATUS_BOX_DRAW_DEAD ||
            model.graphicId != DM1_GFX_DEAD_CHAMPION ||
            model.nameColor != DM1_COLOR_LIGHTEST_GRAY ||
            model.nameBackgroundColor != DM1_COLOR_DARK_GRAY ||
            model.drawActionIcon != 1 || model.stopAfterDead != 1) {
            failures++;
            fprintf(stderr, "FAIL: dead status-box model\n");
        }
    }

    /* Name color */
    if (DM1_ChampionPanel_NameColor(0, 0) != DM1_COLOR_GOLD) { failures++; fprintf(stderr, "FAIL: name color leader\n"); }
    if (DM1_ChampionPanel_NameColor(1, 0) != DM1_COLOR_LIGHTEST_GRAY) { failures++; fprintf(stderr, "FAIL: name color non-leader\n"); }

    /* Dead predicate */
    if (!DM1_ChampionPanel_IsDeadStatusBox(0)) { failures++; fprintf(stderr, "FAIL: dead 0\n"); }
    if (!DM1_ChampionPanel_IsDeadStatusBox(-1)) { failures++; fprintf(stderr, "FAIL: dead -1\n"); }
    if (DM1_ChampionPanel_IsDeadStatusBox(1)) { failures++; fprintf(stderr, "FAIL: alive 1\n"); }

    return failures;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — PANEL.C remaining function citations
 *
 *   PANEL.C:1476 F0343_INVENTORY_D
 *   PANEL.C:534 F0803_D
 *   PANEL.C:545 F0804_D
 *   PANEL.C:802 F0805_C
 *   PANEL.C:841 F0817_S
 * ══════════════════════════════════════════════════════════════════════ */

