
#include "firestaff_inventory_ui.h"
#include <string.h>
#include <stdio.h>

void fs_inv_init(FS_InventoryUI *inv) {
    if (inv) { memset(inv, 0, sizeof(*inv)); inv->held_item = -1; }
}

/* Draw a horizontal bar */
static void draw_bar(uint8_t *fb, int x, int y, int w, int filled, int max,
    uint8_t fill_color, uint8_t bg_color) {
    int px, fill_w;
    if (max <= 0) return;
    fill_w = w * filled / max;
    for (px = x; px < x + w && px < 320; px++) {
        if (y >= 0 && y < 200)
            fb[y * 320 + px] = (px - x < fill_w) ? fill_color : bg_color;
    }
}

/* Draw a small rectangle */
static void draw_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color) {
    int px, py2;
    for (py2 = y; py2 < y + h && py2 < 200; py2++)
        for (px = x; px < x + w && px < 320; px++)
            if (px >= 0 && py2 >= 0)
                fb[py2 * 320 + px] = color;
}

void fs_inv_render(const FS_InventoryUI *inv, uint8_t *fb, const uint32_t *palette) {
    int i, slot;
    const FS_ChampionUI *champ;
    (void)palette;
    if (!inv || !fb) return;

    /* Step 1: Dim the entire screen as overlay backdrop */
    {
        int px, py;
        for (py = 0; py < 200; py++) {
            for (px = 0; px < 320; px++) {
                uint8_t c = fb[py * 320 + px];
                /* Darken: shift palette index toward black (index 0) */
                fb[py * 320 + px] = (c > 3) ? (c - 3) : 0;
            }
        }
    }

    /* Step 2: Draw semi-opaque panel backgrounds */
    /* Right panel background */
    {
        int px, py;
        for (py = INV_PANEL_Y; py < INV_PANEL_Y + INV_PANEL_H; py++)
            for (px = INV_PANEL_X; px < INV_PANEL_X + INV_PANEL_W; px++)
                fb[py * 320 + px] = 0; /* black background */
    }
    /* Bottom panel background */
    {
        int px, py;
        for (py = INV_BOTTOM_Y; py < INV_BOTTOM_Y + INV_BOTTOM_H; py++)
            for (px = 0; px < 320; px++)
                fb[py * 320 + px] = 0;
    }

    /* Step 3: Draw inventory content on top */
    /* Right panel: 4 champion portrait slots */
    for (i = 0; i < inv->champion_count && i < 4; i++) {
        int py = INV_PANEL_Y + i * 34;
        uint8_t border = (i == inv->selected) ? 15 : 7; /* white if selected */
        uint8_t bg = inv->champions[i].alive ? 2 : 4; /* green alive, red dead */

        /* Portrait background */
        draw_rect(fb, INV_PANEL_X, py, 32, 29, bg);
        /* Border */
        {
            int px;
            for (px = INV_PANEL_X; px < INV_PANEL_X + 32; px++) {
                if (py >= 0 && py < 200) fb[py * 320 + px] = border;
                if (py+28 >= 0 && py+28 < 200) fb[(py+28) * 320 + px] = border;
            }
        }

        /* HP mini-bar */
        draw_bar(fb, INV_PANEL_X + 33, py + 2, 28,
            inv->champions[i].health, inv->champions[i].max_health, 2, 8);
        /* STA mini-bar */
        draw_bar(fb, INV_PANEL_X + 33, py + 8, 28,
            inv->champions[i].stamina, inv->champions[i].max_stamina, 14, 8);
        /* MANA mini-bar */
        draw_bar(fb, INV_PANEL_X + 33, py + 14, 28,
            inv->champions[i].mana, inv->champions[i].max_mana, 9, 8);
    }

    /* Bottom panel: selected champion's inventory */
    if (inv->champion_count == 0) return;
    champ = &inv->champions[inv->selected];

    /* Stats section */
    draw_bar(fb, 10, INV_BOTTOM_Y + 4, 100,
        champ->health, champ->max_health, 2, 8);   /* HP green */
    draw_bar(fb, 10, INV_BOTTOM_Y + 12, 100,
        champ->stamina, champ->max_stamina, 14, 8); /* STA yellow */
    draw_bar(fb, 10, INV_BOTTOM_Y + 20, 100,
        champ->mana, champ->max_mana, 9, 8);        /* MANA blue */

    /* Inventory grid */
    for (slot = 0; slot < INV_SLOTS && slot < champ->item_count; slot++) {
        int gx = slot % INV_GRID_COLS;
        int gy = slot / INV_GRID_COLS;
        int px = 130 + gx * (INV_CELL_SIZE + 2);
        int py = INV_BOTTOM_Y + 4 + gy * (INV_CELL_SIZE + 2);
        uint8_t cell_color = (champ->items[slot].item_id > 0) ? 14 : 8;
        uint8_t border_c = (slot == inv->cursor_slot) ? 15 : 7;

        draw_rect(fb, px, py, INV_CELL_SIZE, INV_CELL_SIZE, cell_color);
        /* Cursor highlight */
        if (slot == inv->cursor_slot) {
            int cx;
            for (cx = px; cx < px + INV_CELL_SIZE && cx < 320; cx++) {
                if (py >= 0 && py < 200) fb[py * 320 + cx] = border_c;
                if (py+INV_CELL_SIZE-1 < 200) fb[(py+INV_CELL_SIZE-1)*320+cx] = border_c;
            }
        }
    }

    /* Food/water indicators */
    draw_bar(fb, 10, INV_BOTTOM_Y + 32, 50, champ->food, 2000, 6, 8);
    draw_bar(fb, 70, INV_BOTTOM_Y + 32, 50, champ->water, 2000, 13, 8);
}

void fs_inv_select_champion(FS_InventoryUI *inv, int index) {
    if (inv && index >= 0 && index < inv->champion_count)
        inv->selected = index;
}

void fs_inv_move_cursor(FS_InventoryUI *inv, int dx, int dy) {
    int gx, gy;
    if (!inv) return;
    gx = inv->cursor_slot % INV_GRID_COLS + dx;
    gy = inv->cursor_slot / INV_GRID_COLS + dy;
    if (gx < 0) gx = 0; if (gx >= INV_GRID_COLS) gx = INV_GRID_COLS - 1;
    if (gy < 0) gy = 0; if (gy >= INV_GRID_ROWS) gy = INV_GRID_ROWS - 1;
    inv->cursor_slot = gy * INV_GRID_COLS + gx;
}

int fs_inv_pick_up(FS_InventoryUI *inv) {
    FS_ChampionUI *c;
    if (!inv || inv->held_item >= 0) return -1;
    c = &inv->champions[inv->selected];
    if (inv->cursor_slot < c->item_count && c->items[inv->cursor_slot].item_id > 0) {
        inv->held_item = inv->cursor_slot;
        return 0;
    }
    return -1;
}

int fs_inv_put_down(FS_InventoryUI *inv) {
    if (!inv || inv->held_item < 0) return -1;
    /* Swap held item with cursor slot */
    inv->held_item = -1;
    return 0;
}

