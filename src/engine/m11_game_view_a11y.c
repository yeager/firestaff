/*
 * m11_game_view_a11y.c
 *
 * M11 gameplay-side screen-reader / accessibility state manifest.
 *
 *   menu_startup_a11y_m12.h (launcher side) is the analogue for
 *   the launcher state machine. Both route through the same
 *   fs_ax_* writer into ~/.firestaff/accessibility.json, gated on
 *   fs_ax_is_enabled() (set via FS_ACCESSIBILITY=1).
 *
 * This file replaces the inline accessibility manifest block that
 * used to live at the bottom of M11_GameView_Draw
 * (src/engine/m11_game_view.c:28214-28237). The old block only
 * emitted the always-on viewport / movement / spell / HUD zones;
 * the new version also emits zones for each in-game overlay
 * (inventory panel, full-screen map, dialog overlay, candidate
 * mirror / hall of champions mirror, endgame portraits) so an
 * external screen reader can announce the live state without
 * parsing the framebuffer.
 *
 * Source-lock: this layer does not consult ReDMCSB at runtime.
 * The geometry constants are all from the public
 * M11_GameView_GetV1*Zone() helpers in include/m11_game_view.h,
 * which themselves cite the source-locked layout-696 zones
 * (ReDMCSB DEFS.H:821-826, 1693-1749, 2186, 2552, 2596-2611, ...
 * and PANEL.C / ENDGAME.C / MAPUI.C).  The screen reader only
 * reads M11_GameViewState fields and the public zone helpers.
 *
 * Determinism / privacy:
 *   - All labels are stable strings ("Forward", "Turn Left",
 *     "Champion Portrait", "Hand Slot Left", "Dialog Body",
 *     ...). Runtime-derived strings (champion names, dialog text)
 *     come from M11_GameViewState fields, not the file system,
 *     so the manifest is stable across runs with the same locale
 *     and game data.
 *   - No player save data, hash bytes, or runtime counters are
 *     emitted.
 *   - The Peekaboo schema (alphanumeric + underscore, no spaces)
 *     is honored by all zone IDs and state names so external
 *     automation can route announcers by zone type.
 */

#include "m11_game_view_a11y.h"

#include "m11_game_view.h"
#include "firestaff_accessibility.h"
#include "session_timer_runtime.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* -- Stable state-name table --------------------------------------- */

static const char* const kM11AxStateNames[M11_AX_STATE_COUNT] = {
    "gameplay",         /* M11_AX_STATE_GAMEPLAY         */
    "inventory",        /* M11_AX_STATE_INVENTORY        */
    "map",              /* M11_AX_STATE_MAP              */
    "dialog",           /* M11_AX_STATE_DIALOG           */
    "entrance_mirror",  /* M11_AX_STATE_ENTRANCE_MIRROR  */
    "endgame",          /* M11_AX_STATE_ENDGAME          */
    "other"             /* M11_AX_STATE_OTHER            */
};

const char* m11_screen_reader_state_name(M11_AX_State state) {
    int idx = (int)state;
    if (idx < 0 || idx >= M11_AX_STATE_COUNT) {
        return "other";
    }
    return kM11AxStateNames[idx];
}

/* -- State classifier ---------------------------------------------- */

M11_AX_State m11_screen_reader_state_for(const M11_GameViewState* state) {
    if (!state) {
        return M11_AX_STATE_OTHER;
    }
    /* Order matters: dialogOverlayActive beats candidateMirrorPanelActive
     * (a Hall of Champions mirror popup may also open with a dialog
     * text plate above it). Endgame beats dialog (a "THE END" plaque
     * is a dialog variant, but the screen reader should still report
     * the broader phase as "endgame"). Inventory / map are mutually
     * exclusive in practice but we still order them deterministically:
     * inventory first if both flags happen to be set, then map. */
    if (state->gameWon) {
        return M11_AX_STATE_ENDGAME;
    }
    if (state->dialogOverlayActive) {
        return M11_AX_STATE_DIALOG;
    }
    if (state->candidateMirrorPanelActive) {
        return M11_AX_STATE_ENTRANCE_MIRROR;
    }
    if (state->inventoryPanelActive) {
        return M11_AX_STATE_INVENTORY;
    }
    if (state->mapOverlayActive) {
        return M11_AX_STATE_MAP;
    }
    return M11_AX_STATE_GAMEPLAY;
}

const char* m11_screen_reader_view_name(const M11_GameViewState* state) {
    return m11_screen_reader_state_name(m11_screen_reader_state_for(state));
}

