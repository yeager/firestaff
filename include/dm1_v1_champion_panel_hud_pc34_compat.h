#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_HUD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_HUD_PC34_COMPAT_H

/*
 * DM1 V1 Champion Panel & Inventory HUD — pc34 compat layer.
 *
 * Source reference: ReDMCSB CHAMDRAW.C
 *   F0287_CHAMPION_DrawBarGraphs (line 67):
 *     Bar graph zone iteration: C195_ZONE_FIRST_BAR_GRAPH + champIdx,
 *     stride 4 for HP/stamina/mana. Each bar is 4px wide, 25px max height.
 *     Blank = C12_COLOR_DARKEST_GRAY, fill = G0046_auc_Graphic562_ChampionColor[idx].
 *
 *   F0289_CHAMPION_DrawHealthOrStaminaOrManaValue (line 274):
 *     Numeric display in C550_ZONE_HEALTH_VALUE, C551_ZONE_MANA_VALUE,
 *     C552_ZONE_STAMINA_VALUE. Format: "nnn/nnn" with 3-char padding.
 *
 *   F0290_CHAMPION_DrawHealthStaminaManaValues (line 294):
 *     Calls F0289 for each stat. Note: stamina display divides by 10.
 *
 *   F0291_CHAMPION_DrawSlot (line 307):
 *     Slot rendering: icon from THING, slot box = C033 normal / C034 wounded /
 *     C035 acting. Body slots 0..5 check wound bitmask (1 << slotIndex).
 *
 *   F0292_CHAMPION_DrawState (line 470):
 *     Master draw: dirty-flag driven via Attributes bitmask.
 *
 *   F0622_PrepareChampionIconBitmap (line 32):
 *     Icon: G2080_C19 x G2081_C14 filled with ChampionColor, overlaid from C028.
 *
 * Source reference: ReDMCSB INVNTORY.C
 *   F0354_INVENTORY_DrawStatusBoxPortrait (line 1503):
 *     Portrait: 32x29 from CHAMPION.Portrait, zones C175..C178.
 *
 *   F0355_INVENTORY_Toggle_CPSE (line 1533):
 *     Inventory: C017 backdrop, iterates slots 0..29 via F0291.
 *
 * Source reference: ReDMCSB DEFS.H
 *   Slot indices: C00(0)..C37(37). Attribute masks: 0x0080..0x8000.
 *   Bar: 4px wide, 25px max. Colors: G0046[4] = {7,11,8,14}.
 *   Status box stride: C69 = 69. Portrait: 32x29.
 *   Champion icon: 19x14. Slot box: 18x18.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Champion color table — G0046_auc_Graphic562_ChampionColor[4] ── */
#define DM1_CHAMPION_COUNT 4
extern const uint8_t DM1_ChampionColor[DM1_CHAMPION_COUNT];

/* ── Bar graph constants — CHAMDRAW.C F0287 ── */
#define DM1_BAR_GRAPH_WIDTH      4
#define DM1_BAR_GRAPH_MAX_HEIGHT 25
#define DM1_BAR_GRAPH_COUNT      3

/* ── VGA palette indices — DEFS.H ── */
#define DM1_COLOR_BLACK         0
#define DM1_COLOR_DARK_GRAY     1
#define DM1_COLOR_RED           8
#define DM1_COLOR_GOLD          9
#define DM1_COLOR_YELLOW        11
#define DM1_COLOR_DARKEST_GRAY  12
#define DM1_COLOR_LIGHTEST_GRAY 13
#define DM1_COLOR_WHITE         15

/* ── Attribute dirty flags — DEFS.H ── */
#define DM1_ATTR_NAME_TITLE   0x0080
#define DM1_ATTR_STATISTICS   0x0100
#define DM1_ATTR_LOAD         0x0200
#define DM1_ATTR_ICON         0x0400
#define DM1_ATTR_PANEL        0x0800
#define DM1_ATTR_STATUS_BOX   0x1000
#define DM1_ATTR_WOUNDS       0x2000
#define DM1_ATTR_VIEWPORT     0x4000
#define DM1_ATTR_ACTION_HAND  0x8000

/* ── Slot indices — DEFS.H ── */
#define DM1_SLOT_READY_HAND    0
#define DM1_SLOT_ACTION_HAND   1
#define DM1_SLOT_HEAD          2
#define DM1_SLOT_TORSO         3
#define DM1_SLOT_LEGS          4
#define DM1_SLOT_FEET          5
#define DM1_SLOT_CHEST_1      30
#define DM1_SLOT_CHEST_8      37

