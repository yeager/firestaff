
#ifndef NEXUS_V1_HUD_LAYOUT_H
#define NEXUS_V1_HUD_LAYOUT_H
#include <stddef.h>
#include <stdint.h>

/* Nexus HUD element layout table from DM.BIN yam\menuctrl.c at 0x0376D0.
 * Each entry is (element_id, 0, x, y) as big-endian uint16 quads.
 * Groups are separated by 0xFFFF sentinel entries.
 * Source: DM.BIN 0x0376D0-0x037900, Saturn binary (555,144 bytes). */

typedef struct {
    uint16_t element_id;
    uint16_t x;
    uint16_t y;
} Nexus_HudElement;

#define NEXUS_HUD_LAYOUT_DM_BIN_OFFSET 0x376D0U
#define NEXUS_HUD_LAYOUT_ENTRY_BYTES    8U
#define NEXUS_HUD_LAYOUT_ENTRY_COUNT    80U

/* Element ID constants from the layout table. */
#define NEXUS_HUD_VIEWPORT        0x0040
#define NEXUS_HUD_HP_BAR          0x0090
#define NEXUS_HUD_SPELL_PANEL     0x002E
#define NEXUS_HUD_CHAMPION_1      0x0001
#define NEXUS_HUD_CHAMPION_2      0x0002
#define NEXUS_HUD_CHAMPION_3      0x0003
#define NEXUS_HUD_CHAMPION_4      0x0004
#define NEXUS_HUD_CHAMP_NAME_1    0x0045
#define NEXUS_HUD_CHAMP_NAME_2    0x0046
#define NEXUS_HUD_CHAMP_NAME_3    0x0047
#define NEXUS_HUD_CHAMP_NAME_4    0x0048
#define NEXUS_HUD_COMPASS         0x0094
#define NEXUS_HUD_INV_SLOT        0x0010
#define NEXUS_HUD_EQUIP_SLOT      0x008F
#define NEXUS_HUD_CHAMP_PANEL     0x0093
#define NEXUS_HUD_CHAMP_FRAME     0x0096

/* Layout group indices (separated by 0xFFFF sentinels in the raw table). */
#define NEXUS_HUD_GROUP_MAIN        0  /* viewport, portraits, HP bars, spell panel */
#define NEXUS_HUD_GROUP_MOVEMENT    1  /* movement arrows, compass */
#define NEXUS_HUD_GROUP_INVENTORY   2  /* inventory grid, equipment, action buttons */

/* Number of champion HP bar entries (one per champion slot). */
#define NEXUS_HUD_HP_BAR_COUNT    4

/* HP bar positions from the layout table (entries 12-15).
 * Source: DM.BIN 0x037730 — 4 × (0x0090, 0, x, 4). */
#define NEXUS_HUD_HP_BAR_Y        4
#define NEXUS_HUD_HP_BAR_X_0     64
#define NEXUS_HUD_HP_BAR_X_1    128
#define NEXUS_HUD_HP_BAR_X_2    192
#define NEXUS_HUD_HP_BAR_X_3    256

/* Viewport dimensions from entry 0. */
#define NEXUS_HUD_VIEWPORT_X    256
#define NEXUS_HUD_VIEWPORT_Y    125

/* Champion portrait positions from entries 4-7. */
#define NEXUS_HUD_PORTRAIT_X_0  272
#define NEXUS_HUD_PORTRAIT_Y_0    9
#define NEXUS_HUD_PORTRAIT_X_1  288
#define NEXUS_HUD_PORTRAIT_Y_1    9
#define NEXUS_HUD_PORTRAIT_X_2  272
#define NEXUS_HUD_PORTRAIT_Y_2   25
#define NEXUS_HUD_PORTRAIT_X_3  288
#define NEXUS_HUD_PORTRAIT_Y_3   25

/* Spell panel position from entry 16. */
#define NEXUS_HUD_SPELL_X       288
#define NEXUS_HUD_SPELL_Y        48

/* Inventory grid positions from entries 31-34 (4 slots visible). */
#define NEXUS_HUD_INV_SLOT_COUNT  4

/* Parse the retail menuctrl layout directly from DM.BIN. There is no static
 * fallback table: callers must provide the mounted Saturn binary. */
int nexus_v1_hud_layout_parse_dm_bin(
    const uint8_t *data,
    size_t data_size,
    Nexus_HudElement *out,
    size_t out_capacity,
    size_t *out_count);

#endif