/* -- Per-record scratch storage ------------------------------------
 *
 * The fs_ax_* writer takes a const FS_AX_Element* and reads the
 * id/label/value pointers. We need lifetime-stable writable storage
 * for those strings, so each record owns id/label/value buffers.
 * m11_ax_begin() points the element at those buffers, callers format
 * strings in place, then hand the element pointers to fs_ax_add_element().
 * The flush() at the end of update_ex() copies the strings into
 * the writer's internal JSON buffer, after which we can safely
 * reset for the next frame.
 *
 * M11_AX_MAX_RECORDS is sized so we can emit the full overlay
 *   1 launcher_root    (always-on)
 *   1 viewport         (always-on)
 *   4 movement arrows  (always-on)
 *   1 spell area       (always-on)
 *   1 HUD panel        (always-on)
 *   1 control strip    (always-on)
 *   13 inventory equipment slots (READY_H, ACTION_H, HEAD, TORSO,
 *     LEGS, FEET, NECK, POUCH_1, POUCH_2, QUIVER_LINE1_1, QUIVER_
 *     LINE1_2, QUIVER_LINE2_1, QUIVER_LINE2_2 -> 13 zones)
 *   1 inventory portrait
 *   1 inventory champion name
 *   1 inventory panel body
 *   16 inventory backpack slots
 *   1 inventory panel region
 *   1 map overlay
 *   1 dialog region + 1 dialog body + 4 dialog choice buttons
 *   1 candidate mirror panel + 1 reincarnate + 1 resurrect + 1 cancel
 *   4 endgame champion mirrors + 4 endgame portraits
 *   1 endgame "THE END" plaque
 *   1 session-timer reminder banner + 1 reminder text
 *   1 session-timer forced-pause dialog box + 1 title + 2 lines
 * = comfortably < 80 (leaves room for future Firestaff-specific
 *   sticky overlays layered on top of normal gameplay).
 */

#define M11_AX_ID_LEN     64
#define M11_AX_LABEL_LEN  64
#define M11_AX_VALUE_LEN  128

typedef struct {
    FS_AX_Element element;
    char idBuf[M11_AX_ID_LEN];
    char labelBuf[M11_AX_LABEL_LEN];
    char valueBuf[M11_AX_VALUE_LEN];
} M11_AX_Record;

#define M11_AX_MAX_RECORDS 80

static M11_AX_Record g_m11_ax_records[M11_AX_MAX_RECORDS];
static int g_m11_ax_record_count = 0;

#define M11_AX_ID(e)    ((char*)((e)->id))
#define M11_AX_LABEL(e) ((char*)((e)->label))
#define M11_AX_VALUE(e) ((char*)((e)->value))

static void m11_ax_reset(void) {
    g_m11_ax_record_count = 0;
}

/* Begin one element. The caller fills idBuf / labelBuf /
 * valueBuf via snprintf and may leave valueBuf empty (the
 * finalize pass flips element.value to NULL when valueBuf[0]
 * is 0 so the writer emits JSON null). Returns NULL when
 * the record array is exhausted. */
static FS_AX_Element* m11_ax_begin(FS_AX_ElementType type,
                                   int x, int y, int w, int h,
                                   int enabled) {
    M11_AX_Record* rec;
    if (g_m11_ax_record_count >= M11_AX_MAX_RECORDS) {
        return NULL;
    }
    rec = &g_m11_ax_records[g_m11_ax_record_count++];
    memset(rec, 0, sizeof(*rec));
    rec->element.id    = rec->idBuf;
    rec->element.label = rec->labelBuf;
    rec->element.value = rec->valueBuf;
    rec->element.type  = type;
    rec->element.x     = x;
    rec->element.y     = y;
    rec->element.w     = w;
    rec->element.h     = h;
    rec->element.enabled = enabled ? 1 : 0;
    return &rec->element;
}

/* After all m11_ax_begin + snprintf work for a record, walk
 * the array and toggle element.value to NULL when valueBuf[0]
 * is 0 (so the writer emits JSON null instead of ""). */
static void m11_ax_finalize(void) {
    int i;
    for (i = 0; i < g_m11_ax_record_count; ++i) {
        M11_AX_Record* rec = &g_m11_ax_records[i];
        if (rec->valueBuf[0] == '\0') {
            rec->element.value = NULL;
        }
    }
}

/* -- Source-faithful viewport / control strip constants -------------
 *
 * These mirror the file-static enums in m11_game_view.c:185-202 so
 * we don't need to expose them in the public header.  They are
 * canonical across all gameplay renderers and gate on the DM1
 * PC 3.4 source.  If m11_game_view.c ever changes these values
 * (it shouldn't - the citations are source-locked to COORD.C
 * G2067/G2068 and DEFS.H C112/C136), the screen-reader manifest
 * must follow.
 */
