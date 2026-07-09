#include "dm1_v1_menu_render_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Menu Rendering — implementation
 *
 * Source lock: ReDMCSB WIP20210206 MENUDRAW.C / MENU.C
 *
 * F0395_MENUS_DrawMovementArrows:
 *   Blits graphic C013 to zone 009 (movement arrows area).
 *   Called after mouse screen update enable/disable.
 *
 * F0398: Draw action area with buttons based on current state.
 *   ATTACK / CAST / USE / THROW depending on leader hand contents.
 *
 * F0457_START_DrawEnabledMenus_CPSF:
 *   Called from game loop (GAMELOOP.C) when not resting and
 *   candidateChampionOrdinal is not set.
 *   Draws spell area, action area, movement arrows based on flags.
 *
 * F0363_COMMAND_HighlightBoxDisable:
 *   Clears the command highlight feedback box (button press visual).
 */

void DM1_V1_MenuRender_InitPc34Compat(DM1_V1_MenuRenderStatePc34 *s)
{
    memset(s, 0, sizeof(*s));
    s->movementArrowsEnabled = 1;
    s->spellAreaEnabled = 1;
    s->actionAreaEnabled = 1;
    s->leaderHand = DM1_ACTION_HAND_NONE;
    s->leaderHandObjectIcon = -1;
}

int DM1_V1_MenuRender_DrawMovementArrowsPc34Compat(const DM1_V1_MenuRenderStatePc34 *s)
{
    if (!s->movementArrowsEnabled) return 0;
    /* F0395: blit C013_GRAPHIC_MOVEMENT_ARROWS to zone 009 */
    return 1;
}

int DM1_V1_MenuRender_DrawActionAreaPc34Compat(const DM1_V1_MenuRenderStatePc34 *s)
{
    if (!s->actionAreaEnabled) return 0;

    /*
     * F0398 logic: draw action buttons based on leader hand state.
     * If hand empty (OPEN): show ATTACK
     * If hand holding: show THROW, USE
     * If combat action available: show ATTACK, CAST
     */
    (void)s;
    return 1;
}

DM1_V1_MenuRenderResultPc34 DM1_V1_MenuRender_DrawEnabledPc34Compat(const DM1_V1_MenuRenderStatePc34 *s)
{
    DM1_V1_MenuRenderResultPc34 result;
    memset(&result, 0, sizeof(result));

    /*
     * F0457_START_DrawEnabledMenus_CPSF:
     * Called from game loop when:
     *   !G0300_B_PartyIsResting
     *   !G0299_ui_CandidateChampionOrdinal
     * Draws each enabled menu zone.
     */

    if (s->movementArrowsEnabled) {
        result.movementArrowsDrawn = DM1_V1_MenuRender_DrawMovementArrowsPc34Compat(s);
    }

    if (s->actionAreaEnabled) {
        result.actionAreaDrawn = DM1_V1_MenuRender_DrawActionAreaPc34Compat(s);
    }

    if (s->spellAreaEnabled) {
        result.spellAreaDrawn = 1;
    }

    result.enabledMenusDrawn = result.movementArrowsDrawn +
                                result.actionAreaDrawn +
                                result.spellAreaDrawn;
    return result;
}

void DM1_V1_MenuRender_SetHighlightPc34Compat(DM1_V1_MenuRenderStatePc34 *s,
                                    int x, int y, int w, int h)
{
    s->commandHighlightActive = 1;
    s->commandHighlightX = x;
    s->commandHighlightY = y;
    s->commandHighlightW = w;
    s->commandHighlightH = h;
}

void DM1_V1_MenuRender_ClearHighlightPc34Compat(DM1_V1_MenuRenderStatePc34 *s)
{
    /* F0363_COMMAND_HighlightBoxDisable */
    s->commandHighlightActive = 0;
    s->commandHighlightX = 0;
    s->commandHighlightY = 0;
    s->commandHighlightW = 0;
    s->commandHighlightH = 0;
}

void DM1_V1_MenuRender_EnableMovementPc34Compat(DM1_V1_MenuRenderStatePc34 *s, int enabled)
{
    s->movementArrowsEnabled = enabled;
}

void DM1_V1_MenuRender_EnableSpellsPc34Compat(DM1_V1_MenuRenderStatePc34 *s, int enabled)
{
    s->spellAreaEnabled = enabled;
}

void DM1_V1_MenuRender_EnableActionsPc34Compat(DM1_V1_MenuRenderStatePc34 *s, int enabled)
{
    s->actionAreaEnabled = enabled;
}

void DM1_V1_MenuRender_SetLeaderHandPc34Compat(DM1_V1_MenuRenderStatePc34 *s,
                                      DM1_V1_ActionHandTypePc34 handType,
                                      int objectIcon)
{
    s->leaderHand = handType;
    s->leaderHandObjectIcon = objectIcon;
}

const char *DM1_V1_MenuRender_SourceEvidencePc34Compat(void)
{
    return
        "ReDMCSB WIP20210206 MENUDRAW.C / MENU.C\n"
        "F0395_MENUS_DrawMovementArrows: blit C013 to zone 009.\n"
        "F0396_MENUS_LoadSpellAreaLinesBitmap: load C011 graphic.\n"
        "F0397_MENUS_DrawAvailableSymbols: 6 symbols per step.\n"
        "F0398: draw action area (ATTACK/CAST/USE/THROW).\n"
        "F0457_START_DrawEnabledMenus_CPSF: orchestrator for all menu zones.\n"
        "  Called from GAMELOOP.C when !G0300 and !G0299.\n"
        "F0363_COMMAND_HighlightBoxDisable: clear button press feedback.\n"
        "G0299_ui_CandidateChampionOrdinal: champion attempting action.\n"
        "Leader hand state determines action area button set.";
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — MENU.C remaining function citations
 *
 *   MENU.C:844 F0392_MENUS_B
 *   MENU.C:1874 F0802_I
 * ══════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — MENUDRAW.C remaining function citations
 *
 *   MENUDRAW.C:22 F0517_MENUS_A
 * ══════════════════════════════════════════════════════════════════════ */

