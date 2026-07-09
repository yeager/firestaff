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
#define DM1_V1_ZONE_MOVEMENT_ARROWS_PC34    9
#define DM1_V1_ZONE_SPELL_AREA_PC34        10
#define DM1_V1_ZONE_ACTION_AREA_PC34       11

/* Action hand types */
typedef enum {
    DM1_ACTION_HAND_NONE = 0,
    DM1_ACTION_HAND_OPEN,        /* empty hand — can pick up */
    DM1_ACTION_HAND_HOLDING,     /* holding an object */
    DM1_ACTION_HAND_COMBAT,      /* combat action available */
    DM1_ACTION_HAND_COUNT
} DM1_V1_ActionHandTypePc34;

/* Action area button types */
typedef enum {
    DM1_ACTION_NONE = 0,
    DM1_ACTION_ATTACK,
    DM1_ACTION_CAST,
    DM1_ACTION_USE,
    DM1_ACTION_THROW,
    DM1_ACTION_COUNT
} DM1_V1_ActionTypePc34;

/* Menu render state */
typedef struct {
    int movementArrowsEnabled;
    int spellAreaEnabled;
    int actionAreaEnabled;
    int candidateChampionOrdinal;    /* G0299: champion attempting action */
    DM1_V1_ActionHandTypePc34 leaderHand;
    int leaderHandObjectIcon;        /* bitmap icon index, -1 if empty */
    int commandHighlightActive;      /* F0363 highlight box */
    int commandHighlightX;
    int commandHighlightY;
    int commandHighlightW;
    int commandHighlightH;
} DM1_V1_MenuRenderStatePc34;

/* Menu render result */
typedef struct {
    int movementArrowsDrawn;
    int spellAreaDrawn;
    int actionAreaDrawn;
    int enabledMenusDrawn;
    int highlightCleared;
} DM1_V1_MenuRenderResultPc34;

/*
 * Initialize menu render state.
 */
void DM1_V1_MenuRender_InitPc34Compat(DM1_V1_MenuRenderStatePc34 *s);

/*
 * Draw movement arrows (F0395).
 * Blits C013_GRAPHIC_MOVEMENT_ARROWS to the movement zone.
 */
int DM1_V1_MenuRender_DrawMovementArrowsPc34Compat(const DM1_V1_MenuRenderStatePc34 *s);

/*
 * Draw action area buttons (F0398).
 * Shows available actions based on leader hand state.
 */
int DM1_V1_MenuRender_DrawActionAreaPc34Compat(const DM1_V1_MenuRenderStatePc34 *s);

/*
 * Draw all enabled menus (F0457).
 * Orchestrator: draws spell area, action area, movement arrows
 * based on current enable flags.
 */
DM1_V1_MenuRenderResultPc34 DM1_V1_MenuRender_DrawEnabledPc34Compat(const DM1_V1_MenuRenderStatePc34 *s);

/*
 * Set command highlight box (for button feedback).
 */
void DM1_V1_MenuRender_SetHighlightPc34Compat(DM1_V1_MenuRenderStatePc34 *s,
                                    int x, int y, int w, int h);

/*
 * Clear command highlight (F0363).
 */
void DM1_V1_MenuRender_ClearHighlightPc34Compat(DM1_V1_MenuRenderStatePc34 *s);

/*
 * Enable/disable menu zones.
 */
void DM1_V1_MenuRender_EnableMovementPc34Compat(DM1_V1_MenuRenderStatePc34 *s, int enabled);
void DM1_V1_MenuRender_EnableSpellsPc34Compat(DM1_V1_MenuRenderStatePc34 *s, int enabled);
void DM1_V1_MenuRender_EnableActionsPc34Compat(DM1_V1_MenuRenderStatePc34 *s, int enabled);

/*
 * Set leader hand state for action area rendering.
 */
void DM1_V1_MenuRender_SetLeaderHandPc34Compat(DM1_V1_MenuRenderStatePc34 *s,
                                      DM1_V1_ActionHandTypePc34 handType,
                                      int objectIcon);

/*
 * Source evidence string.
 */
const char *DM1_V1_MenuRender_SourceEvidencePc34Compat(void);

/* Compatibility aliases for older M11 call sites. */
#define M11_ZONE_MOVEMENT_ARROWS DM1_V1_ZONE_MOVEMENT_ARROWS_PC34
#define M11_ZONE_SPELL_AREA DM1_V1_ZONE_SPELL_AREA_PC34
#define M11_ZONE_ACTION_AREA DM1_V1_ZONE_ACTION_AREA_PC34
typedef DM1_V1_ActionHandTypePc34 M11_ActionHandType;
typedef DM1_V1_ActionTypePc34 M11_ActionType;
typedef DM1_V1_MenuRenderStatePc34 M11_MenuRenderState;
typedef DM1_V1_MenuRenderResultPc34 M11_MenuRenderResult;
#define m11_menu_render_init DM1_V1_MenuRender_InitPc34Compat
#define m11_menu_render_draw_movement_arrows DM1_V1_MenuRender_DrawMovementArrowsPc34Compat
#define m11_menu_render_draw_action_area DM1_V1_MenuRender_DrawActionAreaPc34Compat
#define m11_menu_render_draw_enabled DM1_V1_MenuRender_DrawEnabledPc34Compat
#define m11_menu_render_set_highlight DM1_V1_MenuRender_SetHighlightPc34Compat
#define m11_menu_render_clear_highlight DM1_V1_MenuRender_ClearHighlightPc34Compat
#define m11_menu_render_enable_movement DM1_V1_MenuRender_EnableMovementPc34Compat
#define m11_menu_render_enable_spells DM1_V1_MenuRender_EnableSpellsPc34Compat
#define m11_menu_render_enable_actions DM1_V1_MenuRender_EnableActionsPc34Compat
#define m11_menu_render_set_leader_hand DM1_V1_MenuRender_SetLeaderHandPc34Compat
#define m11_menu_render_source_evidence DM1_V1_MenuRender_SourceEvidencePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MENU_RENDER_PC34_COMPAT_H */
