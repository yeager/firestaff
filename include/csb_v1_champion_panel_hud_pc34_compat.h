#ifndef FIRESTAFF_CSB_V1_CHAMPION_PANEL_HUD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHAMPION_PANEL_HUD_PC34_COMPAT_H

/*
 * CSB V1 Champion Panel & Inventory HUD — pc34 compat layer.
 *
 * Source reference: ReDMCSB CHAMDRAW.C (shared with DM1)
 *   F0287_CHAMPION_DrawBarGraphs (line 67):
 *     Bar graph zone iteration: C195_ZONE_FIRST_BAR_GRAPH + champIdx,
 *     stride 4 for HP/stamina/mana. Each bar is 4px wide, 25px max height.
 *     Blank = C12_COLOR_DARKEST_GRAY, fill = G0046_auc_Graphic562_ChampionColor[idx].
 *
 *   F0289_CHAMPION_DrawHealthOrStaminaOrManaValue (line 274):
 *     Numeric display in C550/C551/C552 zones. Format: "nnn/nnn".
 *
 *   F0291_CHAMPION_DrawSlot (line 307):
 *     Slot rendering: icon from THING, slot box = C033/C034/C035.
 *
 *   F0292_CHAMPION_DrawState (line 470):
 *     Master draw: dirty-flag driven via Attributes bitmask.
 *
 * Source reference: ReDMCSB INVNTORY.C
 *   F0354_INVENTORY_DrawStatusBoxPortrait (line 1503):
 *     Portrait: 32x29, zones C175..C178.
 *
 *   F0355_INVENTORY_Toggle_CPSE (line 1533):
 *     Inventory: C017 backdrop, iterates slots 0..29 via F0291.
 *
 * CSB-specific differences from DM1:
 *   - Champion icon graphic = G562 (same base) but from CSBGRAPHICS.DAT
 *   - Panel surfaces C017 (inventory) and C040 (resurrect) from CSB package
 *   - Slot box graphics C033/C034/C035 from CSB graphics archive
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Champion constants — CSB uses same ReDMCSB base as DM1 ── */
#define CSB_CHAMPION_COUNT 4

/* ── Champion color table — G0046_auc_Graphic562_ChampionColor[4] ──
 * Same as DM1: green(7), yellow(11), red(8), blue(14). */
extern const uint8_t CSB_ChampionColor[CSB_CHAMPION_COUNT];

/* ── Bar graph constants — CHAMDRAW.C F0287 ── */
#define CSB_BAR_GRAPH_WIDTH      4
#define CSB_BAR_GRAPH_MAX_HEIGHT 25
#define CSB_BAR_GRAPH_COUNT      3

/* ── VGA palette indices — DEFS.H ── */
#define CSB_COLOR_BLACK         0
#define CSB_COLOR_DARK_GRAY     1
#define CSB_COLOR_LIGHT_GREEN   7
#define CSB_COLOR_RED           8
#define CSB_COLOR_GOLD          9
#define CSB_COLOR_YELLOW        11
#define CSB_COLOR_DARKEST_GRAY  12
#define CSB_COLOR_LIGHTEST_GRAY 13
#define CSB_COLOR_WHITE         15

/* ── Attribute dirty flags — DEFS.H ── */
#define CSB_ATTR_NAME_TITLE   0x0080
#define CSB_ATTR_STATISTICS   0x0100
#define CSB_ATTR_LOAD         0x0200
#define CSB_ATTR_ICON         0x0400
#define CSB_ATTR_PANEL        0x0800
#define CSB_ATTR_STATUS_BOX   0x1000
#define CSB_ATTR_WOUNDS       0x2000
#define CSB_ATTR_VIEWPORT     0x4000
#define CSB_ATTR_ACTION_HAND  0x8000

/* ── Slot indices — DEFS.H ── */
#define CSB_SLOT_READY_HAND    0
#define CSB_SLOT_ACTION_HAND   1
#define CSB_SLOT_HEAD          2
#define CSB_SLOT_TORSO         3
#define CSB_SLOT_LEGS          4
#define CSB_SLOT_FEET          5
#define CSB_SLOT_CHEST_1      30
#define CSB_SLOT_CHEST_8      37

/* ── Status box geometry — DEFS.H ── */
#define CSB_STATUS_BOX_SPACING  69
#define CSB_STATUS_BOX_WIDTH    67
#define CSB_STATUS_BOX_HEIGHT   29

/* ── Portrait — DEFS.H, 32x29 ── */
#define CSB_PORTRAIT_WIDTH      32
#define CSB_PORTRAIT_HEIGHT     29
#define CSB_PORTRAIT_OFFSET_X    7

/* ── Champion icon — G2080/G2081 ── */
#define CSB_CHAMPION_ICON_WIDTH  19
#define CSB_CHAMPION_ICON_HEIGHT 14

/* ── Slot box — 18x18 ── */
#define CSB_SLOT_BOX_SIZE       18

