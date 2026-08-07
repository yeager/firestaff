/* DM1 V2 champion-select focus bridge.
 *
 * ReDMCSB CLIKCHAM.C F0367 dispatches champion-status clicks. COMMAND.C
 * routes the selected champion's name and hands, while CHAMDRAW.C/PANEL.C
 * own live records and all pixels. No PC34 route supplies a detached V2
 * champion array, font, panel geometry or selection animation. */
#include "dm1_v2_champion_select_pc34.h"

static int g_focus_index = -1;

void v2_champion_select_init(void) {
    g_focus_index = -1;
}

void v2_champion_select_render(void) {
    /* No detached V2 framebuffer owner exists. */
}

void v2_champion_select_render_fb(uint8_t* fb, int w, int h) {
    (void)fb;
    (void)w;
    (void)h;
    /* No source-owned champion record and panel surface cross this API. */
}

int v2_champion_select_focus_index_pc34(unsigned int championIndex) {
    if (championIndex >= 4u) return 0;
    g_focus_index = (int)championIndex;
    return 1;
}

int v2_champion_select_current_index_pc34(void) {
    return g_focus_index;
}

unsigned int v2_champion_select_source_lock_ok(void) {
    /* Focus is not source pixel or record provenance. */
    return 0u;
}

const char* v2_champion_select_get_source_evidence(void) {
    return "ReDMCSB CLIKCHAM.C F0367 click dispatch; COMMAND.C champion "
           "subroutes; CHAMDRAW.C/PANEL.C own records and pixels; "
           "V2 retains focus only and draws nothing.";
}
