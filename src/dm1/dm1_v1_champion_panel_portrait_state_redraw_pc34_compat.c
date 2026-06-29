#include "dm1_v1_champion_panel_portrait_state_redraw_pc34_compat.h"

/*
 * ReDMCSB source-locked contract gate only.
 *
 * CHAMDRAW.C F0293:1134-1138 dispatches active champions to F0292 in index
 * order. F0292:771-839 redraws the status box, F0292:898-935 uses C033/C034
 * for normal/warning overlays, and F0291:595-655 selects C033/C034/C035 for
 * hand slot box state. F0292:771-839 and DEFS.H:3783-3795 anchor the
 * champion-panel status-box zones; DATA.C:264-272 anchors top-row hand slot
 * origins.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. CHAMPION.C F0297:243-298 and F0298:270-298 call "
    "F0292 after leader-hand load changes. CHAMDRAW.C F0291:551-552 reads "
    "slot contents and F0291:595-655 selects C033 normal, C034 wounded, or "
    "C035 acting-hand slot boxes. CHAMDRAW.C F0292:771-839 redraws live/dead "
    "status boxes, F0292:898-935 selects C033/C034 mouth/eye overlays, "
    "F0292:1080-1091 redraws the action-hand slot, CHAMDRAW.C F0293:"
    "1117-1143 calls F0292 for every active champion in index order, and "
    "CHAMDRAW.C F0296:1249-1257 scans changed object icons before F0292. "
    "DEFS.H:3783-3795 anchors C151..C154 status-box zones, C159..C162 "
    "name zones, C163..C166 text zones, C175..C178 status-box portrait "
    "zones, C187..C190 bar-graph zones, and C113..C116 champion-icon zones. "
    "DEFS.H:779-781,1873-1878,2188-2195 and DATA.C:264-272 anchor the "
    "hand slots, C033/C034/C035 cascade, and championIndex*69+{4,24}, "
    "y=10 hand boxes.";

#define ENTRY(CHAMPION, STATE, ORDER) \
    { \
        (CHAMPION), \
        (STATE), \
        (ORDER), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_ZONE_BASE_PC34 + \
            (CHAMPION), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_LEFT_PC34 + \
            ((CHAMPION) * DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_STRIDE_X_PC34), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_TOP_PC34, \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_LEFT_PC34 + \
            ((CHAMPION) * DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_STRIDE_X_PC34) + \
            DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_WIDTH_PC34 - 1, \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_TOP_PC34 + \
            DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_HEIGHT_PC34 - 1, \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_NAME_ZONE_BASE_PC34 + (CHAMPION), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TEXT_ZONE_BASE_PC34 + (CHAMPION), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_PORTRAIT_ZONE_BASE_PC34 + (CHAMPION), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_BAR_ZONE_BASE_PC34 + (CHAMPION), \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_ICON_ZONE_BASE_PC34 + (CHAMPION), \
        ((CHAMPION) * DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_STRIDE_X_PC34) + \
            DM1_V1_CHAMPION_PANEL_STATE_REDRAW_READY_HAND_LOCAL_X_PC34, \
        ((CHAMPION) * DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATUS_BOX_STRIDE_X_PC34) + \
            DM1_V1_CHAMPION_PANEL_STATE_REDRAW_ACTION_HAND_LOCAL_X_PC34, \
        DM1_V1_CHAMPION_PANEL_STATE_REDRAW_HAND_LOCAL_Y_PC34 \
    }

#define CHAMPION_ENTRIES(CHAMPION) \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_OK_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C033_FRESH_BLIT_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_WOUNDED_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_POISONED_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_HUNGRY_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_ASLEEP_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_CONFUSED_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_PARALYZED_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C034_WOUNDED_OVERLAY_PC34), \
    ENTRY((CHAMPION), DM1_V1_CHAMPION_PANEL_STATE_DEAD_PC34, \
          DM1_V1_CHAMPION_PANEL_STATE_REDRAW_C035_DEAD_POLYGON_PC34)

const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t
    dm1_v1_champion_panel_state_redraw_table
        [DM1_V1_CHAMPION_PANEL_STATE_REDRAW_TABLE_COUNT_PC34] = {
            CHAMPION_ENTRIES(0),
            CHAMPION_ENTRIES(1),
            CHAMPION_ENTRIES(2),
            CHAMPION_ENTRIES(3)
        };

#undef CHAMPION_ENTRIES
#undef ENTRY

static bool valid_champion_index(int champion_index)
{
    return champion_index >= 0 &&
           champion_index <
               DM1_V1_CHAMPION_PANEL_STATE_REDRAW_CHAMPION_COUNT_PC34;
}

static bool valid_state(
    dm1_v1_champion_panel_state_redraw_state_pc34_compat_t state)
{
    return state >= DM1_V1_CHAMPION_PANEL_STATE_OK_PC34 &&
           state <= DM1_V1_CHAMPION_PANEL_STATE_DEAD_PC34;
}

bool dm1_v1_champion_panel_state_redraw_entry(
    int champion_index,
    dm1_v1_champion_panel_state_redraw_state_pc34_compat_t state,
    const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t **out_entry)
{
    size_t index;

    if (out_entry) {
        *out_entry = NULL;
    }
    if (!valid_champion_index(champion_index) || !valid_state(state)) {
        return false;
    }

    index = (size_t)champion_index *
                DM1_V1_CHAMPION_PANEL_STATE_REDRAW_STATE_COUNT_PC34 +
            (size_t)state;
    if (out_entry) {
        *out_entry = &dm1_v1_champion_panel_state_redraw_table[index];
    }
    return true;
}

int dm1_v1_champion_panel_state_redraw_order(
    int champion_index,
    dm1_v1_champion_panel_state_redraw_state_pc34_compat_t state)
{
    const dm1_v1_champion_panel_state_redraw_entry_pc34_compat_t *entry;

    if (!dm1_v1_champion_panel_state_redraw_entry(champion_index, state,
                                                  &entry)) {
        return DM1_V1_CHAMPION_PANEL_STATE_REDRAW_INVALID_PC34;
    }
    return (int)entry->redraw_order;
}

const char *dm1_v1_champion_panel_state_redraw_source_evidence(void)
{
    return s_source_evidence;
}