/* ── Graphic IDs (CSB PC 3.4 from CSBGRAPHICS.DAT) ── */
#define CSB_GFX_DEAD_CHAMPION    8
#define CSB_GFX_INVENTORY       17
#define CSB_GFX_PANEL_EMPTY     20
#define CSB_GFX_PORTRAITS       26
#define CSB_GFX_CHAMPION_ICONS  28
#define CSB_GFX_SLOT_NORMAL     33
#define CSB_GFX_SLOT_WOUNDED    34
#define CSB_GFX_SLOT_ACTING     35
#define CSB_GFX_READY_HAND_ICON 212

/* Food/Water/Poison label graphics — DEFS.H C030/C031/C032 */
#define CSB_GFX_FOOD_LABEL     30
#define CSB_GFX_WATER_LABEL    31
#define CSB_GFX_POISONED_LABEL 32

/* ── Viewport — DEFS.H ── */
#define CSB_VIEWPORT_X   0
#define CSB_VIEWPORT_Y  33
#define CSB_VIEWPORT_W 224
#define CSB_VIEWPORT_H 136

/* Inventory champion HP/stamina/mana value zones */
#define CSB_ZONE_HEALTH_VALUE  550
#define CSB_ZONE_MANA_VALUE    551
#define CSB_ZONE_STAMINA_VALUE 552

/* Food/Water/Poison label zones — PANEL.C:1598-1606 */
#define CSB_ZONE_FOOD     500
#define CSB_ZONE_WATER    501
#define CSB_ZONE_POISONED 502

/* ── Status box model ── */
enum {
    CSB_STATUS_BOX_DRAW_NONE = 0,
    CSB_STATUS_BOX_DRAW_ALIVE,
    CSB_STATUS_BOX_DRAW_DEAD
};

typedef struct CSB_ChampionPanel_StatusBoxModel {
    int drawKind;
    int fillColor;
    int graphicId;
    int nameColor;
    int nameBackgroundColor;
    int propagatedAttributes;
    int drawPortrait;
    int drawActionIcon;
    int stopAfterDead;
} CSB_ChampionPanel_StatusBoxModel;

/* ── Icon bitmap model ── */
typedef struct CSB_ChampionPanel_IconBitmapModel {
    int width;
    int height;
    int fillColor;
    int graphicId;
    int sourceX;
    int sourceY;
    int transparentColor;
} CSB_ChampionPanel_IconBitmapModel;

/* ── Bar fill model ── */
typedef struct CSB_ChampionPanel_BarFillModel {
    int zoneId;
    int x;
    int y;
    int width;
    int height;
    int blankColor;
    int fillColor;
    int blankX;
    int blankY;
    int blankWidth;
    int blankHeight;
    int fillX;
    int fillY;
    int fillWidth;
    int fillHeight;
    int emitsBlank;
    int emitsFill;
} CSB_ChampionPanel_BarFillModel;

/* ── Status hand slot box model — CHAMDRAW.C F0291 ── */
typedef struct CSB_ChampionPanel_StatusHandSlotBoxModel {
    int championIndex;
    int handIndex;
    int isActionHand;
    int isActingChampion;
    int x;
    int y;
    int width;
    int height;
    int graphicId;
} CSB_ChampionPanel_StatusHandSlotBoxModel;

/* ── Bar graph height — CHAMDRAW.C F0287 ── */
int CSB_ChampionPanel_BarGraphHeight(int current, int maximum, int isMana);

/* ── Bar fill rectangles — CHAMDRAW.C F0287 ── */
int CSB_ChampionPanel_BuildPc34BarFillModel(
    int championIndex, int statIndex, int current, int maximum,
    CSB_ChampionPanel_BarFillModel *outModel);

/* ── Status box — CHAMDRAW.C F0292 ── */
int CSB_ChampionPanel_BuildStatusBoxModel(
    int championIndex, int leaderIndex, int isInventoryChampion,
    int currentHealth, CSB_ChampionPanel_StatusBoxModel *outModel);

/* ── Icon bitmap — F0622 ── */
int CSB_ChampionPanel_BuildIconBitmapModel(
    int championIndex, int championDirection, int partyDirection,
    CSB_ChampionPanel_IconBitmapModel *outModel);

/* ── Status hand slot box — CHAMDRAW.C F0291 ── */
int CSB_ChampionPanel_BuildStatusHandSlotBoxModel(
    int championIndex, int handIndex, int isActingChampion,
    CSB_ChampionPanel_StatusHandSlotBoxModel *outModel);

/* ── Slot box graphic — CHAMDRAW.C F0291 ── */
int CSB_ChampionPanel_SlotBoxGraphic(int slotIndex, uint16_t wounds,
                                     int isActingChampion);

/* ── Portrait screen X — CHAMDRAW.C F0354 ── */
int CSB_ChampionPanel_PortraitScreenX(int champIdx);

/* ── Name zone X — layout-696 C159..C162 ── */
int CSB_ChampionPanel_NameZoneX(int champIdx);

