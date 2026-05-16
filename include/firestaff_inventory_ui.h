
#ifndef FIRESTAFF_INVENTORY_UI_H
#define FIRESTAFF_INVENTORY_UI_H
#include <stdint.h>

/* DM1 Inventory UI — right panel showing champion info.
 *
 * Layout (original 320×200):
 *   Viewport:  33-256, 0-135 (224×136)
 *   Right panel: 257-319, 0-135 (63×136) — champion portraits
 *   Bottom panel: 0-319, 136-199 (320×64) — inventory/status
 *
 * Champion panel shows:
 *   - 4 portrait slots (32×29 each)
 *   - Selected champion's inventory grid
 *   - Stats: HP/STA/MANA bars
 *   - Equipment slots: head, torso, legs, feet, hands, weapon, shield */

#define INV_PANEL_X 257
#define INV_PANEL_Y 0
#define INV_PANEL_W 63
#define INV_PANEL_H 136
#define INV_BOTTOM_Y 136
#define INV_BOTTOM_H 64

#define INV_SLOTS 30     /* max items per champion */
#define INV_GRID_COLS 5
#define INV_GRID_ROWS 6
#define INV_CELL_SIZE 10

typedef struct {
    int item_id;
    int count;
    char name[32];
    int weight;
    int equipped;       /* slot: 0=none, 1=head, 2=torso, etc */
} FS_InventoryItem;

typedef struct {
    char name[32];
    int portrait_index;
    int health, max_health;
    int stamina, max_stamina;
    int mana, max_mana;
    int food, water;
    int alive;
    FS_InventoryItem items[INV_SLOTS];
    int item_count;
} FS_ChampionUI;

typedef struct {
    FS_ChampionUI champions[4];
    int champion_count;
    int selected;           /* 0-3 */
    int inventory_open;
    int cursor_slot;        /* selected inventory slot */
    int held_item;          /* -1 = nothing, else item index */
} FS_InventoryUI;

void fs_inv_init(FS_InventoryUI *inv);
void fs_inv_render(const FS_InventoryUI *inv, uint8_t *framebuffer, const uint32_t *palette);
void fs_inv_select_champion(FS_InventoryUI *inv, int index);
void fs_inv_move_cursor(FS_InventoryUI *inv, int dx, int dy);
int fs_inv_pick_up(FS_InventoryUI *inv);
int fs_inv_put_down(FS_InventoryUI *inv);

#endif