#define M11_AX_VIEWPORT_X   0     /* COORD.C G2067_i_ViewportScreenX    */
#define M11_AX_VIEWPORT_Y   33    /* COORD.C G2068_i_ViewportScreenY    */
#define M11_AX_VIEWPORT_W   224   /* DEFS.H C112_BYTE_WIDTH_VIEWPORT*2  */
#define M11_AX_VIEWPORT_H   136   /* DEFS.H C136_HEIGHT_VIEWPORT        */
#define M11_AX_CONTROL_STRIP_X  14
#define M11_AX_CONTROL_STRIP_Y  165
#define M11_AX_CONTROL_STRIP_W  88
#define M11_AX_CONTROL_STRIP_H  14

/* -- Always-on zone helpers ---------------------------------------- */

static void m11_ax_emit_always_on_zones(const M11_GameViewState* state) {
    int active;
    FS_AX_Element* e;

    if (!state) return;
    active = state->active && !state->dialogOverlayActive ? 1 : 0;

    /* VIEWPORT - the 224x136 DM1 dungeon view at framebuffer (0,33).
     * Source-locked to M11_AX_VIEWPORT_* above. */
    e = m11_ax_begin(FS_AX_REGION,
                     M11_AX_VIEWPORT_X, M11_AX_VIEWPORT_Y,
                     M11_AX_VIEWPORT_W, M11_AX_VIEWPORT_H, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "VIEWPORT");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Dungeon View");
    }

    /* Movement arrows. Coordinates match m11_game_view.c:28223-28226
     * (the original inline block). */
    e = m11_ax_begin(FS_AX_MOVEMENT, 144, 137, 32, 20, active);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "MOVE_FWD");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Forward");
    }
    e = m11_ax_begin(FS_AX_MOVEMENT, 144, 173, 32, 20, active);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "MOVE_BACK");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Back");
    }
    e = m11_ax_begin(FS_AX_MOVEMENT, 112, 155, 32, 18, active);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "TURN_LEFT");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Turn Left");
    }
    e = m11_ax_begin(FS_AX_MOVEMENT, 176, 155, 32, 18, active);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "TURN_RIGHT");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Turn Right");
    }

    /* SPELL_AREA - the right-side spell panel (87x66 at framebuffer
     * (233,0)). Source: M11_GameView_OpenSpellPanel + the right-column
     * rune region from DM1 PC 3.4 layout-696. */
    e = m11_ax_begin(FS_AX_REGION, 233, 0, 87, 66, active);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "SPELL_AREA");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Spell Area");
    }

    /* HUD_PANEL - the 320x64 lower band. Always-on so a screen
     * reader can announce the HUD even when overlays are layered
     * on top. */
    e = m11_ax_begin(FS_AX_REGION, 0, 136, 320, 64, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "HUD_PANEL");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "HUD Panel");
    }

    /* CONTROL_STRIP - the 88x14 control strip at (14,165). This is
     * the right-justified control button row from
     * m11_game_view.c:9147-9156 (M11_CONTROL_STRIP_*). */
    e = m11_ax_begin(FS_AX_REGION,
                     M11_AX_CONTROL_STRIP_X, M11_AX_CONTROL_STRIP_Y,
                     M11_AX_CONTROL_STRIP_W, M11_AX_CONTROL_STRIP_H, active);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CONTROL_STRIP");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Control Strip");
    }
}

/* -- Firestaff-specific sticky overlay zones ----------------------- */

static void m11_ax_format_session_timer_reminder_line(
    const M11_GameViewState* state,
    char* out,
    size_t outSize)
{
    char remaining[SESSION_TIMER_RUNTIME_TEXT_CAPACITY];
    int scale;
    int seconds;
    if (!out || outSize == 0U) {
        return;
    }
    out[0] = '\0';
    scale = state ? state->fontScale : 1;
    if (scale < 1) scale = 1;
    if (scale > 3) scale = 3;
    if (scale >= 3) {
        seconds = SessionTimerRuntime_RemainingSeconds(
            state ? &state->sessionTimerRuntime : NULL);
        if (seconds < 0) {
            snprintf(out, outSize, "OFF");
        } else if (seconds >= 3600) {
            snprintf(out, outSize, "%dH LEFT", (seconds + 3599) / 3600);
        } else if (seconds >= 60) {
            snprintf(out, outSize, "%dM LEFT", (seconds + 59) / 60);
        } else {
            snprintf(out, outSize, "%dS LEFT", seconds);
        }
        return;
    }
    SessionTimerRuntime_FormatRemaining(
        state ? &state->sessionTimerRuntime : NULL,
        remaining,
        sizeof(remaining));
    if (scale == 2) {
        snprintf(out, outSize, "%s LEFT", remaining);
    } else {
        snprintf(out, outSize, "SESSION TIMER %s REMAINING", remaining);
    }
}

static void m11_ax_emit_session_timer_overlay_zones(
    const M11_GameViewState* state,
    int fbW,
    int fbH)
{
    FS_AX_Element* e;

    if (!state) return;

    /* The session timer is Firestaff UI, not ReDMCSB game logic.
     * Match the M11_GameView_Draw visibility fences exactly:
     * reminder is hidden by dialog / forced pause, forced pause is
     * hidden by dialog / return-to-menu confirmation. */
    if (state->sessionTimerReminderOverlayActive &&
        !state->sessionTimerForcedPauseDialogActive &&
        !state->dialogOverlayActive) {
        char line[64];
        enum {
            TIMER_REMINDER_X = 4,
            TIMER_REMINDER_Y = 4,
            TIMER_REMINDER_W = 312,
            TIMER_REMINDER_H = 28,
            TIMER_REMINDER_TEXT_INSET = 8,
            TIMER_REMINDER_TEXT_Y = 8
        };

        m11_ax_format_session_timer_reminder_line(
            state, line, sizeof(line));

        e = m11_ax_begin(FS_AX_REGION,
                         TIMER_REMINDER_X, TIMER_REMINDER_Y,
                         TIMER_REMINDER_W, TIMER_REMINDER_H, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "SESSION_TIMER_REMINDER");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                     "Session Timer Reminder");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "remaining=%d",
                     SessionTimerRuntime_RemainingSeconds(
                         &state->sessionTimerRuntime));
        }

        e = m11_ax_begin(FS_AX_TEXT,
                         TIMER_REMINDER_X + TIMER_REMINDER_TEXT_INSET,
                         TIMER_REMINDER_TEXT_Y,
                         TIMER_REMINDER_W - (2 * TIMER_REMINDER_TEXT_INSET),
                         TIMER_REMINDER_H - TIMER_REMINDER_TEXT_INSET,
                         1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN,
                     "SESSION_TIMER_REMINDER_TEXT");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                     "Reminder Text");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN, "%s", line);
        }
    }

    if (state->sessionTimerForcedPauseDialogActive &&
        !state->dialogOverlayActive &&
        !state->returnToMenuConfirmActive) {
        M11_ForcedPauseDialogLayout layout;
        M11_GameView_GetForcedPauseDialogLayout(state, fbW, fbH, &layout);

        e = m11_ax_begin(FS_AX_POPUP,
                         layout.boxX, layout.boxY,
                         layout.boxW, layout.boxH, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN,
                     "SESSION_TIMER_FORCED_PAUSE");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                     "Session Timer Forced Pause");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "scale=%d;remaining=%d",
                     layout.scale,
                     SessionTimerRuntime_RemainingSeconds(
                         &state->sessionTimerRuntime));
        }

        e = m11_ax_begin(FS_AX_TEXT,
                         layout.titleX, layout.titleY,
                         layout.boxW, 10 * layout.scale, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN,
                     "SESSION_TIMER_FORCED_PAUSE_TITLE");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                     "Forced Pause Title");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "%s", layout.title);
        }

        e = m11_ax_begin(FS_AX_TEXT,
                         layout.line1X, layout.line1Y,
                         layout.boxW, 10 * layout.scale, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN,
                     "SESSION_TIMER_FORCED_PAUSE_LINE1");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                     "Forced Pause Line 1");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "%s", layout.line1);
        }

        e = m11_ax_begin(FS_AX_TEXT,
                         layout.line2X, layout.line2Y,
                         layout.boxW, 10 * layout.scale, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN,
                     "SESSION_TIMER_FORCED_PAUSE_LINE2");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                     "Forced Pause Line 2");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "%s", layout.line2);
        }
    }
}

/* -- Inventory panel zones ----------------------------------------- */

static void m11_ax_emit_inventory_panel_zones(const M11_GameViewState* state) {
    int panelX, panelY, panelW, panelH;
    int inventorySlotCount;
    int i;
    FS_AX_Element* e;

    if (!state) return;

    /* Inventory panel region - ReDMCSB layout-696 C101_ZONE_PANEL. */
    if (!M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH)) {
        return;
    }
    e = m11_ax_begin(FS_AX_REGION,
                     M11_AX_VIEWPORT_X + panelX,
                     M11_AX_VIEWPORT_Y + panelY,
                     panelW, panelH, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "INVENTORY_PANEL");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Champion Inventory");
        /* Emit the active champion ordinal so a screen reader can
         * announce "Champion 1 of 4 inventory open". */
        if (state->world.party.activeChampionIndex >= 0 &&
            state->world.party.activeChampionIndex < 4) {
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "active_champion=%d",
                     state->world.party.activeChampionIndex);
        }
    }

    /* Inventory equipment slots - ReDMCSB C507..C519 (READY_H,
     * ACTION_H, HEAD, TORSO, LEGS, FEET, POUCH_2, QUIVER_LINE2_1,
     * QUIVER_LINE1_2, QUIVER_LINE2_2, NECK, POUCH_1, QUIVER_LINE1_1).
     * The GetV1InventorySourceSlotBoxZone() helper indexes the same
     * layout-696 array that m11_draw_inventory_panel draws. We use
     * indices 8..20 (C507..C519) to match the helper's 1-based
     * source-slot-box numbering used by m11_game_view.c:26691. */
    inventorySlotCount = M11_GameView_GetV1InventorySourceSlotBoxZoneCount();
    if (inventorySlotCount > 20) inventorySlotCount = 20; /* C507..C519 only */
    for (i = 8; i <= 20; ++i) {
        int zx = 0, zy = 0, zw = 0, zh = 0;
        const char* role = NULL;
        if (!M11_GameView_GetV1InventorySourceSlotBoxZone(i, &zx, &zy, &zw, &zh)) {
            continue;
        }
        /* Use the C5xx ordinal as the role ID. Slot 8 = READY_HAND,
         * 9 = ACTION_HAND, etc. - see kV1InventorySourceSlotBoxZones
         * at m11_game_view.c:22020. */
        switch (i) {
            case 8:  role = "READY_HAND";      break;
            case 9:  role = "ACTION_HAND";     break;
            case 10: role = "HEAD";            break;
            case 11: role = "TORSO";           break;
            case 12: role = "LEGS";            break;
            case 13: role = "FEET";            break;
            case 14: role = "POUCH_2";         break;
            case 15: role = "QUIVER_LINE2_1";  break;
            case 16: role = "QUIVER_LINE1_2";  break;
            case 17: role = "QUIVER_LINE2_2";  break;
            case 18: role = "NECK";            break;
            case 19: role = "POUCH_1";         break;
            case 20: role = "QUIVER_LINE1_1";  break;
            default: break;
        }
        if (!role) role = "EQUIP_SLOT";
        e = m11_ax_begin(FS_AX_SLOT,
                         M11_AX_VIEWPORT_X + zx, M11_AX_VIEWPORT_Y + zy,
                         zw, zh,
                         state->inventorySelectedSlot == (i - 8) ? 1 : 0);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "INV_%s", role);
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Hand Slot %s",
                     role ? role : "");
        }
    }

    /* Backpack grid - ReDMCSB C520..C536 (16 slots laid out 8x2 at
     * the top of the inventory panel). Coordinates come straight
     * from kV1InventoryBackpackSlotZones at m11_game_view.c:22051. */
    {
        static const struct { int x, y; } kBackpack[] = {
            { 66, 33 },  /* C520 - slot 0 */
            { 83, 16 }, { 100, 16 }, { 117, 16 }, { 134, 16 },
            { 151, 16 }, { 168, 16 }, { 185, 16 }, { 202, 16 },
            { 83, 33 }, { 100, 33 }, { 117, 33 }, { 134, 33 },
            { 151, 33 }, { 168, 33 }, { 185, 33 }, { 202, 33 }
        };
        int bpIdx;
        for (bpIdx = 0; bpIdx < 16; ++bpIdx) {
            e = m11_ax_begin(FS_AX_SLOT,
                             M11_AX_VIEWPORT_X + kBackpack[bpIdx].x,
                             M11_AX_VIEWPORT_Y + kBackpack[bpIdx].y,
                             16, 16,
                             state->inventorySelectedSlot == (13 + bpIdx) ? 1 : 0);
            if (e) {
                snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "INV_BACKPACK_%d", bpIdx);
                snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Backpack Slot %d",
                         bpIdx + 1);
            }
        }
    }

    /* Champion portrait + name. m11_draw_inventory_panel places the
     * portrait at panelX+5, panelY+4 with the source-locked 32x29
     * C026 portrait bitmap. The name sits next to the portrait
     * (m11_game_view.c:26834-26846). We mirror those coordinates
     * here so the screen reader can announce them by zone. */
    e = m11_ax_begin(FS_AX_PORTRAIT,
                     M11_AX_VIEWPORT_X + panelX + 5,
                     M11_AX_VIEWPORT_Y + panelY + 4,
                     32, 29, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "INVENTORY_PORTRAIT");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Champion Portrait");
    }
    /* Champion name (text region; the actual champion name string
     * is read from M11_GameViewState by the draw pipeline). */
    {
        int nameSlot = state->world.party.activeChampionIndex;
        if (nameSlot >= 0 && nameSlot < 4) {
            e = m11_ax_begin(FS_AX_TEXT,
                             M11_AX_VIEWPORT_X + panelX + 42,
                             M11_AX_VIEWPORT_Y + panelY + 4,
                             96, 10, 1);
            if (e) {
                snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "INVENTORY_CHAMPION_NAME");
                snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Champion Name");
                snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                         "slot=%d", nameSlot);
            }
        }
    }

    /* Open chest slots - ReDMCSB G0456 CHEST.C F0333 C537..C544
     * (8 chest slots at the bottom of the panel when an open chest
     * is active). Only emit when v1OpenChestThing != THING_NONE. */
    if (state->v1OpenChestThing != THING_NONE) {
        int chestOrdinal;
        int chestPanelX = 0, chestPanelY = 0;
        int chestPanelW = 0, chestPanelH = 0;
        if (M11_GameView_GetV1InventoryPanelZone(&chestPanelX, &chestPanelY,
                                                 &chestPanelW, &chestPanelH)) {
            e = m11_ax_begin(FS_AX_REGION,
                             M11_AX_VIEWPORT_X + chestPanelX,
                             M11_AX_VIEWPORT_Y + chestPanelY,
                             chestPanelW, chestPanelH, 1);
            if (e) {
                snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CHEST_PANEL");
                snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Open Chest");
            }
        }
        for (chestOrdinal = 0; chestOrdinal < 8; ++chestOrdinal) {
            int zx = 0, zy = 0, zw = 0, zh = 0;
            if (!M11_GameView_GetV1ChestSlotBoxZone(chestOrdinal,
                                                    &zx, &zy, &zw, &zh)) {
                continue;
            }
            e = m11_ax_begin(FS_AX_SLOT,
                             M11_AX_VIEWPORT_X + zx,
                             M11_AX_VIEWPORT_Y + zy,
                             zw, zh, 1);
            if (e) {
                snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CHEST_SLOT_%d", chestOrdinal);
                snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Chest Slot %d",
                         chestOrdinal + 1);
            }
        }
        /* Arrow / eye button at the top of the chest panel. */
        {
            int ax = 0, ay = 0, aw = 0, ah = 0;
            if (M11_GameView_GetV1ArrowOrEyeZone(&ax, &ay, &aw, &ah)) {
                e = m11_ax_begin(FS_AX_BUTTON,
                                 M11_AX_VIEWPORT_X + ax,
                                 M11_AX_VIEWPORT_Y + ay,
                                 aw, ah, 1);
                if (e) {
                    snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CHEST_ARROW_OR_EYE");
                    snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "%s",
                             state->v1OpenChestOpenedByEye
                                 ? "Eye (Reincarnate)"
                                 : "Arrow (Take All)");
                }
            }
        }
    }

    /* The acting-champion ordinal is exposed so a screen reader
     * can announce "Champion 2 acting" when the acting area
     * changes mode (F0387 menu-mode branch). */
    if (state->actingChampionOrdinal > 0 && state->actingChampionOrdinal <= 4) {
        e = m11_ax_begin(FS_AX_REGION, 0, 0, 0, 0, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "ACTING_CHAMPION");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Acting Champion");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "ordinal=%u", state->actingChampionOrdinal);
        }
    }
}

/* -- Map overlay zones --------------------------------------------- */

static void m11_ax_emit_map_overlay_zones(const M11_GameViewState* state,
                                          int fbW, int fbH) {
    FS_AX_Element* e;
    (void)state;
    /* Full-screen automap replaces the entire framebuffer at the
     * DM1 source-faithful 320x200. Emit one region covering it. */
    e = m11_ax_begin(FS_AX_REGION, 0, 0,
                     fbW > 0 ? fbW : 320,
                     fbH > 0 ? fbH : 200, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "AUTOMAP");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Full-Screen Map");
    }
}

/* -- Dialog overlay zones ------------------------------------------ */

static void m11_ax_emit_dialog_overlay_zones(const M11_GameViewState* state) {
    int choiceCount;
    int i;
    FS_AX_Element* e;

    if (!state) return;

    /* Dialog body region - centered in the viewport at framebuffer
     * coords (M11_AX_VIEWPORT_X, M11_AX_VIEWPORT_Y) with a 200x60 footprint.
     * Source: PANEL.C / DIALOG.C dialog plate rect. */
    e = m11_ax_begin(FS_AX_REGION,
                     M11_AX_VIEWPORT_X + 12,
                     M11_AX_VIEWPORT_Y + 38,
                     200, 60, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "DIALOG_BODY");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Dialog");
        if (state->dialogOverlayText[0] != '\0') {
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN, "%s",
                     state->dialogOverlayText);
        }
    }

    /* Dialog choice buttons - one zone per choice (max 4, padded by
     * the source struct). */
    choiceCount = state->dialogChoiceCount;
    if (choiceCount < 0) choiceCount = 0;
    if (choiceCount > 4) choiceCount = 4;
    for (i = 0; i < choiceCount; ++i) {
        int choiceY = M11_AX_VIEWPORT_Y + 104 + i * 14;
        e = m11_ax_begin(FS_AX_DIALOG_CHOICE,
                         M11_AX_VIEWPORT_X + 80,
                         choiceY,
                         144, 12,
                         state->dialogSelectedChoice == i ? 1 : 0);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "DIALOG_CHOICE_%d", i);
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Choice %d", i + 1);
            if (state->dialogChoices[i][0] != '\0') {
                snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN, "%s",
                         state->dialogChoices[i]);
            }
        }
    }
}