/* ── Bar graph screen XY — layout-696 C195..C206 ── */
void CSB_ChampionPanel_BarGraphScreenXY(int champIdx, int statIndex,
                                        int *outX, int *outY);

/* ── Status hand slot XY — layout-696 C211..C218 ── */
void CSB_ChampionPanel_StatusHandSlotXY(int champIdx, int handSlot,
                                        int *outX, int *outY);

/* ── Name color (leader=gold, else=lightest gray) ── */
int CSB_ChampionPanel_NameColor(int champIdx, int leaderIdx);

/* ── Dead status box predicate ── */
int CSB_ChampionPanel_IsDeadStatusBox(int currentHealth);

/* ── Numeric value format — CHAMDRAW.C F0289 ── */
int CSB_ChampionPanel_FormatStatusValue(int valueIndex,
    int currentHealth, int maximumHealth,
    int currentStamina, int maximumStamina,
    int currentMana, int maximumMana,
    char *out, size_t outSize);
int CSB_ChampionPanel_StatusValueZone(int valueIndex);

/* ── Integer formatting — CHAMDRAW.C F0288 ── */
int CSB_ChampionPanel_FormatIntegerF0288(int value, int paddingEnabled,
                                         int width, char *out,
                                         size_t outSize);

/* ── Inventory slot XY — layout-696 C507..C536 ── */
#define CSB_SLOTBOX_FIRST_STATUS     0
#define CSB_SLOTBOX_FIRST_INVENTORY  8
#define CSB_SLOTBOX_FIRST_CHEST     38

int CSB_ChampionPanel_InventorySlotXY(int slotBoxIndex,
                                       int *outX, int *outY);

/* ── Empty hand icon — DEFS.H C212 ── */
int CSB_ChampionPanel_EmptyHandIconIndex(int slotIndex, uint16_t wounds);

/* ── Statistic panel — PANEL.C F0351 ── */
#define CSB_ZONE_SKILL_VALUE      557
#define CSB_ZONE_STATISTIC_VALUE  559
#define CSB_STATISTIC_ROW_COUNT     6
#define CSB_PANEL_TEXT_CHAR_WIDTH   6
#define CSB_PANEL_TEXT_LINE_HEIGHT  7
#define CSB_STATISTIC_NAME_REL_X    28
#define CSB_STATISTIC_CURRENT_REL_X 94
#define CSB_STATISTIC_FIRST_REL_Y   34

typedef struct CSB_ChampionPanel_StatisticRowModel {
    int currentValue;
    int maximumValue;
    int currentColor;
    int maximumColor;
    char currentText[4];
    char maximumText[5];
} CSB_ChampionPanel_StatisticRowModel;

typedef struct CSB_ChampionPanel_StatisticTextRunModel {
    int statisticIndex;
    int nameZone;
    int valueZone;
    int nameX;
    int currentX;
    int maximumX;
    int y;
    int nameColor;
    int currentColor;
    int maximumColor;
    char currentText[4];
    char maximumText[5];
} CSB_ChampionPanel_StatisticTextRunModel;

int CSB_ChampionPanel_StatisticCurrentColor(int currentValue, int maximumValue);
int CSB_ChampionPanel_StatisticMaximumColor(void);
int CSB_ChampionPanel_FormatStatisticValue(int currentValue, int maximumValue,
                                           char *currentOut, size_t currentOutSize,
                                           char *maximumOut, size_t maximumOutSize);
int CSB_ChampionPanel_BuildStatisticRowModel(
    int currentValue, int maximumValue,
    CSB_ChampionPanel_StatisticRowModel *outRow);
int CSB_ChampionPanel_BuildStatisticTextRunModel(
    int statisticIndex, int currentValue, int maximumValue,
    CSB_ChampionPanel_StatisticTextRunModel *outRun);

/* ── Load display — CHAMDRAW.C F0292 ── */
#define CSB_ZONE_CHAMPION_LOAD_LABEL 554
#define CSB_ZONE_CHAMPION_LOAD_VALUE 555

int CSB_ChampionPanel_LoadColor(int load, int maximumLoad);
int CSB_ChampionPanel_FormatLoadValue(int load, int maximumLoad,
                                      char *out, size_t outSize);
int CSB_ChampionPanel_LoadValueZone(void);

/* ── Food/Water/Poison labels — PANEL.C F0345 ── */
#define CSB_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT 3

typedef struct CSB_ChampionPanel_F0658BlitStepSpec {
    int bitmapId;
    int zoneId;
    int transparentColor;
    int sourceLine;
    int requiresPoisoned;
} CSB_ChampionPanel_F0658BlitStepSpec;

typedef struct CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec {
    int blitCount;
    int sourceStartLine;
    int sourceEndLine;
    int conditionalLine;
    const char *sourceEvidence;
    CSB_ChampionPanel_F0658BlitStepSpec blits[CSB_CHAMPION_PANEL_F0658_POISONED_BLIT_COUNT];
} CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec;

const CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec *
CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec_SourceLocked(void);

/* ── Source evidence ── */
const char *CSB_ChampionPanel_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif
