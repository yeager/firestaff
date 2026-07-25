/*
 * CSB V1 Champion Panel & Inventory HUD — pc34 compat implementation.
 *
 * Every constant and computation in this file is source-locked to
 * ReDMCSB_WIP20210206. CSB shares the same ReDMCSB source as DM1
 * for champion panel geometry, bar graphs, and slot box logic.
 * CSB-specific differences: graphic IDs from CSBGRAPHICS.DAT,
 * panel surfaces C017/C040, no invisibility mechanic.
 */

#include "csb_v1_champion_panel_hud_pc34_compat.h"
#include <string.h>
#include <stdio.h>

/* G0046_auc_Graphic562_ChampionColor[4] — same as DM1.
 * ReDMCSB COORD.C: green(7), yellow(11), red(8), blue(14). */
const uint8_t CSB_ChampionColor[CSB_CHAMPION_COUNT] = { 7, 11, 8, 14 };

/* ReDMCSB CHAMDRAW.C F0287 — bar graph height.
 * Fixed-point 10.10 multiply by 25, round-up on fractional remainder.
 * ceil(current * 25 / maximum) when current > 0.
 * Mana: if current > maximum, height = 25. */
int CSB_ChampionPanel_BarGraphHeight(int current, int maximum, int isMana)
{
    long scaled;
    if (current <= 0) return 0;
    if (maximum <= 0) return 0;
    if (isMana && current > maximum) return CSB_BAR_GRAPH_MAX_HEIGHT;
    scaled = (((long)current << 10) * CSB_BAR_GRAPH_MAX_HEIGHT) / (long)maximum;
    if (scaled & 0x3FFL)
        return (int)(scaled >> 10) + 1;
    return (int)(scaled >> 10);
}

/* ReDMCSB layout-696 C195..C206 — bar graph screen XY.
 * HP bars at champIdx * 69 + 46, stamina +7, mana +14. y=2. */
void CSB_ChampionPanel_BarGraphScreenXY(int champIdx, int statIndex,
                                        int *outX, int *outY)
{
    int baseX;
    static const int barOffsets[3] = { 0, 7, 14 };
    if (champIdx < 0 || champIdx >= CSB_CHAMPION_COUNT ||
        statIndex < 0 || statIndex >= CSB_BAR_GRAPH_COUNT) {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
        return;
    }
    baseX = champIdx * CSB_STATUS_BOX_SPACING + 46;
    if (outX) *outX = baseX + barOffsets[statIndex];
    if (outY) *outY = 2;
}

/* ReDMCSB CHAMDRAW.C F0287 — bar fill model.
 * PC34 branch: blank height shrinks by max(1, height*current/maximum). */
int CSB_ChampionPanel_BuildPc34BarFillModel(
    int championIndex, int statIndex, int current, int maximum,
    CSB_ChampionPanel_BarFillModel *outModel)
{
    int filledHeight;

    if (!outModel ||
        championIndex < 0 || championIndex >= CSB_CHAMPION_COUNT ||
        statIndex < 0 || statIndex >= CSB_BAR_GRAPH_COUNT ||
        maximum <= 0) {
        return 0;
    }

    memset(outModel, 0, sizeof(*outModel));
    CSB_ChampionPanel_BarGraphScreenXY(championIndex, statIndex,
                                       &outModel->x, &outModel->y);

    outModel->zoneId = 195 + championIndex + (statIndex * 4);
    outModel->width = CSB_BAR_GRAPH_WIDTH;
    outModel->height = CSB_BAR_GRAPH_MAX_HEIGHT;
    outModel->blankColor = CSB_COLOR_DARKEST_GRAY;
    outModel->fillColor = CSB_ChampionColor[championIndex];
    outModel->blankX = outModel->x;
    outModel->blankY = outModel->y;
    outModel->blankWidth = outModel->width;
    outModel->fillX = outModel->x;
    outModel->fillWidth = outModel->width;

    if (current < maximum) {
        outModel->blankHeight = outModel->height;
        if (current != 0) {
            filledHeight = (int)(((long)outModel->height * (long)current) /
                                 (long)maximum);
            if (filledHeight < 1)
                filledHeight = 1;
            outModel->blankHeight -= filledHeight;
        }
        outModel->emitsBlank = outModel->blankHeight > 0;
    } else {
        outModel->blankHeight = 0;
        outModel->emitsBlank = 0;
    }

    if (current != 0) {
        outModel->fillY = outModel->y + outModel->blankHeight;
        outModel->fillHeight = outModel->height - outModel->blankHeight;
        outModel->emitsFill = outModel->fillHeight > 0;
    } else {
        outModel->fillY = outModel->y + outModel->height;
        outModel->fillHeight = 0;
        outModel->emitsFill = 0;
    }

    return 1;
}

/* ReDMCSB CHAMDRAW.C F0292 — status box model. */
int CSB_ChampionPanel_BuildStatusBoxModel(
    int championIndex, int leaderIndex, int isInventoryChampion,
    int currentHealth, CSB_ChampionPanel_StatusBoxModel *outModel)
{
    (void)leaderIndex;

    if (!outModel || championIndex < 0 || championIndex >= CSB_CHAMPION_COUNT)
        return 0;

    memset(outModel, 0, sizeof(*outModel));
    outModel->fillColor = -1;
    outModel->graphicId = -1;
    outModel->nameColor = -1;
    outModel->nameBackgroundColor = -1;

    if (currentHealth > 0) {
        outModel->drawKind = CSB_STATUS_BOX_DRAW_ALIVE;
        outModel->fillColor = CSB_COLOR_DARKEST_GRAY;
        if (isInventoryChampion) {
            outModel->drawPortrait = 1;
            outModel->propagatedAttributes = CSB_ATTR_STATISTICS;
        } else {
            outModel->propagatedAttributes =
                CSB_ATTR_NAME_TITLE | CSB_ATTR_STATISTICS |
                CSB_ATTR_WOUNDS | CSB_ATTR_ACTION_HAND;
        }
        return 1;
    }

    outModel->drawKind = CSB_STATUS_BOX_DRAW_DEAD;
    outModel->graphicId = CSB_GFX_DEAD_CHAMPION;
    outModel->nameColor = CSB_COLOR_LIGHTEST_GRAY;
    outModel->nameBackgroundColor = CSB_COLOR_DARK_GRAY;
    outModel->drawActionIcon = 1;
    outModel->stopAfterDead = 1;
    return 1;
}

/* ReDMCSB CHAMDRAW.C F0622 — icon bitmap model.
 * CSB has no invisibility mechanic, so no applyInvisibilityPalette. */
int CSB_ChampionPanel_BuildIconBitmapModel(
    int championIndex, int championDirection, int partyDirection,
    CSB_ChampionPanel_IconBitmapModel *outModel)
{
    int iconIndex;

    if (!outModel ||
        championIndex < 0 || championIndex >= CSB_CHAMPION_COUNT ||
        championDirection < 0 || championDirection > 3 ||
        partyDirection < 0 || partyDirection > 3)
        return 0;

    memset(outModel, 0, sizeof(*outModel));
    iconIndex = (championDirection + 4 - partyDirection) & 0x0003;

    outModel->width = CSB_CHAMPION_ICON_WIDTH;
    outModel->height = CSB_CHAMPION_ICON_HEIGHT;
    outModel->fillColor = CSB_ChampionColor[championIndex];
    outModel->graphicId = CSB_GFX_CHAMPION_ICONS;
    outModel->sourceX = iconIndex * CSB_CHAMPION_ICON_WIDTH;
    outModel->sourceY = 0;
    outModel->transparentColor = CSB_COLOR_DARKEST_GRAY;
    return 1;
}

/* ReDMCSB CHAMDRAW.C F0291 — slot box graphic.
 * For slots 0..5: acting override on action hand, then wound check. */
int CSB_ChampionPanel_SlotBoxGraphic(int slotIndex, uint16_t wounds,
                                     int isActingChampion)
{
    if (slotIndex < 0 || slotIndex > 5) return -1;
    if (slotIndex == CSB_SLOT_ACTION_HAND && isActingChampion)
        return CSB_GFX_SLOT_ACTING;
    if (wounds & (1u << slotIndex))
        return CSB_GFX_SLOT_WOUNDED;
    return CSB_GFX_SLOT_NORMAL;
}

/* ReDMCSB layout-696 C211..C218 — status hand slot XY.
 * Ready hand: champIdx * 69 + 4, y = 10.
 * Action hand: champIdx * 69 + 24, y = 10. */
void CSB_ChampionPanel_StatusHandSlotXY(int champIdx, int handSlot,
                                        int *outX, int *outY)
{
    if (champIdx < 0 || champIdx >= CSB_CHAMPION_COUNT ||
        handSlot < 0 || handSlot > 1) {
        if (outX) *outX = 0;
        if (outY) *outY = 0;
        return;
    }
    if (outX) *outX = champIdx * CSB_STATUS_BOX_SPACING + (handSlot == 0 ? 4 : 24);
    if (outY) *outY = 10;
}

/* ReDMCSB CHAMDRAW.C F0291 — status hand slot box model. */
int CSB_ChampionPanel_BuildStatusHandSlotBoxModel(
    int championIndex, int handIndex, int isActingChampion,
    CSB_ChampionPanel_StatusHandSlotBoxModel *outModel)
{
    int x, y;

    if (!outModel ||
        championIndex < 0 || championIndex >= CSB_CHAMPION_COUNT ||
        handIndex < 0 || handIndex > 1)
        return 0;

    CSB_ChampionPanel_StatusHandSlotXY(championIndex, handIndex, &x, &y);
    memset(outModel, 0, sizeof(*outModel));
    outModel->championIndex = championIndex;
    outModel->handIndex = handIndex;
    outModel->isActionHand = (handIndex == CSB_SLOT_ACTION_HAND) ? 1 : 0;
    outModel->isActingChampion = isActingChampion ? 1 : 0;
    outModel->x = x;
    outModel->y = y;
    outModel->width = CSB_SLOT_BOX_SIZE;
    outModel->height = CSB_SLOT_BOX_SIZE;
    outModel->graphicId = CSB_ChampionPanel_SlotBoxGraphic(
        handIndex, 0u, isActingChampion ? 1 : 0);
    return 1;
}

/* ReDMCSB CHAMDRAW.C F0354 — portrait screen X.
 * champIdx * 69 + 7. */
int CSB_ChampionPanel_PortraitScreenX(int champIdx)
{
    if (champIdx < 0 || champIdx >= CSB_CHAMPION_COUNT) return 0;
    return champIdx * CSB_STATUS_BOX_SPACING + CSB_PORTRAIT_OFFSET_X;
}

/* ReDMCSB CHAMDRAW.C F0292 — name zone X.
 * champIdx * 69. */
int CSB_ChampionPanel_NameZoneX(int champIdx)
{
    if (champIdx < 0 || champIdx >= CSB_CHAMPION_COUNT) return 0;
    return champIdx * CSB_STATUS_BOX_SPACING;
}

/* Name color: leader = gold(9), others = lightest gray(13). */
int CSB_ChampionPanel_NameColor(int champIdx, int leaderIdx)
{
    (void)champIdx;
    return (champIdx >= 0 && champIdx < CSB_CHAMPION_COUNT &&
            champIdx == leaderIdx) ? CSB_COLOR_GOLD : CSB_COLOR_LIGHTEST_GRAY;
}

int CSB_ChampionPanel_IsDeadStatusBox(int currentHealth)
{
    return currentHealth <= 0;
}

/* ReDMCSB CHAMDRAW.C F0289 — status value zone. */
int CSB_ChampionPanel_StatusValueZone(int valueIndex)
{
    switch (valueIndex) {
    case 0: return CSB_ZONE_HEALTH_VALUE;
    case 1: return CSB_ZONE_STAMINA_VALUE;
    case 2: return CSB_ZONE_MANA_VALUE;
    default: return -1;
    }
}

/* ReDMCSB CHAMDRAW.C F0289/F0290 — format "nnn/nnn" status value.
 * Stamina display divides by 10. */
int CSB_ChampionPanel_FormatStatusValue(int valueIndex,
    int currentHealth, int maximumHealth,
    int currentStamina, int maximumStamina,
    int currentMana, int maximumMana,
    char *out, size_t outSize)
{
    int cur, mx;
    if (!out || outSize < 8) return 0;
    switch (valueIndex) {
    case 0: cur = currentHealth;  mx = maximumHealth;  break;
    case 1: cur = currentStamina / 10; mx = maximumStamina / 10; break;
    case 2: cur = currentMana;   mx = maximumMana;    break;
    default: return 0;
    }
    if (cur < 0) cur = 0;
    if (cur > 999) cur = 999;
    if (mx < 0) mx = 0;
    if (mx > 999) mx = 999;
    (void)snprintf(out, outSize, "%3d/%3d", cur, mx);
    return 1;
}

/* ReDMCSB CHAMDRAW.C F0288 — space-padded integer formatting. */
int CSB_ChampionPanel_FormatIntegerF0288(int value, int paddingEnabled,
                                         int width, char *out,
                                         size_t outSize)
{
    char digits[32];
    int digitCount;
    int padCount;
    int i;

    if (!out || outSize == 0) return 0;
    out[0] = '\0';

    digitCount = snprintf(digits, sizeof(digits), "%d", value);
    if (digitCount < 0 || (size_t)digitCount >= sizeof(digits))
        return 0;
    if (!paddingEnabled || width <= digitCount) {
        if ((size_t)digitCount >= outSize) return 0;
        memcpy(out, digits, (size_t)digitCount + 1u);
        return 1;
    }
    if ((size_t)width >= outSize) return 0;
    padCount = width - digitCount;
    for (i = 0; i < padCount; ++i)
        out[i] = ' ';
    memcpy(out + padCount, digits, (size_t)digitCount + 1u);
    return 1;
}

/* ReDMCSB layout-696 C507..C536 — inventory slot XY. */
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

int CSB_ChampionPanel_InventorySlotXY(int slotBoxIndex,
                                       int *outX, int *outY)
{
    int idx = slotBoxIndex - CSB_SLOTBOX_FIRST_INVENTORY;
    if (idx < 0 || idx >= 30) return 0;
    if (outX) *outX = s_inventorySlotX[idx];
    if (outY) *outY = s_inventorySlotY[idx];
    return 1;
}

/* ReDMCSB DEFS.H C212 — empty hand icon index. */
int CSB_ChampionPanel_EmptyHandIconIndex(int slotIndex, uint16_t wounds)
{
    if (slotIndex < 0 || slotIndex > 1) return -1;
    return CSB_GFX_READY_HAND_ICON + slotIndex * 2 +
           ((wounds & (1u << slotIndex)) ? 1 : 0);
}

/* ReDMCSB PANEL.C F0351 — statistic current color. */
int CSB_ChampionPanel_StatisticCurrentColor(int currentValue, int maximumValue)
{
    if (currentValue < maximumValue) return CSB_COLOR_RED;
    if (currentValue > maximumValue) return CSB_COLOR_LIGHT_GREEN;
    return CSB_COLOR_LIGHTEST_GRAY;
}

int CSB_ChampionPanel_StatisticMaximumColor(void)
{
    return CSB_COLOR_LIGHTEST_GRAY;
}

int CSB_ChampionPanel_FormatStatisticValue(int currentValue, int maximumValue,
                                           char *currentOut, size_t currentOutSize,
                                           char *maximumOut, size_t maximumOutSize)
{
    int currentWritten;
    int maximumWritten;

    if (!currentOut || !maximumOut || currentOutSize == 0 || maximumOutSize == 0)
        return 0;

    currentWritten = CSB_ChampionPanel_FormatIntegerF0288(
        currentValue, 1, 3, currentOut, currentOutSize);
    if (maximumOutSize < 2) {
        if (maximumOutSize > 0) maximumOut[0] = '\0';
        return 0;
    }
    maximumOut[0] = '/';
    maximumWritten = CSB_ChampionPanel_FormatIntegerF0288(
        maximumValue, 1, 3, maximumOut + 1, maximumOutSize - 1u);
    return currentWritten && maximumWritten;
}

int CSB_ChampionPanel_BuildStatisticRowModel(
    int currentValue, int maximumValue,
    CSB_ChampionPanel_StatisticRowModel *outRow)
{
    if (!outRow) return 0;
    outRow->currentValue = currentValue;
    outRow->maximumValue = maximumValue;
    outRow->currentColor = CSB_ChampionPanel_StatisticCurrentColor(currentValue, maximumValue);
    outRow->maximumColor = CSB_ChampionPanel_StatisticMaximumColor();
    return CSB_ChampionPanel_FormatStatisticValue(currentValue, maximumValue,
                                                  outRow->currentText, sizeof(outRow->currentText),
                                                  outRow->maximumText, sizeof(outRow->maximumText));
}

int CSB_ChampionPanel_BuildStatisticTextRunModel(
    int statisticIndex, int currentValue, int maximumValue,
    CSB_ChampionPanel_StatisticTextRunModel *outRun)
{
    if (!outRun) return 0;
    if (statisticIndex < 0 || statisticIndex >= CSB_STATISTIC_ROW_COUNT) return 0;

    outRun->statisticIndex = statisticIndex;
    outRun->nameZone = CSB_ZONE_SKILL_VALUE;
    outRun->valueZone = CSB_ZONE_STATISTIC_VALUE;
    outRun->nameX = CSB_STATISTIC_NAME_REL_X;
    outRun->currentX = CSB_STATISTIC_CURRENT_REL_X;
    outRun->maximumX = CSB_STATISTIC_CURRENT_REL_X +
                       CSB_PANEL_TEXT_CHAR_WIDTH * 3;
    outRun->y = CSB_STATISTIC_FIRST_REL_Y +
                CSB_PANEL_TEXT_LINE_HEIGHT * statisticIndex;
    outRun->nameColor = CSB_COLOR_LIGHTEST_GRAY;
    outRun->currentColor =
        CSB_ChampionPanel_StatisticCurrentColor(currentValue, maximumValue);
    outRun->maximumColor = CSB_ChampionPanel_StatisticMaximumColor();
    return CSB_ChampionPanel_FormatStatisticValue(currentValue, maximumValue,
                                                  outRun->currentText, sizeof(outRun->currentText),
                                                  outRun->maximumText, sizeof(outRun->maximumText));
}

/* ReDMCSB CHAMDRAW.C F0292 — load color. */
int CSB_ChampionPanel_LoadColor(int load, int maximumLoad)
{
    if (maximumLoad <= 0)
        return load > 0 ? CSB_COLOR_RED : CSB_COLOR_LIGHTEST_GRAY;
    if (load > maximumLoad)
        return CSB_COLOR_RED;
    if (((long)load << 3) > ((long)maximumLoad * 5L))
        return CSB_COLOR_YELLOW;
    return CSB_COLOR_LIGHTEST_GRAY;
}

int CSB_ChampionPanel_FormatLoadValue(int load, int maximumLoad,
                                      char *out, size_t outSize)
{
    int loadKg, loadTenths, maximumKg;
    char loadKgText[16], maximumKgText[16];

    if (!out || outSize == 0) return 0;

    loadKg = load / 10;
    loadTenths = load - (loadKg * 10);
    maximumKg = (maximumLoad + 5) / 10;
    if (!CSB_ChampionPanel_FormatIntegerF0288(
            loadKg, 1, 3, loadKgText, sizeof(loadKgText)) ||
        !CSB_ChampionPanel_FormatIntegerF0288(
            maximumKg, 1, 3, maximumKgText, sizeof(maximumKgText))) {
        out[0] = '\0';
        return 0;
    }
    return snprintf(out, outSize, "%s.%d/%s KG",
                    loadKgText, loadTenths, maximumKgText) >= 0 &&
           strlen(out) + 1u <= outSize;
}

