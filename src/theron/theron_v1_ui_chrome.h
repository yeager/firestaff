/*
 * theron_v1_ui_chrome.h — Theron's Quest V1 Phase 4: UI Chrome Renderer
 *
 * Theron uses a horizontal split-screen layout different from DM1/CSB/DM2:
 *   Top bar:       y=0..23    — dungeon name, quest item count
 *   Viewport:      x=32, y=24, 192×160 — 2D tile dungeon view
 *   Right panel:   x=224, y=24, 96×160 — Theron stats + compass
 *   Bottom panel:  y=184, 320×56       — 4 champion slots (80×56 each)
 *   Message bar:   y=184..199          — single-line message area
 *
 * The UI chrome is composited on top of the planar viewport framebuffer.
 *
 * Source references:
 *   THQUEST.ASM T600   — UI overlay zones
 *   THQUEST.ASM T800   — champion panel rendering
 *   THQUEST.ASM T900   — message bar
 */

#ifndef THERON_V1_UI_CHROME_H
#define THERON_V1_UI_CHROME_H

#include "theron_v1_palette.h"
#include "theron_v1_viewport.h"
#include "theron_v1_world.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── UI chrome zones ─────────────────────────────────────────────── */
#define TR_UI_TOPBAR_H     24
#define TR_UI_RIGHT_W      96
#define TR_UI_BOTTOM_H     56
#define TR_UI_MSG_H        16

/* ── Champion slot layout (4 slots, bottom panel) ───────────────── */
#define TR_CHAMP_SLOT_W     80
#define TR_CHAMP_SLOT_H     56

/* ── Champion slot colors ───────────────────────────────────────── */
#define TR_CHROME_BG        12   /* dark gray background         */
#define TR_CHROME_FRAME      1   /* gray border                   */
#define TR_CHROME_HP         8   /* red HP                        */
#define TR_CHROME_STAMINA   10   /* tan stamina                   */
#define TR_CHROME_MANA      14   /* blue mana                     */
#define TR_CHROME_NAME      15   /* white                         */

/* ── Render flags ───────────────────────────────────────────────── */
#define TR_UI_TOPBAR      (1U << 0)
#define TR_UI_RIGHT_PANEL (1U << 1)
#define TR_UI_BOTTOM_PANEL (1U << 2)
#define TR_UI_MESSAGE     (1U << 3)
#define TR_UI_ALL         (TR_UI_TOPBAR | TR_UI_RIGHT_PANEL | TR_UI_BOTTOM_PANEL | TR_UI_MESSAGE)

/* ══════════════════════════════════════════════════════════════════════
 * Bar drawing
 * ══════════════════════════════════════════════════════════════════════ */

/* Draw a horizontal bar in the planar framebuffer.
 * x, y: top-left corner within planar fb coords.
 * w, h: bar dimensions in pixels.
 * current, max: bar values (0..max).  If max<=0, treated as max=1.
 * pal_index: palette entry for the filled portion.
 * bg_index: palette entry for the empty/background portion.
 *
 * Source: THQUEST.ASM T800 (HP/stamina bar rendering).
 */
void tr_ui_draw_bar(TQR_PlanarFramebuffer *fb,
                    int x, int y, int w, int h,
                    int current, int max,
                    uint8_t pal_index,
                    uint8_t bg_index);

/* ══════════════════════════════════════════════════════════════════════
 * Champion slot rendering
 * ══════════════════════════════════════════════════════════════════════ */

/* Render a single champion slot (80×56 bottom panel).
 * slot_idx: 0..3 (Theron + companions).
 * x, y: top-left of the slot region.
 * champion: champion data (NULL = empty/dead slot → draws X mark).
 *
 * Slot layout (80×56):
 *   icon (24×24) | name bar + class indicator
 *                | HP bar | stamina bar | mana bar (if applicable)
 *
 * Source: THQUEST.ASM T800 (champion panel rendering).
 */
void tr_ui_draw_champion_slot(TQR_PlanarFramebuffer *fb,
                               int slot_idx,
                               int x, int y,
                               const Theron_V1_Champion *champion);

/* ══════════════════════════════════════════════════════════════════════
 * Zone renderers
 * ══════════════════════════════════════════════════════════════════════ */

/* Render the top bar zone: dungeon name + quest item count.
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void tr_ui_render_topbar(TQR_PlanarFramebuffer *fb,
                         const Theron_V1_World *world,
                         int y_offset);

/* Render the right panel: Theron stats (HP, stamina) + compass direction.
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void tr_ui_render_right_panel(TQR_PlanarFramebuffer *fb,
                               const Theron_V1_World *world,
                               int x_offset);

/* ══════════════════════════════════════════════════════════════════════
 * Master UI renderer
 * ══════════════════════════════════════════════════════════════════════ */

/* Render Theron-specific UI chrome over the planar viewport fb.
 * Flags controls which zones are drawn (OR of TR_UI_* bits).
 *
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void tr_ui_render(TQR_PlanarFramebuffer *fb,
                   const Theron_V1_World *world,
                   uint32_t ui_flags);

/* ── Source citation ─────────────────────────────────────────────── */
const char *tr_ui_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_UI_CHROME_H */