/* ── Status box geometry — DEFS.H ── */
#define DM1_STATUS_BOX_SPACING  69
#define DM1_STATUS_BOX_WIDTH    67
#define DM1_STATUS_BOX_HEIGHT   29

/* ── Portrait — DEFS.H, 32x29, graphic 26 ── */
#define DM1_PORTRAIT_WIDTH      32
#define DM1_PORTRAIT_HEIGHT     29
#define DM1_PORTRAIT_OFFSET_X    7

/* ── Champion icon — G2080/G2081 ── */
#define DM1_CHAMPION_ICON_WIDTH  19
#define DM1_CHAMPION_ICON_HEIGHT 14

/* ── Slot box — 18x18 ── */
#define DM1_SLOT_BOX_SIZE       18

/* ── Graphic IDs (DM1 PC 3.4) ── */
#define DM1_GFX_DEAD_CHAMPION    8
#define DM1_GFX_INVENTORY       17
#define DM1_GFX_PANEL_EMPTY     20
#define DM1_GFX_PORTRAITS       26
#define DM1_GFX_CHAMPION_ICONS  28
#define DM1_GFX_SLOT_NORMAL     33
#define DM1_GFX_SLOT_WOUNDED    34
#define DM1_GFX_SLOT_ACTING     35

/* ── Viewport — DEFS.H ── */
#define DM1_VIEWPORT_X   0
#define DM1_VIEWPORT_Y  33
#define DM1_VIEWPORT_W 224
#define DM1_VIEWPORT_H 136

/* Inventory champion HP/stamina/mana value zones */
#define DM1_ZONE_HEALTH_VALUE  550
#define DM1_ZONE_MANA_VALUE    551
#define DM1_ZONE_STAMINA_VALUE 552

/* ── Slot box index ranges — DEFS.H ── */
#define DM1_SLOTBOX_FIRST_STATUS    0
#define DM1_SLOTBOX_FIRST_INVENTORY 8
#define DM1_SLOTBOX_FIRST_CHEST    38

/* Inventory champion load label/value zones - CHAMDRAW.C F0292 */
#define DM1_ZONE_CHAMPION_LOAD_LABEL 554
#define DM1_ZONE_CHAMPION_LOAD_VALUE 555

enum {
    DM1_STATUS_VALUE_HEALTH = 0,
    DM1_STATUS_VALUE_STAMINA,
    DM1_STATUS_VALUE_MANA
};

/* ── Source-locked bar height — CHAMDRAW.C F0287 ── */
int DM1_ChampionPanel_BarGraphHeight(int current, int maximum, int isMana);

/* ── Slot box graphic — CHAMDRAW.C F0291 ── */
int DM1_ChampionPanel_SlotBoxGraphic(int slotIndex, uint16_t wounds,
                                      int isActingChampion);

/* ── Portrait screen X — CHAMDRAW.C F0354 ── */
int DM1_ChampionPanel_PortraitScreenX(int champIdx);

/* ── Name zone X — layout-696 C159..C162 ── */
int DM1_ChampionPanel_NameZoneX(int champIdx);

/* ── Bar graph screen XY — layout-696 C195..C206 ── */
void DM1_ChampionPanel_BarGraphScreenXY(int champIdx, int statIndex,
                                         int *outX, int *outY);

/* Inventory HP/stamina/mana numeric value format - CHAMDRAW.C F0289/F0290 */
int DM1_ChampionPanel_FormatStatusValue(int valueIndex,
                                        int currentHealth, int maximumHealth,
                                        int currentStamina, int maximumStamina,
                                        int currentMana, int maximumMana,
                                        char *out, size_t outSize);
int DM1_ChampionPanel_StatusValueZone(int valueIndex);

/* Inventory load label/value color and value format - CHAMDRAW.C F0292 */
int DM1_ChampionPanel_LoadColor(int load, int maximumLoad);
int DM1_ChampionPanel_FormatLoadValue(int load, int maximumLoad,
                                      char *out, size_t outSize);
int DM1_ChampionPanel_LoadValueZone(void);

/* ── Inventory slot XY (viewport-relative) ── */
int DM1_ChampionPanel_InventorySlotXY(int slotBoxIndex,
                                       int *outX, int *outY);

/* ── Status hand slot XY — layout-696 C211..C218 ── */
void DM1_ChampionPanel_StatusHandSlotXY(int champIdx, int handSlot,
                                         int *outX, int *outY);

/* ── Name color (leader=gold, else=lightest gray) ── */
int DM1_ChampionPanel_NameColor(int champIdx, int leaderIdx);

/* ── Dead status box predicate ── */
int DM1_ChampionPanel_IsDeadStatusBox(int currentHealth);

/* ── Self-test: returns number of failures ── */
int DM1_ChampionPanel_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif
