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

const char *CSB_ChampionPanel_SourceEvidence(void)
{
    return
        "CSB V1 Champion Panel HUD — ReDMCSB_WIP20210206\n"
        "  CHAMDRAW.C F0287: bar graph height (ceil(current*25/max))\n"
        "  CHAMDRAW.C F0287 PC34: bar fill model (zone C195+champIdx, stride 4)\n"
        "  CHAMDRAW.C F0289/F0290: status value format (nnn/nnn, stamina/10)\n"
        "  CHAMDRAW.C F0291: slot box graphic (C033/C034/C035 cascade)\n"
        "  CHAMDRAW.C F0292: status box model (alive/dead, attribute masks)\n"
        "  CHAMDRAW.C F0622: icon bitmap model (19x14, direction-indexed)\n"
        "  INVNTORY.C F0354: portrait screen X (champIdx*69+7)\n"
        "  Layout-696 C195..C206: bar graph XY (champIdx*69+46, +7, +14)\n"
        "  Layout-696 C211..C218: hand slot XY (champIdx*69+4/+24, y=10)\n"
        "  G0046_auc_Graphic562_ChampionColor[4] = {7,11,8,14}\n";
}