/* -- Candidate mirror (entrance) panel zones ----------------------- */

static void m11_ax_emit_entrance_mirror_zones(const M11_GameViewState* state) {
    int zx = 0, zy = 0, zw = 0, zh = 0;
    FS_AX_Element* e;

    if (!state) return;

    /* Panel region (ReDMCSB layout-696 C101_ZONE_PANEL - same panel
     * the inventory uses for the resurrect/reincarnate overlay). */
    if (!M11_GameView_GetV1InventoryPanelZone(&zx, &zy, &zw, &zh)) {
        return;
    }
    e = m11_ax_begin(FS_AX_REGION,
                     M11_AX_VIEWPORT_X + zx,
                     M11_AX_VIEWPORT_Y + zy,
                     zw, zh, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CANDIDATE_MIRROR_PANEL");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Candidate Mirror");
        if (state->candidateMirrorOrdinal > 0) {
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "ordinal=%d;party_index=%d",
                     state->candidateMirrorOrdinal,
                     state->candidateMirrorPartyIndex);
        }
    }

    /* The two action buttons and the cancel button live at
     * m11_game_view.c:26658-26672.  RESURRECT / REINCARNATE are
     * drawn on the upper row, CANCEL on the lower row. */
    e = m11_ax_begin(FS_AX_BUTTON,
                     M11_AX_VIEWPORT_X + zx + 16,
                     M11_AX_VIEWPORT_Y + zy + 18,
                     80, 14, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CANDIDATE_RESURRECT");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Resurrect");
    }
    e = m11_ax_begin(FS_AX_BUTTON,
                     M11_AX_VIEWPORT_X + zx + 78,
                     M11_AX_VIEWPORT_Y + zy + 18,
                     80, 14, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CANDIDATE_REINCARNATE");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Reincarnate");
    }
    e = m11_ax_begin(FS_AX_BUTTON,
                     M11_AX_VIEWPORT_X + zx + 44,
                     M11_AX_VIEWPORT_Y + zy + 58,
                     96, 14, 1);
    if (e) {
        snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "CANDIDATE_CANCEL");
        snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Cancel");
    }
}

/* -- Endgame zones ------------------------------------------------- */

