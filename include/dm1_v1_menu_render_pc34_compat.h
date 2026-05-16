#ifndef FIRESTAFF_DM1_V1_MENU_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MENU_RENDER_PC34_COMPAT_H

/*
 * DM1 V1 Menu Rendering — source-locked to ReDMCSB MENUDRAW.C / MENU.C
 *
 * Movement arrow rendering, action area, spell area layout,
 * enabled menu zones, champion action hand rendering.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   MENUDRAW.C: F0395 (draw movement arrows),
 *               F0396 (load spell area lines bitmap),
 *               F0397 (draw available symbols),
 *               F0398 (draw action area buttons)
 *   MENU.C: F0457 (draw enabled menus — orchestrator for spell/action/movement)
 *   PANEL.C: referenced for champion panel layout
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Menu zone identifiers — from C009/C010/etc. constants */
#define M11_ZONE_MOVEMENT_ARROWS    9
#define M11_ZONE_SPELL_AREA        10
#define M11_ZONE_ACTION_AREA       11

/* Action hand types */
typedef enum {
    DM1_ACTION_HAND_NONE = 0,
    DM1_ACTION_HAND_OPEN,        /* empty hand — can pick up */
    DM1_ACTION_HAND_HOLDING,     /* holding an object */
    DM1_ACTION_HAND_COMBAT,      /* combat action available */
    DM1_ACTION_HAND_COUNT
} M11_ActionHandType;

/* Action area button types */
typedef enum {
    DM1_ACTION_NONE = 0,
    DM1_ACTION_ATTACK,
    DM1_ACTION_CAST,
    DM1_ACTION_USE,
    DM1_ACTION_THROW,
    DM1_ACTION_COUNT
} M11_ActionType;

/* Menu render state */
typedef struct {
    int movementArrowsEnabled;
    int spellAreaEnabled;
    int actionAreaEnabled;
    int candidateChampionOrdinal;    /* G0299: champion attempting action */
    M11_ActionHandType leaderHand;
    int leaderHandObjectIcon;        /* bitmap icon index, -1 if empty */
    int commandHighlightActive;      /* F0363 highlight box */
    int commandHighlightX;
    int commandHighlightY;
    int commandHighlightW;
    int commandHighlightH;
} M11_MenuRenderState;

/* Menu render result */
typedef struct {
    int movementArrowsDrawn;
    int spellAreaDrawn;
    int actionAreaDrawn;
    int enabledMenusDrawn;
    int highlightCleared;
} M11_MenuRenderResult;

/*
 * Initialize menu render state.
 */
void m11_menu_render_init(M11_MenuRenderState *s);

/*
 * Draw movement arrows (F0395).
 * Blits C013_GRAPHIC_MOVEMENT_ARROWS to the movement zone.
 */
int m11_menu_render_draw_movement_arrows(const M11_MenuRenderState *s);

/*
 * Draw action area buttons (F0398).
 * Shows available actions based on leader hand state.
 */
int m11_menu_render_draw_action_area(const M11_MenuRenderState *s);

/*
 * Draw all enabled menus (F0457).
 * Orchestrator: draws spell area, action area, movement arrows
 * based on current enable flags.
 */
M11_MenuRenderResult m11_menu_render_draw_enabled(const M11_MenuRenderState *s);

/*
 * Set command highlight box (for button feedback).
 */
void m11_menu_render_set_highlight(M11_MenuRenderState *s,
                                    int x, int y, int w, int h);

/*
 * Clear command highlight (F0363).
 */
void m11_menu_render_clear_highlight(M11_MenuRenderState *s);

/*
 * Enable/disable menu zones.
 */
void m11_menu_render_enable_movement(M11_MenuRenderState *s, int enabled);
void m11_menu_render_enable_spells(M11_MenuRenderState *s, int enabled);
void m11_menu_render_enable_actions(M11_MenuRenderState *s, int enabled);

/*
 * Set leader hand state for action area rendering.
 */
void m11_menu_render_set_leader_hand(M11_MenuRenderState *s,
                                      M11_ActionHandType handType,
                                      int objectIcon);

/*
 * Source evidence string.
 */
const char *m11_menu_render_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MENU_RENDER_PC34_COMPAT_H */
