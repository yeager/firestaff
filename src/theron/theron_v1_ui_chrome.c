/*
 * theron_v1_ui_chrome.c — Theron's Quest V1 Phase 4: UI Chrome Renderer
 *
 * Theron UI chrome rendering: top bar, right panel, champion slots, message.
 * Composited on top of the planar viewport framebuffer.
 *
 * Source-lock:
 *   THQUEST.ASM T600   — UI overlay zones
 *   THQUEST.ASM T800   — champion panel rendering
 *   THQUEST.ASM T900   — message bar
 */

#include "theron_v1_ui_chrome.h"
#include "theron_v1_champions.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

_Static_assert(TR_UI_TOPBAR_H  == 24,  "Topbar height mismatch");
_Static_assert(TR_CHAMP_SLOT_W == 80,  "Champion slot width mismatch");
_Static_assert(TR_CHAMP_SLOT_H == 56,  "Champion slot height mismatch");

/* ══════════════════════════════════════════════════════════════════════
 * Bar drawing
 * ══════════════════════════════════════════════════════════════════════ */

void tr_ui_draw_bar(TQR_PlanarFramebuffer *fb,
                    int x, int y, int w, int h,
                    int current, int max,
                    uint8_t pal_index,
                    uint8_t bg_index) {
    if (!fb || !fb->data) return;
    if (x < 0 || y < 0 || x + w > fb->w || y + h > fb->h) return;
    if (max <= 0) max = 1;
    if (current < 0) current = 0;

    int filled = (current * w) / max;
    if (filled > w) filled = w;

    for (int row = 0; row < h; row++) {
        uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
        for (int col = 0; col < filled; col++) {
            row_ptr[x + col] = pal_index;
        }
        for (int col = filled; col < w; col++) {
            row_ptr[x + col] = bg_index;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Dungeon name helper
 * ══════════════════════════════════════════════════════════════════════ */

static const char *dungeon_name(int dungeon_id) {
    static const char *const names[THERON_DUNGEON_COUNT + 1] = {
        [1] = "Hall of Records",
        [2] = "Catacombs",
        [3] = "Caverns",
        [4] = "Castle",
        [5] = "Tower",
        [6] = "Temple",
        [7] = "Final Dungeon",
    };
    if (dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) return "Unknown";
    return names[dungeon_id] ? names[dungeon_id] : "Unknown";
}

/* ══════════════════════════════════════════════════════════════════════
 * Top bar rendering
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * Render the top bar: dungeon name + quest item count.
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void tr_ui_render_topbar(TQR_PlanarFramebuffer *fb,
                         const Theron_V1_World *world,
                         int y_offset) {
    if (!fb || !world) return;

    int y = y_offset;
    uint8_t dark_gray  = TR_CHROME_BG;
    uint8_t light_gray = TR_CHROME_FRAME;

    /* Background fill */
    for (int row = 0; row < TR_UI_TOPBAR_H; row++) {
        uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
        for (int col = 0; col < fb->w; col++) {
            row_ptr[col] = dark_gray;
        }
    }

    /* Dungeon name: rendered as colored pixel blocks (Phase 5: font tiles).
     * Phase 4 placeholder: colored indicator bar + dungeon ID block. */
    int dungeon_id   = world->current_dungeon;
    int quest_items  = world->quest_items_in_dungeon;

    /* Simple dungeon ID indicator: colored squares */
    int x = 8;
    for (int i = 0; i < dungeon_id && x + 4 < fb->w; i++, x += 6) {
        for (int row = 4; row < 12 && y + row < y + TR_UI_TOPBAR_H; row++) {
            uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
            for (int col = 0; col < 4; col++) {
                row_ptr[x + col] = (uint8_t)(10 + (i % 4));
            }
        }
    }

    /* Quest item indicator: small colored pixel blocks on the right */
    {
        int qi_x = fb->w - 8 - quest_items * 5;
        for (int i = 0; i < quest_items && qi_x + i * 5 < fb->w; i++) {
            int bx = qi_x + i * 5;
            for (int row = 8; row < 16 && y + row < y + TR_UI_TOPBAR_H; row++) {
                uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
                for (int col = 0; col < 4; col++) {
                    if (bx + col < fb->w)
                        row_ptr[bx + col] = 9; /* orange/gold for quest items */
                }
            }
        }
        (void)light_gray;
        (void)qi_x;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Right panel rendering
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * Render the right panel: Theron leader stats + compass direction.
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void tr_ui_render_right_panel(TQR_PlanarFramebuffer *fb,
                               const Theron_V1_World *world,
                               int x_offset) {
    if (!fb || !world) return;

    int x = x_offset;
    uint8_t dark_gray = TR_CHROME_BG;

    /* Panel background: x = 160..255 (96px wide), y = y_margin..192 */
    for (int row = 0; row < 160; row++) {
        uint8_t *row_ptr = fb->data + (16 + row) * fb->stride;
        for (int col = 0; col < TR_UI_RIGHT_W; col++) {
            if (x + col < fb->w) row_ptr[x + col] = dark_gray;
        }
    }

    /* Theron (party leader) stats */
    const Theron_V1_Champion *theron = theron_v1_party_leader_c(&world->party);
    if (!theron) return;

    /* HP bar: x+5, y=28, w=80, h=6 */
    tr_ui_draw_bar(fb, x + 5, 28, 80, 6,
                   theron->health, theron->max_health,
                   TR_CHROME_HP, dark_gray);

    /* Stamina bar: x+5, y=40, w=80, h=6 */
    tr_ui_draw_bar(fb, x + 5, 40, 80, 6,
                   theron->stamina, theron->max_stamina,
                   TR_CHROME_STAMINA, dark_gray);

    /* Compass: directional pixel marker */
    int dir = 0;
    {
        int did = world->current_dungeon;
        int lvl = world->current_level;
        if (did >= 1 && did <= THERON_DUNGEON_COUNT &&
            lvl >= 0 && lvl < THERON_MAX_LEVELS_PER_DUNGEON &&
            world->level_loaded[did - 1][lvl]) {
            dir = world->levels[did - 1][lvl].start_dir & 3;
        }
    }
    /* Arrow pointing in dir direction */
    {
        int cx = x + 40;
        int cy = 80;
        /* Arrow tip offset per direction: N/E/S/W */
        static const int8_t arrow_disp[4][2] = {
            [0] = {  0, -3 },  /* N: above center */
            [1] = {  3,  0 },  /* E: right of center */
            [2] = {  0,  3 },  /* S: below center */
            [3] = { -3,  0 },  /* W: left of center */
        };
        int ax = cx + arrow_disp[dir][0];
        int ay = cy + arrow_disp[dir][1];
        if (ax >= 0 && ax < fb->w && ay >= 0 && ay < fb->h) {
            fb->data[ay * fb->stride + ax] = 11; /* yellow marker */
        }
    }

    /* Level indicator: colored block below compass */
    {
        int lvl = world->current_level + 1;
        int lvl_x = x + 10;
        for (int i = 0; i < lvl && lvl_x + 4 < fb->w; i++, lvl_x += 6) {
            for (int row = 95; row < 105; row++) {
                uint8_t *row_ptr = fb->data + (16 + row) * fb->stride;
                for (int col = 0; col < 4; col++) {
                    if (lvl_x + col < fb->w)
                        row_ptr[lvl_x + col] = (uint8_t)(1 + (i % 8));
                }
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Champion slot rendering
 * ══════════════════════════════════════════════════════════════════════ */

/* Champion name string (truncated to fit slot) */
static void champ_slot_name(char *buf, size_t buf_size,
                            const Theron_V1_Champion *c) {
    if (!c || !buf || buf_size == 0) return;
    if (c->name[0] == '\0') {
        snprintf(buf, buf_size, "---");
        return;
    }
    snprintf(buf, buf_size, "%-12s", c->name);
}

/*
 * Render a single champion slot (80×56 bottom panel).
 * Source: THQUEST.ASM T800 (champion panel rendering).
 */
void tr_ui_draw_champion_slot(TQR_PlanarFramebuffer *fb,
                               int slot_idx,
                               int x, int y,
                               const Theron_V1_Champion *champion) {
    if (!fb || !fb->data) return;
    if (slot_idx < 0 || slot_idx >= THERON_MAX_CHAMPIONS) return;

    uint8_t bg         = TR_CHROME_BG;
    uint8_t frame      = TR_CHROME_FRAME;
    uint8_t name_color = TR_CHROME_NAME;
    uint8_t hp_col     = TR_CHROME_HP;
    uint8_t stam_col   = TR_CHROME_STAMINA;
    uint8_t mana_col   = TR_CHROME_MANA;

    /* Slot background fill */
    for (int row = 0; row < TR_CHAMP_SLOT_H; row++) {
        if (y + row >= fb->h) break;
        uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
        for (int col = 0; col < TR_CHAMP_SLOT_W; col++) {
            if (x + col < fb->w) row_ptr[x + col] = bg;
        }
    }

    /* Border: top+bottom row, left+right column */
    for (int col = 0; col < TR_CHAMP_SLOT_W; col++) {
        if (x + col < fb->w) {
            if (y < fb->h) fb->data[y * fb->stride + x + col] = frame;
            if (y + TR_CHAMP_SLOT_H - 1 < fb->h)
                fb->data[(y + TR_CHAMP_SLOT_H - 1) * fb->stride + x + col] = frame;
        }
    }
    for (int row = 0; row < TR_CHAMP_SLOT_H; row++) {
        if (y + row < fb->h) {
            if (x < fb->w) fb->data[(y + row) * fb->stride + x] = frame;
            if (x + TR_CHAMP_SLOT_W - 1 < fb->w)
                fb->data[(y + row) * fb->stride + x + TR_CHAMP_SLOT_W - 1] = frame;
        }
    }

    if (!champion || !champion->alive) {
        /* Empty/dead slot: draw X mark */
        for (int i = 0; i < 16 && x + i < fb->w && y + i < fb->h; i++) {
            if (x + i < fb->w && y + i < fb->h)
                fb->data[(y + i) * fb->stride + x + i] = frame;
            if (x + i < fb->w && y + 16 - 1 - i < fb->h)
                fb->data[(y + 16 - 1 - i) * fb->stride + x + i] = frame;
        }
        return;
    }

    /* Icon area: 24×24 at top-left of slot */
    int icon_x = x + 4;
    int icon_y = y + 4;

    /* Class icon: simple colored square per class */
    {
        uint8_t class_col = 1; /* gray default */
        switch (champion->primary_class) {
            case THERON_CLASS_FIGHTER: class_col = 8;  break; /* red */
            case THERON_CLASS_NINJA:   class_col = 9;  break; /* orange */
            case THERON_CLASS_PRIEST:  class_col = 15; break; /* white */
            case THERON_CLASS_WIZARD:  class_col = 14; break; /* blue */
        }
        for (int r = 0; r < 16 && icon_y + r < fb->h; r++) {
            uint8_t *row_ptr = fb->data + (icon_y + r) * fb->stride;
            for (int c2 = 0; c2 < 16 && icon_x + c2 < fb->w; c2++) {
                row_ptr[icon_x + c2] = class_col;
            }
        }
    }

    /* Name bar (Phase 5: font tiles) */
    {
        char name_buf[16];
        champ_slot_name(name_buf, sizeof(name_buf), champion);
        (void)name_buf;
        int name_x = x + 24;
        int name_y = y + 6;
        /* Colored name bar (placeholder for Phase 5 font rendering) */
        for (int c2 = 0; c2 < 50 && name_x + c2 < fb->w; c2++) {
            uint8_t *row_ptr = fb->data + (name_y) * fb->stride;
            row_ptr[name_x + c2] = name_color;
        }
    }

    /* HP bar: x+24, y+18, w=50, h=4 */
    tr_ui_draw_bar(fb, x + 24, y + 18, 50, 4,
                   champion->health, champion->max_health,
                   hp_col, bg);

    /* Stamina bar: x+24, y+26, w=50, h=4 */
    tr_ui_draw_bar(fb, x + 24, y + 26, 50, 4,
                   champion->stamina, champion->max_stamina,
                   stam_col, bg);

    /* Mana bar (only for magic users) */
    if (champion->max_mana > 0) {
        tr_ui_draw_bar(fb, x + 24, y + 34, 50, 4,
                       champion->mana, champion->max_mana,
                       mana_col, bg);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Master UI renderer
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * tr_ui_render — composite all enabled UI chrome zones onto the planar fb.
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void tr_ui_render(TQR_PlanarFramebuffer *fb,
                   const Theron_V1_World *world,
                   uint32_t ui_flags) {
    if (!fb || !fb->data || !world) return;

    if (ui_flags & TR_UI_TOPBAR) {
        tr_ui_render_topbar(fb, world, 0);
    }

    if (ui_flags & TR_UI_RIGHT_PANEL) {
        /* Right panel starts at x=160 (256 - 96), y=16 */
        tr_ui_render_right_panel(fb, world, 256 - TR_UI_RIGHT_W);
    }

    if (ui_flags & TR_UI_BOTTOM_PANEL) {
        /* Bottom panel: 4 champion slots, each 80×56.
         * Bottom of planar fb is at y=184..223 (within 224-tall fb). */
        int slot_y = 184; /* absolute y in planar fb */
        for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
            int slot_x = i * TR_CHAMP_SLOT_W;
            const Theron_V1_Champion *c = theron_v1_party_getChampion_c(&world->party, i);
            tr_ui_draw_champion_slot(fb, i, slot_x, slot_y, c);
        }
    }
}

/* ── Source citation ─────────────────────────────────────────────── */
const char *tr_ui_source_evidence(void) {
    return "THQUEST.ASM T600 (UI overlay zones)  "
           "+ THQUEST.ASM T800 (champion panel rendering)  "
           "+ THQUEST.ASM T900 (message bar)";
}