int CSB_ChampionPanel_LoadValueZone(void)
{
    return CSB_ZONE_CHAMPION_LOAD_VALUE;
}

/* ReDMCSB PANEL.C F0345 — food/water/poison label blit spec. */
static const char k_csbChampionPanelF0658PoisonedBlitEvidence[] =
    "ReDMCSB PANEL.C:1563-1606 F0345_INVENTORY_DrawPanel_FoodWaterPoisoned; "
    "PANEL.C:1598-1606 F0658(C030,C500,C12), F0658(C031,C501,C12), "
    "if(PoisonEventCount) F0658(C032,C502,C12); "
    "CHAMDRAW.C:1060-1063 F0292 mouth-panel call; "
    "BASE.C:1341-1361 F0658_BlitBitmapIndexToZoneIndexWithTransparency; "
    "DEFS.H:2090 C12, 2190-2192 C030/C031/C032, 3869-3871 C500/C501/C502";

static const CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec
    k_csbChampionPanelF0658PoisonedBlitSpec = {
        CSB_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT,
        1598,
        1606,
        1601,
        k_csbChampionPanelF0658PoisonedBlitEvidence,
        {
            { CSB_GFX_FOOD_LABEL, CSB_ZONE_FOOD, CSB_COLOR_DARKEST_GRAY, 1598, 0 },
            { CSB_GFX_WATER_LABEL, CSB_ZONE_WATER, CSB_COLOR_DARKEST_GRAY, 1599, 0 },
            { CSB_GFX_POISONED_LABEL, CSB_ZONE_POISONED, CSB_COLOR_DARKEST_GRAY, 1606, 1 },
        }
    };

const CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec *
CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec_SourceLocked(void)
{
    return &k_csbChampionPanelF0658PoisonedBlitSpec;
}

/* ReDMCSB INVNTORY.C F0354 — portrait box blit model.
 * Extracts championIndex * 29 row from graphic 26. */
int CSB_ChampionPanel_BuildPortraitBlitModel(
    int championIndex,
    CSB_ChampionPanel_PortraitBlitModel *outModel)
{
    if (!outModel || championIndex < 0 || championIndex >= CSB_CHAMPION_COUNT)
        return 0;

    memset(outModel, 0, sizeof(*outModel));
    outModel->championIndex = championIndex;
    outModel->graphicId = CSB_GFX_PORTRAITS;
    outModel->sourceX = 0;
    outModel->sourceY = championIndex * CSB_PORTRAIT_SOURCE_Y_STRIDE;
    outModel->destX = CSB_ChampionPanel_PortraitScreenX(championIndex);
    outModel->destY = CSB_PORTRAIT_Y;
    outModel->width = CSB_PORTRAIT_WIDTH;
    outModel->height = CSB_PORTRAIT_HEIGHT;
    outModel->transparentColor = CSB_COLOR_DARKEST_GRAY;
    return 1;
}

/* ReDMCSB CHAMPION.C F0320 — damage flash model.
 * After F0319 damage application, F0320:1735-1740 schedules redraws.
 * Flash = champion color on name zone for 2 ticks. */
int CSB_ChampionPanel_BuildDamageFlashModel(
    int championIndex, int newWoundsBitmask,
    CSB_ChampionPanel_DamageFlashModel *outModel)
{
    if (!outModel || championIndex < 0 || championIndex >= CSB_CHAMPION_COUNT)
        return 0;

    memset(outModel, 0, sizeof(*outModel));
    outModel->championIndex = championIndex;
    outModel->flashColor = CSB_ChampionColor[championIndex];
    outModel->normalColor = CSB_COLOR_DARKEST_GRAY;
    outModel->flashTickCount = CSB_DAMAGE_FLASH_TICK_COUNT;
    outModel->scheduledAttributes = CSB_ATTR_STATISTICS;
    outModel->hasNewWounds = newWoundsBitmask != 0 ? 1 : 0;
    if (outModel->hasNewWounds)
        outModel->scheduledAttributes |= CSB_ATTR_WOUNDS;
    return 1;
}

/* ReDMCSB COMMAND.C:473-483 — spell area panel model. */
int CSB_ChampionPanel_BuildSpellAreaModel(
    CSB_ChampionPanel_SpellAreaModel *outModel)
{
    int i;
    if (!outModel) return 0;

    memset(outModel, 0, sizeof(*outModel));
    outModel->backgroundGraphicId = CSB_GFX_SPELL_AREA;
    outModel->areaX = CSB_SPELL_AREA_X;
    outModel->areaY = CSB_SPELL_AREA_Y;
    outModel->areaW = CSB_SPELL_AREA_W;
    outModel->areaH = CSB_SPELL_AREA_H;
    outModel->casterZone = CSB_SPELL_ZONE_CASTER;
    outModel->casterCommandId = 109;
    for (i = 0; i < CSB_SPELL_RUNE_COUNT; ++i) {
        outModel->runeZones[i] = CSB_SPELL_ZONE_RUNE_0 + i;
        outModel->runeCommandIds[i] = 101 + i;
    }
    outModel->castZone = CSB_SPELL_ZONE_CAST;
    outModel->castCommandId = 108;
    outModel->recantZone = CSB_SPELL_ZONE_RECANT;
    outModel->recantCommandId = 107;
    return 1;
}

/* ReDMCSB CHAMDRAW.C F0293 — clock tick repaint model. */
int CSB_ChampionPanel_BuildClockTickRepaintModel(
    CSB_ChampionPanel_ClockTickRepaintModel *outModel)
{
    if (!outModel) return 0;

    memset(outModel, 0, sizeof(*outModel));
    outModel->repaintMask = CSB_CLOCK_TICK_REPAINT_MASK;
    outModel->affectsBarGraphs = 1;
    outModel->affectsStatValues = 1;
    outModel->affectsLoadDisplay = 1;
    return 1;
}

const char *CSB_ChampionPanel_SourceEvidence(void)
{
    return
        "CSB V1 Champion Panel HUD — ReDMCSB_WIP20210206\n"
        "  CHAMDRAW.C F0287: bar graph height (ceil(current*25/max))\n"
        "  CHAMDRAW.C F0287 PC34: bar fill model (zone C195+champIdx, stride 4)\n"
        "  CHAMDRAW.C F0288: integer formatting (space-padded width)\n"
        "  CHAMDRAW.C F0289/F0290: status value format (nnn/nnn, stamina/10)\n"
        "  CHAMDRAW.C F0291: slot box graphic (C033/C034/C035 cascade)\n"
        "  CHAMDRAW.C F0292: status box model (alive/dead, attribute masks)\n"
        "  CHAMDRAW.C F0292: load color (red/yellow/gray thresholds)\n"
        "  CHAMDRAW.C F0292: load value format (nnn.n/nnn KG)\n"
        "  CHAMDRAW.C F0622: icon bitmap model (19x14, direction-indexed)\n"
        "  PANEL.C F0345: food/water/poison label blit spec (C030-C032)\n"
        "  PANEL.C F0351: statistic row model (current/max color, text)\n"
        "  PANEL.C F0351: statistic text run (zone/XY/color layout)\n"
        "  INVNTORY.C F0354: portrait screen X (champIdx*69+7)\n"
        "  INVNTORY.C F0354: portrait blit model (32x29 from graphic 26)\n"
        "  CHAMPION.C F0320: damage flash model (champion color, 2 ticks)\n"
        "  COMMAND.C:473-483: spell area panel model (caster/runes/cast/recant)\n"
        "  CHAMDRAW.C F0293: clock tick repaint model (STATISTICS mask)\n"
        "  Layout-696 C195..C206: bar graph XY (champIdx*69+46, +7, +14)\n"
        "  Layout-696 C211..C218: hand slot XY (champIdx*69+4/+24, y=10)\n"
        "  Layout-696 C507..C536: inventory slot XY (30 slots)\n"
        "  DEFS.H C212: empty hand icon index\n"
        "  G0046_auc_Graphic562_ChampionColor[4] = {7,11,8,14}\n";
}