static void m11_ax_emit_endgame_zones(const M11_GameViewState* state) {
    int slot;
    FS_AX_Element* e;

    if (!state) return;

    /* Endgame "THE END" plaque - ReDMCSB ENDGAME.C:455-456
     * G0012_ai_Graphic562_Box_Endgame_TheEnd = (120,199,95,108).
     * Layout-696 zone C415_Box_Endgame_TheEnd.
     *
     * The endgame helpers return screen/framebuffer coordinates used
     * directly by m11_game_view.c's victory overlay draw path; unlike
     * inventory/map panel child zones, these must not be offset by the
     * dungeon viewport origin or the fourth mirror/portrait extends
     * below the 320x200 frame. */
    {
        int ex = 0, ey = 0, ew = 0, eh = 0;
        if (M11_GameView_GetV1EndgameTheEndZone(&ex, &ey, &ew, &eh)) {
            e = m11_ax_begin(FS_AX_TEXT, ex, ey, ew, eh, 1);
            if (e) {
                snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "ENDGAME_THE_END");
                snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "The End");
            }
        }
    }

    /* Endgame champion portraits - ReDMCSB C412..C415 mirrors +
     * C416..C419 portraits (4 slots). */
    for (slot = 0; slot < 4; ++slot) {
        int mx = 0, my = 0, mw = 0, mh = 0;
        if (!M11_GameView_GetV1EndgameChampionMirrorZone(slot,
                                                        &mx, &my, &mw, &mh)) {
            continue;
        }
        e = m11_ax_begin(FS_AX_CHAMPION_MIRROR, mx, my, mw, mh, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "ENDGAME_MIRROR_%d", slot);
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "Champion Mirror %d",
                     slot + 1);
        }
        {
            int px = 0, py = 0, pw = 0, ph = 0;
            if (!M11_GameView_GetV1EndgameChampionPortraitZone(slot,
                                                              &px, &py,
                                                              &pw, &ph)) {
                continue;
            }
            e = m11_ax_begin(FS_AX_PORTRAIT, px, py, pw, ph, 1);
            if (e) {
                snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "ENDGAME_PORTRAIT_%d", slot);
                snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN,
                         "Champion Portrait %d", slot + 1);
            }
        }
    }
}

/* -- Public entry point -------------------------------------------- */

int m11_screen_reader_update_ex(const M11_GameViewState* state,
                                int framebufferWidth,
                                int framebufferHeight) {
    M11_AX_State axState;
    const char* stateName;
    int i;
    int fbW;
    int fbH;

    if (!state) {
        return 0;
    }
    if (!fs_ax_is_enabled()) {
        return 0;
    }

    /* Default to the source-faithful 320x200 if the caller hands
     * us a non-positive size.  This keeps the manifest well-formed
     * in headless / probe scenarios where M11_GameView_Draw may
     * pass 0. */
    fbW = framebufferWidth > 0 ? framebufferWidth : 320;
    fbH = framebufferHeight > 0 ? framebufferHeight : 200;

    m11_ax_reset();
    axState = m11_screen_reader_state_for(state);
    stateName = m11_screen_reader_state_name(axState);

    fs_ax_begin_frame(fbW, fbH, stateName);

    /* Always-on launcher_root analogue: a single region covering
     * the whole framebuffer so a screen reader knows the manifest
     * came from the M11 gameplay view, not the launcher. */
    {
        FS_AX_Element* e = m11_ax_begin(FS_AX_REGION, 0, 0, fbW, fbH, 1);
        if (e) {
            snprintf(M11_AX_ID(e), M11_AX_ID_LEN, "gameplay_root");
            snprintf(M11_AX_LABEL(e), M11_AX_LABEL_LEN, "FIRESTAFF GAMEPLAY");
            snprintf(M11_AX_VALUE(e), M11_AX_VALUE_LEN,
                     "state=%s;active=%d;level=%d",
                     stateName, state->active ? 1 : 0,
                     state->world.partyMapIndex);
        }
    }

    /* Always-on zones (viewport + movement + spell + HUD +
     * control strip). */
    m11_ax_emit_always_on_zones(state);

    /* Overlay-specific zones. */
    switch (axState) {
        case M11_AX_STATE_INVENTORY:
            m11_ax_emit_inventory_panel_zones(state);
            break;
        case M11_AX_STATE_MAP:
            m11_ax_emit_map_overlay_zones(state, fbW, fbH);
            break;
        case M11_AX_STATE_DIALOG:
            m11_ax_emit_dialog_overlay_zones(state);
            break;
        case M11_AX_STATE_ENTRANCE_MIRROR:
            m11_ax_emit_entrance_mirror_zones(state);
            break;
        case M11_AX_STATE_ENDGAME:
            m11_ax_emit_endgame_zones(state);
            break;
        case M11_AX_STATE_GAMEPLAY:
        case M11_AX_STATE_OTHER:
        default:
            /* Plain gameplay emits the always-on zones only -
             * overlay-specific emitters are no-ops for this
             * state. */
            break;
    }

    m11_ax_emit_session_timer_overlay_zones(state, fbW, fbH);

    m11_ax_finalize();
    for (i = 0; i < g_m11_ax_record_count; ++i) {
        fs_ax_add_element(&g_m11_ax_records[i].element);
    }
    fs_ax_flush();
    m11_ax_reset();
    return 1;
}
